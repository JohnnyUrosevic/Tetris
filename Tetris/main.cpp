#include <SFML/Graphics.hpp>

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

int main()
{
	RenderWindow window(VideoMode(500, 800), "SFML works!");
	
	Texture t;
	t.loadFromFile("../Textures/tiles.png");
	
	Sprite block;
	block.setTexture(t);

	block.setTextureRect(IntRect(0, 0, 18, 18));

	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
		}

		window.clear();

		window.draw(block);
		window.display();
	}

	return 0;
}