#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    print_hand(cout);
    // keep asking for player's move until the move is valid
    while (true) {
        string move;
        cout << "Your move, " << get_name() << ": ";
        cin >> move;
        move = to_upper(move);

        if (move == "PASS") {
            return Move();
        }
        if (move == "EXCHANGE") {
            cin >> move;
            std::vector<TileKind> movetiles;
            // make sure the player actually has the tiles they want to place
            try {
                movetiles = parse_tiles(move);
            } catch (MoveException& e) {
                cerr << e.what() << endl;
                continue;
            }
            return Move(movetiles);
        }
        if (move == "PLACE") {
            char dir;
            size_t row, col;
            string word;
            cin >> dir >> row >> col;
            cin >> word;
            row--;
            col--;
            // make sure the player actually has the tiles they want to place
            std::vector<TileKind> movetiles;
            try {
                movetiles = parse_tiles(word);
            } catch (MoveException& e) {
                cerr << e.what() << endl;
                continue;
            }
            bool error = false;
            if (dir == '|') {
                PlaceResult result = board.test_place(Move(movetiles, row, col, Direction::DOWN));
                if (!result.valid) {
                    cout << result.error << endl;
                    continue;
                }
                // check if words formed are valid words in dictionary
                for (size_t i = 0; i < result.words.size(); i++) {
                    if (!dictionary.is_word(result.words[i])) {
                        cerr << result.words[i] << " is not a word" << endl;
                        error = true;
                        break;
                    }
                }
                if (error) {
                    continue;
                }
                return Move(movetiles, row, col, Direction::DOWN);
            } else {
                PlaceResult result = board.test_place(Move(movetiles, row, col, Direction::ACROSS));
                if (!result.valid) {
                    cout << result.error << endl;
                    continue;
                }
                // check if words formed are valid words in dictionary
                for (size_t i = 0; i < result.words.size(); i++) {
                    if (!dictionary.is_word(result.words[i])) {
                        cerr << result.words[i] << " is not a word" << endl;
                        error = true;
                        break;
                    }
                }
                if (error) {
                    continue;
                }
                return Move(movetiles, row, col, Direction::ACROSS);
            }
        }
    }
}

// helper function to check if player has the tiles they want to use
std::vector<TileKind> HumanPlayer::parse_tiles(std::string& letters) const {

    std::vector<TileKind> tiles;
    map<char, size_t> letterCount;
    TileKind test(0, '\0');
    for (size_t i = 0; i < letters.size(); i++) {
        // check if the player has at least one of the tile they want to use
        try {
            test = collection.lookup_tile(letters[i]);
        } catch (out_of_range& e) {
            throw MoveException(e.what());
        }
        if (test.letter == TileKind::BLANK_LETTER) {
            test.assigned = (char)tolower(letters[++i]);
        }
        // update letters map to insert or increase number of each tilekind they want to use
        if (letterCount.find(letters[i]) == letterCount.end()) {
            letterCount.insert(make_pair(letterCount[i], 1));
        } else {
            letterCount.find(letterCount[i])->second++;
        }
        tiles.push_back(test);
    }
    // check if player has enough of each tilekind in hand
    for (size_t i = 0; i < tiles.size(); i++) {
        if (letterCount[tiles[i].letter] > collection.count_tiles(tiles[i])) {
            throw MoveException("Error in move: Not enough tiles!");
        }
    }
    return tiles;
}

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = collection.count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = collection.cbegin(); it != collection.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (collection.count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
