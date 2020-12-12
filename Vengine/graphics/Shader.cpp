#include "Shader.h"

Shader::Shader(const std::string &filename, ShaderStages stage)
{
    this->binaryCode = vkTools::loadShader(filename);
    this->stage      = stage;
}