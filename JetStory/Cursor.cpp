#include "JetStory.h"

SDL_Cursor  *sdl_cursor;
SDL_Surface *sdl_cursor_surface;
Buffer<RGBA> data;
Image        cursor_image;

void  SetMouseCursor(const Image& image)
{
	if(image.GetSerialId() != cursor_image.GetSerialId()) {
		cursor_image = image;
		SDL_ShowCursor(!IsNull(image));
		if(!IsNull(image)) {
			if(sdl_cursor)
				SDL_FreeCursor(sdl_cursor);
			if(sdl_cursor_surface)
				SDL_FreeSurface(sdl_cursor_surface);
			
			int64 a = image.GetAuxData();
			if(a)
				sdl_cursor = SDL_CreateSystemCursor(SDL_SystemCursor(a - 1));
			else {
				sdl_cursor = NULL;
				data.Alloc(image.GetLength());
				Copy(data, image, image.GetLength());
				sdl_cursor_surface = SDL_CreateRGBSurfaceFrom(~data, image.GetWidth(), image.GetHeight(),
				                                              32, sizeof(RGBA) * image.GetWidth(),
				                                              0xff0000, 0xff00, 0xff, 0xff000000);
				Point h = image.GetHotSpot();
				if(sdl_cursor_surface)
					sdl_cursor = SDL_CreateColorCursor(sdl_cursor_surface, h.x, h.y);
			}
			if(sdl_cursor)
				SDL_SetCursor(sdl_cursor);
		}
	}
}
