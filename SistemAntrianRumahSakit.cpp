#include <iostream>
#include <string>
#include <cstdio> // Untuk operasi file (fopen, fwrite, fread, fclose)
#include <ctime> // Untuk waktu registrasi (time_t, localtime, strftime)
#include <cmath> // Untuk fungsi sqrt() di pencarian
#include <iomanip> // Pastikan ini ada untuk setw

using namespace std;

// --- KONSTANTA dan STRUKTUR DATA --- //
const int MAKS_POLI = 8;
const string daftarPoli[MAKS_POLI] = {"Gigi", "Mata", "Anak", "Umum", "Kandungan", "THT", "Kulit", "Saraf"};
const char daftarKodePoli[MAKS_POLI] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};

// Struktur data untuk pasien (daftar berantai ganda)
struct Pasien {
    string nama;
    string nik;
    string alamat;
    int umur;
    float beratBadan;
    float tinggiBadan;
    string poli; // Menyimpan nama poli lengkap
    time_t waktuPendaftaran;
    Pasien* berikutnya;
    Pasien* sebelumnya;
};

// Struktur data untuk riwayat (daftar berantai ganda)
struct Riwayat {
    string nik;
    string nama;
    string poli;
    time_t waktuPelayanan;
    Riwayat* berikutnya;
    Riwayat* sebelumnya;
};

// Struktur data untuk undo (tumpukan)
struct NodeUndo {
    Pasien* pasien;
    NodeUndo* berikutnya;
};

// Pointer global untuk mengelola daftar berantai dan tumpukan
Pasien* kepalaAntrean = nullptr;
Pasien* ekorAntrean = nullptr;
Riwayat* kepalaRiwayat = nullptr;
Riwayat* ekorRiwayat = nullptr;
NodeUndo* kepalaUndo = nullptr;

// --- FUNGSI BANTU --- //

string keHurufKecil(string teks) {
    for (char &karakter : teks) karakter = tolower(karakter);
    return teks;
}

string inputTeks(const string& perintah) {
    string input;
    while (true) {
        cout << perintah;
        getline(cin, input);
        size_t awal = input.find_first_not_of(" \t");
        if (awal == string::npos) { cout << "Input tidak boleh kosong!\n"; continue; }
        size_t akhir = input.find_last_not_of(" \t");
        input = input.substr(awal, akhir - awal + 1);
        if (!input.empty()) return input;
        cout << "Input tidak boleh kosong!\n";
    }
}

int inputAngkaBulat(const string& perintah) {
    int nilai;
    while (true) {
        cout << perintah;
        if (cin >> nilai && nilai >= 0) { cin.ignore(); return nilai; }
        cout << "Input harus bilangan bulat positif atau 0!\n";
        cin.clear(); cin.ignore(1000, '\n');
    }
}

float inputAngkaDesimal(const string& perintah) {
    float nilai;
    while (true) {
        cout << perintah;
        if (cin >> nilai && nilai > 0) { cin.ignore(); return nilai; }
        cout << "Input harus bilangan positif!\n";
        cin.clear(); cin.ignore(1000, '\n');
    }
}

bool mintaKonfirmasi(const string& perintah) {
    string input;
    while (true) {
        cout << perintah << " (y/t): ";
        getline(cin, input);
        input = keHurufKecil(input);
        if (input == "y") return true;
        if (input == "t") return false;
        cout << "Masukkan 'y' untuk ya atau 't' untuk tidak!\n";
    }
}

string formatWaktu(time_t waktu) {
    char buffer[80];
    struct tm* infoWaktu = localtime(&waktu);
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", infoWaktu);
    return string(buffer);
}

char dapatkanKodePoli(const string& namaPoli) {
    for (int i = 0; i < MAKS_POLI; ++i) {
        if (daftarPoli[i] == namaPoli) return daftarKodePoli[i];
    }
    return '?';
}

