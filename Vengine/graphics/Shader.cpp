#include "Shader.h"

///////////////////////// SHADER BEG //////////////////////////////

Shader::Shader(const std::string &filename, ShaderStages stage)
{
    this->binaryCode = VengineTools::loadShader(filename);
    this->stage      = stage;
}

///////////////////////// SHADER BEG //////////////////////////////