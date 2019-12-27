#include <SFML/Graphics.hpp>
#include <random>
#include <chrono>
#include <iostream>
#include <algorithm>
using namespace sf;

const int BLOCK_TYPES[7][4] = {
	{ 1, 3, 4, 5 }, // J
	{ 1, 2, 3, 5 }, // T
	{ 1, 2, 3, 4 }, // Z
	{ 0, 2, 3, 5 }, // S
	{ 0, 1, 2, 3 }, // O
	{ 0, 2, 4, 6 }, // I
	{ 0, 1, 3, 5 }  // L
};

enum BLOCKS {
	J,
	T,
	Z,
	S,
	O,
	I,
	L,
	None
};

const int TILESIZE = 18;
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;

const int BAG_SIZE = 7;

//CLASSES TODO PUT IN SEPERATE FILES
class Game {
private:
	int board[BOARD_WIDTH][BOARD_HEIGHT]; //stores type values for block
	unsigned int score;
	unsigned int level;
	unsigned int combo;

	bool game_over;
public:
	Game() {
		score = 0;
		level = 1;
		combo = 0;

		game_over = false;

		for (int i = 0; i < BOARD_WIDTH; i++) { //intialize board to BLOCK.None
			for (int j = 0; j < BOARD_HEIGHT; j++) {
				board[i][j] = BLOCKS::None;
			}
		}
	}

	inline void addScore(unsigned int score) {
		this->score += score;
	}

	inline void incrementCombo() {
		this->combo++;
	}

	inline void resetCombo() {
		this->combo = 0;
	}

	//TODO add error checking in these methods
	//Return true if a row is cleared by placing the block
	bool setBlock(Vector2i pos, int type) {
		board[pos.x][pos.y] = type;

		bool rowCleared = true;
		for (int i = 0; i < BOARD_WIDTH; i++) {
			if (board[i][pos.y] == BLOCKS::None) {
				rowCleared = false;
				break;
			}
		}

		//clear row
		if (rowCleared) {
			//Rows above drop down
			for (int y = pos.y; y > 0; y--) {
				for (int x = 0; x < BOARD_WIDTH; x++) {
					board[x][y] = board[x][y - 1];
				}
			}

			//blank top row
			for (int i = 0; i < BOARD_WIDTH; i++) {
				board[i][0] = BLOCKS::None;
			}
		}

		return rowCleared;
	}

	inline void gameOver() {
		game_over = true;
	}

	inline int getBlock(Vector2i pos) {
		return board[pos.x][pos.y];
	}

	inline unsigned int getScore() {
		return score;
	}

	inline unsigned int getLevel() {
		return level;
	}

	inline unsigned int getCombo() {
		return combo;
	}

	inline bool getGameOver() {
		return game_over;
	}

	//Handles graphics for every frame
	void draw(Texture& t, RenderWindow& window) {
		for (int i = 0; i < BOARD_WIDTH; i++) {
			for (int j = 0; j < BOARD_HEIGHT; j++) {
				int type = board[i][j];

				Sprite block;
				block.setTexture(t);

				block.setTextureRect(IntRect(type * TILESIZE, 0, TILESIZE, TILESIZE));

				block.setPosition(i * TILESIZE, j * TILESIZE);
				window.draw(block);
			}
		}
	}
};

class Player {
private:
	int type;
	Vector2i blockPos[4];
	Game* g;

	std::vector<int> bag;
	std::vector<int> next_bag;
	int bagIndex;

	bool can_hold;
	int held_block = BLOCKS::None;
public:
	Player(Game* game) : bag(BAG_SIZE), next_bag(BAG_SIZE) {
		std::srand(unsigned(std::time(0)));

		can_hold = true;

		bag = {0, 1, 2, 3, 4, 5, 6};
		next_bag = {0, 1, 2, 3, 4, 5, 6};

		std::random_shuffle(bag.begin(), bag.end());
		std::random_shuffle(next_bag.begin(), next_bag.end());

		bagIndex = 0;
		g = game;
		newBlock();
	}

