#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <box2d/collision.h>
#include <box2d/id.h>
#include <box2d/math_functions.h>
#include <box2d/types.h>
#include <box2d/box2d.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h" 
#include "utils.h"
#include "render.h"

World world;
Player player;
Object* objects;

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

	// Gets a pointer to an array that defines what keys are being pressed
	world.keys = SDL_GetKeyboardState(NULL);
	world.lastTime = SDL_GetTicks();
}

void connectSDLtoObjects() {
	// Set the defined pixel units in initGameObjects() to SDL Frect 
	for (int i = 0; i < world.numberOfObjects; i++) {
		objects[i].rect.x = objects[i].p.x;
		objects[i].rect.y = objects[i].p.y;
		objects[i].rect.w = objects[i].p.w;
		objects[i].rect.h = objects[i].p.h;
	}
}

void initBox2D() {
	// Create Box2d World
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = (b2Vec2){0.0f, -10.0f};
	world.worldId = b2CreateWorld(&worldDef);

	// Create Static bodies
	for (int i = 0; i < world.numberOfObjects; i++) {
		Object* obj = &objects[i];

		// Create Body definition
		b2BodyDef bodyDef = b2DefaultBodyDef();

		// SDL position and Box2D positions are different, so we convert between our pixel positioning to
		// Box2D positioning here
		bodyDef.position = SDLPositionToBox2D(obj);
		bodyDef.fixedRotation = true;

		// If dealing with a dynamic object, tell box2D we need physics!!!
		if (obj->type == DYNAMIC) bodyDef.type = b2_dynamicBody;
		if (obj->type == KINEMATIC) bodyDef.type = b2_kinematicBody;

		// Create Body
		obj->bodyId = b2CreateBody(world.worldId, &bodyDef);

		// Convert between SDL pixel to Box2D meter
		b2Vec2 size = SDLSizeToBox2D(obj);

		// Set mass Data
		b2MassData mass;
		mass.mass = 40.0f;
		mass.center = (b2Vec2){0, 0};
		mass.rotationalInertia = 0.0;
		b2Body_SetMassData(obj->bodyId, mass); 

		// Create Polygon Shape
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.0f;
		shapeDef.friction = 0.5f;

		if (obj->type == KINEMATIC) shapeDef.friction = 1.0f;

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
		if (obj->type == STATIC || obj->type == KINEMATIC) {
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
		if (e.type == SDL_EVENT_QUIT) world.level.levelStatus = -1;
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
// Return boolean if we succesfully cleared the collectible
bool clearCollectible(b2ShapeId shapeId) {
	for(int i = 0; i < world.numberOfObjects; i++) {
		if (objects[i].shapeId.index1 == shapeId.index1) {
			// If we already have cleared this object
			if (objects[i].draw == false) return false;

			objects[i].draw = false;
			return true;
		}
	}
	return false;
}

b2Vec2 getKinematicVelocity(Object* obj) {
	// Calculate velocity for kinematic platforms
	float phase = fmod((SDL_GetTicks() - world.level.starttime) / 1000.0, obj->kinematic.time);
	float period = obj->kinematic.time / 2.0;

	float xint = pixelToMeter(obj->kinematic.endPos.x - obj->kinematic.startPos.x);
	float yint = pixelToMeter(obj->kinematic.endPos.y - obj->kinematic.startPos.y);
	int sign;

	if (phase <= period) {
		sign = 1;
	} else {
		sign = -1;
	}

	return (b2Vec2){xint / period * sign, -yint / period * sign};
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
			// If we have cleared the collectible, reduce count needed
			if (clearCollectible(beginTouch->sensorShapeId)) {
				world.level.collectiblesNeeded--;
			}
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

	// Calculate next position for our kinmatic objects
	for (int i = 0; i < world.numberOfObjects; i++) {
		Object* obj = &objects[i];

		if (obj->type != KINEMATIC) continue;
		b2Vec2 vel = getKinematicVelocity(obj);
		b2Body_SetLinearVelocity(obj->bodyId, vel);
	}

	// Step physics simulation
	b2World_Step(world.worldId, 1.0f / 60.0f, 8);
}


void render(Uint64 startTime) { // Render background
	SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(world.renderer);

	// Get Camera Offset
	b2Vec2 playerPosition = b2Body_GetPosition(objects[0].bodyId);
	b2Vec2 position = box2DToSDL(playerPosition, &objects[0]);

	// Get the x and y offset from the center of the first screen
	world.xoffset = (float)WIDTH / 2 - position.x;
	world.yoffset = (float)HEIGHT / 2 - position.y;

	// If we are on the boundaries of the world, set the world offset to a constand value, i.e., unchanging
	if (position.x <= world.level.cameraLeftOffset) world.xoffset = 0;
	if (position.x >= world.level.cameraRightOffset) world.xoffset = WIDTH - world.level.levelWidth;
	if (position.y <= world.level.cameraBottomOffset) world.yoffset = 0;
	if (position.y >= world.level.cameraTopOffset) world.yoffset = HEIGHT - world.level.levelHeight;

	// Draw bodies
	for (int i = 0; i < world.numberOfObjects; i++) {
		Object* obj = &objects[i];

		// Determine if we should draw the object
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

	
	// Render text of how many more collectibles we needed
	SDL_SetRenderDrawColor(world.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_SetRenderScale(world.renderer, 2.0f, 2.0f);
	SDL_RenderDebugTextFormat(world.renderer, 10, 10, "Collectibles Needed: %" SDL_PRIu32 "", world.level.collectiblesNeeded); 
	/*SDL_RenderDebugTextFormat(world.renderer, 10, 30, "Player Position: X: %f %f", position.x, position.y); */
	SDL_SetRenderScale(world.renderer, 1.0f, 1.0f);

	// Display To Window
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

	// If we collect all the collectibles, set level status to completed
	if (world.level.collectiblesNeeded <= 0) world.level.levelStatus = 1;

	// Loop in main.c if = 0, else quit
	return world.level.levelStatus;
}

// This justs destroys Box2D so we can create a new level
void cleanLevel() {
	b2DestroyWorld(world.worldId);
	free(objects);
}

// This cleans up everything
void cleanUp() {
	// Clean up SDL
	SDL_DestroyRenderer(world.renderer);
	SDL_DestroyWindow(world.window);
	SDL_Quit();
	cleanLevel();
}

void initalizeLevel3Objects() {
	// Create world, set up level information
	world.level.levelWidth = WIDTH * 3;
	world.level.levelHeight = HEIGHT * 2;
 	world.level.cameraLeftOffset = (float)WIDTH / 2;
	world.level.cameraRightOffset = world.level.levelWidth - world.level.cameraLeftOffset;
	world.level.cameraBottomOffset = (float)HEIGHT / 2;
	world.level.cameraTopOffset = world.level.levelHeight - world.level.cameraBottomOffset;
	world.level.levelStatus = 0;
	world.level.collectiblesNeeded = 17;
	world.numberOfObjects = 38;
	world.level.starttime = SDL_GetTicks();

	// Allocate space for our object array
	objects = (Object*)malloc(sizeof(Object) * world.numberOfObjects);
	if (objects == NULL) {
		puts("Error! Failed to intialized object array!");
	}

	// Initalize objects

	objects[0] = (Object){.p = (p){100, 100 + HEIGHT, 50, 50}, .color = (Color){255, 0, 0, 255}, .type = DYNAMIC}; // Player
	objects[1] = (Object){.p = (p){0,HEIGHT - 20 + HEIGHT, world.level.levelWidth, 20}, .color = (Color){0, 255, 0, 255}, .type = STATIC}; // Ground
	objects[2] = (Object){.p = (p){200,300 + HEIGHT,200,20}, .color = (Color){0, 255, 255, 255}, .type = STATIC}; // Platform 1
	objects[3] = (Object){.p = (p){40,400 + HEIGHT,100,20}, .color = (Color){0, 0, 255, 255}, .type = STATIC}; // p2
	objects[4] = (Object){.p = (p){400,100 + HEIGHT,200,40}, .color = (Color){0, 0, 255, 255}, .type = STATIC}; // p3
	objects[5] = (Object){.p = (p){400,50 + HEIGHT,50,50}, .color = (Color){255, 20, 255, 255}, .type = DYNAMIC}; // p4
	objects[6] = (Object){.p = (p){175,190 + HEIGHT,150,20}, .color = (Color){125, 255, 30, 255}, .type = STATIC}; // p5
	objects[7] = (Object){.p = (p){world.level.levelWidth - 20,0,20,world.level.levelHeight}, .color = (Color){20, 255, 100, 255}, .type = STATIC}; // p6
	objects[8] = (Object){.p = (p){0,0,20,world.level.levelHeight}, .color = (Color){20, 255, 100, 255}, .type = STATIC}; // p7
	objects[9] = (Object){.p = (p){2 * WIDTH + 200,450 + HEIGHT,100,10}, .color = (Color){40, 80, 90, 255}, .type = STATIC}; // p9
	objects[10] = (Object){.p = (p){2 * WIDTH + 400,350 + HEIGHT,40,40}, .color = (Color){40, 80, 90, 255}, .type = STATIC}; // p10
	objects[11] = (Object){.p = (p){WIDTH * 2,0,10,2 * HEIGHT - 120}, .color = (Color){255, 50, 50, 255}, .type = STATIC}; // Wall Left
	objects[12] = (Object){.p = (p){WIDTH * 2 + 100,0,10,2 * HEIGHT - 120}, .color = (Color){50, 120, 255, 255}, .type = STATIC}; // Wall Right
	objects[13] = (Object){.p = (p){300, HEIGHT * 2 - 100, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // collectible 1
	objects[14] = (Object){.p = (p){400, HEIGHT * 2 - 100, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2
	/*objects[19] = (Object){.p = (p){WIDTH + 300, HEIGHT + 100, 200, 10}, .color = (Color){255, 255, 255, 255}, .type = KINEMATIC}; // Colletile 2*/

	objects[15] = (Object){.p = (p){275, 225 + HEIGHT, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2
	objects[16] = (Object){.p = (p){75, 325 + HEIGHT, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2
	objects[17] = (Object){.p = (p){225, HEIGHT + 125, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2
	objects[18] = (Object){.p = (p){500, 25 + HEIGHT, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2
	objects[19] = (Object){.p = (p){400,HEIGHT,50,50}, .color = (Color){255, 20, 255, 255}, .type = DYNAMIC}; // inital box
	objects[20] = (Object){.p = (p){401,HEIGHT - 50,50,50}, .color = (Color){255, 20, 255, 255}, .type = DYNAMIC}; // inital box
	objects[21] = (Object){.p = (p){550, HEIGHT + 100, 250, 10}, .color = (Color){255, 255, 255, 255}, .type = KINEMATIC}; // moving platform
	objects[22] = (Object){.p = (p){WIDTH + 200, HEIGHT + 100, 20, HEIGHT - 20}, .color = (Color){0, 255, 0, 255}, .type = STATIC}; // inner wall
	objects[23] = (Object){.p = (p){WIDTH + 400, HEIGHT + 100, 20, HEIGHT - 20}, .color = (Color){0, 255, 0, 255}, .type = STATIC}; // inner wall
	objects[24] = (Object){.p = (p){WIDTH + 275, HEIGHT * 2 - 75, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall

	objects[25] = (Object){.p = (p){700, 2 * HEIGHT - 120, 10, 100}, .color = (Color){50, 20, 255, 255}, .type = STATIC}; // inner wall
	objects[26] = (Object){.p = (p){800, 2 * HEIGHT - 120, 10, 100}, .color = (Color){50, 20, 255, 255}, .type = STATIC}; // inner wall
	objects[27] = (Object){.p = (p){650, 2 * HEIGHT - 140, 220, 20}, .color = (Color){60, 180, 176, 255}, .type = DYNAMIC}; // inner wall
	objects[28] = (Object){.p = (p){725, 2 * HEIGHT - 75, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall

	objects[29] = (Object){.p = (p){1120, HEIGHT * 2 - 75, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[30] = (Object){.p = (p){1120, HEIGHT * 2 - 130, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	
	objects[31] = (Object){.p = (p){2020, 710, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[32] = (Object){.p = (p){1650, 920, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[33] = (Object){.p = (p){1750, 920, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[34] = (Object){.p = (p){2230, 890, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[35] = (Object){.p = (p){2400, 790, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[36] = (Object){.p = (p){2140, 700, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall
	objects[37] = (Object){.p = (p){2930, 680, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // inner wall

	objects[21].kinematic.time = 10;
	objects[21].kinematic.startPos.x = 750;
	objects[21].kinematic.startPos.y = HEIGHT + 100; 
	objects[21].kinematic.endPos.x = WIDTH + 350;
	objects[21].kinematic.endPos.y = HEIGHT + 100; 
	
	// Initalize player
	player.canJump = false;
	player.maxVelocityX = 10.0f;
	player.jumpBuffer = 0;
	player.bufferFrames = 10;
	player.xForce = 2.0f;
	player.yForce = 3.0f;

	// Set it so everything is visible
	for (int i = 0; i < world.numberOfObjects; i++) objects[i].draw = true;
}

void initalizeLevel2Objects() {
	// Create world, set up level information
	world.level.levelWidth = WIDTH * 2;
	world.level.levelHeight = HEIGHT;
 	world.level.cameraLeftOffset = (float)WIDTH / 2;
	world.level.cameraRightOffset = world.level.levelWidth - world.level.cameraLeftOffset;
	world.level.cameraBottomOffset = (float)HEIGHT / 2;
	world.level.cameraTopOffset = world.level.levelHeight - world.level.cameraBottomOffset;
	world.level.levelStatus = 0;
	world.level.collectiblesNeeded = 9;
	world.level.starttime = SDL_GetTicks();
	world.numberOfObjects = 24;

	// Allocate space for our object array
	objects = (Object*)malloc(sizeof(Object) * world.numberOfObjects);
	if (objects == NULL) {
		puts("Error! Failed to intialized object array!");
	}

	// Initalize objects
	objects[0] = (Object){.p = (p){100, 100, 50, 50}, .color = (Color){255, 0, 0, 255}, .type = DYNAMIC}; // Player
	objects[1] = (Object){.p = (p){0, HEIGHT - 20, WIDTH * 2, 20}, .color = (Color){80, 50, 175, 255}, .type = STATIC}; // Ground
	objects[2] = (Object){.p = (p){0, 0, 20, HEIGHT}, .color = (Color){80, 50, 175, 255}, .type = STATIC}; // Left Wall
	objects[3] = (Object){.p = (p){WIDTH * 2 - 20, 0, 20, HEIGHT}, .color = (Color){80, 50, 175, 255}, .type = STATIC}; // Right Wall

	objects[4] = (Object){.p = (p){200, 420, 100, 20}, .color = (Color){0, 255, 255, 0}, .type = STATIC}; // Right Wall
	objects[5] = (Object){.p = (p){20, 150, 50, 10}, .color = (Color){255, 255, 255, 0}, .type = STATIC}; // Right Wall
	objects[6] = (Object){.p = (p){500, 300, 100, 20}, .color = (Color){0, 255, 0, 0}, .type = STATIC}; // Right Wall
	objects[7] = (Object){.p = (p){750, 300, 50, 10}, .color = (Color){80, 50, 175, 255}, .type = STATIC}; // Right Wall
	objects[8] = (Object){.p = (p){1000, 200, 20, 10}, .color = (Color){255, 50, 100, 0}, .type = STATIC}; // Right Wall, 
	objects[9] = (Object){.p = (p){1200, 300, 20, 280}, .color = (Color){255, 180, 20, 0}, .type = STATIC}; // Right Wall, 
	objects[10] = (Object){.p = (p){1200, 0, 20, 50}, .color = (Color){255, 180, 20, 0}, .type = STATIC}; // Right Wall, 
	objects[11] = (Object){.p = (p){1200, 0, 20, 100}, .color = (Color){255, 255, 255, 0}, .type = KINEMATIC}; // Right Wall, 

	objects[12] = (Object){.p = (p){1400, 400, 100, 10}, .color = (Color){255, 0, 0, 255}, .type = STATIC}; // Right Wall, 
	objects[13] = (Object){.p = (p){1400, 200, 50, 10}, .color = (Color){0, 255, 0, 255}, .type = STATIC}; // Right Wall, 
	objects[14] = (Object){.p = (p){1700, 100, 100, 20}, .color = (Color){0, 0, 255, 255}, .type = STATIC}; // Right Wall, 

	objects[15] = (Object){.p = (p){230, 360, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[16] = (Object){.p = (p){530, 240, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[17] = (Object){.p = (p){25, 90, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[18] = (Object){.p = (p){765, 240, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[19] = (Object){.p = (p){980, 140, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[20] = (Object){.p = (p){1425, 340, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[21] = (Object){.p = (p){1400, 140, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[22] = (Object){.p = (p){1730, 40, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 
	objects[23] = (Object){.p = (p){1915, 40, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall, 

	objects[11].kinematic.time = 5;
	objects[11].kinematic.startPos.x = 1200;
	objects[11].kinematic.startPos.y = 0; 
	objects[11].kinematic.endPos.x = 1200;
	objects[11].kinematic.endPos.y = 300;
	

	/*objects[4] = (Object){.p = (p){200, HEIGHT - 200, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2*/
	/*objects[5] = (Object){.p = (p){300, HEIGHT - 200, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2*/
	/*objects[6] = (Object){.p = (p){400, HEIGHT - 200, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2*/
	/*objects[7] = (Object){.p = (p){500, HEIGHT - 200, 50, 50}, .color = (Color){255, 255, 0, 0}, .type = COLLECTIBLE}; // Colletile 2*/
	
	// Initalize player
	player.canJump = false;
	player.maxVelocityX = 10.0f;
	player.jumpBuffer = 0;
	player.bufferFrames = 10;
	player.xForce = 2.0f;
	player.yForce = 3.0f;

	// Set it so everything is visible
	for (int i = 0; i < world.numberOfObjects; i++) objects[i].draw = true;
}

void initalizeLevel1Objects() {
	// Create world, set up level information
	world.level.levelWidth = WIDTH;
	world.level.levelHeight = HEIGHT;
 	world.level.cameraLeftOffset = (float)WIDTH / 2;
	world.level.cameraRightOffset = world.level.levelWidth - world.level.cameraLeftOffset;
	world.level.cameraBottomOffset = (float)HEIGHT / 2;
	world.level.cameraTopOffset = world.level.levelHeight - world.level.cameraBottomOffset;
	world.level.levelStatus = 0;
	world.level.collectiblesNeeded = 8;
	world.level.starttime = SDL_GetTicks();
	world.numberOfObjects = 17;

	// Allocate space for our object array
	objects = (Object*)malloc(sizeof(Object) * world.numberOfObjects);
	if (objects == NULL) {
		puts("Error! Failed to intialized object array!");
	}

	// Initalize objects
	objects[0] = (Object){.p = (p){100, 100, 50, 50}, .color = (Color){255, 0, 0, 255}, .type = DYNAMIC}; // Player
	objects[1] = (Object){.p = (p){0, HEIGHT - 20, WIDTH, 20}, .color = (Color){175, 50, 80, 255}, .type = STATIC}; // Ground
	objects[2] = (Object){.p = (p){0, 0, 20, HEIGHT}, .color = (Color){175, 50, 80, 255}, .type = STATIC}; // Left Wall
	objects[3] = (Object){.p = (p){WIDTH - 20, 0, 20, HEIGHT}, .color = (Color){175, 50, 80, 255}, .type = STATIC}; // Right Wall
	
	objects[4] = (Object){.p = (p){700, 400, 200, 10}, .color = (Color){255, 255, 255, 255}, .type = KINEMATIC}; // Right Wall

	objects[5] = (Object){.p = (p){300, 400, 100, 20}, .color = (Color){50, 255, 50, 255}, .type = STATIC}; // Right Wall
	objects[6] = (Object){.p = (p){300, 300, 100, 20}, .color = (Color){100, 80, 50, 255}, .type = STATIC}; // Right Wall
	objects[7] = (Object){.p = (p){300, 200, 100, 20}, .color = (Color){124, 200, 176, 255}, .type = STATIC}; // Right Wall
	objects[8] = (Object){.p = (p){300, 100, 100, 20}, .color = (Color){50, 20, 255, 255}, .type = STATIC}; // Right Wall

	objects[9] = (Object){.p = (p){325, 340, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[10] = (Object){.p = (p){325, 240, 50, 20}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[11] = (Object){.p = (p){325, 140, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[12] = (Object){.p = (p){325, 40, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[13] = (Object){.p = (p){20, 40, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[14] = (Object){.p = (p){550, 30, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[15] = (Object){.p = (p){800, 425, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall
	objects[16] = (Object){.p = (p){920, 40, 50, 50}, .color = (Color){255, 255, 0, 255}, .type = COLLECTIBLE}; // Right Wall

	objects[4].kinematic.time = 10;
	objects[4].kinematic.startPos.x = 700;
	objects[4].kinematic.startPos.y = 400; 
	objects[4].kinematic.endPos.x = 500; 
	objects[4].kinematic.endPos.y = 100; 
	
	// Initalize player
	player.canJump = false;
	player.maxVelocityX = 10.0f;
	player.jumpBuffer = 0;
	player.bufferFrames = 10;
	player.xForce = 2.0f;
	player.yForce = 3.0f;

	// Set it so everything is visible
	for (int i = 0; i < world.numberOfObjects; i++) objects[i].draw = true;

}
