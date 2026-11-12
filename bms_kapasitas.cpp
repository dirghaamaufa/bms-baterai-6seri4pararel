// ============================================================
// PENGUKUR KAPASITAS BATERAI Li-ion 6S4P (versi sederhana)
// Praktikum Algoritma & Pemrograman Komputer (NTRO236017)
//
// Input  : tegangan 6 grup seri (Volt)
// Proses : tegangan -> SOC (%) -> kapasitas tersisa (Ah)
// Output : tegangan total, SOC, kapasitas tersisa, status
//
// Peta modul:
//  Modul 3 : const, if-else bertingkat, switch-case
//  Modul 4 : for, do-while (validasi input & ulangi pengukuran)
//  Modul 5 : array 1D, Max-Min Search (cari grup terlemah)
//  Modul 6 : dekomposisi fungsi (Input - Proses - Output)
//  Modul 8 : assert() unit test, setw() & setprecision()
// ============================================================
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cassert>
#include <cstdlib>   // exit()
using namespace std;

// ---------- MODUL 3: Konstanta ----------
const int   JML_SEL       = 6;      // 6S : 6 grup seri
const int   JML_PARALEL   = 4;      // 4P : 4 sel paralel per grup
const float KAP_SEL_AH    = 2.5f;   // kapasitas 1 sel (Ah) - ASUMSI, sesuaikan datasheet
const float KAP_PACK_AH   = JML_PARALEL * KAP_SEL_AH;   // 4 x 2.5 = 10 Ah
const float V_PENUH       = 4.20f;  // tegangan sel penuh  (SOC 100%)
const float V_KOSONG      = 3.00f;  // tegangan sel kosong (SOC 0%)
const float V_MAKS_INPUT  = 5.00f;  // batas input masuk akal

// Deklarasi (prototipe) fungsi - definisinya ada di bagian MODUL INPUT
bool ubahKeAngka(string teks, float& hasil);

// ================= MODUL PROSES =================

// Tegangan -> persen isi baterai (interpolasi linear 3.0 V .. 4.2 V)
float hitungSOC(float v) {
    float soc = (v - V_KOSONG) / (V_PENUH - V_KOSONG) * 100.0f;
    if (soc < 0.0f)   soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    return soc;
}

// Persen isi -> kapasitas tersisa (Ah)
float hitungKapasitasTersisa(float soc) {
    return soc / 100.0f * KAP_PACK_AH;
}

// Jumlah tegangan semua grup (seri -> dijumlah)
float hitungTotal(const float v[], int n) {
    float total = 0.0f;
    for (int i = 0; i < n; i++) total += v[i];
    return total;
}

// Max-Min Search: cari grup dengan tegangan terendah (grup terlemah)
int cariGrupTerlemah(const float v[], int n) {
    int idx = 0;
    for (int i = 1; i < n; i++)
        if (v[i] < v[idx]) idx = i;
    return idx;
}

// if-else bertingkat -> kode status (dasar untuk switch-case)
int tentukanStatus(float soc) {
    if (soc >= 95.0f)      return 1;   // penuh
    else if (soc >= 50.0f) return 2;   // baik
    else if (soc >= 20.0f) return 3;   // sedang
    else if (soc > 0.0f)   return 4;   // rendah
    else                   return 5;   // habis
}

// switch-case -> pesan status
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

// Bar visual isi baterai: [##########----------]
string buatBar(float soc) {
    const int PANJANG = 20;
    int terisi = (int)(soc / 100.0f * PANJANG + 0.5f);
    string bar = "[";
    for (int i = 0; i < PANJANG; i++)
        bar += (i < terisi) ? '#' : '-';
    bar += "]";
    return bar;
}

// ---------- MODUL 8: Unit test ----------
void jalankanUnitTest() {
    assert(fabs(hitungSOC(3.0f) - 0.0f)   < 0.01f);
    assert(fabs(hitungSOC(4.2f) - 100.0f) < 0.01f);
    assert(fabs(hitungSOC(3.6f) - 50.0f)  < 0.01f);
    assert(hitungSOC(2.5f) == 0.0f);                 // di bawah kosong -> 0%
    assert(hitungSOC(4.5f) == 100.0f);               // di atas penuh  -> 100%
    assert(fabs(hitungKapasitasTersisa(50.0f) - 5.0f)  < 0.01f);
    assert(fabs(hitungKapasitasTersisa(100.0f) - 10.0f) < 0.01f);
    float uji[4] = {3.8f, 3.9f, 3.7f, 3.85f};
    assert(cariGrupTerlemah(uji, 4) == 2);
    assert(fabs(hitungTotal(uji, 4) - 15.25f) < 0.0001f);
    assert(tentukanStatus(100.0f) == 1);
    assert(tentukanStatus(70.0f)  == 2);
    assert(tentukanStatus(30.0f)  == 3);
    assert(tentukanStatus(10.0f)  == 4);
    assert(tentukanStatus(0.0f)   == 5);
    // pengujian konversi teks -> angka
    float h = 0.0f;
    assert(ubahKeAngka("3.85", h) && fabs(h - 3.85f) < 0.0001f);
    assert(ubahKeAngka("3,85", h) && fabs(h - 3.85f) < 0.0001f);   // koma desimal
    assert(!ubahKeAngka("abc", h));
    assert(!ubahKeAngka("3.85V", h));
    assert(!ubahKeAngka("", h));
    cout << "[UNIT TEST PASSED] Seluruh pengujian logika valid!" << endl;
}

