/**
 * ╔══════════════════════════════════════════════════════════════╗
 * ║             TIC TAC TOE DISAPPEAR  –  main.cpp              ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 * TOPIK   : Tic Tac Toe Disappear (Variant permainan strategi)
 *
 * STRUKTUR DATA YANG DIGUNAKAN:
 * ─────────────────────────────
 *  1. Array 2D  (std::array<std::array<Cell,3>,3>)
 *       └─ Merepresentasikan papan 3x3.
 *          Akses sel O(1), memori statis, tidak perlu alokasi heap.
 *
 *  2. Queue  (std::deque sebagai queue FIFO)
 *       └─ Menyimpan urutan penempatan bidak tiap pemain.
 *          Bidak terlama ada di front(); ketika antrian penuh (≥3),
 *          front() di-pop dan bidaknya dihapus dari papan.
 *          Inilah inti mekanik "disappear".
 *
 *  3. Struct Pos
 *       └─ Menyimpan koordinat (baris, kolom) sebuah bidak.
 *
 * FITUR WAJIB:
 * ────────────
 *  ✔ Tambah Data  → addPiece()     meletakkan bidak baru ke papan
 *  ✔ Hapus Data   → removePiece()  menghapus bidak terlama otomatis
 *  ✔ Cari Data    → searchWin()    mencari tiga bidak sejajar (menang)
 *  ✔ Tampil Data  → FTXUI renderer menampilkan papan, antrian, status
 *
 * ATURAN PERMAINAN:
 * ─────────────────
 *  • Dua pemain (X dan O) bergantian menaruh bidak di papan 3x3.
 *  • Setiap pemain MAKSIMAL punya 3 bidak di papan sekaligus.
 *  • Saat meletakkan bidak ke-4, bidak PERTAMA (terlama) menghilang.
 *  • Pemain yang membuat 3 bidaknya sejajar (baris/kolom/diagonal) menang.
 *  • Tidak ada seri – permainan terus hingga ada pemenang.
 *
 * KONTROL:
 * ────────
 *  ↑↓←→   Gerak kursor          Enter   Letakkan bidak
 *  R       Restart permainan     Q       Keluar
 */

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <array>
#include <deque>
#include <optional>
#include <string>
#include <vector>

using namespace ftxui;

// ════════════════════════════════════════════════════════════════
//  Konstanta
// ════════════════════════════════════════════════════════════════
constexpr int BOARD_SIZE = 3;  ///< Ukuran papan (3×3)
constexpr int MAX_PIECES = 3;  ///< Maksimum bidak per pemain

// ════════════════════════════════════════════════════════════════
//  Tipe Dasar
// ════════════════════════════════════════════════════════════════
enum class Cell   { Empty, X, O };           ///< Isi satu sel papan
enum class Status { Playing, XWins, OWins }; ///< Status permainan

/// Posisi (baris, kolom) di papan
struct Pos {
    int r = 0, c = 0;
    bool operator==(const Pos& o) const noexcept { return r == o.r && c == o.c; }
};

// ════════════════════════════════════════════════════════════════
//  Kelas Game  –  logika permainan
// ════════════════════════════════════════════════════════════════
class Game {
public:
    // ── Struktur Data ─────────────────────────────────────────────
    /// Array 2D: papan permainan 3×3
    std::array<std::array<Cell, BOARD_SIZE>, BOARD_SIZE> board{};

    /// Queue FIFO: urutan bidak pemain X (depan = terlama)
    std::deque<Pos> q_x;
    /// Queue FIFO: urutan bidak pemain O (depan = terlama)
    std::deque<Pos> q_o;

    // ── State ──────────────────────────────────────────────────────
    bool   x_turn      = true;
    Status status      = Status::Playing;
    int    cursor_r    = 1;   ///< Posisi kursor (baris)
    int    cursor_c    = 1;   ///< Posisi kursor (kolom)
    int    total_move  = 0;   ///< Total langkah yang sudah dimainkan
    int    x_wins      = 0;   ///< Jumlah kemenangan X (antar sesi)
    int    o_wins      = 0;   ///< Jumlah kemenangan O (antar sesi)

