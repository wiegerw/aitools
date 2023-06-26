// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file number_test.cpp
/// \brief Tests for iterator_range.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <algorithm>
#include <array>
#include <cassert>
#include "aitools/utilities/iterator_range.h"

using index_range = aitools::utilities::iterator_range<std::vector<int>::iterator>;

void f(const index_range& I)
{
  std::sort(I.begin(), I.end());
}

void g(const index_range& I)
{
  std::stable_partition(I.begin(), I.end(), [](std::size_t i) { return i % 2 == 0; });
}

TEST_CASE("test1")
{
  std::vector<int> v = { 4, 3, 1, 2 };
  index_range w(v.begin(), v.end());
  CHECK_EQ(w.size(), v.size());

  f(w);
  CHECK_EQ(v.front(), 1);
  CHECK_EQ(w.front(), 1);

  g(w);
  CHECK_EQ(v.front(), 2);
  CHECK_EQ(w.front(), 2);

  index_range r1(v);
}
