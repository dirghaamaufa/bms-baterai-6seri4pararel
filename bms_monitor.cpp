// ============================================================
// SISTEM PEMANTAUAN TEGANGAN SEL BATERAI (BMS) - Li-ion 6S4P
// Praktikum Algoritma & Pemrograman Komputer (NTRO236017)
//
// Peta konsep modul:
//  Modul 3 : const, if-else bertingkat, switch-case, operator logika
//  Modul 4 : for/while, break, continue, exit()
//  Modul 5 : Array 1D & 2D, Max-Min Search, Function Overloading
//  Modul 6 : Parsing string (find/substr), dekomposisi fungsi (IPO)
//  Modul 8 : assert() unit test, exp() <cmath>, setw/setprecision
//
// Format frame : $BMS,V,v1,v2,v3,v4,v5,v6,I#
//   v1..v6 = tegangan tiap grup seri (Volt)
//   I      = arus pack (Ampere): (+) charging, (-) discharging
// Jalankan     : ./bms          (jeda 1 detik per frame = realtime)
//                ./bms 0        (tanpa jeda, untuk pengujian cepat)
// ============================================================
#include <iostream>
#include <string>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <thread>
#include <chrono>
using namespace std;

// ---------- MODUL 3: Konstanta bertipe (const) ----------
// Konfigurasi pack
const int   JML_SEL      = 6;      // 6S : 6 grup seri (titik pantau)
const int   JML_PARALEL  = 4;      // 4P : 4 sel paralel per grup
const float KAP_SEL_AH   = 2.5f;   // kapasitas 1 sel (Ah) - ASUMSI, sesuaikan datasheet
const float KAP_PACK_AH  = JML_PARALEL * KAP_SEL_AH;   // kapasitas pack
const int   MAKS_SAMPEL  = 20;

// Batas tegangan per grup (Li-ion NMC/NCA)
const float V_CRIT_HIGH  = 4.30f;
const float V_OVER       = 4.20f;
const float V_UNDER      = 3.00f;
const float V_CRIT_LOW   = 2.80f;
const float V_NOMINAL    = 3.70f;
const float V_SENSOR_MAX = 5.00f;
const float SELISIH_MAKS = 0.10f;

// Batas tegangan total pack (= batas grup x jumlah grup seri)
const float V_PACK_CRIT_HIGH = JML_SEL * V_CRIT_HIGH;
const float V_PACK_OVER      = JML_SEL * V_OVER;
const float V_PACK_UNDER     = JML_SEL * V_UNDER;
const float V_PACK_CRIT_LOW  = JML_SEL * V_CRIT_LOW;

// Batas arus pack (ASUMSI: nominal = 1C), pola sama dengan OCR Modul 3
const float I_NOMINAL    = KAP_PACK_AH * 1.0f;   // 1C = 10 A
const float I_SENSOR_MAX = 100.0f;
const float I_IDLE       = 0.1f;                 // |I| < ini dianggap IDLE

const int   MAKS_INVALID = 3;

// ---------- MODUL 3: Zona tegangan grup (if-else bertingkat) ----------
int tentukanZona(float v) {
    if (v > V_CRIT_HIGH || v < V_CRIT_LOW) return 4;
    else if (v > V_OVER)                   return 2;
    else if (v < V_UNDER)                  return 3;
    else                                   return 1;
}

// Zona tegangan TOTAL pack
int tentukanZonaPack(float total) {
    if (total > V_PACK_CRIT_HIGH || total < V_PACK_CRIT_LOW) return 4;
    else if (total > V_PACK_OVER)                            return 2;
    else if (total < V_PACK_UNDER)                           return 3;
    else                                                     return 1;
}

// Zona ARUS pack (mirip proteksi OCR Modul 3)
int tentukanZonaArus(float arus) {
    float a = fabs(arus);
    if (a <= I_NOMINAL)             return 1;   // normal
    else if (a <= 1.2f * I_NOMINAL) return 2;   // warning
    else if (a <= 2.0f * I_NOMINAL) return 3;   // trip delay
    else                            return 4;   // hubung singkat
}

