// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_tree_range.h
/// \brief add your file description here.

#ifndef AITOOLS_INDEX_RANGE_H
#define AITOOLS_INDEX_RANGE_H

#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>
#include "aitools/datasets/missing.h"
#include "aitools/utilities/iterator_range.h"
#include "aitools/utilities/print.h"

namespace aitools {

/// \brief An iterator range of indices
using index_range = utilities::iterator_range<std::vector<std::uint32_t>::iterator>;

inline
std::ostream& operator<<(std::ostream& out, const index_range& I)
{
  return out << aitools::print_list(I);
}

/// \brief Return the last position i in I that does not have the value NaN.
/// \pre One of the positions in I does not have the value NaN.
inline
std::size_t find_last_not_missing(const index_range& I)
{
  assert(!I.empty());
  auto i = --I.end();
  while (is_missing(*i))
  {
    --i;
  }
  return i - I.begin();
}

/// \brief Transforms an index range to another vector of indices.
inline
index_range convert_index_range(const index_range& I, const std::vector<std::uint32_t>& old_indices, std::vector<std::uint32_t>& new_indices)
{
  return index_range(new_indices.begin() + (I.begin() - old_indices.begin()), new_indices.begin() + (I.end() - old_indices.begin()));
}

inline
bool is_valid_range(const index_range& I, std::uint32_t max_index)
{
  for (auto i: I)
  {
    if (i >= max_index)
    {
      std::cout << "invalid index " << i << " max-index = " << max_index << std::endl;
      return false;
    }
  }
  return true;
}

inline
bool equal_ranges(const index_range& I1, const index_range& I2)
{
  std::set<std::uint32_t> S1(I1.begin(), I1.end());
  std::set<std::uint32_t> S2(I2.begin(), I1.end());
  if (S1 != S2)
  {
    std::cout << "S1 = " << print_set(S1) << " S2 = " << print_set(S2) << std::endl;
  }
  return S1 == S2;
}

} // namespace aitools

#endif // AITOOLS_INDEX_RANGE_H
