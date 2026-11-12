# Sistem Pemantauan Kapasitas Baterai Li-ion 6S4P (BMS)

Proyek mata kuliah **Algoritma dan Pemrograman Komputer** (NTRO236017), Teknik Elektro, Universitas Negeri Malang.

Program menghitung sisa kapasitas baterai Li-ion **6S4P** (6 grup seri, 4 sel paralel, 2.500 mAh per sel, total 10 Ah) dari tegangan tiap grup.

## Demo web

https://USERNAMEKAMU.github.io/NAMA-REPOSITORY/

## Cara kerja singkat

1. Jumlahkan tegangan 6 grup (seri) untuk mendapat tegangan total.
2. Cari grup dengan tegangan terendah (grup terlemah).
3. Hitung SOC: `(V - 3,0) / (4,2 - 3,0) x 100%`.
4. Kapasitas tersisa = SOC x 10 Ah.
5. Tentukan status: Penuh, Baik, Sedang, Rendah, atau Habis.

## Isi repository

| File | Keterangan |
|---|---|
| `index.html` | Versi web (3 halaman: Start, Input, Hasil) |
| `bms_kapasitas_3halaman.cpp` | Versi C++ console 3 halaman |
| `bms_kapasitas.cpp` | Versi C++ console sederhana |
| `bms_monitor.cpp` | Versi C++ lengkap (tegangan, arus, dashboard) |

## Menjalankan versi C++

```bash
g++ bms_kapasitas_3halaman.cpp -o bms
./bms          # Windows: bms.exe
```

## Materi modul yang dipakai

| Modul | Materi |
|---|---|
| 3 | const, if-else, switch-case |
| 4 | for, while, do-while, break, continue, exit() |
| 5 | Array, Max-Min Search, function overloading |
| 6 | Dekomposisi fungsi, parsing string |
| 8 | assert() unit test, setw, setprecision, exp() |

## Catatan

Kapasitas dihitung dengan hubungan tegangan-SOC linear (3,0 V = 0%, 4,2 V = 100%). Ini perkiraan sederhana, kurva baterai sebenarnya tidak linear.
