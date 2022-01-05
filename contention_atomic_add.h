#pragma once

#include "tools.h"

namespace excon {
   auto contention_atomic_add(serialize_type& data, int n) -> void;
}
