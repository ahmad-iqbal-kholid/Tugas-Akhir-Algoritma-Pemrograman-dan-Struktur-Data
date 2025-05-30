#include <iostream>
#include <string>
#include <cstdio> // Untuk operasi file
#include <ctime>  // Untuk waktu registrasi
#include <cmath>  // Untuk fungsi sqrt()

using namespace std;

// --- KONSTANTA dan STRUCT --- //
const int MAX_POLI = 8;
const string poliList[MAX_POLI] = {
    "Gigi", "Mata", "Anak", "Umum", "Kandungan", "THT", "Kulit", "Saraf"
};

// Struct untuk pasien (doubly linked list)
struct Pasien {
    string nama;
    string nik;
    string alamat;
    int umur;
    float bb;  // berat badan
    float tb;  // tinggi badan
    string poli;
    time_t waktuRegistrasi;
    Pasien* next;
    Pasien* prev; // Pointer prev untuk doubly linked list
};

// Struct untuk riwayat (doubly linked list)
struct Riwayat {
    string nik;
    string nama;
    string poli;
    time_t waktuPelayanan;
    Riwayat* next;
    Riwayat* prev;
};

// Struct untuk undo (stack)
struct UndoNode {
    Pasien* pasien;
    UndoNode* next;
};

// Pointer global
Pasien* head = nullptr;
Pasien* tail = nullptr;
Riwayat* headRiwayat = nullptr;
Riwayat* tailRiwayat = nullptr;
UndoNode* headUndo = nullptr;

// --- FUNGSI BANTU --- //

// Fungsi untuk mengubah string ke lowercase
string toLower(string str) {
    for (char &c : str) {
        c = tolower(c);
    }
    return str;
}

// Validasi input tidak kosong
string inputString(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Hapus spasi di awal dan akhir
        size_t start = input.find_first_not_of(" \t");
        if (start == string::npos) {
            cout << "Input tidak boleh kosong!\n";
            continue;
        }
        size_t end = input.find_last_not_of(" \t");
        input = input.substr(start, end - start + 1);
        
        if (!input.empty()) {
            return input;
        }
        cout << "Input tidak boleh kosong!\n";
    }
}

// Validasi input integer positif (termasuk 0)
int inputInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= 0) {
            cin.ignore();
            return value;
        }
        cout << "Input harus bilangan bulat positif atau 0!\n";
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

