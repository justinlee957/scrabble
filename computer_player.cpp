
#include "computer_player.h"

#include <memory>
#include <string>


void ComputerPlayer::left_part(
        Board::Position anchor_pos,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        size_t limit,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {

    if (partial_move.direction == Direction::DOWN) {
        partial_move.row = anchor_pos.row - partial_word.size();
    } else {
        partial_move.column = anchor_pos.column - partial_word.size();
    }
    // call extend right on every recursive call
    extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board);

    if (limit == 0) {
        return;
    }

    for (auto it = node->nexts.begin(); it != node->nexts.end(); it++) {
        // check if player has blank tile
        try {
            TileKind add(remaining_tiles.lookup_tile(TileKind::BLANK_LETTER));
            add.assigned = it->first;
            partial_move.tiles.push_back(add);
            remaining_tiles.remove_tile(remaining_tiles.lookup_tile(TileKind::BLANK_LETTER));
            left_part(
                    anchor_pos,
                    partial_word + it->first,
                    partial_move,
                    it->second,
                    limit - 1,
                    remaining_tiles,
                    legal_moves,
                    board);
            // backtrack
            remaining_tiles.add_tile(partial_move.tiles.back());
            partial_move.tiles.pop_back();
        } catch (std::out_of_range& e) {
        }
        // check if player has specific tile
        try {
            partial_move.tiles.push_back(remaining_tiles.lookup_tile(it->first));
            remaining_tiles.remove_tile(remaining_tiles.lookup_tile(it->first));
        } catch (std::out_of_range& e) {
            continue;
        }
        left_part(
                anchor_pos,
                partial_word + it->first,
                partial_move,
                it->second,
                limit - 1,
                remaining_tiles,
                legal_moves,
                board);
        // backtrack
        remaining_tiles.add_tile(partial_move.tiles.back());
        partial_move.tiles.pop_back();
    }
}

void ComputerPlayer::extend_right(
        Board::Position square,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {

    if (node->is_final) {
        legal_moves.push_back(partial_move);
    }
    if (!board.is_in_bounds(square)) {
        return;
    }

    if (!board.in_bounds_and_has_tile(square)) {
        for (auto it = node->nexts.begin(); it != node->nexts.end(); it++) {
            // check if player has blank tile
            try {
                TileKind add(remaining_tiles.lookup_tile(TileKind::BLANK_LETTER));
                add.assigned = it->first;
                partial_move.tiles.push_back(add);
                remaining_tiles.remove_tile(remaining_tiles.lookup_tile(TileKind::BLANK_LETTER));
                extend_right(
                        square.translate(partial_move.direction),
                        partial_word + it->first,
                        partial_move,
                        it->second,
                        remaining_tiles,
                        legal_moves,
                        board);
                // backtrack
                remaining_tiles.add_tile(partial_move.tiles.back());
                partial_move.tiles.pop_back();
            } catch (std::out_of_range& e) {
            }
            // check if player has specific tile
            try {
                partial_move.tiles.push_back(remaining_tiles.lookup_tile(it->first));
                remaining_tiles.remove_tile(remaining_tiles.lookup_tile(it->first));
            } catch (std::out_of_range& e) {
                continue;
            }
            extend_right(
                    square.translate(partial_move.direction),
                    partial_word + it->first,
                    partial_move,
                    it->second,
                    remaining_tiles,
                    legal_moves,
                    board);
            // backtrack
            remaining_tiles.add_tile(partial_move.tiles.back());
            partial_move.tiles.pop_back();
        }
        // if next square is not vacant
    } else {
        // if the tile already placed can be used to build a word
        if (node->nexts.find(board.letter_at(square)) != node->nexts.end()) {
            extend_right(
                    square.translate(partial_move.direction),
                    partial_word + board.letter_at(square),
                    partial_move,
                    node->nexts.find(board.letter_at(square))->second,
                    remaining_tiles,
                    legal_moves,
                    board);
        }
    }
}

Move ComputerPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    std::vector<Move> legal_moves;
    std::vector<Board::Anchor> anchors = board.get_anchors();
    TileCollection remaining_tiles = this->collection;
    std::vector<TileKind> tiles;
    for (size_t i = 0; i < anchors.size(); i++) {
        // call left on anchor spots with limit > 0
        if (anchors[i].limit > 0) {
            left_part(
                    anchors[i].position,
                    "",
                    Move(tiles, anchors[i].position.row, anchors[i].position.column, anchors[i].direction),
                    dictionary.get_root(),
                    anchors[i].limit,
                    remaining_tiles,
                    legal_moves,
                    board);
            // call extend right on anchor spots with limit = 0
        } else if (anchors[i].limit == 0) {
            std::string partial = "";
            Board::Position p = anchors[i].position;
            p = p.translate(anchors[i].direction, -1);
            // build the partial word to the left or above the anchor spot with
            // the tiles already placed
            while (board.in_bounds_and_has_tile(p)) {
                partial = board.letter_at(p) + partial;
                p = p.translate(anchors[i].direction, -1);
            }
            // make sure the partial word has a prefix
            if (dictionary.find_prefix(partial) != nullptr) {
                extend_right(
                        anchors[i].position,
                        partial,
                        Move(tiles, anchors[i].position.row, anchors[i].position.column, anchors[i].direction),
                        dictionary.find_prefix(partial),
                        remaining_tiles,
                        legal_moves,
                        board);
            }
        }
    }

    return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(
        std::vector<Move> legal_moves, const Board& board, const Dictionary& dictionary) const {
    Move best_move = Move();  // Pass if no move found
    size_t highest = 0;
    int index = -1;
    bool valid;

    for (size_t i = 0; i < legal_moves.size(); i++) {
        valid = true;
        if (legal_moves[i].tiles.size() == 0) {
            continue;
        }
        PlaceResult result = board.test_place(legal_moves[i]);

        if (result.valid) {
            // check if all the resulting words are real words in the dictionary
            for (size_t j = 0; j < result.words.size(); j++) {
                if (!dictionary.is_word(result.words[j])) {
                    valid = false;
                    break;
                }
            }
            if (!valid) {
                continue;
            }
            if (result.points > highest) {
                highest = result.points;
                index = i;
            }
        }
    }
    if (index == -1) {
        return best_move;
    }

    best_move = legal_moves[index];

    return best_move;
}
