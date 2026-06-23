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

// Explicit using untuk menghindari ambiguity dengan ftxui::Cell
using ftxui::ScreenInteractive;
using ftxui::Renderer;
using ftxui::CatchEvent;
using ftxui::Elements;
using ftxui::Element;
using ftxui::Event;
using ftxui::Color;
using ftxui::text;
using ftxui::bold;
using ftxui::color;
using ftxui::bgcolor;
using ftxui::border;
using ftxui::center;
using ftxui::dim;
using ftxui::underlined;
using ftxui::separator;
using ftxui::hbox;
using ftxui::vbox;
using ftxui::flex;

// ===============================================
//  KONSTANTA
// ===============================================
constexpr int BOARD_SIZE = 3;
constexpr int MAX_PIECES = 3;

// ===============================================
//  TIPE DATA
// ===============================================
enum class CellState { Empty, X, O };
enum class GameStatus { Playing, XWins, OWins };

struct Pos {
    int r = 0, c = 0;
    bool operator==(const Pos& o) const noexcept {
        return r == o.r && c == o.c;
    }
};

// ===============================================
//  GAME CLASS
// ===============================================
class Game {
public:
    // STRUKTUR DATA
    std::array<std::array<CellState, BOARD_SIZE>, BOARD_SIZE> board{};
    std::deque<Pos> q_x;
    std::deque<Pos> q_o;

    // STATE
    bool   x_turn      = true;
    GameStatus status  = GameStatus::Playing;
    int    cursor_r    = 1;
    int    cursor_c    = 1;
    int    total_move  = 0;
    int    x_wins      = 0;
    int    o_wins      = 0;

    void reset() {
        for (auto& row : board) row.fill(CellState::Empty);
        q_x.clear();
        q_o.clear();
        x_turn     = true;
        status     = GameStatus::Playing;
        cursor_r   = 1;
        cursor_c   = 1;
        total_move = 0;
    }

    // === FITUR 1: TAMBAH DATA ===
    bool addPiece() {
        if (status != GameStatus::Playing) return false;

        auto& q   = x_turn ? q_x : q_o;
        CellState cur = x_turn ? CellState::X : CellState::O;
        Pos target{cursor_r, cursor_c};

        std::optional<Pos> dying;
        if (static_cast<int>(q.size()) >= MAX_PIECES) dying = q.front();

        bool will_be_empty =
            (board[target.r][target.c] == CellState::Empty) ||
            (dying.has_value() && *dying == target &&
             board[target.r][target.c] == cur);

        if (!will_be_empty) return false;

        // === FITUR 2: HAPUS DATA ===
        if (dying.has_value()) {
            removePiece(*dying);
            q.pop_front();
        }

        board[target.r][target.c] = cur;
        q.push_back(target);
        ++total_move;

        // === FITUR 3: CARI DATA ===
        if (searchWin(cur)) {
            status = x_turn ? GameStatus::XWins : GameStatus::OWins;
            if (x_turn) ++x_wins; else ++o_wins;
        } else {
            x_turn = !x_turn;
        }
        return true;
    }

    // === FITUR 2: HAPUS DATA ===
    void removePiece(const Pos& p) {
        board[p.r][p.c] = CellState::Empty;
    }

    // === FITUR 3: CARI DATA ===
    bool searchWin(CellState c) const {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            if (board[i][0] == c && board[i][1] == c && board[i][2] == c)
                return true;
            if (board[0][i] == c && board[1][i] == c && board[2][i] == c)
                return true;
        }
        if (board[0][0] == c && board[1][1] == c && board[2][2] == c) return true;
        if (board[0][2] == c && board[1][1] == c && board[2][0] == c) return true;
        return false;
    }

    std::optional<Pos> dyingPiece() const {
        if (status != GameStatus::Playing) return std::nullopt;
        const auto& q = x_turn ? q_x : q_o;
        if (static_cast<int>(q.size()) >= MAX_PIECES) return q.front();
        return std::nullopt;
    }

    std::string currentPlayerStr()  const {
        return x_turn ? "X" : "O";
    }

    Color currentPlayerColor() const {
        return x_turn ? Color::Cyan : Color::Magenta;
    }
};

// ===============================================
//  === FITUR 4: TAMPIL DATA ===
//  Helper rendering FTXUI
// ===============================================

static Element makeCellElem(CellState cell, bool is_cursor, bool is_dying) {
    std::string sym   = "   ";
    Color       scol  = Color::White;

    if (cell == CellState::X) { sym = " X "; scol = Color::Cyan;    }
    if (cell == CellState::O) { sym = " O "; scol = Color::Magenta; }

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
        if (game.status == GameStatus::XWins) {
            status_el = text("  PEMAIN X MENANG!  Tekan [R] untuk bermain lagi  ")
                      | bold | color(Color::Black) | bgcolor(Color::Cyan);
        } else if (game.status == GameStatus::OWins) {
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

        // Right panel
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

        if (game.status == GameStatus::Playing) {
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
