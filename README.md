# TIC TAC TOE DISAPPEAR

Game strategi seru dengan twist menarik! Nih main tic tac toe tapi dengan mekanik **bidak yang hilang**. Jadi gak bisa hanya duduk diam, harus terus gerak dan strategi dong.

```
     PAPAN 3X3
   1    2    3
1 [ X ][ O ][   ]
2 [   ][ X ][   ]
3 [ O ][   ][ X ]
```

## Main Gimana Sih?

**Aturan Dasar:**
- 2 pemain: **X** (Cyan) dan **O** (Magenta)
- Papan: 3x3 (total 9 kotak)
- Setiap pemain MAKSIMAL punya 3 bidak di papan sekaligus
- Bidak ke-4 yang di-letakkan? Bidak pertama (terlama) **HILANG OTOMATIS** (ini yang seru!)
- Tujuan: Susun 3 bidakmu sejajar (baris, kolom, atau diagonal) = **MENANG**

**Contoh Gameplay:**
1. Pemain X letakkan bidak di [1,1]
2. Pemain O letakkan bidak di [1,2]
3. Pemain X letakkan bidak di [2,2]
4. Pemain O letakkan bidak di [3,3]
5. Pemain X letakkan bidak ke-4 di [1,3]
   - Bidak X pertama (di [1,1]) **HILANG!**
6. Permainan berlanjut...

## Kontrol

| Tombol | Fungsi |
|--------|--------|
| **↑↓←→** | Gerak kursor ke atas/bawah/kiri/kanan |
| **Enter** | Letakkan bidak di posisi kursor |
| **R** | Restart permainan (sesi baru) |
| **Q** | Keluar dari game |

## Fitur yang Diimplemen

1. **TAMBAH DATA** (addPiece)
   - Letakkan bidak baru di papan
   - Otomatis hapus bidak terlama jika sudah 3

2. **HAPUS DATA** (removePiece)
   - Hapus bidak dari koordinat tertentu
   - Dipanggil otomatis saat mekanisme disappear

3. **CARI DATA** (searchWin)
   - Cek 8 kemungkinan menang (3 baris + 3 kolom + 2 diagonal)
   - Instant win detection

4. **TAMPIL DATA** (Renderer FTXUI)
   - Papan interaktif dengan warna-warni
   - Antrian bidak real-time
   - Status game & statistik kemenangan

## Visual & Warna

```
[   ] = Kursor (biru)
[XXX] = Bidak akan hilang (kuning)
[ X ] = Pemain X (cyan)
[ O ] = Pemain O (magenta)
```

Antrian bidak ditampilkan di bawah papan dengan format:
```
Antrian X: [1,1] -> [2,2] -> [3,3]_
Antrian O: [1,2]
```

Garis bawah (_) menunjukkan bidak yang **paling tua dan akan hilang duluan**.

## Build & Jalankan

### Prerequisites
- C++17 compiler (g++, clang, MSVC)
- CMake >= 3.14

### Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Jalankan
```bash
./tictac
```
atau di Windows:
```cmd
.\tictac.exe
```

## Tech Stack

- **Language**: C++17
- **UI Framework**: FTXUI (Full Terminal eXtendable User Interface)
- **Data Structure**: 
  - `std::array<std::array<Cell,3>,3>` untuk papan
  - `std::deque<Pos>` untuk antrian bidak (FIFO)
- **Build System**: CMake

## Struktur Kode

```
tic-tac-toe-diss.cpp
├── Constants (BOARD_SIZE, MAX_PIECES)
├── Type Definitions (Cell, Status, Pos)
├── Game Class
│   ├── Data Members (board, q_x, q_o)
│   ├── Fitur 1: addPiece() - TAMBAH DATA
│   ├── Fitur 2: removePiece() - HAPUS DATA
│   └── Fitur 3: searchWin() - CARI DATA
├── Rendering Functions
│   ├── makeCellElem() - visual sel
│   └── makeQueueElem() - visual antrian
└── main()
    ├── Screen setup
    ├── Fitur 4: Renderer - TAMPIL DATA
    └── Event handler
```

## Algoritma Kunci

### Queue (FIFO) untuk Disappear Mechanic
```cpp
std::deque<Pos> q_x, q_o;  // depan = terlama, belakang = terbaru

if (q.size() >= 3) {
    Pos dying = q.front();      // bidak terlama
    removePiece(dying);         // hapus dari papan
    q.pop_front();              // keluarkan dari antrian
}
q.push_back(new_piece);         // tambah di belakang
```

### Win Detection (8 kondisi)
```cpp
// 3 baris
// 3 kolom
// 2 diagonal
bool searchWin(Cell c) {
    for (int i = 0; i < 3; ++i) {
        if (row[i] == c == c == c) return true;
        if (col[i] == c == c == c) return true;
    }
    if (diag1[0,0] == c && diag1[1,1] == c && diag1[2,2] == c) return true;
    if (diag2[0,2] == c && diag2[1,1] == c && diag2[2,0] == c) return true;
}
```

## Tips Bermain

1. **Monitor antrian** - liat bidak mana yang mau hilang
2. **Strategi dua arah** - bentuk 3 sejajar sambil potong strategi lawan
3. **Posisi strategis** - tengah (1,1) penting karena masuk 4 kemungkinan menang
4. **Timing** - jangan asal letakkan, pikirkan bidak mana yang mau hilang

## File yang Ada

- `tic-tac-toe-diss.cpp` - Source code utama
- `CMakeLists.txt` - Build configuration
- `README.md` - File ini
- `DOC.md` - Dokumentasi teknis detail

---

**Selamat bermain! Semoga bisa ngalahin lawanmu. Good luck! ;)**