// Validasi input float positif
float inputFloat(const string& prompt) {
    float value;
    while (true) {
        cout << prompt;
        if (cin >> value && value > 0) {
            cin.ignore();
            return value;
        }
        cout << "Input harus bilangan positif!\n";
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

// Konfirmasi ya/tidak
bool konfirmasi(const string& prompt) {
    string input;
    while (true) {
        cout << prompt << " (y/t): ";
        getline(cin, input);
        input = toLower(input);
        if (input == "y") return true;
        if (input == "t") return false;
        cout << "Masukkan 'y' untuk ya atau 't' untuk tidak!\n";
    }
}

// Cek apakah poli valid
bool isPoliValid(const string& poli) {
    string poliLower = toLower(poli);
    for (int i = 0; i < MAX_POLI; i++) {
        if (toLower(poliList[i]) == poliLower) {
            return true;
        }
    }
    return false;
}

// Format waktu ke string
string waktuToString(time_t waktu) {
    char buffer[80];
    struct tm* timeinfo = localtime(&waktu);
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", timeinfo);
    return string(buffer);
}

// --- FUNGSI OPERASI FILE (C-style) --- //

void simpanDataKeFile() {
    FILE* file = fopen("data_pasien.dat", "wb");
    if (!file) {
        cout << "Gagal membuka file untuk penyimpanan.\n";
        return;
    }

    // Simpan jumlah pasien
    int jumlah = 0;
    Pasien* current = head;
    while (current) {
        jumlah++;
        current = current->next;
    }
    fwrite(&jumlah, sizeof(int), 1, file);

    // Simpan data pasien
    current = head;
    while (current) {
        // Simpan setiap field satu per satu
        int namaLen = current->nama.length();
        fwrite(&namaLen, sizeof(int), 1, file);
        fwrite(current->nama.c_str(), sizeof(char), namaLen, file);

        int nikLen = current->nik.length();
        fwrite(&nikLen, sizeof(int), 1, file);
        fwrite(current->nik.c_str(), sizeof(char), nikLen, file);

        int alamatLen = current->alamat.length();
        fwrite(&alamatLen, sizeof(int), 1, file);
        fwrite(current->alamat.c_str(), sizeof(char), alamatLen, file);

        fwrite(&current->umur, sizeof(int), 1, file);
        fwrite(&current->bb, sizeof(float), 1, file);
        fwrite(&current->tb, sizeof(float), 1, file);

        int poliLen = current->poli.length();
        fwrite(&poliLen, sizeof(int), 1, file);
        fwrite(current->poli.c_str(), sizeof(char), poliLen, file);

        fwrite(&current->waktuRegistrasi, sizeof(time_t), 1, file);

        current = current->next;
    }

    fclose(file);
    cout << "Data pasien berhasil disimpan ke file.\n";
}

void bacaDataDariFile() {
    FILE* file = fopen("data_pasien.dat", "rb");
    if (!file) {
        cout << "File data tidak ditemukan atau gagal dibuka.\n";
        return;
    }

    // Kosongkan list terlebih dahulu
    while (head) {
        Pasien* temp = head;
        head = head->next;
        delete temp;
    }
    head = tail = nullptr;

    // Baca jumlah pasien
    int jumlah;
    fread(&jumlah, sizeof(int), 1, file);

    for (int i = 0; i < jumlah; i++) {
        Pasien* pasien = new Pasien();

        // Baca nama
        int namaLen;
        fread(&namaLen, sizeof(int), 1, file);
        char* namaBuffer = new char[namaLen + 1];
        fread(namaBuffer, sizeof(char), namaLen, file);
        namaBuffer[namaLen] = '\0';
        pasien->nama = string(namaBuffer);
        delete[] namaBuffer;

        // Baca NIK
        int nikLen;
        fread(&nikLen, sizeof(int), 1, file);
        char* nikBuffer = new char[nikLen + 1];
        fread(nikBuffer, sizeof(char), nikLen, file);
        nikBuffer[nikLen] = '\0';
        pasien->nik = string(nikBuffer);
        delete[] nikBuffer;

        // Baca alamat
        int alamatLen;
        fread(&alamatLen, sizeof(int), 1, file);
        char* alamatBuffer = new char[alamatLen + 1];
        fread(alamatBuffer, sizeof(char), alamatLen, file);
        alamatBuffer[alamatLen] = '\0';
        pasien->alamat = string(alamatBuffer);
        delete[] alamatBuffer;

        fread(&pasien->umur, sizeof(int), 1, file);
        fread(&pasien->bb, sizeof(float), 1, file);
        fread(&pasien->tb, sizeof(float), 1, file);

        // Baca poli
        int poliLen;
        fread(&poliLen, sizeof(int), 1, file);
        char* poliBuffer = new char[poliLen + 1];
        fread(poliBuffer, sizeof(char), poliLen, file);
        poliBuffer[poliLen] = '\0';
        pasien->poli = string(poliBuffer);
        delete[] poliBuffer;

        fread(&pasien->waktuRegistrasi, sizeof(time_t), 1, file);

        // Tambahkan ke linked list
        pasien->next = nullptr;
        pasien->prev = tail;

        if (!head) {
            head = tail = pasien;
        } else {
            tail->next = pasien;
            tail = pasien;
        }
    }

    fclose(file);
    cout << "Data pasien berhasil dimuat dari file (" << jumlah << " pasien).\n";
}

// --- FUNGSI SORTING --- //

// Bubble sort untuk mengurutkan berdasarkan waktu registrasi (ascending)
void sortByRegistrationTime() {
    if (!head || !head->next) return;

    bool swapped;
    do {
        swapped = false;
        Pasien* current = head;
        Pasien* prev = nullptr;
        
        while (current->next) {
            if (current->waktuRegistrasi > current->next->waktuRegistrasi) {
                // Tukar node
                Pasien* nextNode = current->next;
                
                if (prev) {
                    prev->next = nextNode;
                } else {
                    head = nextNode;
                }
                
                current->next = nextNode->next;
                nextNode->next = current;
                
                // Update prev pointers
                nextNode->prev = prev;
                current->prev = nextNode;
                if (current->next) {
                    current->next->prev = current;
                }
                
                swapped = true;
                prev = nextNode;
            } else {
                prev = current;
                current = current->next;
            }
        }
        
        // Update tail jika diperlukan
        tail = current;
    } while (swapped);
}

// Insertion sort untuk mengurutkan berdasarkan nama (ascending)
void sortByName() {
    if (!head || !head->next) return;

    Pasien* sorted = nullptr;
    Pasien* current = head;

    while (current) {
        Pasien* next = current->next;
        
        if (!sorted || toLower(sorted->nama) >= toLower(current->nama)) {
            current->next = sorted;
            current->prev = nullptr;
            if (sorted) sorted->prev = current;
            sorted = current;
        } else {
            Pasien* temp = sorted;
            while (temp->next && toLower(temp->next->nama) < toLower(current->nama)) {
                temp = temp->next;
            }
            current->next = temp->next;
            current->prev = temp;
            if (temp->next) temp->next->prev = current;
            temp->next = current;
        }
        
        current = next;
    }

    // Update head dan tail
    head = sorted;
    tail = sorted;
    while (tail && tail->next) {
        tail = tail->next;
    }
}

// --- FUNGSI SEARCHING --- //

// Binary search untuk mencari berdasarkan NIK (harus diurutkan terlebih dahulu)
Pasien* binarySearchByNIK(const string& nik) {
    // Konversi linked list ke array untuk binary search
    int count = 0;
    Pasien* current = head;
    while (current) {
        count++;
        current = current->next;
    }

    if (count == 0) return nullptr;

    Pasien** arr = new Pasien*[count];
    current = head;
    for (int i = 0; i < count; i++) {
        arr[i] = current;
        current = current->next;
    }

    int left = 0, right = count - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid]->nik == nik) {
            Pasien* result = arr[mid];
            delete[] arr;
            return result;
        }
        
        if (arr[mid]->nik < nik) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    delete[] arr;
    return nullptr;
}

