// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/container_utility.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_CONTAINER_UTILITY_H
#define AITOOLS_UTILITIES_CONTAINER_UTILITY_H

#include <algorithm>
#include <set>

namespace aitools::utilities {

/// \brief Returns the union of two sets.
/// \param x A set
/// \param y A set
/// \return The union of two sets.
template <typename T>
std::set<T> set_union(const std::set<T>& x, const std::set<T>& y)
{
  std::set<T> result;
  std::set_union(x.begin(), x.end(), y.begin(), y.end(), std::inserter(result, result.begin()));
  return result;
}

/// \brief Returns the difference of two sets.
/// \param x A set
/// \param y A set
/// \return The difference of two sets.
template <typename T>
std::set<T> set_difference(const std::set<T>& x, const std::set<T>& y)
{
  std::set<T> result;
  std::set_difference(x.begin(), x.end(), y.begin(), y.end(), std::inserter(result, result.begin()));
  return result;
}

/// \brief Returns the intersection of two sets.
/// \param x A set
/// \param y A set
/// \return The intersection of two sets.
template <typename T>
std::set<T> set_intersection(const std::set<T>& x, const std::set<T>& y)
{
  std::set<T> result;
  std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), std::inserter(result, result.begin()));
  return result;
}

/// \brief Returns if y is included in x.
/// \param x A set
/// \param y A set
/// \return True if x includes y.
template <typename T>
bool set_includes(const std::set<T>& x, const std::set<T>& y)
{
  return std::includes(x.begin(), x.end(), y.begin(), y.end());
}

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_CONTAINER_UTILITY_H