    // ── Inisialisasi / Reset ───────────────────────────────────────
    void reset() {
        for (auto& row : board) row.fill(Cell::Empty);
        q_x.clear();
        q_o.clear();
        x_turn     = true;
        status     = Status::Playing;
        cursor_r   = 1;
        cursor_c   = 1;
        total_move = 0;
    }

    // ────────────────────────────────────────────────────────────────
    //  TAMBAH DATA
    //  addPiece() – menempatkan bidak baru pada posisi kursor.
    //  Jika antrian penuh, bidak terlama dihapus terlebih dahulu.
    //  Return: true jika berhasil, false jika posisi tidak valid.
    // ────────────────────────────────────────────────────────────────
    bool addPiece() {
        if (status != Status::Playing) return false;

        auto& q   = x_turn ? q_x : q_o;
        Cell  cur = x_turn ? Cell::X : Cell::O;
        Pos   target{cursor_r, cursor_c};

        // Tentukan bidak yang akan hilang (jika antrian sudah penuh)
        std::optional<Pos> dying;
        if (static_cast<int>(q.size()) >= MAX_PIECES) dying = q.front();

        // Validasi: sel target harus (akan) kosong
        //   → kosong sekarang, ATAU sama dengan bidak terlama yang akan dihapus
        bool will_be_empty =
            (board[target.r][target.c] == Cell::Empty) ||
            (dying.has_value() && *dying == target &&
             board[target.r][target.c] == cur);

        if (!will_be_empty) return false;  // sel diisi lawan atau tidak valid

        // HAPUS DATA: Hapus bidak terlama dari papan & keluarkan dari queue
        if (dying.has_value()) {
            removePiece(*dying);  // ← Fitur: Menghapus data
            q.pop_front();
        }

        // Tambahkan bidak baru ke papan dan masukkan ke belakang queue
        board[target.r][target.c] = cur;
        q.push_back(target);
        ++total_move;

        // CARI DATA: Periksa kondisi menang
        if (searchWin(cur)) {           // ← Fitur: Mencari data
            status = x_turn ? Status::XWins : Status::OWins;
            if (x_turn) ++x_wins; else ++o_wins;
        } else {
            x_turn = !x_turn;           // Ganti giliran
        }
        return true;
    }

    // ────────────────────────────────────────────────────────────────
    //  HAPUS DATA
    //  removePiece() – menghapus bidak dari posisi tertentu di papan.
    //  Dipanggil otomatis saat antrian penuh (mekanik "disappear").
    // ────────────────────────────────────────────────────────────────
    void removePiece(const Pos& p) {
        board[p.r][p.c] = Cell::Empty;
    }

    // ────────────────────────────────────────────────────────────────
    //  CARI DATA
    //  searchWin() – mencari apakah bidak c membentuk tiga sejajar.
    //  Memeriksa semua baris, kolom, dan dua diagonal.
    //  Return: true jika kondisi menang ditemukan.
    // ────────────────────────────────────────────────────────────────
    bool searchWin(Cell c) const {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            // Cek baris ke-i
            if (board[i][0] == c && board[i][1] == c && board[i][2] == c)
                return true;
            // Cek kolom ke-i
            if (board[0][i] == c && board[1][i] == c && board[2][i] == c)
                return true;
        }
        // Diagonal utama (↘)
        if (board[0][0] == c && board[1][1] == c && board[2][2] == c) return true;
        // Diagonal kedua (↙)
        if (board[0][2] == c && board[1][1] == c && board[2][0] == c) return true;
        return false;
    }

    // ── Helper ────────────────────────────────────────────────────
    /// Kembalikan bidak yang akan hilang pada giliran saat ini (jika ada)
    std::optional<Pos> dyingPiece() const {
        if (status != Status::Playing) return std::nullopt;
        const auto& q = x_turn ? q_x : q_o;
        if (static_cast<int>(q.size()) >= MAX_PIECES) return q.front();
        return std::nullopt;
    }

    std::string currentPlayerStr()  const { return x_turn ? "X" : "O"; }
    Color       currentPlayerColor() const { return x_turn ? Color::Cyan : Color::Magenta; }
};

