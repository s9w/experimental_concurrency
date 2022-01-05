#pragma once

#include "tools.h"

namespace excon {
   auto scoped_lock_latency(serialize_type& data, const int n) -> void;
}
