#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>
#include <box2d/box2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h" 
#include "utils.h"
#include "render.h"

#define NUM_BODIES 19

World world;
Player player;
Level level1;
Object objects[NUM_BODIES];

void initGameObjects() {
	Object square, ground, platform1, platform2, platform3, platform4, platform5, platform6, platform7;
	Object platform8, platform9, platform10;
	Object box1, box2, box3, wallleft, wallright;
	Object circle1, circle2;

	level1.levelWidth = WIDTH * 3;
	level1.levelHeight = HEIGHT * 2;
 	level1.cameraLeftOffset = (float)WIDTH / 2;
	level1.cameraRightOffset = level1.levelWidth - level1.cameraLeftOffset;
	level1.cameraBottomOffset = (float)HEIGHT / 2;
	level1.cameraTopOffset = level1.levelHeight - level1.cameraBottomOffset;
	level1.levelStatus = 0;

	// Units are in pixels
	square.p = (p){ 100, 100 + HEIGHT, 50, 50};
	square.type = DYNAMIC;
	square.color = (Color){255, 0, 0, 255};

	ground.p = (p){0,HEIGHT - 20 + HEIGHT,level1.levelWidth,20};
	ground.type = STATIC;
	ground.color = (Color){0, 255, 0, 255};

	platform1.p = (p){200,300 + HEIGHT,200,20};
	platform1.type = STATIC;
	platform1.color = (Color){0, 255, 255, 255};

	platform2.p = (p){40,400 + HEIGHT,100,20};
	platform2.type = STATIC;
	platform2.color = (Color){0, 0, 255, 255};

	platform3.p = (p){400,100 + HEIGHT,200,40};
	platform3.type = STATIC;
	platform3.color = (Color){0, 0, 255, 255};

	platform4.p = (p){400,50 + HEIGHT,50,50};
	platform4.type = DYNAMIC;
	platform4.color = (Color){255, 20, 255, 255};

	platform5.p = (p){175,190 + HEIGHT,150,20};
	platform5.type = STATIC;
	platform5.color = (Color){125, 255, 30, 255};

	platform6.p = (p){level1.levelWidth - 20,0,20,level1.levelHeight};
	platform6.type = STATIC;
	platform6.color = (Color){20, 255, 100, 255};

	platform7.p = (p){0,0,20,level1.levelHeight};
	platform7.type = STATIC;
	platform7.color = (Color){20, 255, 100, 255};

	platform8.p = (p){WIDTH + 200,400 + HEIGHT,200,50};
	platform8.type = STATIC;
	platform8.color = (Color){40, 80, 90, 255};

	platform9.p = (p){2 * WIDTH + 200,450 + HEIGHT,100,10};
	platform9.type = STATIC;
	platform9.color = (Color){40, 80, 90, 255};

	platform10.p = (p){2 * WIDTH + 400,350 + HEIGHT,40,40};
	platform10.type = STATIC;
	platform10.color = (Color){40, 80, 90, 255};

	box1.p = (p){WIDTH + 250,300 + HEIGHT,50,50};
	box1.type = DYNAMIC;
	box1.color = (Color){255, 0, 255, 255};

	box2.p = (p){WIDTH + 250,250 + HEIGHT,50,50};
	box2.type = DYNAMIC;
	box2.color = (Color){255, 40, 255, 255};

	box3.p = (p){WIDTH + 250,200 + HEIGHT,50,50};
	box3.type = DYNAMIC;
	box3.color = (Color){255, 0, 255, 255};

	wallleft.p = (p){WIDTH * 2,0,10,2 * HEIGHT - 100};
	wallleft.type = STATIC;
	wallleft.color = (Color){255, 50, 50, 255};

	wallright.p = (p){WIDTH * 2 + 100,0,10,2 * HEIGHT - 100};
	wallright.type = STATIC;
	wallright.color = (Color){50, 120, 255, 255};

	circle1.p = (p){300, HEIGHT * 2 - 50, 50, 50};
	circle1.type = COLLECTIBLE;
	circle1.color = (Color){255, 255, 0, 0};

	circle2.p = (p){350, HEIGHT * 2 - 50, 50, 50};
	circle2.type = COLLECTIBLE;
	circle2.color = (Color){255, 255, 0, 0};

	player.canJump = false;
	player.maxVelocityX = 10.0f;
	player.jumpBuffer = 0;
	player.bufferFrames = 10;
	player.xForce = 2.0f;
	player.yForce = 3.0f;

	objects[0] = square;
	objects[1] = ground;
	objects[2] = platform1;
	objects[3] = platform2;
	objects[4] = platform3;
	objects[5] = platform4;
	objects[6] = platform5;
	objects[7] = platform6;
	objects[8] = platform7;
	objects[9] = platform8;
	objects[10] = platform9;
	objects[11] = platform10;
	objects[12] = box1;
	objects[13] = box2;
	objects[14] = box3;
	objects[15] = wallleft;
	objects[16] = wallright;
	objects[17] = circle1;
	objects[18] = circle2;

	for (int i = 0; i < NUM_BODIES; i++) objects[i].draw = true;
}

