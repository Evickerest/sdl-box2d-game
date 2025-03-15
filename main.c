#include "game.h" 

int main() {
	initGameObjects(); 
	initBox2D(); 
	initSDL(); 
	while (gameLoop()); 
	kill(); 
	return 0;
}
