#pragma once

#include <string>

#include "tools.h"

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

    Shader() = default;
    Shader(const std::string &filename, ShaderStages stage);
};

