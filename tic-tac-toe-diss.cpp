/**
 * Tic Tac Toe Disappear - main.cpp
 *
 * Variant permainan strategi tic tac toe dengan mekanik "disappear".
 * Dua pemain (X dan O) maksimal punya 3 bidak sekaligus di papan 3x3.
 * Bidak terlama hilang otomatis saat pemain meletakkan bidak ke-4.
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

// ===============================================
//  KONSTANTA
// ===============================================
constexpr int BOARD_SIZE = 3;  // papan 3x3
constexpr int MAX_PIECES = 3;  // maks 3 bidak per pemain

// ===============================================
//  TIPE DATA
// ===============================================
enum class Cell   { Empty, X, O };           // isi satu sel
enum class Status { Playing, XWins, OWins }; // status game

struct Pos { // koordinat (baris, kolom)
    int r = 0, c = 0;
    bool operator==(const Pos& o) const noexcept { return r == o.r && c == o.c; }
};

// ===============================================
//  GAME CLASS - logika permainan
// ===============================================
class Game {
public:
    // STRUKTUR DATA
    std::array<std::array<Cell, BOARD_SIZE>, BOARD_SIZE> board{};
    std::deque<Pos> q_x;  // queue bidak X (depan = terlama)
    std::deque<Pos> q_o;  // queue bidak O (depan = terlama)

    // STATE
    bool   x_turn      = true;
    Status status      = Status::Playing;
    int    cursor_r    = 1;   // posisi kursor row
    int    cursor_c    = 1;   // posisi kursor col
    int    total_move  = 0;   // total langkah
    int    x_wins      = 0;   // kemenangan X
    int    o_wins      = 0;   // kemenangan O

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

    // === FITUR 1: TAMBAH DATA ===
    // Letakkan bidak baru di posisi kursor.
    // Jika antrian penuh (>=3), otomatis hapus bidak terlama dulu.
    bool addPiece() {
        if (status != Status::Playing) return false;

        auto& q   = x_turn ? q_x : q_o;
        Cell  cur = x_turn ? Cell::X : Cell::O;
        Pos   target{cursor_r, cursor_c};

        // cek apakah bidak terlama akan dihapus
        std::optional<Pos> dying;
        if (static_cast<int>(q.size()) >= MAX_PIECES) dying = q.front();

        // validasi: sel target harus kosong atau sama dgn bidak terlama
        bool will_be_empty =
            (board[target.r][target.c] == Cell::Empty) ||
            (dying.has_value() && *dying == target &&
             board[target.r][target.c] == cur);

        if (!will_be_empty) return false;

        // === FITUR 2: HAPUS DATA ===
        if (dying.has_value()) {
            removePiece(*dying);
            q.pop_front();
        }

        // tambah bidak baru
        board[target.r][target.c] = cur;
        q.push_back(target);
        ++total_move;

        // === FITUR 3: CARI DATA ===
        if (searchWin(cur)) {
            status = x_turn ? Status::XWins : Status::OWins;
            if (x_turn) ++x_wins; else ++o_wins;
        } else {
            x_turn = !x_turn;
        }
        return true;
    }

    // === FITUR 2: HAPUS DATA ===
    // Hapus bidak dari papan di posisi tertentu.
    void removePiece(const Pos& p) {
        board[p.r][p.c] = Cell::Empty;
    }

    // === FITUR 3: CARI DATA ===
    // Cek apakah ada 3 bidak sejajar (baris/kolom/diagonal).
    bool searchWin(Cell c) const {
        // cek semua baris
        for (int i = 0; i < BOARD_SIZE; ++i) {
            if (board[i][0] == c && board[i][1] == c && board[i][2] == c)
                return true;
            // cek semua kolom
            if (board[0][i] == c && board[1][i] == c && board[2][i] == c)
                return true;
        }
        // diagonal utama \
        if (board[0][0] == c && board[1][1] == c && board[2][2] == c) return true;
        // diagonal kedua /
        if (board[0][2] == c && board[1][1] == c && board[2][0] == c) return true;
        return false;
    }

    // HELPER
    std::optional<Pos> dyingPiece() const {
        if (status != Status::Playing) return std::nullopt;
        const auto& q = x_turn ? q_x : q_o;
        if (static_cast<int>(q.size()) >= MAX_PIECES) return q.front();
        return std::nullopt;
    }

    std::string currentPlayerStr()  const { return x_turn ? "X" : "O"; }
    Color       currentPlayerColor() const { return x_turn ? Color::Cyan : Color::Magenta; }
};

// ===============================================
//  === FITUR 4: TAMPIL DATA ===
//  Helper rendering FTXUI
// ===============================================

// buat visual untuk satu sel
static Element makeCellElem(Cell cell, bool is_cursor, bool is_dying) {
    std::string sym   = "   ";
    Color       scol  = Color::White;

    if (cell == Cell::X) { sym = " X "; scol = Color::Cyan;    }
    if (cell == Cell::O) { sym = " O "; scol = Color::Magenta; }

    Element inner = text(sym) | bold | color(scol);

    if (is_dying && is_cursor) {
        inner = text(sym) | bold | color(Color::Black) | bgcolor(Color::Yellow);
    } else if (is_dying) {
        inner = text(sym) | bold | color(Color::Black) | bgcolor(Color::Yellow);
    } else if (is_cursor) {
        inner = text(sym) | bold | color(scol) | bgcolor(Color::Blue);
    }

    return inner | border;
}

// buat visual antrian bidak
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
            Color c = dying ? Color::Yellow : label_col;
            Element e = text(lbl) | bold | color(c);
            if (dying) e = e | underlined;
            elems.push_back(e);
            if (i < static_cast<int>(q.size()) - 1)
                elems.push_back(text(" -> ") | dim);
        }
    }
    return hbox(elems);
}

// ===============================================
//  MAIN
// ===============================================
int main() {
    auto screen = ScreenInteractive::Fullscreen();
    Game game;
    game.reset();

    // RENDERER - tampilkan game state
    auto renderer = Renderer([&]() -> Element {
        auto dying = game.dyingPiece();

        // Papan 3x3
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

        // Status bar
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

        // Queue display
        Element qx_el = makeQueueElem(game.q_x, "Antrian X: ", Color::Cyan);
        Element qo_el = makeQueueElem(game.q_o, "Antrian O: ", Color::Magenta);

        // Right panel: kontrol, statistik, legenda
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

        // Layout utama
        return vbox({
            text(" TIC TAC TOE DISAPPEAR ") | bold | center | border,
            hbox({
                vbox({
                    board_el,
                    separator(),
                    qx_el,
                    qo_el,
                    separator(),
                    status_el | center,
                }) | flex,
                right_panel,
            }),
        });
    });

    // EVENT HANDLER
    auto component = CatchEvent(renderer, [&](Event ev) -> bool {
        if (ev == Event::Character('q') || ev == Event::Character('Q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        if (ev == Event::Character('r') || ev == Event::Character('R')) {
            game.reset();
            return true;
        }

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
            if (ev == Event::Return) {
                game.addPiece();
                return true;
            }
        }

        return false;
    });

    screen.Loop(component);
    return 0;
}
