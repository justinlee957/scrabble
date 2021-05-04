#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);

    // TODO: complete implementation of reading in board from file here.
    char c;
    board.squares.resize(rows);
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            file >> c;
            if (c == '.') {
                board.squares[i].push_back(BoardSquare(1, 1));
            } else if (c == '2') {
                board.squares[i].push_back(BoardSquare(2, 1));
            } else if (c == '3') {
                board.squares[i].push_back(BoardSquare(3, 1));
            } else if (c == 'd') {
                board.squares[i].push_back(BoardSquare(1, 2));
            } else {
                board.squares[i].push_back(BoardSquare(1, 3));
            }
        }
    }
    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_Down(const Move& move) const {
    bool hasAdjacent = false;
    // checking if it's 1st move and starts on starting tile
    if (!squares[start.row][start.column].has_tile()) {
        bool starts = false;
        hasAdjacent = true;
        for (size_t i = 0; i < move.tiles.size(); i++) {
            if (move.row + i == start.row && move.column == start.column) {
                starts = true;
                break;
            }
        }
        if (!starts) {
            return PlaceResult("Must place tile on start square");
        }
    }

    string s;
    vector<string> words;
    int points = 0;
    int bonus = 1;

    // if an adjacent tile is on top of the 1st placed tile
    // insert each letter to the front of the 's' string and update points
    for (size_t i = 1;
         is_in_bounds(Position(move.row - i, move.column)) && squares[move.row - i][move.column].has_tile();
         i++) {
        if (squares[move.row - i][move.column].get_tile_kind().letter == TileKind::BLANK_LETTER) {
            s.insert(0, 1, squares[move.row - i][move.column].get_tile_kind().assigned);
        } else {
            s.insert(0, 1, squares[move.row - i][move.column].get_tile_kind().letter);
        }

        points += squares[move.row - i][move.column].get_tile_kind().points;
        hasAdjacent = true;
    }

    // len keeps track of how much the position of the tile placement shifted
    // because an existing tile exists between the tiles
    size_t len = 0;
    // if there are squares with existing tiles in between where the tiles would be placed,
    // add those letters to the back of the 's' string and update points
    for (size_t i = 0, j = 0; i != move.tiles.size(); j++) {
        if (squares[move.row + j][move.column].has_tile()) {
            hasAdjacent = true;
            if (squares[move.row + j][move.column].get_tile_kind().letter == TileKind::BLANK_LETTER) {
                s.push_back(squares[move.row + j][move.column].get_tile_kind().assigned);
            } else {
                s.push_back(squares[move.row + j][move.column].get_tile_kind().letter);
            }
            points += squares[move.row + j][move.column].get_tile_kind().points;
            len++;
            continue;
        }
        if (move.tiles[i].letter == TileKind::BLANK_LETTER) {
            s.push_back(move.tiles[i].assigned);
        } else {
            s.push_back(move.tiles[i].letter);
        }
        points += move.tiles[i].points * squares[move.row + i + len][move.column].letter_multiplier;
        bonus *= squares[move.row + i + len][move.column].word_multiplier;
        i++;
    }

    // if an adjacent tile on the bottom of the last placed tile
    // add those letters to the back of the 's' string and update points
    for (size_t i = 0; is_in_bounds(Position(move.row + move.tiles.size() + i + len, move.column))
                       && squares[move.row + move.tiles.size() + i + len][move.column].has_tile();
         i++) {
        if (squares[move.row + move.tiles.size() + i + len][move.column].get_tile_kind().letter
            == TileKind::BLANK_LETTER) {
            s.push_back(squares[move.row + move.tiles.size() + i + len][move.column].get_tile_kind().letter);
        } else {
            s.push_back(squares[move.row + move.tiles.size() + i + len][move.column].get_tile_kind().letter);
        }
        points += squares[move.row + move.tiles.size() + i + len][move.column].get_tile_kind().points;
        hasAdjacent = true;
    }

    // check if size of string is as long as the min word length
    // set as 2 because gtests don't factor in min word length
    if (s.size() >= 2) {
        words.push_back(s);
        points *= bonus;
    } else {
        points = 0;
    }
    bonus = 1;
    string str;
    int subpoints = 0;
    // if there are adjacent tiles to the left or right of where
    // player wishes to place tiles
    len = 0;
    for (size_t i = 0; i < s.size(); i++) {
        // if there isn't an existing tile on the column where we want to place but an existing tile left
        // or there isn't an existing tile on the column where we want to place but an existing tile right
        // a new word is formed
        if (in_bounds_and_has_tile(Position(move.row + i, move.column))) {
            len++;
            continue;
        }
        if ((in_bounds_and_has_tile(Position(move.row + i, move.column + 1))
             && !in_bounds_and_has_tile(Position(move.row + i, move.column)))
            || (in_bounds_and_has_tile(Position(move.row + i, move.column - 1))
                && !in_bounds_and_has_tile(Position(move.row + i, move.column)))) {
            hasAdjacent = true;
            // add the desired tile's information across the variables
            if (move.tiles[i - len].letter == TileKind::BLANK_LETTER) {
                str.push_back(move.tiles[i - len].assigned);
            } else {
                str.push_back(move.tiles[i - len].letter);
            }
            subpoints += move.tiles[i - len].points * squares[move.row + i][move.column].letter_multiplier;
            bonus *= squares[move.row + i][move.column].word_multiplier;
            // adds all the tiles on the left
            for (size_t j = 1; squares[move.row + i][move.column - j].has_tile(); j++) {
                if (squares[move.row + i][move.column - j].get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    str.insert(0, 1, squares[move.row + i][move.column - j].get_tile_kind().assigned);
                } else {
                    str.insert(0, 1, squares[move.row + i][move.column - j].get_tile_kind().letter);
                }
                subpoints += squares[move.row + i][move.column - j].get_tile_kind().points;
            }

            // adds all the tiles on the right
            for (size_t j = 1; squares[move.row + i][move.column + j].has_tile(); j++) {
                if (squares[move.row + i][move.column + j].get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    str.push_back(squares[move.row + i][move.column + j].get_tile_kind().assigned);
                } else {
                    str.push_back(squares[move.row + i][move.column + j].get_tile_kind().letter);
                }
                subpoints += squares[move.row + i][move.column + j].get_tile_kind().points;
            }
            points += subpoints * bonus;
            words.push_back(str);

            // reinitialize the variables for next iteration
            // if there is another adjacent tile on the next row square
            bonus = 1;
            subpoints = 0;
            str.clear();
        }
    }

    if (hasAdjacent) {
        return PlaceResult(words, points);
    }
    return PlaceResult("Needs at least one adjacent tile");
}
PlaceResult Board::test_Across(const Move& move) const {
    bool hasAdjacent = false;

    if (!is_in_bounds(Position(move.row, move.column + move.tiles.size() - 1))) {
        return PlaceResult("Out of bounds");
    }
    // checking if it's 1st move and starts on starting tile
    if (!squares[start.row][start.column].has_tile()) {
        bool starts = false;
        hasAdjacent = true;
        for (size_t i = 0; i < move.tiles.size(); i++) {
            if (move.row == start.row && move.column + i == start.column) {
                starts = true;
                break;
            }
        }
        if (!starts) {
            return PlaceResult("Must place tile on start square");
        }
    }

    string s;
    vector<string> words;
    int points = 0;
    int bonus = 1;
    // if an adjacent tile is on the left of the 1st placed tile
    // insert each letter to the front of the 's' string and update points
    for (size_t i = 1;
         is_in_bounds(Position(move.row, move.column - i)) && squares[move.row][move.column - i].has_tile();
         i++) {
        if (squares[move.row][move.column - i].get_tile_kind().letter == TileKind::BLANK_LETTER) {
            s.insert(0, 1, squares[move.row][move.column - i].get_tile_kind().assigned);
        } else {
            s.insert(0, 1, squares[move.row][move.column - i].get_tile_kind().letter);
        }
        points += squares[move.row][move.column - i].get_tile_kind().points;
        hasAdjacent = true;
    }
    // len keeps track of how much the position of the tile placement shifted
    // because an existing tile exists between the tiles
    size_t len = 0;
    // if there are squares with existing tiles in between where the tiles would be placed,
    // add those letters to the back of the 's' string and update points
    for (size_t j = 0, i = 0; i != move.tiles.size(); j++) {
        if (squares[move.row][move.column + j].has_tile()) {
            hasAdjacent = true;
            if (squares[move.row][move.column + j].get_tile_kind().letter == TileKind::BLANK_LETTER) {
                s.push_back(squares[move.row][move.column + j].get_tile_kind().assigned);
            } else {
                s.push_back(squares[move.row][move.column + j].get_tile_kind().letter);
            }
            points += squares[move.row][move.column + j].get_tile_kind().points;
            len++;
            continue;
        }

        if (move.tiles[i].letter == TileKind::BLANK_LETTER) {
            s.push_back(move.tiles[i].assigned);
        } else {
            s.push_back(move.tiles[i].letter);
        }
        points += move.tiles[i].points * squares[move.row][move.column + i + len].letter_multiplier;
        bonus *= squares[move.row][move.column + i + len].word_multiplier;

        i++;
    }

    // if an adjacent tile on the right of the last placed tile
    // add those letters to the back of the 's' string and update points
    for (size_t i = 0; is_in_bounds(Position(move.row, move.column + move.tiles.size() + i + len))
                       && squares[move.row][move.column + move.tiles.size() + i + len].has_tile();
         i++) {
        if (squares[move.row][move.column + i].get_tile_kind().letter == TileKind::BLANK_LETTER) {
            s.push_back(squares[move.row][move.column + move.tiles.size() + i + len].get_tile_kind().assigned);
        } else {
            s.push_back(squares[move.row][move.column + move.tiles.size() + i + len].get_tile_kind().letter);
        }

        points += squares[move.row][move.column + move.tiles.size() + i + len].get_tile_kind().points;
        hasAdjacent = true;
    }
    if (s.size() >= 2) {
        words.push_back(s);
        points *= bonus;
    } else {
        points = 0;
    }

    bonus = 1;

    string str;
    int subpoints = 0;
    len = 0;
    // if there are adjacent tiles to the top or bottom of where
    // player wishes to place tiles
    for (size_t i = 0; i < s.size(); i++) {
        // if there isn't an existing tile on the row where we want to place but an existing tile above
        // or there isn't an existing tile on the row where we want to place but an existing tile below
        // a new word is formed
        if (in_bounds_and_has_tile(Position(move.row, move.column + i))) {
            len++;
            continue;
        }
        if ((in_bounds_and_has_tile(Position(move.row + 1, move.column + i))
             && !in_bounds_and_has_tile(Position(move.row, move.column + i)))
            || (in_bounds_and_has_tile(Position(move.row - 1, move.column + i))
                && !in_bounds_and_has_tile(Position(move.row, move.column + i)))) {
            hasAdjacent = true;
            // add the desired tile's information across the variables
            if (move.tiles[i - len].letter == TileKind::BLANK_LETTER) {
                str.push_back(move.tiles[i - len].assigned);
            } else {
                str.push_back(move.tiles[i - len].letter);
            }
            subpoints += move.tiles[i - len].points * squares[move.row][move.column + i].letter_multiplier;
            bonus *= squares[move.row][move.column + i].word_multiplier;

            // processes all squares above
            for (size_t j = 1; squares[move.row - j][move.column + i].has_tile(); j++) {
                if (squares[move.row - j][move.column + i].get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    str.insert(0, 1, squares[move.row - j][move.column + i].get_tile_kind().assigned);
                } else {
                    str.insert(0, 1, squares[move.row - j][move.column + i].get_tile_kind().letter);
                }

                subpoints += squares[move.row - j][move.column + i].get_tile_kind().points;
            }

            // processes all squares below
            for (size_t j = 1; squares[move.row + j][move.column + i].has_tile(); j++) {
                if (squares[move.row + j][move.column + i].get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    str.push_back(squares[move.row + j][move.column + i].get_tile_kind().assigned);
                } else {
                    str.push_back(squares[move.row + j][move.column + i].get_tile_kind().letter);
                }
                subpoints += squares[move.row + j][move.column + i].get_tile_kind().points;
            }

            points += subpoints * bonus;
            words.push_back(str);
            // reinitialize the variables for next iteration
            // if there is another adjacent tile on the next column square
            bonus = 1;
            subpoints = 0;
            str.clear();
        }
    }

    if (hasAdjacent) {
        return PlaceResult(words, points);
    }
    return PlaceResult("Needs at least one adjacent tile");
}
PlaceResult Board::test_place(const Move& move) const {
    if (in_bounds_and_has_tile(Position(move.row, move.column))) {
        return PlaceResult("Cannot start word on an existing tile.");
    }

    if (move.direction == Direction::DOWN) {
        return test_Down(move);
        // same logic as DOWN direction, but now it's across
    } else {
        return test_Across(move);
    }
}

