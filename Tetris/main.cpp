#include <SFML/Graphics.hpp>
#include <random>;

using namespace sf;

const int BLOCK_TYPES[7][4] = {
	{ 0, 2, 4, 6 }, // I
	{ 0, 1, 2, 3 }, // O
	{ 1, 2, 3, 5 }, // T
	{ 0, 3, 5, 6 }, // L
	{ 0, 1, 2, 4 }, // J
	{ 0, 2, 3, 7 }, // S
	{ 1, 2, 3, 6 }  // Z
};

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

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
		}

		window.clear();

	

		for (int i = 0; i < 4; i++) {
			Sprite block;
			block.setTexture(t);


			block.setTextureRect(IntRect(0, 0, 18, 18));

			block.setPosition((BLOCK_TYPES[type][i] % 2) * 18, (BLOCK_TYPES[type][i] / 2) * 18);
			window.draw(block);
		}

		window.display();
	}

	return 0;
}