// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/utilities/interval.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_INTERVAL_H
#define AITOOLS_UTILITIES_INTERVAL_H

#include <limits>
#include <iostream>

namespace aitools {

struct interval
{
  static constexpr double min = std::numeric_limits<double>::lowest();
  static constexpr double max = std::numeric_limits<double>::max();

  double a = min;
  double b = max;

  constexpr bool contains(double x) const
  {
    return a <= x && x <= b;
  }

  constexpr bool is_maximal() const
  {
    return a == min && b == max;
  }
};

inline
std::ostream& operator<<(std::ostream& out, const interval& i)
{
  return out << '[' << i.a << ", " << i.b << ']';
}

} // namespace aitools

#endif // AITOOLS_UTILITIES_INTERVAL_H