PlaceResult Board::place(const Move& move) {
    PlaceResult test = test_place(move);
    if (!test.valid) {
        return test;
    }
    // permanently place the tiles on the board
    if (move.direction == Direction::DOWN) {
        for (size_t i = 0, j = 0; i != move.tiles.size(); j++) {
            if (squares[move.row + j][move.column].has_tile()) {
                continue;
            }
            squares[move.row + j][move.column].set_tile_kind(move.tiles[i]);
            i++;
        }
    } else {
        for (size_t i = 0, j = 0; i != move.tiles.size(); j++) {
            if (squares[move.row][move.column + j].has_tile()) {
                continue;
            }
            squares[move.row][move.column + j].set_tile_kind(move.tiles[i]);
            i++;
        }
    }
    return test;
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - to_string(square.get_points()).size()) << FG_COLOR_SCORE
                        << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}

char Board::letter_at(Position p) const {
    if (at(p).get_tile_kind().letter == TileKind::BLANK_LETTER) {
        return at(p).get_tile_kind().assigned;
    }
    return at(p).get_tile_kind().letter;
}

bool Board::is_anchor_spot(Position p) const {
    if (!is_in_bounds(p) || in_bounds_and_has_tile(p)) {
        return false;
    }
    // checking for adjacent squares with tiles
    if (in_bounds_and_has_tile(Position(p.row + 1, p.column)) || in_bounds_and_has_tile(Position(p.row - 1, p.column))
        || in_bounds_and_has_tile(Position(p.row, p.column + 1))
        || in_bounds_and_has_tile(Position(p.row, p.column - 1))) {
        return true;
    }
    // check if position is start
    if (p == start) {
        return true;
    }
    return false;
}

std::vector<Board::Anchor> Board::get_anchors() const {
    std::vector<Board::Anchor> anchors;
    size_t x, y, limit;
    // iterate through every square and figure out
    // the limits for each anchor
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < columns; j++) {
            if (is_anchor_spot(Position(i, j))) {
                x = i;
                y = j;
                limit = 0;
                // go left and increment limit
                while (is_in_bounds(Position(x, y - 1)) && !in_bounds_and_has_tile(Position(x, y - 1))
                       && !is_anchor_spot(Position(x, y - 1))) {
                    limit++;
                    y--;
                }
                anchors.push_back(Board::Anchor(Position(i, j), Direction::ACROSS, limit));
                y = j;
                limit = 0;
                // go up and increment limit
                while (is_in_bounds(Position(x - 1, y)) && !in_bounds_and_has_tile(Position(x - 1, y))
                       && !is_anchor_spot(Position(x - 1, y))) {
                    limit++;
                    x--;
                }
                anchors.push_back(Board::Anchor(Position(i, j), Direction::DOWN, limit));
            }
        }
    }
    return anchors;
}