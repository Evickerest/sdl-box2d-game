#pragma once

#include "game.h"
#include <SDL3/SDL_render.h>
#include <box2d/math_functions.h>

void renderRectangle(SDL_Renderer* renderer, Object* object); 
void renderCircle(SDL_Renderer* renderer, Object* object); 
