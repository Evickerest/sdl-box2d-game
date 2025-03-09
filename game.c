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

void initGameObjects();
void initSDL();
void initBox2D();
void gameLoop();
void kill();

const float MS_PER_SECOND = 1000.0 / 60.0;
const float PIXELS_PER_METER = 32.0f;
const int WIDTH = 640;
const int HEIGHT = 480;

struct World {
	SDL_Window *window;
	SDL_Renderer *renderer;
	b2WorldId worldId;
	const bool *keys;
};

struct Object {
	float x;
	float y;
	float w;
	float h;
	SDL_FRect rect;
	b2BodyId bodyId;
	b2Polygon polygon;
};

struct World world;
struct Object ground;
struct Object square;

int isRunning = 1;
Uint64 lastTime;

float pixelToMeter(const float value) {
	return value / PIXELS_PER_METER;
}

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

b2Vec2 SDLPositionToBox2D(struct Object *object) {
	float x = (object->w / 2) + object->x;
	float y = (object->h / 2) + object->y;
	return SDLXYToBox2D(x, y);
}

b2Vec2 SDLSizeToBox2D(struct Object *object) {
	float w = object->w / 2;
	float h = object->h / 2;
	return (b2Vec2){pixelToMeter(w), pixelToMeter(h)};
}

int main() {
	initGameObjects();
	initBox2D();
	initSDL();
	while (isRunning) gameLoop();
	kill();
	return 0;
}

void initGameObjects() {
	// Units are in pixels
	square.x = square.y = 100;
	square.w = square.h = 50;

	ground.w = WIDTH;
	ground.h = 20;
	ground.x = 0;
	ground.y = HEIGHT - ground.h;
}

void initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    if (!SDL_CreateWindowAndRenderer("Game", WIDTH, HEIGHT, 0, &world.window, &world.renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        exit(1);
    }

	world.keys = SDL_GetKeyboardState(NULL);

	square.rect.x = square.x;
	square.rect.y = square.y;
	square.rect.w = square.w;
	square.rect.h = square.h;

	ground.rect.x = ground.x;
	ground.rect.y = ground.y;
	ground.rect.w = ground.w;
	ground.rect.h = ground.h;

	lastTime = SDL_GetTicks();
}

void initBox2D() {
	// Create Box2d World
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = (b2Vec2){0.0f, -10.0f};

	world.worldId = b2CreateWorld(&worldDef);

	// Create Ground 
	b2BodyDef groundBodyDef =  b2DefaultBodyDef();
	groundBodyDef.position = SDLPositionToBox2D(&ground); 

	ground.bodyId = b2CreateBody(world.worldId, &groundBodyDef);
	b2Vec2 groundSize = SDLSizeToBox2D(&ground);
	ground.polygon = b2MakeBox(groundSize.x, groundSize.y);

	b2ShapeDef groundShapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(ground.bodyId, &groundShapeDef, &ground.polygon);

	// Create Dyanmic square
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = SDLPositionToBox2D(&square); 

	square.bodyId = b2CreateBody(world.worldId, &bodyDef);
	b2Vec2 squareSize = SDLSizeToBox2D(&square);
	square.polygon = b2MakeBox(squareSize.x, squareSize.y);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.friction = 0.1f;

	b2CreatePolygonShape(square.bodyId, &shapeDef, &square.polygon);
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


	if (world.keys[SDL_SCANCODE_UP]) {
		b2Vec2 force = {0, 3 * elapsed};
		b2Body_ApplyForceToCenter(square.bodyId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_DOWN]) {
		b2Vec2 force = {0, -0.9 * elapsed};
		b2Body_ApplyForceToCenter(square.bodyId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_LEFT]) {
		b2Vec2 force = {-0.9 * elapsed, 0};
		b2Body_ApplyForceToCenter(square.bodyId, force, true); 
	}
	if (world.keys[SDL_SCANCODE_RIGHT]) {
		b2Vec2 force = {1 * elapsed, 0};
		b2Body_ApplyForceToCenter(square.bodyId, force, true); 
	}

	b2World_Step(world.worldId, 1.0f / 60.0f, 4);

	b2Vec2 position = box2DToSDL(b2Body_GetPosition(square.bodyId), &square);

	square.rect.x = position.x;
	square.rect.y = position.y;

	SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(world.renderer);
	SDL_SetRenderDrawColor(world.renderer, 255, 0, 0, 0);
	SDL_RenderFillRect(world.renderer, &square.rect);

	// Draw Floor
	SDL_SetRenderDrawColor(world.renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(world.renderer, &ground.rect);

	SDL_RenderPresent(world.renderer);

	const Uint64 elapsedTime = SDL_GetTicks() - current;
	if (elapsedTime < MS_PER_SECOND) {
		SDL_Delay(MS_PER_SECOND - elapsedTime);
	}
	lastTime = current;
}

void kill() {
	SDL_DestroyRenderer(world.renderer);
	SDL_DestroyWindow(world.window);
	SDL_Quit();
}
