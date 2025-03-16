#include "game.h" 

int main() {
	int levelStatus;
	initGameObjects(); 
	initBox2D(); 
	initSDL(); 

	while ((levelStatus = gameLoop()) == 0); 
	if (levelStatus == -1) {
		kill();
		return 0;
	}

	return 0;
}
