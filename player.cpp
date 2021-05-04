#include "player.h"
using namespace std;

void Player::add_points(size_t points) { this->points += points; }

void Player::subtract_points(size_t points) {
    if ((int)this->points - (int)points > 0) {
        this->points -= points;
    } else {
        this->points = 0;
    }
}

size_t Player::get_points() const { return this->points; }

const std::string& Player::get_name() const { return name; }

size_t Player::count_tiles() const { return this->collection.count_tiles(); }

void Player::remove_tiles(const std::vector<TileKind>& tiles) {
    for (size_t i = 0; i < tiles.size(); i++) {
        this->collection.remove_tile(tiles[i]);
    }
}

void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (size_t i = 0; i < tiles.size(); i++) {
        this->collection.add_tile(tiles[i]);
    }
}

bool Player::has_tile(TileKind tile) {
    if (this->collection.count_tiles(tile) == 0) {
        return false;
    }
    return true;
}

unsigned int Player::get_hand_value() const { return this->collection.total_points(); }

size_t Player::get_hand_size() const { return this->hand_size; }