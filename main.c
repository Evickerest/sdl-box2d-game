#include "game.h" 

int main() {
	int levelStatus;

	initSDL(); 
	initalizeLevel1Objects(); 
	connectSDLtoObjects();
	initBox2D(); 

	// Loop until we quit or level is cleared
	while ((levelStatus = gameLoop()) == 0);
	if (levelStatus == -1) {
		cleanUp();
		return 0;
	}

	cleanLevel();
	initalizeLevel2Objects();
	connectSDLtoObjects();
	initBox2D(); 

	while ((levelStatus = gameLoop()) == 0);
	if (levelStatus == -1) {
		cleanUp();
		return 0;
	}

	cleanLevel();
	initalizeLevel3Objects();
	connectSDLtoObjects();
	initBox2D(); 

	while ((levelStatus = gameLoop()) == 0);

	cleanUp();
	return 0;
}