// Linear search dengan jump untuk mencari berdasarkan nama
void jumpSearchByName(const string& nama) {
    if (!head) {
        cout << "Antrian kosong.\n";
        return;
    }

    int n = 0;
    Pasien* current = head;
    while (current) {
        n++;
        current = current->next;
    }

    int step = sqrt(n);
    int prev = 0;
    bool found = false;
    string namaLower = toLower(nama);

    Pasien* prevNode = nullptr;
    current = head;

    while (current) {
        int count = 0;
        while (count < step && current) {
            prevNode = current;
            current = current->next;
            count++;
        }

        if (!current || toLower(prevNode->nama) >= namaLower) {
            break;
        }
    }

    // Linear search backward
    while (prevNode) {
        if (toLower(prevNode->nama).find(namaLower) != string::npos) {
            cout << "Ditemukan: " << prevNode->nama << " (NIK: " << prevNode->nik << ")\n";
            found = true;
        }
        prevNode = prevNode->prev;
    }

    if (!found) {
        cout << "Pasien dengan nama '" << nama << "' tidak ditemukan.\n";
    }
}

// --- FUNGSI UTAMA --- //

// Tambah pasien ke antrian (queue)
void tambahAntrian(Pasien* pasien) {
    pasien->waktuRegistrasi = time(nullptr);
    pasien->next = nullptr;
    pasien->prev = tail;

    if (!head) {
        head = tail = pasien;
    } else {
        tail->next = pasien;
        tail = pasien;
    }
}

