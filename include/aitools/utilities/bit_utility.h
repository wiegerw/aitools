// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/bit_utility.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_BIT_UTILITY_H
#define AITOOLS_UTILITIES_BIT_UTILITY_H

#include <cstdlib>

namespace aitools::utilities {

/// Returns true if the bit at position i in x is set.
template <typename Number>
constexpr bool is_bit_set(Number x, unsigned int i)
{
  const std::size_t one{1};
  return (x & (one << i)) != 0;
}

/// Sets the bit at position i in x to enabled.
template <typename Number>
constexpr void set_bit(Number& x, unsigned int i, bool enabled)
{
  const std::size_t one{1};
  if (enabled)
  {
    x |= (one << i);
  }
  else
  {
    x &= ~(one << i);
  }
}

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_BIT_UTILITY_H
