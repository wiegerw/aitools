// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/datasets/missing.h
/// \brief add your file description here.

#ifndef AITOOLS_MISSING_H
#define AITOOLS_MISSING_H

#include <cmath>

namespace aitools {

inline
bool is_missing(double x)
{
  return std::isnan(x);
}

} // namespace aitools

#endif // AITOOLS_MISSING_H
