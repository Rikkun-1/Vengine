#include "Texture.h"

#include "vkTools.h"

Texture::Texture(std::string path)
{
    loadFromFile(path);
}

int Texture::getWidth()
{
    return width;
}

int Texture::getHeigt()
{
    return height;
}

void Texture::loadFromFile(std::string path)
{
    vkTools::loadImage(path, width, height, channels);
}

void Texture::setPixel(int x, int y, Pixel pixel)
{
    pixels[y * height + x] = pixel;
}

Pixel Texture::getPixel(int x, int y, Pixel pixel)
{
    return pixels[y * height + x];
}

Pixel *Texture::getRaw()
{
    return pixels.data();
}
