#pragma once

#include <string>

#include "vkTools.h"

enum class ShaderStages
{
    VERTEX_STAGE   = 0,
    FRAGMENT_STAGE = 1
};

class Shader
{
public:
    ShaderStages      stage;
    std::string       entry = "main";
    std::vector<char> binaryCode;

    Shader(const std::string &filename, ShaderStages stage);
};
