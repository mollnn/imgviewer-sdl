#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include "Bitmap.h"
#include "image.hpp"
#include "color.hpp"
#include "jpegdecoder.h"

Image createImageFromBMP(std::string filePath);
Image createImageFromJPEG(std::string filePath);

void renderImage(Image &image);

void PutPixel24(SDL_Surface *surface, int x, int y, Uint32 color);

int main(int argc, char *argv[])
{
	if (argc != 2)
		return 1;
	std::string filePath = argv[1];

	size_t dotPosition = filePath.find_last_of('.');
	if (dotPosition == -1)
	{
		std::cerr << "Bad Filename Ext" << std::endl;
		return 0;
	}
	std::string strExt = filePath.substr(dotPosition + 1);

	Image image;

	if (strExt == "bmp")
	{
		image = createImageFromBMP(filePath);
	}
	else if (strExt == "jpg" || strExt == "jpeg")
	{
		image = createImageFromJPEG(filePath);
	}
	else
	{
		std::cerr << "Bad Filename Ext" << std::endl;
		return 0;
	}

	renderImage(image);

	return 0;
}

void PutPixel24(SDL_Surface *surface, int x, int y, Uint32 color)
{
	uint8_t *pixel = (uint8_t *)surface->pixels;
	pixel += (y * surface->pitch) + (x * sizeof(uint8_t) * 4);

	pixel[0] = (color >> 0) & 0xFF;	 //a
	pixel[1] = (color >> 8) & 0xFF;	 //r
	pixel[2] = (color >> 16) & 0xFF; //g
	pixel[3] = (color >> 24) & 0xFF; //b
}

Image createImageFromBMP(std::string filePath)
{
	Bitmap bitmap(filePath);
	if (!bitmap.isLoaded())
	{
		std::cerr << "BMP Load error." << std::endl;
		exit(0);
	}

	BITMAP_INFO_HEADER &infoHeader = bitmap.getInfoHeader();

	int img_width = infoHeader.width;
	int img_height = infoHeader.height;

	Image image(img_width, img_height);
	uint8_t *pixels = bitmap.getPixels();

	for (int y = 0; y < img_height; y++)
	{
		for (int x = 0; x < img_width; x++)
		{
			int i = x * 3 + (img_height - y - 1) * img_width * 3;
			uint8_t r = pixels[i + 2];
			uint8_t g = pixels[i + 1];
			uint8_t b = pixels[i + 0];
			image.Set(x, y, colorUint8ToFloat(r, g, b));
		}
	}

	return image;
}

Image createImageFromJPEG(std::string filePath)
{
	ifstream ifsFile(filePath, ios::binary);
	filebuf *pFileBuffer = ifsFile.rdbuf();
	size_t bufferSize = pFileBuffer->pubseekoff(0, ios::end, ios::in);
	pFileBuffer->pubseekpos(0, ios::in);
	uint8_t *fileBuffer = new uint8_t[bufferSize + 1];
	pFileBuffer->sgetn(reinterpret_cast<char *>(fileBuffer), bufferSize);
	fileBuffer[bufferSize] = '\0';
	ifsFile.close();

	Jpeg::Decoder decoder(fileBuffer, bufferSize);
	int img_width = decoder.GetWidth();
	int img_height = decoder.GetHeight();

	// 默认彩色

	Image image(img_width, img_height);
	size_t img_size = decoder.GetImageSize();
	uint8_t *img_buffer = new uint8_t[img_size + 1];

	memcpy(img_buffer, decoder.GetImage(), img_size);

	for (int img_y = 0; img_y < img_height; img_y++)
	{
		for (int img_x = 0; img_x < img_width; img_x++)
		{
			size_t id = img_y * img_width + img_x;
			uint8_t r = img_buffer[id * 3 + 0];
			uint8_t g = img_buffer[id * 3 + 1];
			uint8_t b = img_buffer[id * 3 + 2];
			image.Set(img_x, img_y, colorUint8ToFloat(r, g, b));
		}
	}
	delete[] img_buffer;
	delete[] fileBuffer;

	return image;
}

void renderImage(Image &image)
{
	int img_width = image.size_x;
	int img_height = image.size_y;

	//init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		return;
	}
	SDL_Window *window = SDL_CreateWindow("Image Viewer", 100, 100, img_width, img_height, SDL_WINDOW_SHOWN);
	if (!window)
	{
		SDL_Quit();
		return;
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
		return;
	}

	//Create SW surface
	SDL_Surface *surface = SDL_CreateRGBSurface(0, img_width, img_height, 24,
												0x0000FF00, 0x00FF0000, 0xFF000000, 0x00000000);
	if (!surface)
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return;
	}

	SDL_LockSurface(surface);
	for (int y = 0; y < img_height; y++)
	{
		for (int x = 0; x < img_width; x++)
		{
			vec3 c = image.Get(x, y);
			auto [r, g, b] = colorFloatToUint8(c);
			uint32 color = SDL_MapRGB(surface->format, r, g, b);
			PutPixel24(surface, x, y, color);
		}
	}
	SDL_UnlockSurface(surface);

	//Create HW surface
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!surface)
	{
		SDL_FreeSurface(surface);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return;
	}
	SDL_FreeSurface(surface);

	//Main loop
	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		//Draw BMP
		SDL_Rect dst;
		dst.x = 0;
		dst.y = 0;
		dst.w = img_width;
		dst.h = img_height;
		SDL_RenderCopy(renderer, texture, NULL, &dst);

		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
