#include "queueFamilyIndices.h"

bool QueueFamilyIndices::has_value()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}