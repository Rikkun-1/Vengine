#include "Texture.h"

#include "vkTools.h"

///////////////////////// TEXTURE BEG //////////////////////////////

Texture::Texture()
{
    int width    = 0;
    int height   = 0;
    int channels = 0;
}

Texture::~Texture() {}

Texture::Texture(std::string path)
{
    loadFromFile(path);
}

int Texture::getWidth()    { return width;  }
int Texture::getHeight()   { return height; }
int Texture::getChannels() { return channels; }

void Texture::loadFromFile(std::string path)
{
    vkTools::loadImage(path, width, height, channels, pixels);
}

void Texture::setPixel(int x, int y, Pixel pixel)
{
    pixels[(y * width + x)] = pixel;
}

Pixel Texture::getPixel(int x, int y)
{
    return pixels[(y * width + x)];
}

///////////////////////// TEXTURE END //////////////////////////////