// ---------- MODUL 3: switch-case untuk pesan status ----------
string pesanZona(int zona) {
    switch (zona) {
        case 1:  return "NORMAL";
        case 2:  return "OVER-VOLTAGE (hentikan charging)";
        case 3:  return "UNDER-VOLTAGE (hentikan discharge)";
        case 4:  return "KRITIS (SHUTDOWN)";
        default: return "TIDAK DIKENAL";
    }
}
string pesanZonaArus(int zona) {
    switch (zona) {
        case 1:  return "NORMAL";
        case 2:  return "WARNING (arus di atas nominal)";
        case 3:  return "TRIP DELAY (overcurrent)";
        case 4:  return "INSTANT TRIP (hubung singkat)";
        default: return "TIDAK DIKENAL";
    }
}
string modeArus(float arus) {
    if (arus > I_IDLE)       return "CHARGING";
    else if (arus < -I_IDLE) return "DISCHARGING";
    else                     return "IDLE";
}

// ---------- MODUL 5: Function Overloading ----------
float hitungSelisih(float a, float b) {
    return fabs(a - b);
}
float hitungSelisih(const float v[], int n) {      // Max-Min Search
    float vMax = v[0], vMin = v[0];
    for (int i = 1; i < n; i++) {
        if (v[i] > vMax) vMax = v[i];
        if (v[i] < vMin) vMin = v[i];
    }
    return vMax - vMin;
}
float hitungTotal(const float v[], int n) {
    float total = 0.0f;                            // akumulator (Modul 4)
    for (int i = 0; i < n; i++) total += v[i];
    return total;
}
float hitungRataRata(const float v[], int n) {
    return hitungTotal(v, n) / n;
}
int cariIndeksMax(const float v[], int n) {
    int idx = 0;
    for (int i = 1; i < n; i++) if (v[i] > v[idx]) idx = i;
    return idx;
}
int cariIndeksMin(const float v[], int n) {
    int idx = 0;
    for (int i = 1; i < n; i++) if (v[i] < v[idx]) idx = i;
    return idx;
}

// Estimasi SOC linear (3.0 V = 0%, 4.2 V = 100%)
float estimasiSOC(float v) {
    float soc = (v - V_UNDER) / (V_OVER - V_UNDER) * 100.0f;
    if (soc < 0.0f)   soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    return soc;
}

// ---------- Validasi data sensor ----------
bool sensorValid(const float v[], int n, float arus) {
    for (int i = 0; i < n; i++)
        if (v[i] < 0.0f || v[i] > V_SENSOR_MAX) return false;
    if (fabs(arus) > I_SENSOR_MAX) return false;
    return true;
}

// ---------- MODUL 6: Parser frame serial ----------
// $BMS,V,v1,...,v6,I#  -> v[0..5] = tegangan, arus = I
bool parseFrame(const string& frame, float v[], float& arus) {
    if (frame.length() < 3 || frame.front() != '$' || frame.back() != '#')
        return false;

    string isi = frame.substr(1, frame.length() - 2);

    size_t pos = isi.find(',');
    if (pos == string::npos || isi.substr(0, pos) != "BMS") return false;
    isi = isi.substr(pos + 1);

    pos = isi.find(',');
    if (pos == string::npos || isi.substr(0, pos) != "V") return false;
    isi = isi.substr(pos + 1);

    const int JML_FIELD = JML_SEL + 1;             // 6 tegangan + 1 arus
    float f[JML_FIELD];
    for (int i = 0; i < JML_FIELD; i++) {
        pos = isi.find(',');
        bool terakhir = (i == JML_FIELD - 1);
        if (terakhir && pos != string::npos) return false;    // field berlebih
        if (!terakhir && pos == string::npos) return false;   // field kurang
        string token = (pos == string::npos) ? isi : isi.substr(0, pos);
        try { f[i] = stof(token); }
        catch (...) { return false; }
        if (!terakhir) isi = isi.substr(pos + 1);
    }
    for (int i = 0; i < JML_SEL; i++) v[i] = f[i];
    arus = f[JML_SEL];
    return true;
}