void initSDL() {
	// Initalize the SDL library
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

	// Creates Window
    if (!SDL_CreateWindowAndRenderer("Game", WIDTH, HEIGHT, 0, &world.window, &world.renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        exit(1);
    }

	// Set the defined pixel units in initGameObjects() to SDL Frect 
	for (int i = 0; i < NUM_BODIES; i++) {
		objects[i].rect.x = objects[i].p.x;
		objects[i].rect.y = objects[i].p.y;
		objects[i].rect.w = objects[i].p.w;
		objects[i].rect.h = objects[i].p.h;
	}

	// Gets a pointer to an array that defines what keys are being pressed
	world.keys = SDL_GetKeyboardState(NULL);
	world.lastTime = SDL_GetTicks();
}

void initBox2D() {
	// Create Box2d World
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = (b2Vec2){0.0f, -10.0f};
	world.worldId = b2CreateWorld(&worldDef);

	// Create Static bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		Object* obj = &objects[i];

		// Create Body definition
		b2BodyDef bodyDef = b2DefaultBodyDef();

		// SDL position and Box2D positions are different, so we convert between our pixel positioning to
		// Box2D positioning here
		bodyDef.position = SDLPositionToBox2D(obj);

		// If dealing with a dynamic object, tell box2D we need physics!!!
		if (obj->type == DYNAMIC) bodyDef.type = b2_dynamicBody;

		// Create Body
		obj->bodyId = b2CreateBody(world.worldId, &bodyDef);

		// Convert between SDL pixel to Box2D meter
		b2Vec2 size = SDLSizeToBox2D(obj);

		// Set mass Data
		b2MassData mass;
		mass.mass = 23.0f;
		b2Body_SetMassData(obj->bodyId, mass); 

		// Create Polygon Shape
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.friction = 0.5f;

		// Add shape to polygon depending on object type
		if (obj->type != COLLECTIBLE) {
			// Make object shape depending on what type of object we are dealing with
			obj->polygon = b2MakeBox(size.x, size.y);

			// Add polygon box to our shape
			obj->shapeId = b2CreatePolygonShape(obj->bodyId, &shapeDef, &obj->polygon);
			
		} else {
			// Create circle
			b2Circle circle;
			circle.radius = size.x; 
			circle.center = (b2Vec2){0,0}; 

			// Set is sensor to turn of collisions
			shapeDef.isSensor = true;
			shapeDef.userData = "collectible";

			// Add circle to our shape
			obj->shapeId = b2CreateCircleShape(obj->bodyId, &shapeDef, &circle);
		}

		// If the object is static, add a ground and 2 wall sensors to detect
		// what side of the object we have hit
		if (obj->type == STATIC) {
			b2ShapeDef ground = b2DefaultShapeDef();
			b2ShapeDef lwall = b2DefaultShapeDef();
			b2ShapeDef rwall = b2DefaultShapeDef();

			ground.isSensor = lwall.isSensor = rwall.isSensor = true;
			lwall.userData = rwall.userData = "wall";
			ground.userData = "ground";

			// This creates a shape that is offset from the center of the main body
			// That "1" in the b2Rot took me like 2 hours to figure out :(
			b2Polygon groundPol = b2MakeOffsetBox(size.x * .95, size.y * 0.1, (b2Vec2){0, size.y * .9}, (b2Rot){1, 0});
			b2Polygon lwallPol = b2MakeOffsetBox(size.x * 0.1, size.y * 0.95, (b2Vec2){-size.x * 0.9, 0}, (b2Rot){1, 0});
			b2Polygon rwallPol = b2MakeOffsetBox(size.x * 0.1, size.y * 0.1, (b2Vec2){size.x + 0.9, 0}, (b2Rot){1, 0});

			// Add sensors to polygon
			b2CreatePolygonShape(obj->bodyId, &ground, &groundPol);
			b2CreatePolygonShape(obj->bodyId, &lwall, &lwallPol);
			b2CreatePolygonShape(obj->bodyId, &rwall, &rwallPol);
		} 
	}
}

