#pragma once

#include "logging/LoggingAll.h"
#include "memory/Handle.h"

#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>

#include "various/linmathext.h"

namespace l::rendering::vulkan {

	bool PlatformSupported();

}