// ---------- MODUL 8: Unit Test dengan assert() ----------
void jalankanUnitTest() {
    // zona tegangan grup
    assert(tentukanZona(3.70f) == 1);
    assert(tentukanZona(4.25f) == 2);
    assert(tentukanZona(2.90f) == 3);
    assert(tentukanZona(4.40f) == 4);
    assert(tentukanZona(2.50f) == 4);
    assert(tentukanZona(4.20f) == 1);               // tepat di batas = normal
    // zona tegangan total pack (6S: 18.0 - 25.2 V)
    assert(tentukanZonaPack(23.15f) == 1);
    assert(tentukanZonaPack(25.50f) == 2);
    assert(tentukanZonaPack(17.50f) == 3);
    assert(tentukanZonaPack(26.00f) == 4);
    assert(tentukanZonaPack(16.00f) == 4);
    // zona arus (nominal 10 A)
    assert(tentukanZonaArus(5.0f)   == 1);
    assert(tentukanZonaArus(-10.0f) == 1);          // tepat di batas = normal
    assert(tentukanZonaArus(-11.0f) == 2);
    assert(tentukanZonaArus(15.0f)  == 3);
    assert(tentukanZonaArus(-25.0f) == 4);
    // mode arus
    assert(modeArus(5.0f)  == "CHARGING");
    assert(modeArus(-5.0f) == "DISCHARGING");
    assert(modeArus(0.0f)  == "IDLE");
    // selisih (overloading) + Max-Min
    assert(fabs(hitungSelisih(4.0f, 3.9f) - 0.1f) < 0.0001f);
    float uji[4] = {3.8f, 3.9f, 3.7f, 3.85f};
    assert(fabs(hitungSelisih(uji, 4) - 0.2f) < 0.0001f);
    assert(cariIndeksMax(uji, 4) == 1);
    assert(cariIndeksMin(uji, 4) == 2);
    assert(fabs(hitungTotal(uji, 4) - 15.25f) < 0.0001f);
    // parser
    float v[JML_SEL], arus;
    assert(parseFrame("$BMS,V,3.85,3.87,3.86,3.84,3.85,3.88,-5.0#", v, arus) == true);
    assert(fabs(v[1] - 3.87f) < 0.0001f);
    assert(fabs(v[5] - 3.88f) < 0.0001f);
    assert(fabs(arus - (-5.0f)) < 0.0001f);
    assert(parseFrame("INVALID_PACKET", v, arus) == false);
    assert(parseFrame("$BMS,V,3.85,3.87#", v, arus) == false);
    assert(parseFrame("$BMS,V,3.85,3.87,3.86,3.84,3.85,3.88#", v, arus) == false);  // tanpa arus
    assert(parseFrame("$BMS,V,3.85,abc,3.86,3.84,3.85,3.88,-5.0#", v, arus) == false);
    // SOC
    assert(estimasiSOC(3.0f) == 0.0f);
    assert(estimasiSOC(4.2f) == 100.0f);
    cout << "[UNIT TEST PASSED] Seluruh pengujian logika BMS valid!" << endl;
}

// ---------- MODUL 8: exp() - simulasi balancing pasif ----------
void simulasiBalancing(float v0, float R, float C) {
    float tau = R * C;
    cout << "\n--- Simulasi Balancing Pasif (model RC, tau = "
         << fixed << setprecision(1) << tau << " s) ---" << endl;
    cout << left << setw(12) << "Waktu t (s)" << right << setw(14) << "V(t) (V)" << endl;
    cout << fixed << setprecision(3);
    for (int k = 0; k <= 3; k++) {
        float t = k * tau;
        cout << left << setw(12) << t << right << setw(14) << v0 * exp(-t / tau) << endl;
    }
}