bool validasiInputPoli(const string& inputPoli, string& namaPoliLengkap) {
    string inputLower = keHurufKecil(inputPoli);
    for (int i = 0; i < MAKS_POLI; i++) {
        if (keHurufKecil(daftarPoli[i]) == inputLower) { namaPoliLengkap = daftarPoli[i]; return true; }
        if (inputPoli.length() == 1 && keHurufKecil(string(1, daftarKodePoli[i])) == inputLower) { namaPoliLengkap = daftarPoli[i]; return true; }
    }
    namaPoliLengkap = "";
    return false;
}

// --- FUNGSI OPERASI FILE (Gaya C) --- //

void simpanData() {
    FILE* berkas = fopen("data_pasien.dat", "wb");
    if (!berkas) { cout << "Gagal membuka file untuk penyimpanan.\n"; return; }

    int jumlah = 0; Pasien* saatIni = kepalaAntrean;
    while (saatIni) { jumlah++; saatIni = saatIni->berikutnya; }
    fwrite(&jumlah, sizeof(int), 1, berkas);

    saatIni = kepalaAntrean;
    while (saatIni) {
        int panjangNama = saatIni->nama.length(); fwrite(&panjangNama, sizeof(int), 1, berkas); fwrite(saatIni->nama.c_str(), sizeof(char), panjangNama, berkas);
        int panjangNik = saatIni->nik.length(); fwrite(&panjangNik, sizeof(int), 1, berkas); fwrite(saatIni->nik.c_str(), sizeof(char), panjangNik, berkas);
        int panjangAlamat = saatIni->alamat.length(); fwrite(&panjangAlamat, sizeof(int), 1, berkas); fwrite(saatIni->alamat.c_str(), sizeof(char), panjangAlamat, berkas);
        fwrite(&saatIni->umur, sizeof(int), 1, berkas);
        fwrite(&saatIni->beratBadan, sizeof(float), 1, berkas);
        fwrite(&saatIni->tinggiBadan, sizeof(float), 1, berkas);
        int panjangPoli = saatIni->poli.length(); fwrite(&panjangPoli, sizeof(int), 1, berkas); fwrite(saatIni->poli.c_str(), sizeof(char), panjangPoli, berkas);
        fwrite(&saatIni->waktuPendaftaran, sizeof(time_t), 1, berkas);
        saatIni = saatIni->berikutnya;
    }
    fclose(berkas);
    cout << "Data pasien berhasil disimpan ke file.\n";
}

void muatData() {
    FILE* berkas = fopen("data_pasien.dat", "rb");
    if (!berkas) { cout << "File data tidak ditemukan atau gagal dibuka.\n"; return; }

    while (kepalaAntrean) { Pasien* temp = kepalaAntrean; kepalaAntrean = kepalaAntrean->berikutnya; delete temp; }
    kepalaAntrean = ekorAntrean = nullptr;

    int jumlah; fread(&jumlah, sizeof(int), 1, berkas);
    for (int i = 0; i < jumlah; i++) {
        Pasien* pasienBaru = new Pasien();
        int panjang; char* buffer;

        fread(&panjang, sizeof(int), 1, berkas); buffer = new char[panjang + 1]; fread(buffer, sizeof(char), panjang, berkas); buffer[panjang] = '\0'; pasienBaru->nama = string(buffer); delete[] buffer;
        fread(&panjang, sizeof(int), 1, berkas); buffer = new char[panjang + 1]; fread(buffer, sizeof(char), panjang, berkas); buffer[panjang] = '\0'; pasienBaru->nik = string(buffer); delete[] buffer;
        fread(&panjang, sizeof(int), 1, berkas); buffer = new char[panjang + 1]; fread(buffer, sizeof(char), panjang, berkas); buffer[panjang] = '\0'; pasienBaru->alamat = string(buffer); delete[] buffer;
        fread(&pasienBaru->umur, sizeof(int), 1, berkas);
        fread(&pasienBaru->beratBadan, sizeof(float), 1, berkas);
        fread(&pasienBaru->tinggiBadan, sizeof(float), 1, berkas);
        fread(&panjang, sizeof(int), 1, berkas); buffer = new char[panjang + 1]; fread(buffer, sizeof(char), panjang, berkas); buffer[panjang] = '\0'; pasienBaru->poli = string(buffer); delete[] buffer;
        fread(&pasienBaru->waktuPendaftaran, sizeof(time_t), 1, berkas);

        pasienBaru->berikutnya = nullptr; pasienBaru->sebelumnya = ekorAntrean;
        if (!kepalaAntrean) kepalaAntrean = ekorAntrean = pasienBaru;
        else { ekorAntrean->berikutnya = pasienBaru; ekorAntrean = pasienBaru; }
    }
    fclose(berkas);
    cout << "Data pasien berhasil dimuat dari file (" << jumlah << " pasien).\n";
}

