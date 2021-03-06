#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <algorithm>

#include "scrabble_config.h"
#include "board.h"
#include "dictionary.h"
#include "tile_kind.h"
#include "human_player.h"
#include "computer_player.h"

#define DICT_PATH "config/english-dictionary.txt"

using namespace std;


// Helper functions for placing words in get_anchors() and get_move() tests
void print_words(PlaceResult res, Move m){
	std::cout << m.row + 1 << ' ' << m.column + 1 << ' ';
	if (m.direction == Direction::ACROSS)
		std::cout << "- ";
	else
		std::cout << "| ";
	for (auto word : res.words)
		std::cout << word << " ";
	std::cout << std::endl;
}

void place_simple_word(Board &b) {
	vector<TileKind> t;
    t.push_back(TileKind('H', 1));
    t.push_back(TileKind('I', 1));

    Move m = Move(t, 7, 7, Direction::ACROSS);
    b.place(m);
}

void place_two_words(Board &b) {
	vector<TileKind> t1;
    t1.push_back(TileKind('H', 1));
    t1.push_back(TileKind('I', 1));

    Move m1 = Move(t1, 7, 7, Direction::ACROSS);
    b.place(m1);

	vector<TileKind> t2;
    t2.push_back(TileKind('A', 1));
    t2.push_back(TileKind('A', 1));

    Move m2 = Move(t2, 5, 7, Direction::DOWN);
    b.place(m2);
}

void place_long_word(Board &b) {
	vector<TileKind> t1;
    t1.push_back(TileKind('B', 1));
    t1.push_back(TileKind('L', 1));
	t1.push_back(TileKind('I', 1));
	t1.push_back(TileKind('Z', 1));
	t1.push_back(TileKind('Z', 1));
	t1.push_back(TileKind('A', 1));
	t1.push_back(TileKind('R', 1));
	t1.push_back(TileKind('D', 1));

    Move m1 = Move(t1, 7, 7, Direction::ACROSS);
    b.place(m1);
}

void place_concave_words(Board &b) {
	vector<TileKind> t1;
    t1.push_back(TileKind('A', 1));
    t1.push_back(TileKind('U', 1));
	t1.push_back(TileKind('N', 1));
	t1.push_back(TileKind('T', 1));
	t1.push_back(TileKind('Y', 1));
    Move m1 = Move(t1, 7, 7, Direction::ACROSS);
    b.place(m1);

	vector<TileKind> t2;
    t2.push_back(TileKind('B', 1));
    t2.push_back(TileKind('E', 1));
    t2.push_back(TileKind('R', 1));
    Move m2 = Move(t2, 5, 7, Direction::DOWN);
    b.place(m2);

	vector<TileKind> t3;
    t3.push_back(TileKind('A', 1));
    t3.push_back(TileKind('N', 1));
    t3.push_back(TileKind('L', 1));
	t3.push_back(TileKind('E', 1));
	t3.push_back(TileKind('R', 1));
    Move m3 = Move(t3, 5, 10, Direction::DOWN);
    b.place(m3);

	vector<TileKind> t4;
    t4.push_back(TileKind('I', 1));
    Move m4 = Move(t4, 6, 9, Direction::DOWN);
    b.place(m4);
}


class ComputerPlayerTest : public testing::Test {
protected:
	ComputerPlayerTest() {}
	virtual ~ComputerPlayerTest() {}
	void crappy_print_hand();
	void test_pts(PlaceResult res, unsigned int exp_pts);
};

void ComputerPlayerTest::test_pts(PlaceResult res, unsigned int exp_pts) {
	bool pass = false;
	if(res.points == exp_pts) {
		pass = true;		
	} else if(res.points == exp_pts+50) {
		pass = true;
	}

	if(!pass) {
		std::cerr << "TEST FAILED: res.points = " << res.points << std::endl;
		std::cerr << "    EXPECTED: " << exp_pts << " or " << exp_pts+50 << std::endl;
	}
	EXPECT_TRUE(pass);
}


TEST_F(ComputerPlayerTest, empty_no_multipliers_no_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	vector<TileKind> t;
    t.push_back(TileKind('A', 3));
    t.push_back(TileKind('B', 1));
	t.push_back(TileKind('F', 2));
	t.push_back(TileKind('T', 1));
	t.push_back(TileKind('N', 3));
	t.push_back(TileKind('O', 7));
	t.push_back(TileKind('S', 4));

	cpu.add_tiles(t);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 8 8 - batons
	test_pts(res, 19);
}

// Multipliers make no difference from the first move... so have a free one :)
TEST_F(ComputerPlayerTest, empty_with_multipliers_no_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	vector<TileKind> t;
    t.push_back(TileKind('A', 3));
    t.push_back(TileKind('B', 1));
	t.push_back(TileKind('F', 2));
	t.push_back(TileKind('T', 1));
	t.push_back(TileKind('N', 3));
	t.push_back(TileKind('O', 7));
	t.push_back(TileKind('S', 4));

	cpu.add_tiles(t);


	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 8 8 - batons
	test_pts(res, 19);
}

