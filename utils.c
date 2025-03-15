#include "game.h"
#include "utils.h"

float pixelToMeter(const float value) {
	return value / PIXELS_PER_METER;
}

float meterToPixel(const float value) {
	return value * PIXELS_PER_METER;
}

b2Vec2 box2DToSDL(b2Vec2 vector, Object *object) {
	vector.x = meterToPixel(vector.x) - object->w / 2;
	vector.y = HEIGHT - meterToPixel(vector.y) - object->h / 2;
	return vector;
}

b2Vec2 SDLToBox2D(b2Vec2 vector, Object *object) {
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

b2Vec2 SDLPositionToBox2D(Object *object) {
	float x = (object->w / 2) + object->x;
	float y = (object->h / 2) + object->y;
	return SDLXYToBox2D(x, y);
}

b2Vec2 SDLSizeToBox2D(Object *object) {
	float w = object->w / 2;
	float h = object->h / 2;
	return (b2Vec2){pixelToMeter(w), pixelToMeter(h)};
}
