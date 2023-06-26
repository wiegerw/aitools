// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/numerics/math_utility.h
/// \brief add your file description here.

#ifndef AITOOLS_NUMERICS_MATH_UTILITY_H
#define AITOOLS_NUMERICS_MATH_UTILITY_H

#include <algorithm>
#include <iostream>
#include <numeric>
#include <set>

namespace aitools {

template <typename Number>
Number square(Number x)
{
  return x * x;
}

template <typename NumberSequence>
auto sum(const NumberSequence& counts)
{
  using T = typename NumberSequence::value_type;
  return std::accumulate(counts.begin(), counts.end(), T{});
}

template <typename NumberSequence>
std::size_t max_value(const NumberSequence& counts)
{
  return *std::max_element(counts.begin(), counts.end());
}

/// \brief Returns true if the union of I1 and I2 is I. Duplicate elements are allowed.
template <typename NumberSequence>
bool is_partition(const NumberSequence& I, const NumberSequence& I1, const NumberSequence& I2)
{
  if (I.size() != I1.size() + I2.size())
  {
    std::cout << "|I| = " << I.size() << std::endl;
    std::cout << "|I1| = " << I1.size() << std::endl;
    std::cout << "|I2| = " << I2.size() << std::endl;
    return false;
  }

  using T = typename NumberSequence::value_type;
  std::multiset<T> V(I.begin(), I.end());
  std::multiset<T> W(I1.begin(), I1.end());
  W.insert(I2.begin(), I2.end());

  return V == W;
}

template <typename NumberSequence>
double mean(const NumberSequence& x)
{
  return static_cast<double>(sum(x)) / x.size();
}

} // namespace aitools

#endif // AITOOLS_NUMERICS_MATH_UTILITY_H