// Handles game inputs, returns a vector of the desired player velocity
void handleInputs(double elapsed) {
	// Pump events gets us the next events for the game
	SDL_PumpEvents();
	SDL_Event e;

	// Poll for Events
	while (SDL_PollEvent(&e) != 0) {
		// Quit game
		if (e.type == SDL_EVENT_QUIT) level1.levelStatus = -1;
	}

	// Get player velocity
	const b2BodyId playerId = objects[0].bodyId; 
	b2Vec2 velocity = b2Body_GetLinearVelocity(playerId);

	// Define desired force
	b2Vec2 force = {0, 0};

	// If holding up, and we can jump, then jump
	// The jump buffer makes it so that we can have a dynamic jump boost based on 
	// how long you hold up, to a certain fram elimit
	if (world.keys[SDL_SCANCODE_UP] && (player.canJump || player.jumpBuffer > 0)) {
		player.canJump = false;
		player.jumpBuffer--;
		force.y += player.yForce * elapsed;
	} 

	// Move Left
	if (world.keys[SDL_SCANCODE_LEFT]) {
		force.x += -player.xForce * elapsed;
	}

	// Move Right
	if (world.keys[SDL_SCANCODE_RIGHT]) {
		force.x += player.xForce * elapsed;
	}

	// If we are moving left or right, and that is greater than our max x velocity, then
	// set the x force to 0, i.e., don't move in x axis
	bool canMoveX = !((force.x < 0 && velocity.x < -player.maxVelocityX) || (force.x > 0 && velocity.x > player.maxVelocityX));

	if (!canMoveX) force.x = 0;
	player.desiredVelocity = force;
}

// Locate collectible object and set draw to false so we don't draw it
void clearCollectible(b2ShapeId shapeId) {
	for(int i = 0; i < NUM_BODIES; i++) {
		if (objects[i].shapeId.index1 == shapeId.index1) {
			objects[i].draw = false;
			break;
		}
	}
}

