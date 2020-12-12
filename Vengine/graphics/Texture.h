#pragma once

#include <string>
#include <vector>

#include "Pixel.h"

class Texture
{
public:
    Texture();
    ~Texture();
    Texture(std::string path);

    void loadFromFile(std::string path);

    int getWidth();
    int getHeight();
    int getChannels();

    void  setPixel(int x, int y, Pixel pixel);
    Pixel getPixel(int x, int y);

    void *getRaw();

private:
    int width;
    int height;
    int channels;

    std::vector<Pixel> pixels;
};

