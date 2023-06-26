// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/statistics/distribution.h
/// \brief add your file description here.

#ifndef AITOOLS_STATISTICS_DISTRIBUTION_H
#define AITOOLS_STATISTICS_DISTRIBUTION_H

#include <variant>
#include "aitools/statistics/sampling.h"

namespace aitools {

using distribution = std::variant<uniform_distribution, normal_distribution, truncated_normal_distribution, categorical_distribution>;

/// \brief Draw a sample from the distribution \c dist.
/// \param rng The random number generator that is used for sampling.
inline
double sample(const distribution& dist, std::mt19937& rng)
{
  auto f = [&rng](const auto& dist)
  {
    return static_cast<double>(sample(dist, rng));
  };
  return std::visit(f, dist);
}

/// \brief Returns the category count of the distribution \c dist.
inline
unsigned int category_count(const distribution& dist)
{
  auto f = [](const auto& dist)
  {
    return dist.category_count();
  };
  return std::visit(f, dist);
}

} // namespace aitools

#endif // AITOOLS_STATISTICS_DISTRIBUTION_H
