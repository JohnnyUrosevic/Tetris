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

struct Player {
	int type;
	Vector2i blockPos[4];

	Player() {
		newBlock();
	}

	void newBlock() {
		type = uni(rng);
		for (int i = 0; i < 4; i++) {
			int n = BLOCK_TYPES[type][i];
			blockPos[i].x = BOARD_WIDTH / 2 + n % 2;
			blockPos[i].y = n / 2;
			std::cout << blockPos[i].x << blockPos[i].y << std::endl;
		}
	}

	void rotate(int board[BOARD_WIDTH][BOARD_HEIGHT], int dir) { //-1 is left, 1 is right
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
			if (board[temp[i].x][temp[i].y] != -1) //block in way
				return;
		}

		for (int i = 0; i < 4; i++) { //commit temp to block pos
			blockPos[i] = temp[i];
		}
	}


	void hardDrop(int board[BOARD_WIDTH][BOARD_HEIGHT]) {
		int dy = 0;
		bool canMoveY = true;
		while (canMoveY) {
			dy++;
			for (int i = 3; i >= 0; i--) { //loop bottom to top
				Vector2i v = blockPos[i];
				std::cout << v.y + dy << std::endl;
				if (v.y + dy == BOARD_HEIGHT - 1 || board[v.x][v.y+dy+1] != -1) {
					canMoveY = false;
					break;
				}
			}
		}

		//place block
		for (int i = 0; i < 4; i++) {
			Vector2i v = blockPos[i];
			board[v.x][v.y + dy] = type;
		}

		newBlock();
	}
};


int main()
{
	int board[BOARD_WIDTH][BOARD_HEIGHT]; //stores type values for block

	for (int i = 0; i < BOARD_WIDTH; i++) { //intialize board to -1
		for (int j = 0; j < BOARD_HEIGHT; j++) {
			board[i][j] = -1;
		}
	}

	RenderWindow window(VideoMode(TILESIZE * BOARD_WIDTH, TILESIZE * BOARD_HEIGHT), "Tetris");

	Texture t;
	t.loadFromFile("../Textures/tiles.png");

	Player p;

	int dx;
	int dy;

	bool rotated = false;
	bool flipped = false;

	//clock
	std::chrono::steady_clock::time_point now;
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point lastMove = std::chrono::steady_clock::now();

	std::chrono::duration<double> delta;

	double timeBetweenDrops;
	double moveDelay = .09;

	while (window.isOpen())
	{
		timeBetweenDrops = .25;
		dx = 0;
		dy = 0;

		//Input

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Space) {
					p.hardDrop(board);
				}

				if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) {
					p.rotate(board, 1);
				}

				if (event.key.code == Keyboard::Z) {
					p.rotate(board, -1);
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

		delta = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime); //block gravity
		if (delta.count() >= timeBetweenDrops) {
			startTime = std::chrono::steady_clock::now();
			dy = 1; //drop one
		}

		//move block

		bool canMoveX = true;
		bool canMoveY = true;

		for (int i = 0; i < 4; i++) {
			Vector2i v = p.blockPos[i];
			if (board[v.x + dx][v.y] != -1 || v.x + dx > BOARD_WIDTH - 1 || v.x + dx < 0) //block in way horizontally
				canMoveX = false;
			if (board[v.x][v.y + dy] != -1 || (v.y + dy) == BOARD_HEIGHT) //block in way vertically or hit floor
				canMoveY = false;
		}

		if (canMoveX) {
			for (int i = 0; i < 4; i++) {
				p.blockPos[i].x += dx;
			}
		}
		if (canMoveY) {
			for (int i = 0; i < 4; i++) {
				p.blockPos[i].y += dy;
			}
		}
		else {
			//Place block down
			for (int i = 0; i < 4; i++) {
				Vector2i v = p.blockPos[i];
				board[v.x][v.y] = p.type;
			}
			//get new block
			p.newBlock();
		}

		//Drawing
		window.clear();


		//draw player piece
		for (int i = 0; i < 4; i++) {
			Sprite block;
			block.setTexture(t);

			block.setTextureRect(IntRect(p.type * TILESIZE, 0, TILESIZE, TILESIZE));

			Vector2i v = p.blockPos[i];

			block.setPosition(v.x * TILESIZE, v.y * TILESIZE);

			window.draw(block);
		}

		//draw board
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

		window.display();
	}

	return 0;
}
