#include "handmade.h"

internal void
RenderWeirdGradient(GameOffscreenBuffer *buffer, int xOffset, int yOffset)
{
	uint8* row = (uint8*)buffer->memory;
	for ( int y = 0; y < buffer->height ; ++y )
	{
		uint32_t* pixel = (uint32_t*)row;
		for ( int x = 0; x < buffer->width ; ++x )
		{
			uint8 blue = (x + xOffset);
			uint8 green = (y + yOffset);
			
			*pixel++ = ((green << 8) | blue);
		}

		row += buffer->pitch;
	}
}

internal void 
GameUpdateAndRender(GameOffscreenBuffer *buffer, int xOffset, int yOffset)
{
	RenderWeirdGradient(buffer, xOffset, yOffset);
}
