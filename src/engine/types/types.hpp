#pragma once

#include <optional>
#include <functional>
#include <vector>
#include <algorithm>
#include <utility>

#include "idallocator.hpp"
#include "signal.hpp"
#include "sorted_vec.hpp"

namespace eng {
	template <typename T> using OptionalReference = std::optional<std::reference_wrapper<T>>;

   
} // namespace eng
