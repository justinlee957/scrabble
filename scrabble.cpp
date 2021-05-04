#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)) {}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    size_t pass = 0;
    board.minimum_word_length = this->minimum_word_length;
    cin.ignore();
    // keep looping till game is over
    for (size_t i = 0; true; i++) {
        board.print(cout);
        // get_move will error check the user inputs
        Move move = players[i]->get_move(this->board, this->dictionary);

        if (move.kind == MoveKind::PASS) {
            pass++;
        } else if (move.kind == MoveKind::EXCHANGE) {
            // remove the tiles from player's hand
            players[i]->remove_tiles(move.tiles);
            // add the tiles back to the bag
            for (size_t i = 0; i < move.tiles.size(); i++) {
                tile_bag.add_tile(move.tiles[i]);
            }
            // add the size of removed tiles from the tilebag to the player's hand
            players[i]->add_tiles(tile_bag.remove_random_tiles(move.tiles.size()));
            cout << "Your tiles have been exhanged." << endl;
            if (players[i]->is_human()) {
                pass = 0;
            }
        } else if (move.kind == MoveKind::PLACE) {
            // permanently place the word on the board
            PlaceResult placed = board.place(move);
            // update the player's points
            players[i]->add_points(placed.points);
            if (move.tiles.size() == hand_size) {
                players[i]->add_points(50);
            }
            cout << "You gained " << SCORE_COLOR << placed.points << rang::style::reset << " points!" << endl;
            cout << "Your current score: " << SCORE_COLOR << players[i]->get_points() << rang::style::reset << endl;
            // remove the placed tiles from player's hand
            players[i]->remove_tiles(move.tiles);
            // add the size of the placed tiles from the tilebag to the player's hand
            players[i]->add_tiles(tile_bag.remove_random_tiles(move.tiles.size()));
            if (players[i]->is_human()) {
                pass = 0;
            }
        }
        if (players[i]->is_human()) {
            cin.ignore();
        }
        cout << endl << "Press [enter] to continue.";
        cin.ignore();
        // game ends if player has no tiles
        if (players[i]->count_tiles() == 0) {
            return;
        }
        // game ends if all players pass consecutively
        if ((pass == this->num_human_players) && (i + 1 == players.size())) {
            return;
        }
        if (i + 1 == players.size()) {
            i = -1;
        }
    }
}

// Performs final score subtraction.
void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    size_t allHandSum = 0;
    // get the last hand value and subtract from their points
    for (size_t i = 0; i < plrs.size(); i++) {
        plrs[i]->subtract_points(plrs[i]->get_hand_value());
        allHandSum += plrs[i]->get_hand_value();
    }
    // whoever cleared their hand receives all the points lost by the other players.
    for (size_t i = 0; i < plrs.size(); i++) {
        if (plrs[i]->count_tiles() == 0) {
            plrs[i]->add_points(allHandSum);
        }
    }
}

void Scrabble::add_players() {
    int num;
    cout << "How many players?: ";
    cin >> num;
    string name;
    char choice;
    for (int i = 0; i < num; i++) {
        cout << "Enter name of player " << i + 1 << ": ";
        cin >> name;
        cout << "Is this player a computer? (y/n): ";
        cin >> choice;
        if (choice == 'y') {
            players.push_back(make_shared<ComputerPlayer>(name, hand_size));
        } else if (choice == 'n') {
            players.push_back(make_shared<HumanPlayer>(name, hand_size));
            this->num_human_players++;
        }
        players[i]->add_tiles(tile_bag.remove_random_tiles(hand_size));
        cout << "Player " << i + 1 << " named " << name << " created." << endl;
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    cout << rang::style::reset << endl;

    // now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
             << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