// ════════════════════════════════════════════════════════════════
//  TAMPIL DATA  –  Helper rendering FTXUI
// ════════════════════════════════════════════════════════════════

/// Buat elemen visual untuk satu sel papan
static Element makeCellElem(Cell cell, bool is_cursor, bool is_dying) {
    std::string sym   = "   ";
    Color       scol  = Color::White;

    if (cell == Cell::X) { sym = " X "; scol = Color::Cyan;    }
    if (cell == Cell::O) { sym = " O "; scol = Color::Magenta; }

    Element inner = text(sym) | bold | color(scol);

    if (is_dying && is_cursor) {
        // Kursor tepat di bidak yang akan hilang → kuning pekat
        inner = text(sym) | bold | color(Color::Black) | bgcolor(Color::Yellow);
    } else if (is_dying) {
        // Bidak akan hilang → highlight kuning
        inner = text(sym) | bold | color(Color::Black) | bgcolor(Color::Yellow);
    } else if (is_cursor) {
        // Kursor biasa → biru gelap
        inner = text(sym) | bold | color(scol) | bgcolor(Color::Blue);
    }

    return inner | border;
}

/// Buat elemen visual antrian bidak
static Element makeQueueElem(const std::deque<Pos>& q,
                              const std::string&     label,
                              Color                  label_col)
{
    Elements elems;
    elems.push_back(text(label) | bold | color(label_col));

    if (q.empty()) {
        elems.push_back(text("(kosong)") | dim);
    } else {
        for (int i = 0; i < static_cast<int>(q.size()); ++i) {
            bool dying = (i == 0 && static_cast<int>(q.size()) >= MAX_PIECES);
            std::string lbl = "[" + std::to_string(q[i].r + 1) +
                              "," + std::to_string(q[i].c + 1) + "]";
            // Elemen terlama (akan hilang) = kuning, sisanya = warna pemain
            Color c = dying ? Color::Yellow : label_col;
            Element e = text(lbl) | bold | color(c);
            if (dying) e = e | underlined;  // garis bawah tanda "akan hilang"
            elems.push_back(e);
            if (i < static_cast<int>(q.size()) - 1)
                elems.push_back(text(" -> ") | dim);
        }
    }
    return hbox(elems);
}