// --- FUNGSI PENGURUTAN --- //

void urutkanWaktuRegistrasi() {
    if (!kepalaAntrean || !kepalaAntrean->berikutnya) return;
    bool tertukar;
    do {
        tertukar = false; Pasien* saatIni = kepalaAntrean; Pasien* sebelumnya = nullptr;
        while (saatIni->berikutnya) {
            if (saatIni->waktuPendaftaran > saatIni->berikutnya->waktuPendaftaran) {
                Pasien* nodeBerikutnya = saatIni->berikutnya;
                if (sebelumnya) { sebelumnya->berikutnya = nodeBerikutnya; } else { kepalaAntrean = nodeBerikutnya; }
                saatIni->berikutnya = nodeBerikutnya->berikutnya; nodeBerikutnya->berikutnya = saatIni;
                nodeBerikutnya->sebelumnya = sebelumnya; saatIni->sebelumnya = nodeBerikutnya;
                if (saatIni->berikutnya) { saatIni->berikutnya->sebelumnya = saatIni; }
                tertukar = true;
                sebelumnya = nodeBerikutnya;
            } else { sebelumnya = saatIni; saatIni = saatIni->berikutnya; }
        }
        ekorAntrean = saatIni;
    } while (tertukar);
}

void urutkanNama() {
    if (!kepalaAntrean || !kepalaAntrean->berikutnya) return;
    Pasien* terurut = nullptr; Pasien* saatIni = kepalaAntrean;
    while (saatIni) {
        Pasien* berikutnya = saatIni->berikutnya;
        if (!terurut || keHurufKecil(terurut->nama) >= keHurufKecil(saatIni->nama)) {
            saatIni->berikutnya = terurut; saatIni->sebelumnya = nullptr;
            if (terurut) terurut->sebelumnya = saatIni;
            terurut = saatIni;
        } else {
            Pasien* temp = terurut;
            while (temp->berikutnya && keHurufKecil(temp->berikutnya->nama) < keHurufKecil(saatIni->nama)) temp = temp->berikutnya;
            saatIni->berikutnya = temp->berikutnya; saatIni->sebelumnya = temp;
            if (temp->berikutnya) temp->berikutnya->sebelumnya = saatIni;
            temp->berikutnya = saatIni;
        }
        saatIni = berikutnya;
    }
    kepalaAntrean = terurut; ekorAntrean = terurut;
    while (ekorAntrean && ekorAntrean->berikutnya) ekorAntrean = ekorAntrean->berikutnya;
}

// --- FUNGSI PENCARIAN --- //

Pasien* cariBinerNIK(const string& nikDicari) {
    int hitung = 0; Pasien* saatIni = kepalaAntrean;
    while (saatIni) { hitung++; saatIni = saatIni->berikutnya; }
    if (hitung == 0) return nullptr;
    Pasien** larik = new Pasien*[hitung];
    saatIni = kepalaAntrean;
    for (int i = 0; i < hitung; i++) { larik[i] = saatIni; saatIni = saatIni->berikutnya; }

    int kiri = 0, kanan = hitung - 1;
    while (kiri <= kanan) {
        int tengah = kiri + (kanan - kiri) / 2;
        if (larik[tengah]->nik == nikDicari) { Pasien* hasil = larik[tengah]; delete[] larik; return hasil; }
        if (larik[tengah]->nik < nikDicari) kiri = tengah + 1; else kanan = tengah - 1;
    }
    delete[] larik; return nullptr;
}

