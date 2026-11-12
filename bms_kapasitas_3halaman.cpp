// ============================================================
// PENGUKUR KAPASITAS BATERAI Li-ion 6S4P  (versi 3 halaman)
// Praktikum Algoritma & Pemrograman Komputer (NTRO236017)
//
// HALAMAN 1 : Judul + tombol START
// HALAMAN 2 : Input tegangan 6 grup
// HALAMAN 3 : Langkah perhitungan + hasil, lalu reset ke Halaman 1
//
// Peta modul:
//  Modul 3 : const, if-else, switch-case (pengatur halaman), prototipe fungsi
//  Modul 4 : do-while (menu & validasi), while (loop program), for
//  Modul 5 : array 1D, Max-Min Search (grup terlemah)
//  Modul 6 : dekomposisi fungsi (1 halaman = 1 fungsi), parsing string
//  Modul 8 : assert() unit test, setw() & setprecision()
// ============================================================
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cassert>
#include <cstdlib>
using namespace std;

// ---------- MODUL 3: Konstanta ----------
const int   JML_SEL       = 6;      // 6S : 6 grup seri
const int   JML_PARALEL   = 4;      // 4P : 4 sel paralel per grup
const float V_NOMINAL     = 3.70f;  // tegangan nominal 1 sel
const float KAP_SEL_AH    = 2.5f;   // kapasitas 1 sel = 2.500 mAh
const float KAP_PACK_AH   = JML_PARALEL * KAP_SEL_AH;   // 4 x 2.5 = 10 Ah
const float V_PENUH       = 4.20f;  // sel penuh  (SOC 100%)
const float V_KOSONG      = 3.00f;  // sel kosong (SOC 0%)
const float V_MAKS_INPUT  = 5.00f;  // batas input masuk akal
const int   LEBAR         = 58;     // lebar garis tampilan

// Deklarasi (prototipe) fungsi - definisinya ada di bagian bawah
bool ubahKeAngka(string teks, float& hasil);

// ================= MODUL PROSES =================
float hitungSOC(float v) {
    float soc = (v - V_KOSONG) / (V_PENUH - V_KOSONG) * 100.0f;
    if (soc < 0.0f)   soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    return soc;
}

float hitungKapasitasTersisa(float soc) {
    return soc / 100.0f * KAP_PACK_AH;
}

float hitungTotal(const float v[], int n) {
    float total = 0.0f;
    for (int i = 0; i < n; i++) total += v[i];
    return total;
}

int cariGrupTerlemah(const float v[], int n) {      // Max-Min Search (minimum)
    int idx = 0;
    for (int i = 1; i < n; i++)
        if (v[i] < v[idx]) idx = i;
    return idx;
}

int tentukanStatus(float soc) {
    if (soc >= 95.0f)      return 1;
    else if (soc >= 50.0f) return 2;
    else if (soc >= 20.0f) return 3;
    else if (soc > 0.0f)   return 4;
    else                   return 5;
}

string pesanStatus(int kode) {
    switch (kode) {
        case 1:  return "PENUH";
        case 2:  return "BAIK";
        case 3:  return "SEDANG - segera isi ulang";
        case 4:  return "RENDAH - isi ulang sekarang!";
        case 5:  return "HABIS - hentikan pemakaian!";
        default: return "TIDAK DIKENAL";
    }
}

string buatBar(float soc) {
    const int PANJANG = 20;
    int terisi = (int)(soc / 100.0f * PANJANG + 0.5f);
    string bar = "[";
    for (int i = 0; i < PANJANG; i++)
        bar += (i < terisi) ? '#' : '-';
    bar += "]";
    return bar;
}

// ---------- MODUL 8: Unit test (senyap; program berhenti bila ada yang salah) ----------
void jalankanUnitTest() {
    assert(fabs(hitungSOC(3.0f) - 0.0f)   < 0.01f);
    assert(fabs(hitungSOC(4.2f) - 100.0f) < 0.01f);
    assert(fabs(hitungSOC(3.6f) - 50.0f)  < 0.01f);
    assert(hitungSOC(2.5f) == 0.0f);
    assert(hitungSOC(4.5f) == 100.0f);
    assert(fabs(hitungKapasitasTersisa(50.0f)  - 5.0f)  < 0.01f);
    assert(fabs(hitungKapasitasTersisa(100.0f) - 10.0f) < 0.01f);
    float uji[4] = {3.8f, 3.9f, 3.7f, 3.85f};
    assert(cariGrupTerlemah(uji, 4) == 2);
    assert(fabs(hitungTotal(uji, 4) - 15.25f) < 0.0001f);
    assert(tentukanStatus(100.0f) == 1);
    assert(tentukanStatus(70.0f)  == 2);
    assert(tentukanStatus(30.0f)  == 3);
    assert(tentukanStatus(10.0f)  == 4);
    assert(tentukanStatus(0.0f)   == 5);
    float h = 0.0f;
    assert(ubahKeAngka("3.85", h) && fabs(h - 3.85f) < 0.0001f);
    assert(ubahKeAngka("3,85", h) && fabs(h - 3.85f) < 0.0001f);
    assert(ubahKeAngka(" 3.85 ", h));
    assert(!ubahKeAngka("abc", h));
    assert(!ubahKeAngka("3.85V", h));
    assert(!ubahKeAngka("", h));
}