// ════════════════════════════════════════════════════════════════
//  Main
// ════════════════════════════════════════════════════════════════
int main() {
    auto screen = ScreenInteractive::Fullscreen();
    Game game;
    game.reset();

    // ── Renderer: TAMPIL DATA ─────────────────────────────────────
    auto renderer = Renderer([&]() -> Element {
        auto dying = game.dyingPiece();

        // ──────────────────────────────────────────────────────────
        //  Papan 3×3
        // ──────────────────────────────────────────────────────────
        Elements board_rows;
        for (int r = 0; r < BOARD_SIZE; ++r) {
            Elements row_cells;
            for (int c = 0; c < BOARD_SIZE; ++c) {
                bool is_cur   = (r == game.cursor_r && c == game.cursor_c);
                bool is_dying = dying.has_value() &&
                                dying->r == r && dying->c == c;
                row_cells.push_back(makeCellElem(game.board[r][c], is_cur, is_dying));
            }
            board_rows.push_back(hbox(row_cells));
        }
        Element board_el = vbox(board_rows) | center;

        // ──────────────────────────────────────────────────────────
        //  Status Bar
        // ──────────────────────────────────────────────────────────
        Element status_el;
        if (game.status == Status::XWins) {
            status_el = text("  PEMAIN X MENANG!  Tekan [R] untuk bermain lagi  ")
                      | bold | color(Color::Black) | bgcolor(Color::Cyan);
        } else if (game.status == Status::OWins) {
            status_el = text("  PEMAIN O MENANG!  Tekan [R] untuk bermain lagi  ")
                      | bold | color(Color::Black) | bgcolor(Color::Magenta);
        } else {
            Elements st;
            st.push_back(text("Giliran: ") | bold);
            st.push_back(text("Pemain " + game.currentPlayerStr()) | bold
                        | color(game.currentPlayerColor()));
            if (dying.has_value()) {
                st.push_back(text("  [ ! Bidak terlama akan HILANG! ]")
                            | color(Color::Yellow) | bold);
            }
            status_el = hbox(st);
        }

        // ──────────────────────────────────────────────────────────
        //  Tampilan Antrian (Queue)
        // ──────────────────────────────────────────────────────────
        Element qx_el = makeQueueElem(game.q_x, "Antrian X: ", Color::Cyan);
        Element qo_el = makeQueueElem(game.q_o, "Antrian O: ", Color::Magenta);

        // ──────────────────────────────────────────────────────────
        //  Panel Kanan (Kontrol, Statistik, Legenda)
        // ──────────────────────────────────────────────────────────
        Element right_panel = vbox({
            vbox({
                text(" KONTROL ") | bold | center,
                separator(),
                text(" [Panah]  Gerak kursor  "),
                text(" [Enter]  Letakkan bidak "),
                text(" [R]      Restart        "),
                text(" [Q]      Keluar         "),
            }) | border,

            vbox({
                text(" STATISTIK ") | bold | center,
                separator(),
                text(" Langkah    : " + std::to_string(game.total_move)         ),
                text(" Bidak X    : " + std::to_string(game.q_x.size()) +
                     "/" + std::to_string(MAX_PIECES)                           ),
                text(" Bidak O    : " + std::to_string(game.q_o.size()) +
                     "/" + std::to_string(MAX_PIECES)                           ),
                separator(),
                text(" Menang X   : " + std::to_string(game.x_wins)            )
                    | color(Color::Cyan),
                text(" Menang O   : " + std::to_string(game.o_wins)            )
                    | color(Color::Magenta),
            }) | border,

            vbox({
                text(" LEGENDA ") | bold | center,
                separator(),
                hbox({text("[   ]") | bgcolor(Color::Blue),
                      text(" = Kursor         ")}),
                hbox({text("[XXX]") | bgcolor(Color::Yellow) | color(Color::Black),
                      text(" = Akan menghilang")}),
                separator(),
                text(" -> = urutan bidak (Queue)") | dim,
                text(" garis bawah = terlama   ") | dim,
            }) | border,
        });

        // ──────────────────────────────────────────────────────────
        //  Layout Utama
        // ──────────────────────────────────────────────────────────
        return vbox({
            // Judul
            text(" TIC TAC TOE DISAPPEAR ") | bold | center | border,
            hbox({
                // Kiri: papan + antrian + status
                vbox({
                    board_el,
                    separator(),
                    qx_el,
                    qo_el,
                    separator(),
                    status_el | center,
                }) | flex,
                // Kanan: panel info
                right_panel,
            }),
        });
    });

    // ── Event Handler ─────────────────────────────────────────────
    auto component = CatchEvent(renderer, [&](Event ev) -> bool {
        // Keluar
        if (ev == Event::Character('q') || ev == Event::Character('Q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        // Restart
        if (ev == Event::Character('r') || ev == Event::Character('R')) {
            game.reset();
            return true;
        }

        // Gerak kursor (wrap-around antar tepi papan)
        if (game.status == Status::Playing) {
            if (ev == Event::ArrowUp) {
                game.cursor_r = (game.cursor_r - 1 + BOARD_SIZE) % BOARD_SIZE;
                return true;
            }
            if (ev == Event::ArrowDown) {
                game.cursor_r = (game.cursor_r + 1) % BOARD_SIZE;
                return true;
            }
            if (ev == Event::ArrowLeft) {
                game.cursor_c = (game.cursor_c - 1 + BOARD_SIZE) % BOARD_SIZE;
                return true;
            }
            if (ev == Event::ArrowRight) {
                game.cursor_c = (game.cursor_c + 1) % BOARD_SIZE;
                return true;
            }
            // Letakkan bidak (input tidak valid = diabaikan)
            if (ev == Event::Return) {
                game.addPiece();  // return false = sel terisi, diabaikan
                return true;
            }
        }

        return false;  // Event lain diabaikan
    });

    screen.Loop(component);
    return 0;
}