// Hapus pasien dari antrian berdasarkan NIK
Pasien* hapusAntrian(const string& nik) {
    Pasien* current = head;

    while (current) {
        if (current->nik == nik) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                head = current->next;
            }

            if (current->next) {
                current->next->prev = current->prev;
            } else {
                tail = current->prev;
            }

            current->next = nullptr;
            current->prev = nullptr;
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

// Layani pasien (pindahkan dari antrian ke riwayat)
void layaniPasien() {
    if (!konfirmasi("\nApakah Anda yakin ingin melayani pasien berikutnya?")) {
        cout << "Pelayanan pasien dibatalkan.\n";
        return;
    }

    if (!head) {
        cout << "\nAntrian kosong, tidak ada pasien yang bisa dilayani.\n";
        return;
    }

    Pasien* pasien = head;
    head = head->next;
    if (head) head->prev = nullptr;
    if (!head) tail = nullptr;

    // Tambahkan ke riwayat (stack)
    Riwayat* riwayatBaru = new Riwayat;
    riwayatBaru->nik = pasien->nik;
    riwayatBaru->nama = pasien->nama;
    riwayatBaru->poli = pasien->poli;
    riwayatBaru->waktuPelayanan = time(nullptr);
    riwayatBaru->next = headRiwayat;
    riwayatBaru->prev = nullptr;
    
    if (headRiwayat) headRiwayat->prev = riwayatBaru;
    headRiwayat = riwayatBaru;
    if (!tailRiwayat) tailRiwayat = riwayatBaru;

    cout << "\nPasien dengan NIK " << pasien->nik << " (" << pasien->nama 
         << ") telah dilayani di poli " << pasien->poli << ".\n";

    delete pasien;
    simpanDataKeFile();
}

// Batalkan antrian pasien
void batalAntrian() {
    if (!head) {
        cout << "\nAntrian kosong, tidak ada pasien untuk dibatalkan.\n";
        return;
    }

    string nik = inputString("\nMasukkan NIK pasien yang akan dibatalkan: ");
    Pasien* pasien = hapusAntrian(nik);

    if (!pasien) {
        cout << "Pasien dengan NIK " << nik << " tidak ditemukan.\n";
        return;
    }

    if (!konfirmasi("Apakah Anda yakin ingin membatalkan antrian pasien ini?")) {
        tambahAntrian(pasien);
        cout << "Pembatalan antrian dibatalkan.\n";
        return;
    }

    // Simpan di stack undo
    UndoNode* undoNode = new UndoNode;
    undoNode->pasien = pasien;
    undoNode->next = headUndo;
    headUndo = undoNode;

    cout << "Antrian pasien dengan NIK " << nik << " berhasil dibatalkan.\n";
    simpanDataKeFile();
}

// Undo pembatalan terakhir
void undoBatal() {
    if (!headUndo) {
        cout << "\nTidak ada aksi batal yang bisa di-undo.\n";
        return;
    }

    if (!konfirmasi("\nApakah Anda yakin ingin mengembalikan pembatalan terakhir?")) {
        cout << "Undo dibatalkan.\n";
        return;
    }

    UndoNode* undoNode = headUndo;
    headUndo = headUndo->next;

    tambahAntrian(undoNode->pasien);
    cout << "\nUndo berhasil. Pasien dengan NIK " << undoNode->pasien->nik 
         << " dikembalikan ke antrian.\n";

    delete undoNode;
    simpanDataKeFile();
}

// Tampilkan semua antrian
void tampilkanAntrian() {
    if (!head) {
        cout << "\nAntrian kosong.\n";
        return;
    }

    cout << "\n=== DAFTAR ANTRIAN PASIEN ===\n";
    cout << "No  Nama                NIK                 Poli           Waktu Registrasi\n";
    cout << "--------------------------------------------------------------------------\n";

    Pasien* current = head;
    int nomor = 1;
    while (current) {
        string namaTampil = current->nama;
        if (namaTampil.length() > 18) {
            namaTampil = namaTampil.substr(0, 15) + "...";
        }
        
        cout << nomor++ << "   ";
        cout << namaTampil;
        for (int i = namaTampil.length(); i < 19; i++) cout << " ";
        
        cout << current->nik << "  ";
        cout << current->poli;
        for (int i = current->poli.length(); i < 14; i++) cout << " ";
        
        cout << waktuToString(current->waktuRegistrasi) << endl;
        current = current->next;
    }
}

// Tampilkan riwayat pelayanan (dalam urutan terbalik)
void tampilkanRiwayat() {
    if (!headRiwayat) {
        cout << "\nBelum ada riwayat pelayanan.\n";
        return;
    }

    cout << "\n=== RIWAYAT PELAYANAN (TERBARU -> TERLAMA) ===\n";
    cout << "No  Nama                NIK                 Poli           Waktu Pelayanan\n";
    cout << "--------------------------------------------------------------------------\n";

    Riwayat* current = headRiwayat;
    int nomor = 1;
    while (current) {
        string namaTampil = current->nama;
        if (namaTampil.length() > 18) {
            namaTampil = namaTampil.substr(0, 15) + "...";
        }
        
        cout << nomor++ << "   ";
        cout << namaTampil;
        for (int i = namaTampil.length(); i < 19; i++) cout << " ";
        
        cout << current->nik << "  ";
        cout << current->poli;
        for (int i = current->poli.length(); i < 14; i++) cout << " ";
        
        cout << waktuToString(current->waktuPelayanan) << endl;
        current = current->next;
    }
}

// Menu registrasi pasien
void menuRegistrasi() {
    cout << "\n=== REGISTRASI PASIEN ===\n";
    
    Pasien* pasien = new Pasien();
    pasien->nama = inputString("Nama Lengkap: ");
    pasien->nik = inputString("NIK: ");
    pasien->alamat = inputString("Alamat: ");
    pasien->umur = inputInt("Umur (tahun): ");
    pasien->bb = inputFloat("Berat Badan (kg): ");
    pasien->tb = inputFloat("Tinggi Badan (cm): ");

    // Input poli dengan validasi
    cout << "\nDaftar Poli: ";
    for (int i = 0; i < MAX_POLI; i++) {
        cout << poliList[i];
        if (i < MAX_POLI - 1) cout << ", ";
    }
    cout << endl;

    while (true) {
        string poli = inputString("Poli yang Dituju: ");
        for (int i = 0; i < MAX_POLI; i++) {
            if (toLower(poliList[i]) == toLower(poli)) {
                pasien->poli = poliList[i];
                break;
            }
        }
        
        if (!pasien->poli.empty()) break;
        cout << "Poli tidak valid! Silakan coba lagi.\n";
    }

    // Konfirmasi
    cout << "\nKonfirmasi Data Pasien:\n";
    cout << "Nama       : " << pasien->nama << "\n";
    cout << "NIK        : " << pasien->nik << "\n";
    cout << "Alamat     : " << pasien->alamat << "\n";
    cout << "Umur       : " << pasien->umur << " tahun\n";
    cout << "Berat Badan: " << pasien->bb << " kg\n";
    cout << "Tinggi Badan: " << pasien->tb << " cm\n";
    cout << "Poli       : " << pasien->poli << "\n";

    if (konfirmasi("Tambahkan pasien ke antrian?")) {
        tambahAntrian(pasien);
        cout << "Pasien berhasil didaftarkan ke antrian.\n";
        simpanDataKeFile();
    } else {
        delete pasien;
        cout << "Pendaftaran dibatalkan.\n";
    }
}

// Menu sorting
void menuSorting() {
    if (!head || !head->next) {
        cout << "\nTidak cukup data untuk diurutkan.\n";
        return;
    }

    cout << "\n=== MENU PENGURUTAN ===\n";
    cout << "1. Urutkan berdasarkan waktu registrasi (terlama -> terbaru)\n";
    cout << "2. Urutkan berdasarkan nama (A -> Z)\n";
    cout << "3. Kembali\n";

    int pilihan = inputInt("Pilih metode pengurutan: ");

    switch (pilihan) {
        case 1:
            sortByRegistrationTime();
            cout << "Data berhasil diurutkan berdasarkan waktu registrasi.\n";
            tampilkanAntrian();
            break;
        case 2:
            sortByName();
            cout << "Data berhasil diurutkan berdasarkan nama.\n";
            tampilkanAntrian();
            break;
        case 3:
            break;
        default:
            cout << "Pilihan tidak valid!\n";
    }
}

// Menu searching
void menuSearching() {
    cout << "\n=== MENU PENCARIAN ===\n";
    cout << "1. Cari berdasarkan NIK (binary search)\n";
    cout << "2. Cari berdasarkan nama (jump search)\n";
    cout << "3. Kembali\n";

    int pilihan = inputInt("Pilih metode pencarian: ");

    switch (pilihan) {
        case 1: {
            string nik = inputString("Masukkan NIK yang dicari: ");
            sortByName(); // Untuk contoh, kita gunakan sort by name
            Pasien* hasil = binarySearchByNIK(nik);
            if (hasil) {
                cout << "\nData Pasien Ditemukan:\n";
                cout << "Nama       : " << hasil->nama << "\n";
                cout << "NIK        : " << hasil->nik << "\n";
                cout << "Alamat     : " << hasil->alamat << "\n";
                cout << "Umur       : " << hasil->umur << " tahun\n";
                cout << "Berat Badan: " << hasil->bb << " kg\n";
                cout << "Tinggi Badan: " << hasil->tb << " cm\n";
                cout << "Poli       : " << hasil->poli << "\n";
                cout << "Waktu Registrasi: " << waktuToString(hasil->waktuRegistrasi) << "\n";
            } else {
                cout << "Pasien dengan NIK " << nik << " tidak ditemukan.\n";
            }
            break;
        }
        case 2: {
            string nama = inputString("Masukkan nama yang dicari: ");
            jumpSearchByName(nama);
            break;
        }
        case 3:
            break;
        default:
            cout << "Pilihan tidak valid!\n";
    }
}

// Menu utama
void tampilkanMenu() {
    cout << "\n=== SISTEM ANTRIAN RUMAH SAKIT ===\n";
    cout << "1. Registrasi Pasien Baru\n";
    cout << "2. Layani Pasien Berikutnya\n";
    cout << "3. Batalkan Antrian Pasien\n";
    cout << "4. Undo Pembatalan Terakhir\n";
    cout << "5. Tampilkan Semua Antrian\n";
    cout << "6. Tampilkan Riwayat Pelayanan\n";
    cout << "7. Urutkan Antrian\n";
    cout << "8. Cari Pasien\n";
    cout << "9. Simpan Data ke File\n";
    cout << "10. Muat Data dari File\n";
    cout << "0. Keluar\n";
}

// Fungsi utama
int main() {
    // Muat data dari file saat program dimulai
    bacaDataDariFile();

    while (true) {
        tampilkanMenu();
        int pilihan = inputInt("Pilih menu: ");

        switch (pilihan) {
            case 1: menuRegistrasi(); break;
            case 2: layaniPasien(); break;
            case 3: batalAntrian(); break;
            case 4: undoBatal(); break;
            case 5: tampilkanAntrian(); break;
            case 6: tampilkanRiwayat(); break;
            case 7: menuSorting(); break;
            case 8: menuSearching(); break;
            case 9: simpanDataKeFile(); break;
            case 10: bacaDataDariFile(); break;
            case 0: 
                if (konfirmasi("\nApakah Anda yakin ingin keluar?")) {
                    cout << "\nTerima kasih telah menggunakan sistem ini.\n";
                    
                    // Bersihkan memori
                    while (head) {
                        Pasien* temp = head;
                        head = head->next;
                        delete temp;
                    }
                    
                    while (headRiwayat) {
                        Riwayat* temp = headRiwayat;
                        headRiwayat = headRiwayat->next;
                        delete temp;
                    }
                    
                    while (headUndo) {
                        UndoNode* temp = headUndo;
                        headUndo = headUndo->next;
                        delete temp->pasien;
                        delete temp;
                    }
                    
                    return 0;
                }
                break;
            default:
                cout << "Pilihan tidak valid! Silakan coba lagi.\n";
        }
    }
}