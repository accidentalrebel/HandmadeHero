#if !defined(HANDMADE_H)

struct GameOffscreenBuffer
{
	void* memory;
	int width;
	int height;
	int pitch;
	int bytesPerPixel;
};

internal void 
GameUpdateAndRender(GameOffscreenBuffer *buffer);

#define HANDMADE_H
#endif