	void newBlock() {
		Vector2i dy = Vector2i(0, 0);

		type = bag[bagIndex];
		for (int i = 0; i < 4; i++) {
			int n = BLOCK_TYPES[type][i];
			blockPos[i].x = BOARD_WIDTH / 2 + n % 2;
			blockPos[i].y = n / 2;

			if (g->getBlock(blockPos[i]) != BLOCKS::None) {
				dy.y--;
			}
		}

		for (int i = 0; i < 4; i++) {
			blockPos[i] += dy;
		}

		bagIndex++;
		if (bagIndex == BAG_SIZE) {
			bagIndex = 0;
			bag = next_bag;
			std::random_shuffle(next_bag.begin(), next_bag.end());
		}
	}

	void rotate(int dir) { //-1 is left, 1 is right
		if (type == BLOCKS::O) //O
			return;
		Vector2i temp[4]; //store new positions temporarily
		Vector2i origin = blockPos[2];
		for (int i = 0; i < 4; i++) {
			if (i == 2) {
				temp[i] = blockPos[i];
				continue;
			}
			temp[i].x = dir * (blockPos[i].y - origin.y);
			temp[i].y = -1 * dir * (blockPos[i].x - origin.x);
			temp[i].x += origin.x;
			if (temp[i].x < 0 || temp[i].x >= BOARD_WIDTH) //x out of bounds
				return;
			temp[i].y += origin.y;
			if (temp[i].y < 0 || temp[i].y >= BOARD_HEIGHT) //y out of bounds
				return;
			if (g->getBlock(temp[i]) != BLOCKS::None) //block in way
				return;
		}

		const auto compare = [](Vector2i& a, Vector2i b) -> bool {
			return a.y < b.y || (a.y == b.y && a.x < b.x);
		};

		//sort array of positions top to bottom for line clearing logic
		std::sort(temp, temp+4, compare);

		for (int i = 0; i < 4; i++) { //commit temp to block pos
			blockPos[i] = temp[i];
		}

	}

	void placeBlock(Vector2i dy) {
		int linesCleared = 0;
		for (int i = 0; i < 4; i++) {
			if (blockPos[i].y < 0) {
				g->gameOver();
				return;
			}
			linesCleared += (int) g->setBlock(blockPos[i] + dy, type);
		}

		switch (linesCleared) {
			case 1:
				g->addScore(100 * g->getLevel());
				break;
			case 2:
				g->addScore(300 * g->getLevel());
				break;
			case 3:
				g->addScore(500 * g->getLevel());
				break;
			case 4:
				g->addScore(800 * g->getLevel());
				break;
		}

		if (linesCleared) {
			g->incrementCombo();
		}
		else {
			g->resetCombo();
		}

		if (g->getCombo() > 1) {
			g->addScore(800 * g->getCombo() * g->getLevel());
		}

		std::cout << "Score: " << g->getScore() << " Combo: " << g->getCombo() << std::endl;
		std::fflush(stdout);

		newBlock();

		can_hold = true;
	}

	Vector2i getDropDistance() {
		Vector2i dy(0, -1);
		Vector2i below(0, 1);
		bool canMoveY = true;
		while (canMoveY) {
			dy.y++;
			for (int i = 3; i >= 0; i--) { //loop bottom to top
				if ((blockPos[i] + dy).y == BOARD_HEIGHT - 1 || g->getBlock(blockPos[i] + dy + below) != BLOCKS::None) {
					canMoveY = false;
					break;
				}
			}
		}

		return dy;
	}

	void hardDrop() {
		Vector2i dy = getDropDistance();

		placeBlock(dy);
	}

	void hold() {
		if (can_hold) {
			if (held_block != BLOCKS::None) {
				bagIndex--;
				bag[bagIndex] = held_block;
			}

			held_block = type;
			newBlock();

			can_hold = false;
		}
	}

	void move(int dx, int dy) {
		bool canMoveX = true;
		bool canMoveY = true;

		for (int i = 0; i < 4; i++) {
			Vector2i vx(dx, 0);
			Vector2i vy(0, dy);
			if (g->getBlock(blockPos[i] + vx) != BLOCKS::None || blockPos[i].x + dx > BOARD_WIDTH - 1 || blockPos[i].x + dx < 0) //block in way horizontally
				canMoveX = false;
			if (g->getBlock(blockPos[i] + vy) != BLOCKS::None || (blockPos[i].y + dy) == BOARD_HEIGHT) //block in way vertically or hit floor
				canMoveY = false;
		}

		if (canMoveX) {
			for (int i = 0; i < 4; i++) {
				blockPos[i].x += dx;
			}
		}

		if (canMoveY) {
			for (int i = 0; i < 4; i++) {
				blockPos[i].y += dy;
			}
		}
		else {
			placeBlock(Vector2i(0, 0));
		}

	}