void handlePhysics() {
	const b2BodyId playerId = objects[0].bodyId;

	// Apply desired force caluclated from handleInputs() to player
	b2Body_ApplyForceToCenter(playerId, player.desiredVelocity, true);

	// Get the sensor events in the world, i.e., if we hit a ground or wall sensor
	b2SensorEvents sensorEvents = b2World_GetSensorEvents(world.worldId);
	
	// A bit hacky but for some reason why box 2d starts it says we have hit a bunch
	// of objects, so this is to give some buffer space between when the game starts
	// and the player is able to do anything
	if (SDL_GetTicks() > 500) {

	// Go through all the objects we are collided with
	for (int i = 0; i < sensorEvents.beginCount; i++) {
		b2SensorBeginTouchEvent* beginTouch = sensorEvents.beginEvents + i;
		char* sensor = b2Shape_GetUserData(beginTouch->sensorShapeId);

		// If we touch sensor object with tag "ground", then we can jump again
		if (strcmp(sensor, "ground") == 0) {
			player.canJump = true;
			player.jumpBuffer = player.bufferFrames;
		}

		// If we touch a collectible
		if (strcmp(sensor, "collectible") == 0) {
			clearCollectible(beginTouch->sensorShapeId);
		}

		// If we touch a wall sensor and holding shift
		if (strcmp(sensor, "wall") == 0 && world.keys[SDL_SCANCODE_LSHIFT]) {
		}
	}

	// Go through all the objects we are leaving be colided with
	for (int i = 0; i < sensorEvents.endCount; i++) {
		b2SensorEndTouchEvent* endTouch = sensorEvents.endEvents + i;
		char* sensor = b2Shape_GetUserData(endTouch->sensorShapeId);

		// If we leave the ground and we can jump, set it so that we can't jump
		if (strcmp(sensor, "ground") == 0 && player.canJump) {
			player.canJump = false;
			player.jumpBuffer = 0;
		}
	}
	}

	// Step physics simulation
	b2World_Step(world.worldId, 1.0f / 60.0f, 8);
}


void render(Uint64 startTime) {
	// Render background
	SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(world.renderer);

	// Get Camera Offset
	b2Vec2 playerPosition = b2Body_GetPosition(objects[0].bodyId);
	b2Vec2 position = box2DToSDL(playerPosition, &objects[0]);

	// Get the x and y offset from the center of the first screen
	world.xoffset = (float)WIDTH / 2 - position.x;
	world.yoffset = (float)HEIGHT / 2 - position.y;

	// If we are on the boundaries of the world, set the world offset to a constand value, i.e., unchanging
	if (position.x <= level1.cameraLeftOffset) world.xoffset = 0;
	if (position.x >= level1.cameraRightOffset) world.xoffset = WIDTH - level1.levelWidth;
	if (position.y <= level1.cameraBottomOffset) world.yoffset = 0;
	if (position.y >= level1.cameraTopOffset) world.yoffset = HEIGHT - level1.levelHeight;

	// Draw bodies
	for (int i = 0; i < NUM_BODIES; i++) {
		Object* obj = &objects[i];

		if (!obj->draw) continue;

		// Get the Box2D object's position as SDL, add offsets to it
		b2Vec2 position = box2DToSDL(b2Body_GetPosition(obj->bodyId), obj); 

		// Add to our objects position the world offsets
		obj->rect.x = position.x + world.xoffset;
		obj->rect.y = position.y + world.yoffset;

		// Draw object shape depending on object type
		if (obj->type != COLLECTIBLE) {
			renderRectangle(world.renderer, obj);
		} else {
			renderCircle(world.renderer, obj);
		}
	}

	// Display Screen
	SDL_RenderPresent(world.renderer);

	// Wait for frame based on how long we have calculated for
	const Uint64 elapsedTime = SDL_GetTicks() - startTime;
	if (elapsedTime < MS_PER_SECOND) SDL_Delay(MS_PER_SECOND - elapsedTime);

	world.lastTime = startTime;
}

int gameLoop() {
	const Uint64 startTime = SDL_GetTicks();
	const double elapsedTime = startTime - world.lastTime;

	// Handle game inputs, calculate desired player velocity
	// Take in elapsed Time to apply to force calculations
	handleInputs(elapsedTime);

	// Step through physics, apply desired player velocity to player
	handlePhysics();

	// Convert Box2D positions to SDL, render
	// Take in startTime to calculate how much to wait for this frame
	render(startTime);

	// Loop in main.c if = 0, else quit
	return level1.levelStatus;
}


void cleanup() {
	// Clean up SDL
	SDL_DestroyRenderer(world.renderer);
	SDL_DestroyWindow(world.window);
	SDL_Quit();

	// Destroy Box2D world, also destroys everything else
	b2DestroyWorld(world.worldId);
}