// ================= ALAT BANTU TAMPILAN =================
void bersihkanLayar() {                  // pindah halaman = layar dibersihkan
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void cetakGaris(char c) {
    cout << string(LEBAR, c) << endl;
}

void cetakHeader(int no, const string& judul) {
    cetakGaris('=');
    cout << "  PENGUKUR KAPASITAS BATERAI 6S4P      HALAMAN " << no << " / 3" << endl;
    cout << "  " << judul << endl;
    cetakGaris('=');
}

// Baca 1 baris dari keyboard (aman: tidak pernah membuat cin error)
string bacaBaris() {
    string baris;
    if (!getline(cin, baris)) {
        cout << "\n[ERROR] Input terputus. Program dihentikan." << endl;
        exit(1);
    }
    return baris;
}

void tungguEnter(const string& pesan) {
    cout << pesan;
    bacaBaris();
}

// Teks -> angka. Koma desimal ("3,85") dianggap titik. false bila bukan angka murni.
bool ubahKeAngka(string teks, float& hasil) {
    string bersih = "";
    for (size_t i = 0; i < teks.length(); i++) {
        if (teks[i] == ',')       bersih += '.';
        else if (teks[i] != ' ' && teks[i] != '\t' && teks[i] != '\r') bersih += teks[i];
    }
    if (bersih.empty()) return false;
    try {
        size_t posisi;
        hasil = stof(bersih, &posisi);
        if (posisi != bersih.length()) return false;
    } catch (...) {
        return false;
    }
    return true;
}

// ================= HALAMAN 1 : JUDUL + START =================
// return true = START, false = KELUAR
bool halaman1() {
    string pilih;
    bool valid;
    do {
        bersihkanLayar();
        cetakGaris('=');
        cout << "\n";
        cout << "        SISTEM PENGUKUR KAPASITAS BATERAI" << endl;
        cout << "         Battery Management System (BMS)" << endl;
        cout << "           Baterai Li-ion 6S4P (24 sel)" << endl;
        cout << "\n";
        cetakGaris('=');
        cout << fixed << setprecision(1);
        cout << "  Susunan sel       : " << JML_SEL << " seri x " << JML_PARALEL << " paralel" << endl;
        cout << "  Tegangan nominal  : " << JML_SEL << " x " << V_NOMINAL << " V = "
             << JML_SEL * V_NOMINAL << " V" << endl;
        cout << "  Kapasitas pack    : " << JML_PARALEL << " x " << KAP_SEL_AH * 1000
             << " mAh = " << KAP_PACK_AH * 1000 << " mAh (" << KAP_PACK_AH << " Ah)" << endl;
        cetakGaris('-');
        cout << "   [1]  START   - mulai pengukuran" << endl;
        cout << "   [0]  KELUAR  - tutup program" << endl;
        cetakGaris('-');
        cout << "  Pilih menu : ";
        pilih = bacaBaris();

        valid = (pilih == "1" || pilih == "0");
        if (!valid) {
            cout << "  [ERROR] Pilihan tidak valid! Ketik 1 atau 0." << endl;
            tungguEnter("  Tekan Enter untuk mencoba lagi...");
        }
    } while (!valid);
    return (pilih == "1");
}

// ================= HALAMAN 2 : INPUT NILAI =================
void halaman2(float v[]) {
    bersihkanLayar();
    cetakHeader(2, "Memasukkan Tegangan Tiap Grup");
    cout << "  Ukur tegangan tiap grup (S1P4 s.d. S" << JML_SEL << "P4) dengan voltmeter," << endl;
    cout << "  lalu masukkan nilainya. Rentang 0 - " << V_MAKS_INPUT
         << " V (contoh: 3.85)" << endl;
    cetakGaris('-');

    for (int i = 0; i < JML_SEL; i++) {
        bool valid;
        do {
            cout << "  Grup S" << i + 1 << "P" << JML_PARALEL << " (Volt) : ";
            valid = ubahKeAngka(bacaBaris(), v[i]) && v[i] >= 0.0f && v[i] <= V_MAKS_INPUT;
            if (!valid)
                cout << "  [ERROR] Masukkan angka 0 - " << V_MAKS_INPUT
                     << " (contoh: 3.85)!" << endl;
        } while (!valid);
    }

    cetakGaris('-');
    cout << fixed << setprecision(2);
    cout << "  Data yang dimasukkan : ";
    for (int i = 0; i < JML_SEL; i++) cout << v[i] << (i < JML_SEL - 1 ? " | " : " V");
    cout << endl;
    cetakGaris('-');
    tungguEnter("  Tekan Enter untuk MENGHITUNG...");
}

// ================= HALAMAN 3 : LANGKAH + HASIL =================
void halaman3(const float v[]) {
    // --- Proses ---
    float total     = hitungTotal(v, JML_SEL);
    int   idx       = cariGrupTerlemah(v, JML_SEL);
    float vLemah    = v[idx];
    float socMentah = (vLemah - V_KOSONG) / (V_PENUH - V_KOSONG) * 100.0f;
    float soc       = hitungSOC(vLemah);
    float kapSisa   = hitungKapasitasTersisa(soc);
    float kapPakai  = KAP_PACK_AH - kapSisa;
    int   kode      = tentukanStatus(soc);

    // --- Output ---
    bersihkanLayar();
    cetakHeader(3, "Langkah Perhitungan & Hasil");
    cout << fixed << setprecision(2);

    cout << "\nLANGKAH 1 : Jumlahkan tegangan semua grup (rangkaian seri)" << endl;
    cout << "   ";
    for (int i = 0; i < JML_SEL; i++) cout << v[i] << (i < JML_SEL - 1 ? " + " : "");
    cout << " = " << total << " V" << endl;

    cout << "\nLANGKAH 2 : Cari grup terlemah (tegangan paling rendah)" << endl;
    cout << "   Grup terlemah = S" << idx + 1 << "P" << JML_PARALEL
         << " dengan tegangan " << vLemah << " V" << endl;
    cout << "   (seperti rantai: kekuatannya ditentukan mata rantai terlemah)" << endl;

    cout << "\nLANGKAH 3 : Hitung SOC (persen isi) grup terlemah" << endl;
    cout << "   SOC = (V - Vkosong) / (Vpenuh - Vkosong) x 100%" << endl;
    cout << "       = (" << vLemah << " - " << V_KOSONG << ") / (" << V_PENUH << " - "
         << V_KOSONG << ") x 100%" << endl;
    cout << "       = " << socMentah << " %" << endl;
    if (socMentah != soc)
        cout << "   (dibatasi pada rentang 0 - 100 %, menjadi " << soc << " %)" << endl;

    cout << "\nLANGKAH 4 : Hitung kapasitas tersisa" << endl;
    cout << "   Kapasitas pack = " << JML_PARALEL << " x " << KAP_SEL_AH << " Ah = "
         << KAP_PACK_AH << " Ah" << endl;
    cout << "   Tersisa        = " << soc << " % x " << KAP_PACK_AH << " Ah = "
         << kapSisa << " Ah" << endl;

    cout << "\nLANGKAH 5 : Hitung kapasitas terpakai" << endl;
    cout << "   Terpakai = " << KAP_PACK_AH << " - " << kapSisa << " = "
         << kapPakai << " Ah" << endl;

    cout << "\nLANGKAH 6 : Tentukan status baterai" << endl;
    cout << "   SOC " << setprecision(0) << soc << setprecision(2) << " %  ->  "
         << pesanStatus(kode) << endl;

    // --- Hasil akhir ---
    cout << "\n";
    cetakGaris('=');
    cout << "  HASIL AKHIR" << endl;
    cetakGaris('=');
    cout << left << setw(10) << "  Grup" << setw(10) << "Tegangan" << "SOC" << endl;
    for (int i = 0; i < JML_SEL; i++) {
        cout << "  " << left << setw(8) << ("S" + to_string(i + 1) + "P" + to_string(JML_PARALEL))
             << setw(10) << v[i] << setprecision(0) << hitungSOC(v[i]) << " %"
             << setprecision(2) << (i == idx ? "   <-- terlemah" : "") << endl;
    }
    cetakGaris('-');
    cout << left << setw(24) << "  Tegangan Total Pack"  << ": " << total    << " V"  << endl;
    cout << left << setw(24) << "  Kapasitas Pack"       << ": " << KAP_PACK_AH << " Ah" << endl;
    cout << left << setw(24) << "  Kapasitas Tersisa"    << ": " << kapSisa  << " Ah"  << endl;
    cout << left << setw(24) << "  Kapasitas Terpakai"   << ": " << kapPakai << " Ah"  << endl;
    cout << left << setw(24) << "  Isi Baterai"          << ": " << buatBar(soc)
         << " " << setprecision(0) << soc << " %" << setprecision(2) << endl;
    cout << left << setw(24) << "  Status"               << ": " << pesanStatus(kode) << endl;
    cetakGaris('=');
    tungguEnter("  Tekan Enter untuk RESET dan kembali ke Halaman 1...");
}

// ================= PROGRAM UTAMA =================
void resetData(float v[]) {
    for (int i = 0; i < JML_SEL; i++) v[i] = 0.0f;
}

int main() {
    jalankanUnitTest();                  // bila ada yang salah, program berhenti di sini

    float v[JML_SEL] = {0};
    int  halaman = 1;                    // penanda halaman yang sedang tampil
    bool jalan   = true;

    while (jalan) {
        switch (halaman) {
            case 1:
                if (halaman1()) halaman = 2;
                else            jalan = false;           // pilih KELUAR
                break;
            case 2:
                halaman2(v);
                halaman = 3;
                break;
            case 3:
                halaman3(v);
                resetData(v);                            // data dihapus
                halaman = 1;                             // kembali ke Halaman 1
                break;
        }
    }

    bersihkanLayar();
    cout << "Terima kasih. Program selesai." << endl;
    return 0;
}
