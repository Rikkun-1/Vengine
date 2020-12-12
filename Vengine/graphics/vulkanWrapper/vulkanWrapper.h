#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vertex.h"
#include "validationLayers.h"
#include "instance.h"
#include "device.h"

#include "commandPool.h"
#include "commandBuffer.h"
#include "buffer.h"
#include "image.h"
#include "frameBuffer.h"

#include "swapChain.h"
#include "renderPass.h"
#include "shaderModule.h"
#include "pipelineLayout.h"
#include "pipeline.h"


#include "transitionImageLayout.h"
#include "texture.h"

#include "descriptorSetLayout.h"
#include "descriptorPool.h"
#include "descriptorSet.h"

#include "synchronization.h"