void cariLompatNama(const string& namaDicari) {
    if (!kepalaAntrean) { cout << "Antrian kosong.\n"; return; }
    int n = 0; Pasien* saatIni = kepalaAntrean;
    while (saatIni) { n++; saatIni = saatIni->berikutnya; }
    int langkah = sqrt(n);
    bool ditemukan = false; string namaDicariKecil = keHurufKecil(namaDicari);
    Pasien* nodeSebelumnya = nullptr; saatIni = kepalaAntrean;

    while (saatIni) {
        int hitungLangkah = 0;
        while (hitungLangkah < langkah && saatIni) { nodeSebelumnya = saatIni; saatIni = saatIni->berikutnya; hitungLangkah++; }
        if (!saatIni || keHurufKecil(nodeSebelumnya->nama) >= namaDicariKecil) break;
    }
    while (nodeSebelumnya) {
        if (keHurufKecil(nodeSebelumnya->nama).find(namaDicariKecil) != string::npos) {
            cout << "Ditemukan: " << nodeSebelumnya->nama << " (NIK: " << nodeSebelumnya->nik << ")\n";
            ditemukan = true;
        }
        nodeSebelumnya = nodeSebelumnya->sebelumnya;
    }
    if (!ditemukan) cout << "Pasien dengan nama '" << namaDicari << "' tidak ditemukan.\n";
}

// --- FUNGSI UTAMA OPERASI ANTRIAN --- //

void tambahAntreanPasien(Pasien* pasienBaru) {
    pasienBaru->waktuPendaftaran = time(nullptr);
    pasienBaru->berikutnya = nullptr; pasienBaru->sebelumnya = ekorAntrean;
    if (!kepalaAntrean) kepalaAntrean = ekorAntrean = pasienBaru;
    else { ekorAntrean->berikutnya = pasienBaru; ekorAntrean = pasienBaru; }
}

Pasien* hapusAntreanPasien(const string& nik) {
    Pasien* saatIni = kepalaAntrean;
    while (saatIni) {
        if (saatIni->nik == nik) {
            if (saatIni->sebelumnya) saatIni->sebelumnya->berikutnya = saatIni->berikutnya; else kepalaAntrean = saatIni->berikutnya;
            if (saatIni->berikutnya) saatIni->berikutnya->sebelumnya = saatIni->sebelumnya; else ekorAntrean = saatIni->sebelumnya;
            saatIni->berikutnya = nullptr; saatIni->sebelumnya = nullptr;
            return saatIni;
        }
        saatIni = saatIni->berikutnya;
    }
    return nullptr;
}

void layaniPasienBerikutnya() {
    if (!mintaKonfirmasi("\nApakah Anda yakin ingin melayani pasien berikutnya?")) { cout << "Pelayanan pasien dibatalkan.\n"; return; }
    if (!kepalaAntrean) { cout << "\nAntrian kosong, tidak ada pasien yang bisa dilayani.\n"; return; }

    Pasien* pasienDilayani = kepalaAntrean;
    kepalaAntrean = kepalaAntrean->berikutnya;
    if (kepalaAntrean) kepalaAntrean->sebelumnya = nullptr;
    if (!kepalaAntrean) ekorAntrean = nullptr;

    Riwayat* riwayatBaru = new Riwayat;
    riwayatBaru->nik = pasienDilayani->nik; riwayatBaru->nama = pasienDilayani->nama; riwayatBaru->poli = pasienDilayani->poli;
    riwayatBaru->waktuPelayanan = time(nullptr);
    riwayatBaru->berikutnya = kepalaRiwayat; riwayatBaru->sebelumnya = nullptr;
    if (kepalaRiwayat) kepalaRiwayat->sebelumnya = riwayatBaru;
    kepalaRiwayat = riwayatBaru;
    if (!ekorRiwayat) ekorRiwayat = riwayatBaru;

    cout << "\nPasien dengan NIK " << pasienDilayani->nik << " (" << pasienDilayani->nama 
         << ") telah dilayani di poli " << pasienDilayani->poli << ".\n";
    delete pasienDilayani;
    simpanData();
}

