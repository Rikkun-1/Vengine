#include "Shader.h"

///////////////////////// SHADER BEG //////////////////////////////

Shader::Shader(const std::string &filename, ShaderStages stage)
{
    this->binaryCode = vkTools::loadShader(filename);
    this->stage      = stage;
}

///////////////////////// SHADER BEG //////////////////////////////