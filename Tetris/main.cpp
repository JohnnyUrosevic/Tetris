#include <SFML/Graphics.hpp>
#include <random>
#include <cmath>
#include <chrono>

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
const int BOARD_WIDTH = 27;
const int BOARD_HEIGHT = 44;

//random
std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
std::uniform_int_distribution<int> uni(0,6); // guaranteed unbiased

void getBlockXY(int i, int type, bool rotated, bool flipped, int playerX, int playerY, int& x, int& y);

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
	
	int type = uni(rng); //randomly generate block

	int playerX = 0;
	int playerY = 0;

	int dx;
	int dy;

	bool rotated = false;
	bool flipped = false;

	//clock
	std::chrono::steady_clock::time_point now;
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

	std::chrono::duration<double> delta;

	double timeBetweenDrops;


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

				}

				if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) {
					//flipped = true;
					rotated = !rotated; //flip value of rotated
				}

				if (event.key.code == Keyboard::A || event.key.code == Keyboard::Left) {
					if (playerX != 0)
						dx = -1;
				}

				if (event.key.code == Keyboard::D || event.key.code == Keyboard::Right) {
					if (playerX + 1 < BOARD_WIDTH - 1 || (type == 0 && playerX < BOARD_WIDTH - 1))
						dx = 1;
				}
			}
		}

		if (Keyboard::isKeyPressed(Keyboard::S )|| Keyboard::isKeyPressed(Keyboard::Down)) { //accelerate
			timeBetweenDrops /= 4;
		}


		now = std::chrono::steady_clock::now();
		delta = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime);
		if (delta.count() >= timeBetweenDrops) {
			startTime = std::chrono::steady_clock::now();
			dy = 1; //drop one
		}

		//move block

		bool canMoveX = true;
		bool canMoveY = true;

		for (int i = 0; i < 4; i++) {
				int x;
				int y;
				getBlockXY(i, type, rotated, flipped, playerX, playerY, x, y);
				if (board[x + dx][y] != -1) //block in way horizontally
					canMoveX = false;
				if (board[x][y + dy] != -1 || (y + dy) == BOARD_HEIGHT) //block in way vertically or hit floor
					canMoveY = false;
		}

		if (canMoveX) { playerX += dx; }
		if (canMoveY) { 
			playerY += dy; 
		}
		else {
			//Place block down
			for (int i = 0; i < 4; i++) {
				int x;
				int y;
				getBlockXY(i, type, rotated, flipped, playerX, playerY, x, y);
				board[x][y] = type;
			}
			//get new block
			type = uni(rng);
			playerX = playerY = 0;
		}

		//Drawing
		window.clear();


		//draw player piece
		for (int i = 0; i < 4; i++) {
			Sprite block;
			block.setTexture(t);

			block.setTextureRect(IntRect(type * TILESIZE, 0, TILESIZE, TILESIZE));

			int x;
			int y;
			getBlockXY(i, type, rotated, flipped, playerX, playerY, x, y);

			block.setPosition(x * TILESIZE, y * TILESIZE);

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


void getBlockXY(int i, int type, bool rotated, bool flipped, int playerX, int playerY, int& x, int& y) {
	int n = BLOCK_TYPES[type][i];
	int flipfactor = 0;

	if (flipped) { 
		flipfactor = (n % 2 == 1) ? -1 : 1;
	}

	if (rotated) {
		x = playerX + n / 2;
		y = playerY + (n + flipfactor) % 2;
	}
	else {
		x = playerX + (n + flipfactor) % 2;
		y = playerY + n / 2;
	}
}