TEST_F(ComputerPlayerTest, empty_no_multipliers_one_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	vector<TileKind> t;
    t.push_back(TileKind('A', 3));
    t.push_back(TileKind('B', 1));
	t.push_back(TileKind('F', 2));
	t.push_back(TileKind('T', 1));
	t.push_back(TileKind('N', 3));
	t.push_back(TileKind('?', 1));
	t.push_back(TileKind('S', 4));

	cpu.add_tiles(t);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 8 8 - faints
	test_pts(res, 14);
}

TEST_F(ComputerPlayerTest, empty_no_multipliers_two_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	vector<TileKind> t;
    t.push_back(TileKind('?', 1));
    t.push_back(TileKind('B', 1));
	t.push_back(TileKind('?', 1));
	t.push_back(TileKind('T', 1));
	t.push_back(TileKind('N', 3));
	t.push_back(TileKind('O', 7));
	t.push_back(TileKind('S', 4));

	cpu.add_tiles(t);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 8 8 - bastion
	test_pts(res, 18);
}

TEST_F(ComputerPlayerTest, simple_no_multipliers_no_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_simple_word(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 9 5 - ho in batons
	test_pts(res, 31);
}

TEST_F(ComputerPlayerTest, simple_no_multipliers_one_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_simple_word(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('?', 1));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 9 7 - ho in confab
	test_pts(res, 29);
}

TEST_F(ComputerPlayerTest, simple_with_multipliers_no_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_simple_word(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 7 6 - ah si boast
	test_pts(res, 40);
}

TEST_F(ComputerPlayerTest, simple_with_multipliers_one_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_simple_word(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('?', 1));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 6 10 | hit oaten
	test_pts(res, 38);
}

TEST_F(ComputerPlayerTest, two_words_no_multipliers_no_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_two_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);
	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 2 7 | na sa batons
	test_pts(res, 28);
}


TEST_F(ComputerPlayerTest, two_words_no_multipliers_one_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_two_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('?', 1));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 4 9 | an at fontina
	test_pts(res, 24);
}


TEST_F(ComputerPlayerTest, two_words_with_multipliers_no_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_two_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 6 5 - sonata
	test_pts(res, 39);
}

TEST_F(ComputerPlayerTest, two_words_with_multipliers_one_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_two_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('?', 1));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 6 10 | hit oaten
	test_pts(res, 38);
}

TEST_F(ComputerPlayerTest, long_no_multipliers_no_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_long_word(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 2));
    t0.push_back(TileKind('B', 3));
	t0.push_back(TileKind('K', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);		
	// 7 7 - ob al si boas
	test_pts(res, 32);
}

TEST_F(ComputerPlayerTest, long_with_multipliers_no_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_long_word(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 2));
    t0.push_back(TileKind('B', 3));
	t0.push_back(TileKind('K', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 9 6 - bb lo it sabot
	test_pts(res, 47);
}

TEST_F(ComputerPlayerTest, concave_words_no_multipliers_no_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_concave_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 10 10 | se or sofa
	test_pts(res, 29);
}

TEST_F(ComputerPlayerTest, concave_words_no_multipliers_one_blank) {
	Board b = Board::read("config/board0.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_concave_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('?', 1));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 5 7 | ab ne jaunty or banjo
	test_pts(res, 37);
}

TEST_F(ComputerPlayerTest, concave_words_with_multipliers_no_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_concave_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('S', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 11 12 | rf fatso
	test_pts(res, 51);
}

TEST_F(ComputerPlayerTest, concave_words_with_multipliers_one_blank) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 7);

	place_concave_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
    t0.push_back(TileKind('B', 1));
	t0.push_back(TileKind('F', 2));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('N', 3));
	t0.push_back(TileKind('O', 7));
	t0.push_back(TileKind('?', 1));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 5 7 | ab ne jaunty or banjo
	test_pts(res, 57);
}

TEST_F(ComputerPlayerTest, stress_test) {
	Board b = Board::read("config/standard-board.txt");	
	Dictionary d = Dictionary::read(DICT_PATH);
	ComputerPlayer cpu("cpu", 10);

	place_concave_words(b);
	//b.print(cout);

	vector<TileKind> t0;
    t0.push_back(TileKind('A', 3));
	t0.push_back(TileKind('?', 1));
	t0.push_back(TileKind('T', 1));
	t0.push_back(TileKind('M', 3));
	t0.push_back(TileKind('?', 1));
	t0.push_back(TileKind('S', 4));
	t0.push_back(TileKind('Z', 7));
	t0.push_back(TileKind('P', 2));
	t0.push_back(TileKind('D', 3));
	t0.push_back(TileKind('F', 4));

	cpu.add_tiles(t0);

	Move m = cpu.get_move(b, d);
	PlaceResult res = b.test_place(m);
	// 3 10 | ma el se flamines
	test_pts(res, 57);
}
