#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <box2d/box2d.h>

#define NUM_BODIES 5

void initGameObjects();
void initSDL();
void initBox2D();
void gameLoop();
void kill();

struct World {
	SDL_Window *window;
	SDL_Renderer *renderer;
	b2WorldId worldId;
	const bool *keys;
};

struct Color {
	int r;
	int g;
	int b;
	int a;
};

struct Object {
	float x;
	float y;
	float w;
	float h;
	int isStatic;
	SDL_FRect rect;
	b2BodyId bodyId;
	b2Polygon polygon;
	struct Color color;
};

const float MS_PER_SECOND = 1000.0 / 60.0;
const float PIXELS_PER_METER = 32.0f;
const int WIDTH = 640;
const int HEIGHT = 480;
struct World world;
struct Object staticObjects[NUM_BODIES];
int isRunning = 1;
Uint64 lastTime;

// Convert SDL Pixel unit to Box 2d Meter Unit
float pixelToMeter(const float value) {
	return value / PIXELS_PER_METER;
}

// Convert box2d meter unit to SDL pixel unit
float meterToPixel(const float value) {
	return value * PIXELS_PER_METER;
}

b2Vec2 box2DToSDL(b2Vec2 vector, struct Object *object) {
	vector.x = meterToPixel(vector.x) - object->w / 2;
	vector.y = HEIGHT - meterToPixel(vector.y) - object->h / 2;
	return vector;
}

b2Vec2 SDLToBox2D(b2Vec2 vector, struct Object *object) {
	vector.x = pixelToMeter(vector.x + object->w / 2); 
	vector.y = pixelToMeter(HEIGHT - vector.y + object->h / 2);
	return vector;
}

b2Vec2 Box2DXYToSDL(float x, float y) {
	return (b2Vec2){meterToPixel(x), HEIGHT - meterToPixel(y)};
}

b2Vec2 SDLXYToBox2D(float x, float y) {
	return (b2Vec2){pixelToMeter(x), pixelToMeter(HEIGHT - y)};
}

// Convert SDL x,y position to Box2D x,y position
b2Vec2 SDLPositionToBox2D(struct Object *object) {
	float x = (object->w / 2) + object->x;
	float y = (object->h / 2) + object->y;
	return SDLXYToBox2D(x, y);
}

// Convert Box2D x,y position to SDL x,y position
b2Vec2 SDLSizeToBox2D(struct Object *object) {
	float w = object->w / 2;
	float h = object->h / 2;
	return (b2Vec2){pixelToMeter(w), pixelToMeter(h)};
}

int main() {
	initGameObjects(); // Create game objects
	initBox2D(); // Initalize Box2D world and FRects
	initSDL(); // Initialize SDL window
	while (isRunning) gameLoop(); // Game loop, get inputs, update, render
	kill(); // Clean Up resources
	return 0;
}

void initGameObjects() {
	struct Object square, ground, platform1, platform2, platform3;

	// Units are in pixels
	square.x = 100;
	square.y = 100;
	square.w = 50;
	square.h = 50;
	square.isStatic = 0;
	square.color = (struct Color){255, 0, 0, 255};

	ground.w = WIDTH;
	ground.h = 20;
	ground.x = 0;
	ground.y = HEIGHT - ground.h;
	ground.isStatic = 1;
	ground.color = (struct Color){0, 255, 0, 255};

	platform1.w = 200;
	platform1.h = 20;
	platform1.x = 200;
	platform1.y = 300;
	platform1.isStatic = 1;
	platform1.color = (struct Color){0, 0, 255, 255};

	platform2.w = 100;
	platform2.h = 20;
	platform2.x = 20;
	platform2.y = 400;
	platform2.isStatic = 1;
	platform2.color = (struct Color){0, 0, 255, 255};

	platform3.w = 200;
	platform3.h = 40;
	platform3.x = 400;
	platform3.y = 100;
	platform3.isStatic = 1;
	platform3.color = (struct Color){0, 0, 255, 255};

	staticObjects[0] = square;
	staticObjects[1] = ground;
	staticObjects[2] = platform1;
	staticObjects[3] = platform2;
	staticObjects[4] = platform3;
}

void initSDL() {
	// Initalize the SDL library
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    if (!SDL_CreateWindowAndRenderer("Game", WIDTH, HEIGHT, 0, &world.window, &world.renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        exit(1);
    }

	// Set the defined pixel units in initGameObjects() to SDL Frect 
	for (int i = 0; i < NUM_BODIES; i++) {
		staticObjects[i].rect.x = staticObjects[i].x;
		staticObjects[i].rect.y = staticObjects[i].y;
		staticObjects[i].rect.w = staticObjects[i].w;
		staticObjects[i].rect.h = staticObjects[i].h;
	}

	// Gets a pointer to an array that defines what keys are being pressed
	world.keys = SDL_GetKeyboardState(NULL);
	lastTime = SDL_GetTicks();
}

void initBox2D() {
	// Create Box2d World
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = (b2Vec2){0.0f, -10.0f};
	world.worldId = b2CreateWorld(&worldDef);

	// Create Static bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		// Create Body definition
		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.position = SDLPositionToBox2D(&staticObjects[i]);

		if (staticObjects[i].isStatic == 0) bodyDef.type = b2_dynamicBody;

		// Create Body
		staticObjects[i].bodyId = b2CreateBody(world.worldId, &bodyDef);
		b2Vec2 size = SDLSizeToBox2D(&staticObjects[i]);
		staticObjects[i].polygon = b2MakeBox(size.x, size.y);

		// Create Polygon Shape
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.friction = 0.1f;

		b2CreatePolygonShape(staticObjects[i].bodyId, &shapeDef, &staticObjects[i].polygon);
	}
}

void gameLoop() {
	const Uint64 current = SDL_GetTicks();
	const Uint64 elapsed = current - lastTime;
	SDL_Event e;
	SDL_PumpEvents();

	// Poll for Events
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_EVENT_QUIT) {
			isRunning = 0;
		}
	}

	const b2BodyId playerId = staticObjects[0].bodyId; 

	// Check if an arrow key is being pressed, if so, apply force in that direction
	if (world.keys[SDL_SCANCODE_UP]) {
		b2Vec2 force = {0, 3 * elapsed};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_DOWN]) {
		b2Vec2 force = {0, -0.9 * elapsed};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_LEFT]) {
		b2Vec2 force = {-0.9 * elapsed, 0};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_RIGHT]) {
		b2Vec2 force = {1 * elapsed, 0};
		b2Body_ApplyForceToCenter(playerId, force, true); 
	}

	// Step physics simulation
	b2World_Step(world.worldId, 1.0f / 60.0f, 4);

	// Render background
	SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(world.renderer);

	// Draw Static bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		// Update position
		b2Vec2 position = box2DToSDL(b2Body_GetPosition(staticObjects[i].bodyId), &staticObjects[i]);
		staticObjects[i].rect.x = position.x;
		staticObjects[i].rect.y = position.y;
		struct Color color = staticObjects[i].color;

		SDL_SetRenderDrawColor(world.renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(world.renderer, &staticObjects[i].rect);
	}

	// Display Screen
	SDL_RenderPresent(world.renderer);

	// Wait for frame
	const Uint64 elapsedTime = SDL_GetTicks() - current;
	if (elapsedTime < MS_PER_SECOND) {
		SDL_Delay(MS_PER_SECOND - elapsedTime);
	}
	lastTime = current;
}

void kill() {
	// Clean up SDL
	SDL_DestroyRenderer(world.renderer);
	SDL_DestroyWindow(world.window);
	SDL_Quit();
}
