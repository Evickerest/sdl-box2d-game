#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <box2d/box2d.h>

#define SPEED 10;

void init();
void gameLoop();
void kill();

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_FRect rect;
b2WorldDef world;

const bool *keys;
const float MS_PER_SECOND = 1000.0 / 60.0;

float x = 100;
float y = 100;
int isRunning = 1;
Uint64 lastTime;
 

int main() {
	init();
	while (isRunning) gameLoop();
	kill();
	return 0;
}

void init() {
	// Create SDL Window
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    if (!SDL_CreateWindowAndRenderer("Game", 640, 480, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        exit(1);
    }

	SDL_PumpEvents();
	keys = SDL_GetKeyboardState(NULL);

	rect.x = x;
	rect.y = y;
	rect.h = rect.w = 100;

	lastTime = SDL_GetTicks();

	// Create Box2d World
	world = b2DefaultWorldDef();
}

void gameLoop() {
	const Uint64 current = SDL_GetTicks();
	const Uint64 elapsed = current - lastTime;
	SDL_Event e;
	SDL_PumpEvents();

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_EVENT_QUIT) {
			isRunning = 0;
		}
	}

	float speed = 0.1 * elapsed; 
	if (keys[SDL_SCANCODE_UP])    y -= speed;
	if (keys[SDL_SCANCODE_DOWN])  y += speed;
	if (keys[SDL_SCANCODE_LEFT])  x -= speed;
	if (keys[SDL_SCANCODE_RIGHT]) x += speed;

	rect.x = x;
	rect.y = y;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderRect(renderer, &rect);
	SDL_RenderPresent(renderer);

	const Uint64 elapsedTime = SDL_GetTicks() - current;
	if (elapsedTime < MS_PER_SECOND) {
		SDL_Delay(MS_PER_SECOND - elapsedTime);
	}
	lastTime = current;
}

void kill() {
	SDL_DestroyWindow(window);
	SDL_Quit();
}