	void drawBlock(Texture& t, RenderWindow& window, int type, const Vector2i pos[], bool ghost) {
		for (int i = 0; i < 4; i++) {
			if (pos[i].y < 0) {
				continue;
			}

			Sprite block;

			block.setTexture(t);

			//Slices specific color we want
			block.setTextureRect(IntRect(type * TILESIZE, ((int) ghost) * TILESIZE, TILESIZE, TILESIZE));

			Vector2i v = pos[i];

			block.setPosition(v.x * TILESIZE, v.y * TILESIZE);

			window.draw(block);
		}
	}

	void drawGhost(Texture& t, RenderWindow& window) {
		Vector2i dy = getDropDistance();

		Vector2i pos[4];
		for (int i = 0; i < 4; i++) { 
			pos[i] = blockPos[i] + dy;
		}
		drawBlock(t, window, type, pos, true);
	}

	void drawHold(Texture& t, RenderWindow& window) {
		if (held_block == BLOCKS::None) {
			return;
		}

		Vector2i pos[4];
		for (int i = 0; i < 4; i++) {
			int n = BLOCK_TYPES[held_block][i];
			pos[i].x = BOARD_WIDTH + 2 + n % 2;
			pos[i].y = n / 2 + 2;
		}

		drawBlock(t, window, held_block, pos, false);
	}

	void draw(Texture& t, RenderWindow& window) {
		drawBlock(t, window, type, blockPos, false);
	}
};

int main() {
	RenderWindow window(VideoMode(TILESIZE * BOARD_WIDTH + 108, TILESIZE * BOARD_HEIGHT), "Tetris");

	Texture t;
	t.loadFromFile("../Textures/tiles.png");

	Game game;
	Player p(&game);

	int dx;
	int dy;

	//clock
	std::chrono::steady_clock::time_point now;
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point lastMove = std::chrono::steady_clock::now();

	std::chrono::duration<double> delta;

	double timeBetweenDrops;
	double moveDelay = .09;

	//Game Loop
	while (window.isOpen() && !game.getGameOver()) {
		timeBetweenDrops = .25;
		dx = 0;
		dy = 0;

		//Input

		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Space) {
					p.hardDrop();
					startTime = std::chrono::steady_clock::now();
				}

				if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) {
					p.rotate(1);
				}

				if (event.key.code == Keyboard::Z) {
					p.rotate(-1);
				}

				if (event.key.code == Keyboard::LShift || event.key.code == Keyboard::RShift) {
					p.hold();
				}
			}
		}

		if (Keyboard::isKeyPressed(Keyboard::S)|| Keyboard::isKeyPressed(Keyboard::Down)) { //accelerate
			timeBetweenDrops /= 4;
		}

		now = std::chrono::steady_clock::now(); //get current time
		delta = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastMove); //time between moves
		if(delta.count() >= moveDelay) {
			if (Keyboard::isKeyPressed(Keyboard::A)|| Keyboard::isKeyPressed(Keyboard::Left)) {
				dx = -1;
				lastMove = std::chrono::steady_clock::now();
			}
			if (Keyboard::isKeyPressed(Keyboard::D)|| Keyboard::isKeyPressed(Keyboard::Right)) {
				if (dx == -1) { //holding both directions
					dx = 0;
				}
				else {
					dx = 1;
					lastMove = std::chrono::steady_clock::now();
				}
			}
		}


		//Calculate movement
		delta = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime); //block gravity
		if (delta.count() >= timeBetweenDrops) {
			startTime = std::chrono::steady_clock::now();
			dy = 1; //drop one
		}

		//move block appropriately
		p.move(dx, dy);

		//Drawing
		window.clear();

		//draw board
		game.draw(t, window);

		//draw player piece
		p.drawGhost(t, window);
		p.drawHold(t, window);
		p.draw(t, window);

		window.display();
	}

	return 0;
}