// ================= MODUL INPUT =================
// Mengubah teks menjadi angka. Mengembalikan false bila bukan angka murni.
// Koma desimal ("3,85") otomatis dianggap titik ("3.85").
bool ubahKeAngka(string teks, float& hasil) {
    for (size_t i = 0; i < teks.length(); i++)
        if (teks[i] == ',') teks[i] = '.';
    try {
        size_t posisi;
        hasil = stof(teks, &posisi);
        if (posisi != teks.length()) return false;   // ada sisa huruf, misal "3.85V"
    } catch (...) {
        return false;                                // bukan angka / terlalu besar
    }
    return true;
}

// Membaca SATU kata dari keyboard lalu diubah ke angka.
// Dibaca sebagai teks supaya cin tidak pernah masuk keadaan error.
bool bacaTegangan(float& hasil) {
    string teks;
    if (!(cin >> teks)) {                            // input terputus (Ctrl+Z / Ctrl+D)
        cout << "\n[ERROR] Input terputus. Program dihentikan." << endl;
        exit(1);
    }
    return ubahKeAngka(teks, hasil);
}

// do-while: ulangi sampai input valid (angka 0 .. 5 V)
void inputTegangan(float v[]) {
    cout << "\nMasukkan tegangan tiap grup (Volt):" << endl;
    for (int i = 0; i < JML_SEL; i++) {
        bool valid;
        do {
            cout << "  Grup S" << i + 1 << "P" << JML_PARALEL << " : ";
            valid = bacaTegangan(v[i]) && v[i] >= 0.0f && v[i] <= V_MAKS_INPUT;
            if (!valid)
                cout << "  [ERROR] Masukkan angka 0 - " << V_MAKS_INPUT
                     << " V (contoh: 3.85)!" << endl;
        } while (!valid);
    }
}

// ================= MODUL OUTPUT =================
void tampilkanHasil(const float v[]) {
    // --- Proses ---
    float total      = hitungTotal(v, JML_SEL);
    int   terlemah   = cariGrupTerlemah(v, JML_SEL);
    float socTerlemah = hitungSOC(v[terlemah]);
    float kapSisa    = hitungKapasitasTersisa(socTerlemah);
    float kapPakai   = KAP_PACK_AH - kapSisa;
    int   kode       = tentukanStatus(socTerlemah);

    // --- Output ---
    cout << "\n=========== HASIL PENGUKURAN KAPASITAS ===========" << endl;
    cout << fixed << setprecision(2);
    cout << left << setw(10) << "Grup" << setw(12) << "Tegangan" << "SOC" << endl;
    cout << "--------------------------------------------------" << endl;
    for (int i = 0; i < JML_SEL; i++) {
        cout << left << setw(10) << ("S" + to_string(i + 1) + "P" + to_string(JML_PARALEL))
             << setw(12) << (to_string(v[i]).substr(0, 4) + " V")
             << setprecision(0) << hitungSOC(v[i]) << " %"
             << setprecision(2) << (i == terlemah ? "   <-- terlemah" : "") << endl;
    }
    cout << "--------------------------------------------------" << endl;
    cout << left << setw(26) << "Tegangan Total Pack"   << ": " << total << " V" << endl;
    cout << left << setw(26) << "Kapasitas Pack"        << ": " << KAP_PACK_AH << " Ah ("
         << JML_PARALEL << " x " << KAP_SEL_AH << " Ah)" << endl;
    cout << left << setw(26) << "Kapasitas Tersisa"     << ": " << kapSisa << " Ah" << endl;
    cout << left << setw(26) << "Kapasitas Terpakai"    << ": " << kapPakai << " Ah" << endl;
    cout << left << setw(26) << "Isi Baterai (SOC)"     << ": " << buatBar(socTerlemah)
         << " " << setprecision(0) << socTerlemah << " %" << setprecision(2) << endl;
    cout << left << setw(26) << "Status"                << ": " << pesanStatus(kode) << endl;
    cout << "==================================================" << endl;
    cout << "(Kapasitas dihitung dari grup terlemah: S" << terlemah + 1 << ")" << endl;
}

int main() {
    cout << "=== PENGUKUR KAPASITAS BATERAI Li-ion 6S4P ===" << endl;
    jalankanUnitTest();

    float v[JML_SEL] = {0};
    char lagi = 'n';
    do {
        inputTegangan(v);
        tampilkanHasil(v);
        cout << "\nUkur lagi? (y/n): ";
        if (!(cin >> lagi)) lagi = 'n';          // input terputus -> selesai
    } while (lagi == 'y' || lagi == 'Y');

    cout << "Pengukuran selesai." << endl;
    return 0;
}
