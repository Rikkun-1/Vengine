#include "Texture.h"

#include "vkTools.h"

Texture::Texture()
{
    int width    = 0;
    int height   = 0;
    int channels = 0;
}

Texture::Texture(std::string path)
{
    loadFromFile(path);
}

int Texture::getWidth()    { return width;  }
int Texture::getHeight()   { return height; }
int Texture::getChannels() { return height; }

void Texture::loadFromFile(std::string path)
{
    vkTools::loadImage(path, width, height, channels);
}

void Texture::setPixel(int x, int y, Pixel pixel)
{
    pixels[y * height + x] = pixel;
}

Pixel Texture::getPixel(int x, int y)
{
    return pixels[y * height + x];
}

void *Texture::getRaw()
{
    return static_cast<void *>(pixels.data());
}