// ---------- Output: DASHBOARD REALTIME ----------
void tampilkanDashboard(int detik, const float v[], float arus) {
    const string garis(58, '=');
    const string strip(58, '-');

    float total   = hitungTotal(v, JML_SEL);
    float rata    = total / JML_SEL;
    float soc     = estimasiSOC(rata);
    float kapSisa = soc / 100.0f * KAP_PACK_AH;
    float energi  = total * kapSisa;                      // Wh = V x Ah
    float selisih = hitungSelisih(v, JML_SEL);
    int   zonaV   = tentukanZonaPack(total);
    int   zonaI   = tentukanZonaArus(arus);

    cout << "\n" << garis << endl;
    cout << "  BMS DASHBOARD 6S4P  |  Detik ke-" << detik << endl;
    cout << garis << endl;

    // --- Tegangan tiap grup ---
    cout << fixed << setprecision(2);
    cout << left << setw(8) << "Grup" << setw(10) << "V (V)" << setw(9) << "SOC(%)"
         << "Status" << endl;
    cout << strip << endl;
    for (int i = 0; i < JML_SEL; i++) {
        cout << left << setw(8) << ("S" + to_string(i + 1) + "P" + to_string(JML_PARALEL))
             << setw(10) << v[i] << setw(9) << setprecision(0) << estimasiSOC(v[i])
             << setprecision(2) << pesanZona(tentukanZona(v[i])) << endl;
    }
    cout << "Selisih Max-Min : " << selisih << " V (S" << cariIndeksMax(v, JML_SEL) + 1
         << " tertinggi, S" << cariIndeksMin(v, JML_SEL) + 1 << " terendah)";
    if (selisih > SELISIH_MAKS) cout << "  [WARNING] Grup sel tidak seimbang!";
    cout << endl;

    // --- Kapasitas ---
    cout << strip << endl;
    cout << "[KAPASITAS BATERAI]" << endl;
    cout << left << setw(22) << "  Kapasitas Nominal" << ": " << KAP_PACK_AH << " Ah ("
         << JML_PARALEL << " x " << KAP_SEL_AH << " Ah)" << endl;
    cout << left << setw(22) << "  Kapasitas Tersisa" << ": " << kapSisa << " Ah ("
         << setprecision(0) << soc << " %)" << setprecision(2) << endl;
    cout << left << setw(22) << "  Energi Tersimpan" << ": " << energi << " Wh" << endl;

    // --- Tegangan total ---
    cout << strip << endl;
    cout << "[TEGANGAN TOTAL PACK]" << endl;
    cout << left << setw(22) << "  Tegangan Total" << ": " << total << " V   ["
         << pesanZona(zonaV) << "]" << endl;
    cout << left << setw(22) << "  Rentang Aman" << ": " << V_PACK_UNDER << " - "
         << V_PACK_OVER << " V (nominal " << JML_SEL * V_NOMINAL << " V)" << endl;
    cout << left << setw(22) << "  Rata-rata per Grup" << ": " << rata << " V" << endl;

    // --- Arus total ---
    cout << strip << endl;
    cout << "[ARUS TOTAL PACK]" << endl;
    cout << left << setw(22) << "  Arus Pack" << ": " << showpos << arus << noshowpos
         << " A   [" << pesanZonaArus(zonaI) << "]" << endl;
    cout << left << setw(22) << "  Mode" << ": " << modeArus(arus) << endl;
    cout << left << setw(22) << "  Arus Nominal (1C)" << ": " << I_NOMINAL << " A   | Beban: "
         << setprecision(0) << fabs(arus) / I_NOMINAL * 100.0f << " % dari nominal"
         << setprecision(2) << endl;
    cout << left << setw(22) << "  Daya Pack" << ": " << total * arus << " W" << endl;
    if (arus < -I_IDLE) {
        cout << left << setw(22) << "  Estimasi Habis" << ": "
             << kapSisa / fabs(arus) * 60.0f << " menit" << endl;
    } else if (arus > I_IDLE) {
        cout << left << setw(22) << "  Estimasi Penuh" << ": "
             << (KAP_PACK_AH - kapSisa) / arus * 60.0f << " menit" << endl;
    }
    cout << garis << endl;
}

// ---------- Ringkasan (array 2D + array 1D arus) ----------
void ringkasan(float log[][JML_SEL], const float arusLog[], int jml) {
    cout << "\n=== RINGKASAN (" << jml << " sampel valid) ===" << endl;
    if (jml == 0) { cout << "Tidak ada data valid." << endl; return; }
    cout << fixed << setprecision(2);
    for (int s = 0; s < JML_SEL; s++) {
        float vMax = log[0][s], vMin = log[0][s], total = 0.0f;
        for (int k = 0; k < jml; k++) {                    // nested loop
            if (log[k][s] > vMax) vMax = log[k][s];
            if (log[k][s] < vMin) vMin = log[k][s];
            total += log[k][s];
        }
        cout << "Grup-" << s + 1 << "  Vmax=" << vMax << "  Vmin=" << vMin
             << "  Vrata2=" << total / jml << endl;
    }
    float iMax = arusLog[0], iMin = arusLog[0], iTotal = 0.0f;
    for (int k = 0; k < jml; k++) {
        if (arusLog[k] > iMax) iMax = arusLog[k];
        if (arusLog[k] < iMin) iMin = arusLog[k];
        iTotal += arusLog[k];
    }
    cout << "Arus   Imax(charge)=" << showpos << iMax << "  Imin(discharge)=" << iMin
         << noshowpos << "  Irata2=" << iTotal / jml << " A" << endl;
}

