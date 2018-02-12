#include <SFML/Graphics.hpp>
#include <random>;

using namespace sf;

const int BLOCK_TYPES[7][4] = {
	{ 0, 2, 4, 6 }, // I
	{ 0, 1, 2, 3 }, // O
	{ 1, 2, 3, 5 }, // T
	{ 1, 3, 4, 5 }, // L
	{ 0, 1, 2, 4 }, // J
	{ 0, 2, 3, 5 }, // S
	{ 1, 2, 3, 4 }  // Z
};

const int TILESIZE = 18;

//random
std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
std::uniform_int_distribution<int> uni(0,6); // guaranteed unbiased

int main()
{
	RenderWindow window(VideoMode(500, 800), "SFML works!");
	
	Texture t;
	t.loadFromFile("../Textures/tiles.png");
	
	int type = uni(rng); //randomly generate block

	int x, y; //block position
	x = y = 0;

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Space) {
					type = uni(rng);
				}

				if (event.key.code == Keyboard::A || event.key.code == Keyboard::Left) {
					if (x > 0) {
						x--;
					}
				}

				if (event.key.code == Keyboard::D || event.key.code == Keyboard::Right) {
					if (x < 26 || (x == 26 && type == 0)) {
						x++;
					}
				}
			}

		}

		window.clear();


		for (int i = 0; i < 4; i++) {
			Sprite block;
			block.setTexture(t);


			block.setTextureRect(IntRect(type * TILESIZE, 0, TILESIZE, TILESIZE));

			block.setPosition((x + BLOCK_TYPES[type][i] % 2) * 18, (y + BLOCK_TYPES[type][i] / 2) * 18);
			window.draw(block);
		}

		window.display();
	}

	return 0;
}