void batalkanAntreanPasien() {
    if (!kepalaAntrean) { cout << "\nAntrian kosong, tidak ada pasien untuk dibatalkan.\n"; return; }
    string nik = inputTeks("\nMasukkan NIK pasien yang akan dibatalkan: ");
    Pasien* pasienDibatalkan = hapusAntreanPasien(nik);

    if (!pasienDibatalkan) { cout << "Pasien dengan NIK " << nik << " tidak ditemukan.\n"; return; }
    if (!mintaKonfirmasi("Apakah Anda yakin ingin membatalkan antrian pasien ini?")) {
        tambahAntreanPasien(pasienDibatalkan); cout << "Pembatalan antrian dibatalkan.\n"; return;
    }
    NodeUndo* nodeUndo = new NodeUndo;
    nodeUndo->pasien = pasienDibatalkan; nodeUndo->berikutnya = kepalaUndo;
    kepalaUndo = nodeUndo;

    cout << "Antrian pasien dengan NIK " << nik << " berhasil dibatalkan.\n";
    simpanData();
}

void batalkanUndo() {
    if (!kepalaUndo) { cout << "\nTidak ada aksi batal yang bisa di-undo.\n"; return; }
    if (!mintaKonfirmasi("\nApakah Anda yakin ingin mengembalikan pembatalan terakhir?")) { cout << "Undo dibatalkan.\n"; return; }

    NodeUndo* nodeUndo = kepalaUndo;
    kepalaUndo = kepalaUndo->berikutnya;
    tambahAntreanPasien(nodeUndo->pasien);
    cout << "\nUndo berhasil. Pasien dengan NIK " << nodeUndo->pasien->nik 
         << " dikembalikan ke antrian.\n";
    delete nodeUndo;
    simpanData();
}

// Menampilkan semua pasien dalam antrean aktif
void tampilkanSemuaAntrean() {
    if (!kepalaAntrean) { cout << "\nAntrian kosong.\n"; return; }
    cout << "\n=== DAFTAR ANTRIAN PASIEN ===\n";
    cout << left << setw(4) << "No"
         << left << setw(20) << "Nama"
         << left << setw(20) << "NIK"
         << left << setw(18) << "Poli (Kode)"
         << left << "Waktu Registrasi\n";
    cout << "------------------------------------------------------------------------------------\n";

    Pasien* saatIni = kepalaAntrean; int nomor = 1;
    while (saatIni) {
        string namaTampil = saatIni->nama;
        if (namaTampil.length() > 19) namaTampil = namaTampil.substr(0, 16) + "...";
        
        string poliTampil = saatIni->poli + " (" + dapatkanKodePoli(saatIni->poli) + ")";
        if (poliTampil.length() > 17) poliTampil = poliTampil.substr(0, 14) + "...";

        cout << left << setw(4) << (to_string(nomor++) + ".");
        cout << left << setw(20) << namaTampil; 
        cout << left << setw(20) << saatIni->nik;
        cout << left << setw(18) << poliTampil;
        cout << left << formatWaktu(saatIni->waktuPendaftaran) << endl;
        saatIni = saatIni->berikutnya;
    }
    cout << "------------------------------------------------------------------------------------\n";
}

// Menampilkan riwayat pelayanan (urutan terbaru ke terlama)
void tampilkanRiwayatPelayanan() {
    if (!kepalaRiwayat) { cout << "\nBelum ada riwayat pelayanan.\n"; return; }
    cout << "\n=== RIWAYAT PELAYANAN (TERBARU -> TERLAMA) ===\n";
    cout << left << setw(4) << "No"
         << left << setw(20) << "Nama"
         << left << setw(20) << "NIK"
         << left << setw(18) << "Poli (Kode)" 
         << left << "Waktu Pelayanan\n";
    cout << "------------------------------------------------------------------------------------\n"; // Garis juga disesuaikan

    Riwayat* saatIni = kepalaRiwayat; int nomor = 1;
    while (saatIni) {
        string namaTampil = saatIni->nama;
        if (namaTampil.length() > 19) namaTampil = namaTampil.substr(0, 16) + "...";
        
        string poliTampil = saatIni->poli + " (" + dapatkanKodePoli(saatIni->poli) + ")";
        if (poliTampil.length() > 17) poliTampil = poliTampil.substr(0, 14) + "...";
        
        cout << left << setw(4) << (to_string(nomor++) + ".");
        cout << left << setw(20) << namaTampil;
        cout << left << setw(20) << saatIni->nik;
        cout << left << setw(18) << poliTampil;
        cout << left << formatWaktu(saatIni->waktuPelayanan) << endl;
        saatIni = saatIni->berikutnya;
    }
    cout << "------------------------------------------------------------------------------------\n";
}