int main(int argc, char* argv[]) {
    int tundaMs = 1000;                               // jeda realtime (ms)
    if (argc > 1) tundaMs = atoi(argv[1]);

    cout << "=== SISTEM PEMANTAUAN BATERAI (BMS) Li-ion 6S4P ===" << endl;
    cout << "Konfigurasi : " << JML_SEL << "S" << JML_PARALEL << "P | Kapasitas "
         << KAP_PACK_AH << " Ah | Nominal " << JML_SEL * V_NOMINAL << " V | Penuh "
         << V_PACK_OVER << " V | Cut-off " << V_PACK_UNDER << " V" << endl;
    jalankanUnitTest();

    // Data simulasi frame serial (satu frame per detik)
    const int JML_FRAME = 7;
    string frame[JML_FRAME] = {
        "$BMS,V,3.85,3.87,3.86,3.84,3.85,3.88,-5.0#",    // normal, discharging
        "DATA_RUSAK",                                    // format salah
        "$BMS,V,3.70,-1.00,3.72,3.71,3.69,3.70,-4.0#",   // sensor negatif -> continue
        "$BMS,V,4.22,3.80,3.79,3.81,3.80,3.82,8.0#",     // over-voltage, charging
        "$BMS,V,3.20,3.18,2.95,3.22,3.19,3.21,-11.0#",   // under-voltage, arus warning
        "$BMS,V,3.30,3.28,3.31,3.29,3.30,3.32,-15.0#",   // arus trip delay
        "$BMS,V,3.60,3.61,3.59,3.60,3.62,3.60,-25.0#"    // hubung singkat -> exit(1)
    };

    float log[MAKS_SAMPEL][JML_SEL];
    float arusLog[MAKS_SAMPEL];
    int jmlLog = 0, invalidBerturut = 0;

    for (int t = 0; t < JML_FRAME; t++) {
        cout << "\n>> Detik ke-" << t + 1 << " | Frame: " << frame[t] << endl;
        float v[JML_SEL], arus = 0.0f;

        if (!parseFrame(frame[t], v, arus)) {
            invalidBerturut++;
            cout << "[ERROR] Format frame tidak valid!" << endl;
            if (invalidBerturut >= MAKS_INVALID) {
                cout << "Sensor/komunikasi bermasalah, periksa kalibrasi!" << endl;
                break;
            }
            continue;
        }
        if (!sensorValid(v, JML_SEL, arus)) {
            invalidBerturut++;
            cout << "[WARNING] Pembacaan sensor di luar rentang (diabaikan)." << endl;
            if (invalidBerturut >= MAKS_INVALID) {
                cout << "Sensor bermasalah, periksa kalibrasi!" << endl;
                break;
            }
            continue;
        }
        invalidBerturut = 0;

        for (int s = 0; s < JML_SEL; s++) log[jmlLog][s] = v[s];
        arusLog[jmlLog] = arus;
        jmlLog++;

        tampilkanDashboard(t + 1, v, arus);

        // Emergency shutdown: sel kritis, tegangan total kritis, atau hubung singkat
        bool kritis = (tentukanZonaPack(hitungTotal(v, JML_SEL)) == 4)
                   || (tentukanZonaArus(arus) == 4);
        for (int s = 0; s < JML_SEL; s++)
            if (tentukanZona(v[s]) == 4) kritis = true;

        if (kritis) {
            cout << "\n[CRITICAL] Kondisi kritis terdeteksi! EMERGENCY SHUTDOWN (exit)!" << endl;
            ringkasan(log, arusLog, jmlLog);
            exit(1);
        }
        this_thread::sleep_for(chrono::milliseconds(tundaMs));   // realtime
    }

    ringkasan(log, arusLog, jmlLog);
    simulasiBalancing(4.20f, 10.0f, 0.5f);
    return 0;
}
