#include <SFML/Graphics.hpp>
#include <random>
#include <chrono>
#include <iostream>
using namespace sf;

const int BLOCK_TYPES[7][4] = {
	{ 0, 2, 4, 6 }, // I
	{ 0, 1, 2, 3 }, // O
	{ 1, 2, 3, 5 }, // T
	{ 0, 1, 3, 5 }, // L
	{ 1, 3, 4, 5 }, // J
	{ 0, 2, 3, 5 }, // S
	{ 1, 2, 3, 4 }  // Z
};

const int TILESIZE = 18;
const int BOARD_WIDTH = 15;
const int BOARD_HEIGHT = 25;

//random
std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
std::uniform_int_distribution<int> uni(0,6); // guaranteed unbiased


//CLASSES TODO PUT IN SEPERATE FILES
class Game {
private:
	int board[BOARD_WIDTH][BOARD_HEIGHT]; //stores type values for block
	unsigned int score;

public:
	Game() {
		for (int i = 0; i < BOARD_WIDTH; i++) { //intialize board to -1
			for (int j = 0; j < BOARD_HEIGHT; j++) {
				board[i][j] = -1;
			}
		}
	}

	inline void addScore(unsigned int score) {
		this->score += score;
	}

	//TODO add error checking in these methods
	//Return true if a row is cleared by placing the block
	bool setBlock(Vector2i pos, int type) {
		board[pos.x][pos.y] = type;

		bool rowCleared = true;
		for (int i = 0; i < BOARD_WIDTH; i++) {
			if (board[i][pos.y] == -1) {
				rowCleared = false;
				break;
			}
		}

		//clear row
		if (rowCleared) {
			//TODO CLEAR ROW
			for (int i = 0; i < BOARD_WIDTH; i++) {
				board[i][pos.y] = -1;
			}
		}

		return rowCleared;
	}

	inline int getBlock(Vector2i pos) {
		return board[pos.x][pos.y];
	}

	//Handles graphics for every frame
	void draw(Texture& t, RenderWindow& window) {
		for (int i = 0; i < BOARD_WIDTH; i++) {
			for (int j = 0; j < BOARD_HEIGHT; j++) {
				int type = board[i][j];

				if (type != -1) { //not a block
					Sprite block;
					block.setTexture(t);

					block.setTextureRect(IntRect(type * TILESIZE, 0, TILESIZE, TILESIZE));

					block.setPosition(i * TILESIZE, j * TILESIZE);
					window.draw(block);
				}
			}
		}
	}
};

class Player {
private:
	int type;
	Vector2i blockPos[4];
	Game* g;

public:
	Player(Game* game) {
		g = game;
		newBlock();
	}

	void newBlock() {
		type = uni(rng);
		for (int i = 0; i < 4; i++) {
			int n = BLOCK_TYPES[type][i];
			blockPos[i].x = BOARD_WIDTH / 2 + n % 2;
			blockPos[i].y = n / 2;
		}
	}

	void rotate(int dir) { //-1 is left, 1 is right
		if (type == 1) //O
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
			if (g->getBlock(temp[i]) != -1) //block in way
				return;
		}

		for (int i = 0; i < 4; i++) { //commit temp to block pos
			blockPos[i] = temp[i];
		}
	}


	void hardDrop() {
		Vector2i dy(0, -1);
		Vector2i below(0, 1);
		bool canMoveY = true;
		while (canMoveY) {
			dy.y++;
			for (int i = 3; i >= 0; i--) { //loop bottom to top
				if ((blockPos[i] + dy).y == BOARD_HEIGHT - 1 || g->getBlock(blockPos[i] + dy + below) != -1) {
					canMoveY = false;
					break;
				}
			}
		}

		//Place block
		int linesCleared = 0;
		for (int i = 0; i < 4; i++) {
				linesCleared += (int) g->setBlock(blockPos[i] + dy, type);
		}
		//TODO ADD TO SCORE

		newBlock();
	}

	void move(int dx, int dy) {

		bool canMoveX = true;
		bool canMoveY = true;

		for (int i = 0; i < 4; i++) {
			Vector2i vx(dx, 0);
			Vector2i vy(0, dy);
			if (g->getBlock(blockPos[i] + vx) != -1 || blockPos[i].x + dx > BOARD_WIDTH - 1 || blockPos[i].x + dx < 0) //block in way horizontally
				canMoveX = false;
			if (g->getBlock(blockPos[i] + vy) != -1 || (blockPos[i].y + dy) == BOARD_HEIGHT) //block in way vertically or hit floor
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
			//Place block down
			int linesCleared = 0;
			for (int i = 0; i < 4; i++) {
				linesCleared += (int) g->setBlock(blockPos[i], type);
			}
			//TODO ADD TO SCORE
			//Get new block
			newBlock();
		}

	}

	void draw(Texture& t, RenderWindow& window) {
		for (int i = 0; i < 4; i++) {
			Sprite block;

			block.setTexture(t);

			//Slices specific color we want
			block.setTextureRect(IntRect(type * TILESIZE, 0, TILESIZE, TILESIZE));

			Vector2i v = blockPos[i];

			block.setPosition(v.x * TILESIZE, v.y * TILESIZE);

			window.draw(block);
		}
	}
};

int main() {
	RenderWindow window(VideoMode(TILESIZE * BOARD_WIDTH, TILESIZE * BOARD_HEIGHT), "Tetris");

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
	while (window.isOpen()) {
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
				}

				if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) {
					p.rotate(1);
				}

				if (event.key.code == Keyboard::Z) {
					p.rotate(-1);
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

		//draw player piece
		p.draw(t, window);

		//draw board
		game.draw(t, window);

		window.display();
	}

	return 0;
}