// Menu pendaftaran pasien baru
void menuPendaftaranPasien() {
    cout << "\n=== REGISTRASI PASIEN ===\n";
    Pasien* pasienBaru = new Pasien();
    pasienBaru->nama = inputTeks("Nama Lengkap: ");
    pasienBaru->nik = inputTeks("NIK: ");
    pasienBaru->alamat = inputTeks("Alamat: ");
    pasienBaru->umur = inputAngkaBulat("Umur (tahun): ");
    pasienBaru->beratBadan = inputAngkaDesimal("Berat Badan (kg): ");
    pasienBaru->tinggiBadan = inputAngkaDesimal("Tinggi Badan (cm): ");

    cout << "\nDaftar Poli:\n"; // Tampilkan daftar poli dengan kode
    for (int i = 0; i < MAKS_POLI; i++) {
        cout << "  " << daftarKodePoli[i] << ". " << daftarPoli[i] << "\n";
    }
    cout << endl;

    string inputPoliPasien;
    string namaPoliDitemukan;
    while (true) {
        inputPoliPasien = inputTeks("Poli yang Dituju (Kode/Nama): ");
        if (validasiInputPoli(inputPoliPasien, namaPoliDitemukan)) {
            pasienBaru->poli = namaPoliDitemukan;
            break;
        }
        cout << "Poli tidak valid! Silakan coba lagi.\n";
    }

    // Konfirmasi data pasien
    cout << "\nKonfirmasi Data Pasien:\n"
         << "Nama       : " << pasienBaru->nama << "\n"
         << "NIK        : " << pasienBaru->nik << "\n"
         << "Alamat     : " << pasienBaru->alamat << "\n"
         << "Umur       : " << pasienBaru->umur << " tahun\n"
         << "Berat Badan: " << pasienBaru->beratBadan << " kg\n"
         << "Tinggi Badan: " << pasienBaru->tinggiBadan << " cm\n"
         << "Poli       : " << pasienBaru->poli << " (" << dapatkanKodePoli(pasienBaru->poli) << ")\n";

    if (mintaKonfirmasi("Tambahkan pasien ke antrian?")) {
        tambahAntreanPasien(pasienBaru);
        cout << "Pasien berhasil didaftarkan ke antrian.\n";
        simpanData();
    } else { delete pasienBaru; cout << "Pendaftaran dibatalkan.\n"; }
}

// Menu pengurutan antrean
void menuPengurutan() {
    if (!kepalaAntrean || !kepalaAntrean->berikutnya) { cout << "\nTidak cukup data untuk diurutkan.\n"; return; }
    cout << "\n=== MENU PENGURUTAN ===\n"
         << "1. Urutkan berdasarkan waktu registrasi (terlama -> terbaru)\n"
         << "2. Urutkan berdasarkan nama (A -> Z)\n"
         << "3. Kembali\n";

    int pilihan = inputAngkaBulat("Pilih metode pengurutan: ");
    switch (pilihan) {
        case 1: urutkanWaktuRegistrasi(); cout << "Data berhasil diurutkan berdasarkan waktu registrasi.\n"; tampilkanSemuaAntrean(); break;
        case 2: urutkanNama(); cout << "Data berhasil diurutkan berdasarkan nama.\n"; tampilkanSemuaAntrean(); break;
        case 3: break;
        default: cout << "Pilihan tidak valid!\n";
    }
}

