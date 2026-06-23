# Dokumentasi Teknis - TIC TAC TOE DISAPPEAR

Dokumentasi lengkap untuk developer yang ingin paham atau modify kode ini.

## Daftar Isi
1. [Overview](#overview)
2. [Struktur Data](#struktur-data)
3. [Class Game](#class-game)
4. [Fitur Wajib](#fitur-wajib)
5. [Rendering & UI](#rendering--ui)
6. [Event Handling](#event-handling)
7. [Algoritma Penting](#algoritma-penting)

## Overview

**Tujuan Program**: 
Game tic tac toe dengan variant "disappear" dimana bidak otomatis hilang saat pemain udah punya 3 bidak dan meletakkan bidak ke-4.

**Main Components**:
- **Game Logic**: `class Game` - handle state, move validation, win detection
- **Rendering**: FTXUI - render UI di terminal
- **Event Loop**: Handle input keyboard dan update game state
- **Data Structures**: Array 2D + Queue FIFO

## Struktur Data

### 1. Papan (2D Array)
```cpp
std::array<std::array<Cell, 3>, 3> board{};
```

- Ukuran: 3x3 (9 sel total)
- Setiap sel bisa: `Empty`, `X`, atau `O`
- Access time: O(1)
- Memory: Statis (stack), gak perlu alokasi heap

**Layout di memori**:
```
board[0][0] board[0][1] board[0][2]
board[1][0] board[1][1] board[1][2]
board[2][0] board[2][1] board[2][2]
```

### 2. Queue Antrian (FIFO)
```cpp
std::deque<Pos> q_x;  // antrian bidak pemain X
std::deque<Pos> q_o;  // antrian bidak pemain O
```

- Struktur: Double-ended queue
- Operasi: `push_back()`, `pop_front()`, `front()`, `size()`
- Purpose: Track urutan penempatan bidak
- Depan queue = bidak terlama (akan hilang duluan)
- Belakang queue = bidak terbaru (baru di-letakkan)

**Contoh urutan**:
```
q_x sebelum letakkan bidak ke-4:
[1,1] [2,2] [3,3]  <- front() = terlama
 ^    ^     ^
 1st  2nd   3rd

User letakkan bidak ke-4 di [1,2]:
- pop_front() -> [1,1] dihapus dari papan
- push_back() -> [1,2] ditambah ke papan
- Hasilnya:
[2,2] [3,3] [1,2]  <- front() = terlama sekarang
```

### 3. Posisi (Struct)
```cpp
struct Pos {
    int r = 0, c = 0;  // row, column (0-2)
    bool operator==(const Pos& o) const {
        return r == o.r && c == o.c;
    }
};
```

- Menyimpan koordinat (baris, kolom)
- Support operator `==` untuk compare dua posisi
- Default value: (0, 0)

### 4. Enum Status & Cell
```cpp
enum class Cell   { Empty, X, O };
enum class Status { Playing, XWins, OWins };
```

- **Cell**: Status satu sel di papan
- **Status**: Status keseluruhan game

## Class Game

Container untuk semua game logic dan state.

### Member Variables

#### Data Structure
```cpp
std::array<std::array<Cell, 3>, 3> board{};
std::deque<Pos> q_x, q_o;
```

#### Game State
```cpp
bool x_turn      = true;    // siapa giliran skrg
Status status    = Status::Playing;
int cursor_r     = 1;       // row kursor (0-2)
int cursor_c     = 1;       // col kursor (0-2)
int total_move   = 0;       // total langkah sejak restart
int x_wins       = 0;       // kemenangan X (cumulative)
int o_wins       = 0;       // kemenangan O (cumulative)
```

### Member Functions

#### `void reset()`
Reset semua state untuk permainan baru.

```cpp
void reset() {
    for (auto& row : board) row.fill(Cell::Empty);
    q_x.clear();
    q_o.clear();
    x_turn     = true;
    status     = Status::Playing;
    cursor_r   = 1;
    cursor_c   = 1;
    total_move = 0;
    // NOTE: x_wins dan o_wins TIDAK di-reset!
}
```

**Timeouts**: O(1)

#### `std::optional<Pos> dyingPiece()`
Return posisi bidak yang akan hilang di turn sekarang.

```cpp
std::optional<Pos> dyingPiece() const {
    if (status != Status::Playing) return std::nullopt;
    const auto& q = x_turn ? q_x : q_o;
    if (static_cast<int>(q.size()) >= MAX_PIECES) {
        return q.front();  // bidak terlama
    }
    return std::nullopt;  // belum ada yang hilang
}
```

**Return**: 
- `std::nullopt` jika tidak ada bidak yang akan hilang
- `q.front()` jika antrian sudah full (>=3)

**Gunakan untuk**: Highlight visual bidak yang akan hilang di UI.

#### Helper Functions
```cpp
std::string currentPlayerStr()  const;
Color currentPlayerColor() const;
```

Return nama dan warna pemain yang lagi turn.

## Fitur Wajib

### 1. TAMBAH DATA - `bool addPiece()`

Letakkan bidak baru di posisi kursor. Jika antrian penuh, bidak terlama otomatis dihapus.

**Pseudocode**:
```
1. Check apakah game masih Playing
2. Ambil queue pemain sekarang (q_x atau q_o)
3. Tentukan bidak mana yang akan hilang (jika ada)
4. Validasi sel target:
   - Kosong, ATAU
   - Sama dengan bidak terlama yang akan dihapus
5. Jika validasi gagal: return false
6. HAPUS bidak terlama (jika ada)
7. TAMBAH bidak baru ke papan & push ke belakang queue
8. Increment total_move
9. CARI kondisi menang
10. Jika menang: update status dan increment win counter
11. Jika belum menang: switch pemain (x_turn = !x_turn)
12. return true
```

**Return Values**:
- `true` - Berhasil letakkan bidak
- `false` - Sel tidak valid atau game sudah selesai

**Contoh Eksekusi**:
```
Turn 1 (X): Letakkan di [1,1]
  - board[1][1] = X
  - q_x = [[1,1]]
  - total_move = 1

Turn 2 (O): Letakkan di [0,0]
  - board[0][0] = O
  - q_o = [[0,0]]
  - total_move = 2

Turn 3 (X): Letakkan di [2,2]
  - board[2][2] = X
  - q_x = [[1,1], [2,2]]
  - total_move = 3

Turn 4 (O): Letakkan di [1,1] (tempat X lama)
  - dying = q_x.front() = [1,1]
  - Validasi OK (target kosong = bidak X akan dihapus)
  - removePiece([1,1]) -> board[1][1] = Empty
  - q_x.pop_front() -> q_x = [[2,2]]
  - board[1,1] = O
  - q_o.push_back([1,1]) -> q_o = [[0,0], [1,1]]
  - total_move = 4
  - searchWin(O) -> false
  - x_turn = true
```

### 2. HAPUS DATA - `void removePiece(const Pos& p)`

Hapus bidak dari papan di posisi tertentu.

**Code**:
```cpp
void removePiece(const Pos& p) {
    board[p.r][p.c] = Cell::Empty;
}
```

**Timeouts**: O(1)

**Catatan**: 
- Dipanggil otomatis dari `addPiece()` saat mekanisme disappear
- Bisa dipanggil dari fitur lain jika perlu (misalnya undo)

### 3. CARI DATA - `bool searchWin(Cell c)`

Cek apakah pemain dengan bidak `c` (X atau O) menang.

**Kondisi Menang**: 3 bidak sejajar (baris, kolom, atau diagonal)

**Logic**:
```
Cek 8 kondisi:
- Baris 0: [0][0] == [0][1] == [0][2]
- Baris 1: [1][0] == [1][1] == [1][2]
- Baris 2: [2][0] == [2][1] == [2][2]
- Kolom 0: [0][0] == [1][0] == [2][0]
- Kolom 1: [0][1] == [1][1] == [2][1]
- Kolom 2: [0][2] == [1][2] == [2][2]
- Diagonal 1: [0][0] == [1][1] == [2][2]
- Diagonal 2: [0][2] == [1][1] == [2][0]
```

**Implementation**:
```cpp
bool searchWin(Cell c) const {
    // Cek semua baris dan kolom
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] == c && board[i][1] == c && board[i][2] == c)
            return true;
        if (board[0][i] == c && board[1][i] == c && board[2][i] == c)
            return true;
    }
    // Cek diagonal 1 (\)
    if (board[0][0] == c && board[1][1] == c && board[2][2] == c)
        return true;
    // Cek diagonal 2 (/)
    if (board[0][2] == c && board[1][1] == c && board[2][0] == c)
        return true;
    return false;
}
```

**Timeouts**: O(1) - fixed 8 checks

**Return**:
- `true` - Pemain menang
- `false` - Belum menang

### 4. TAMPIL DATA - Renderer & UI

Bukan fitur query, tapi display semua data yang ada.

**Components**:
1. **Papan 3x3** - Visual grid dengan border
2. **Antrian** - Display q_x dan q_o dengan urutan bidak
3. **Status** - Siapa giliran, peringatan bidak akan hilang, atau winner
4. **Panel Kanan** - Kontrol, statistik, legenda
5. **Title** - Judul game

**Helper Functions**:
```cpp
Element makeCellElem(Cell cell, bool is_cursor, bool is_dying);
Element makeQueueElem(const std::deque<Pos>& q, 
                      const std::string& label, 
                      Color label_col);
```

## Rendering & UI

### FTXUI Framework

FTXUI = Full Terminal eXtendable User Interface.

**Include**:
```cpp
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
```

### Main Components

#### 1. Screen
```cpp
auto screen = ScreenInteractive::Fullscreen();
```
Setup layar fullscreen terminal.

#### 2. Renderer
```cpp
auto renderer = Renderer([&]() -> Element { ... });
```
Function yang di-call setiap frame untuk render ulang UI.

**Return**: `Element` (root element dari UI tree)

#### 3. Event Catcher
```cpp
auto component = CatchEvent(renderer, [&](Event ev) -> bool { ... });
```
Catch keyboard events dan handle input.

**Return**: 
- `true` - Event udah handled, jangan propagate
- `false` - Event gak ditangani

#### 4. Loop
```cpp
screen.Loop(component);
```
Start event loop. Terus render dan accept input sampai `screen.ExitLoopClosure()` di-call.

### UI Structure (Hierarchy)

```
vbox (vertical box) {
    Title
    hbox (horizontal box) {
        vbox (left) {
            board_el (papan)
            separator
            qx_el (antrian X)
            qo_el (antrian O)
            separator
            status_el (status)
        }
        right_panel (vbox) {
            kontrol box
            statistik box
            legenda box
        }
    }
}
```

### Color & Styling

```cpp
Color::Cyan
Color::Magenta
Color::Yellow
Color::Blue
Color::White
Color::Black

| bold        // text tebal
| dim         // text abu-abu
| underlined  // garis bawah
| center      // center align
| color(c)    // warna text
| bgcolor(c)  // warna background
| border      // tambah border
| flex        // fill available space
```

## Event Handling

### Input Keys

```cpp
if (ev == Event::Character('q')) { /* handle Q */ }
if (ev == Event::Character('r')) { /* handle R */ }
if (ev == Event::ArrowUp)        { /* handle UP */ }
if (ev == Event::ArrowDown)      { /* handle DOWN */ }
if (ev == Event::ArrowLeft)      { /* handle LEFT */ }
if (ev == Event::ArrowRight)     { /* handle RIGHT */ }
if (ev == Event::Return)         { /* handle ENTER */ }
```

### Event Loop Logic

```cpp
CatchEvent(renderer, [&](Event ev) -> bool {
    // 1. Keluar game
    if (ev == 'Q' || ev == 'q') {
        screen.ExitLoopClosure()();
        return true;
    }
    
    // 2. Restart game
    if (ev == 'R' || ev == 'r') {
        game.reset();
        return true;
    }
    
    // 3. Handle gerakan & placing (hanya saat Playing)
    if (game.status == Status::Playing) {
        if (ev == ArrowUp) {
            game.cursor_r = (game.cursor_r - 1 + 3) % 3;
            return true;
        }
        // ... (similar untuk Down, Left, Right)
        
        if (ev == Return) {
            game.addPiece();
            return true;
        }
    }
    
    return false;  // event gak ditangani
});
```

## Algoritma Penting

### 1. Modulo untuk Wrap-Around

Kursor gerak melingkar di papan:

```cpp
// ArrowUp: pindah ke baris atas (wrap ke bawah jika di baris 0)
cursor_r = (cursor_r - 1 + BOARD_SIZE) % BOARD_SIZE;

// ArrowDown: pindah ke baris bawah (wrap ke atas jika di baris 2)
cursor_r = (cursor_r + 1) % BOARD_SIZE;

// Contoh:
// cursor_r = 0, tekan UP
// (0 - 1 + 3) % 3 = 2 % 3 = 2 (wraps to bottom row)

// cursor_r = 2, tekan DOWN
// (2 + 1) % 3 = 3 % 3 = 0 (wraps to top row)
```

### 2. FIFO Queue untuk Disappear Mechanic

Bidak terlama (first in) adalah yang pertama hilang (first out):

```cpp
// Saat queue penuh dan user letakkan bidak baru:
std::optional<Pos> dying = std::nullopt;
if (q.size() >= MAX_PIECES) {
    dying = q.front();  // bidak terlama
}

// ... validasi ...

if (dying.has_value()) {
    removePiece(*dying);  // hapus dari papan
    q.pop_front();        // keluarkan dari queue
}

q.push_back(new_piece);   // tambah bidak baru di belakang
```

**Time Complexity**: O(1) untuk `front()`, `pop_front()`, `push_back()`

### 3. Win Detection (Exhaustive Check)

Cek semua 8 kemungkinan menang:

```cpp
bool searchWin(Cell c) const {
    // Row check (3)
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] == c && board[i][1] == c && board[i][2] == c)
            return true;
    }
    // Col check (3)
    for (int i = 0; i < 3; ++i) {
        if (board[0][i] == c && board[1][i] == c && board[2][i] == c)
            return true;
    }
    // Diagonal check (2)
    if (board[0][0] == c && board[1][1] == c && board[2][2] == c) return true;
    if (board[0][2] == c && board[1][1] == c && board[2][0] == c) return true;
    return false;
}
```

**Time**: O(1) - fixed 8 checks
**Why**: Papan kecil (3x3), jadi bisa hardcode semua check

### 4. Board Indexing

```
  0   1   2  (column)
0 [.][.][.]
1 [.][.][.]
2 [.][.][.]
(row)
```

Access: `board[row][col]`

**Zero-indexed**: 0 sampai 2

Untuk UI display yang friendly (1-indexed):
```cpp
int display_row = row + 1;  // 1-3
int display_col = col + 1;  // 1-3
```

## Flow Diagram

### Game Flow
```
START
  |
  v
Reset Game (clear board, antrian, state)
  |
  v
LOOP:
  |-> Render UI (TAMPIL DATA)
  |-> Wait for input
  |-> Handle input (TAMBAH, HAPUS, CARI DATA)
  |-> Update state
  |-> Check if game over (status == XWins or OWins)
  |-> Continue loop atau EXIT
  |
  v
END
```

### addPiece() Execution Flow
```
addPiece()
  |
  +-> Check game.status != Playing? -> return false
  |
  +-> Get current player queue (q_x or q_o)
  |
  +-> Determine dying piece (jika queue penuh)
  |
  +-> Validate target cell
  |   - Empty, atau
  |   - Same as dying piece & same player
  |   - Jika invalid -> return false
  |
  +-> Remove dying piece
  |   +-> removePiece()
  |   +-> q.pop_front()
  |
  +-> Add new piece
  |   +-> board[r][c] = player cell
  |   +-> q.push_back(new_pos)
  |   +-> total_move++
  |
  +-> Check win condition
  |   +-> searchWin(cur_cell)?
  |   |   - YES: status = XWins/OWins, increment win counter
  |   |   - NO: switch player (x_turn = !x_turn)
  |
  +-> return true
```

## Tips untuk Development

### Extend Features

1. **Undo Move**:
   - Store move history: `std::vector<GameState> history`
   - Pop dari history dan restore state

2. **AI Opponent**:
   - Implement minimax atau simple heuristic
   - Add difficulty level selector

3. **Persistent Stats**:
   - Save win counter ke file
   - Load saat startup

4. **Animated Disappear**:
   - Add frame counter
   - Animate cell fade-out sebelum hapus

5. **Custom Board Size**:
   - Template `class Game<int SIZE>`
   - Support 4x4, 5x5, dll

### Testing

1. **Unit Test Scenarios**:
   - Test `searchWin()` dengan semua 8 kondisi
   - Test `addPiece()` dengan invalid moves
   - Test queue behavior saat full

2. **Integration Test**:
   - Full game play-through
   - Verify win detection
   - Verify score persistence

---

**Happy coding! Semoga dokumentasi ini helpful buat extend atau maintain kode ini!** :)