// Menu pencarian pasien
void menuPencarian() {
    cout << "\n=== MENU PENCARIAN ===\n"
         << "1. Cari berdasarkan NIK (pencarian biner)\n"
         << "2. Cari berdasarkan nama (pencarian lompat)\n"
         << "3. Kembali\n";

    int pilihan = inputAngkaBulat("Pilih metode pencarian: ");
    switch (pilihan) {
        case 1: {
            string nik = inputTeks("Masukkan NIK yang dicari: ");
            urutkanNama();
            Pasien* hasil = cariBinerNIK(nik);
            if (hasil) {
                cout << "\nData Pasien Ditemukan:\n"
                     << "Nama       : " << hasil->nama << "\n"
                     << "NIK        : " << hasil->nik << "\n"
                     << "Alamat     : " << hasil->alamat << "\n"
                     << "Umur       : " << hasil->umur << " tahun\n"
                     << "Berat Badan: " << hasil->beratBadan << " kg\n"
                     << "Tinggi Badan: " << hasil->tinggiBadan << " cm\n"
                     << "Poli       : " << hasil->poli << " (" << dapatkanKodePoli(hasil->poli) << ")\n"
                     << "Waktu Registrasi: " << formatWaktu(hasil->waktuPendaftaran) << "\n";
            } else { cout << "Pasien dengan NIK " << nik << " tidak ditemukan.\n"; }
            break;
        }
        case 2: {
            string nama = inputTeks("Masukkan nama yang dicari: ");
            cariLompatNama(nama);
            break;
        }
        case 3: break;
        default: cout << "Pilihan tidak valid!\n";
    }
}

// Menampilkan menu utama aplikasi
void tampilkanMenuUtama() {
    cout << "\n=== SISTEM ANTRIAN RUMAH SAKIT ===\n"
         << "1. Registrasi Pasien Baru\n"
         << "2. Layani Pasien Berikutnya\n"
         << "3. Batalkan Antrian Pasien\n"
         << "4. Batalkan Undo Pembatalan Terakhir\n"
         << "5. Tampilkan Semua Antrian\n"
         << "6. Tampilkan Riwayat Pelayanan\n"
         << "7. Urutkan Antrian\n"
         << "8. Cari Pasien\n"
         << "9. Simpan Data ke File\n"
         << "10. Muat Data dari File\n"
         << "0. Keluar\n";
}

void balikKeMenu(){
    cout << "\nTekan Enter untuk kembali ke menu utama...";
    cin.ignore();
    system("cls");
}

// Fungsi utama program
int main() {
    muatData();
    while (true) {
        tampilkanMenuUtama();
        int pilihan = inputAngkaBulat("Pilih menu: ");
        system("cls");
        switch (pilihan) {
            case 1: menuPendaftaranPasien(); balikKeMenu(); break;
            case 2: layaniPasienBerikutnya(); balikKeMenu(); break;
            case 3: batalkanAntreanPasien(); balikKeMenu(); break;
            case 4: batalkanUndo(); balikKeMenu(); break;
            case 5: tampilkanSemuaAntrean(); balikKeMenu(); break;
            case 6: tampilkanRiwayatPelayanan(); balikKeMenu(); break;
            case 7: menuPengurutan(); balikKeMenu(); break;
            case 8: menuPencarian(); balikKeMenu(); break;
            case 9: simpanData(); balikKeMenu(); break;
            case 10: muatData(); balikKeMenu(); break;
            case 0: 
                if (mintaKonfirmasi("\nApakah Anda yakin ingin keluar?")) {
                    cout << "\nTerima kasih telah menggunakan sistem ini.\n";
                    while (kepalaAntrean) { Pasien* temp = kepalaAntrean; kepalaAntrean = kepalaAntrean->berikutnya; delete temp; }
                    while (kepalaRiwayat) { Riwayat* temp = kepalaRiwayat; kepalaRiwayat = kepalaRiwayat->berikutnya; delete temp; }
                    while (kepalaUndo) { NodeUndo* temp = kepalaUndo; kepalaUndo = kepalaUndo->berikutnya; delete temp->pasien; delete temp; }
                    return 0;
                }
                break;
            default: cout << "Pilihan tidak valid! Silakan coba lagi.\n";
        }
    }
}
