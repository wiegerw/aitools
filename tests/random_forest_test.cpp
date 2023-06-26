// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file random_forest_test.cpp
/// \brief Tests for probability distributions.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <set>
#include "aitools/datasets/random.h"
#include "aitools/random_forests/random_forest.h"
#include "aitools/random_forests/io.h"
#include "aitools/utilities/string_utility.h"

TEST_CASE("test_io")
{
  using namespace aitools;

  std::string text = R"(
random_forest: 1.0
forest_size: 2
binary_decision_tree: 1.0
tree_size: 17
category_counts: 0 0 0 0 4 0 9 0 3
classes: 1 1 1 1 2 2 2 0 0 0 0 0 0 0 2 0 0 2 1 2
indices: 17 10 6 15 16 7 8 9 5 4 3 1 2 18 0 12 13 11 14 19
vertex: 0 [1 2] ThresholdSplit(3, 131.37) 0 20
vertex: 1 [3 4] ThresholdSplit(0, 144.973) 0 10
vertex: 2 [5 6] ThresholdSplit(5, 125.527) 10 20
vertex: 3 [7 8] SingleSplit(6, 0) 0 8
vertex: 4 [] NoSplit() 8 10
vertex: 5 [9 10] ThresholdSplit(1, 171.459) 10 18
vertex: 6 [] NoSplit() 18 20
vertex: 7 [] NoSplit() 0 1
vertex: 8 [11 12] ThresholdSplit(0, 100.76) 1 8
vertex: 9 [13 14] ThresholdSplit(2, 157.214) 10 16
vertex: 10 [] NoSplit() 16 18
vertex: 11 [15 16] ThresholdSplit(0, 99.3751) 1 3
vertex: 12 [] NoSplit() 3 8
vertex: 13 [] NoSplit() 10 15
vertex: 14 [] NoSplit() 15 16
vertex: 15 [] NoSplit() 1 2
vertex: 16 [] NoSplit() 2 3
binary_decision_tree: 1.0
tree_size: 17
category_counts: 0 0 0 0 4 0 9 0 3
classes: 1 1 1 1 2 2 2 0 0 0 0 0 0 0 2 0 0 2 1 2
indices: 17 10 6 15 16 7 8 9 5 4 3 1 2 18 0 12 13 11 14 19
vertex: 0 [1 2] ThresholdSplit(3, 131.37) 0 20
vertex: 1 [3 4] ThresholdSplit(0, 144.973) 0 10
vertex: 2 [5 6] ThresholdSplit(5, 125.527) 10 20
vertex: 3 [7 8] SingleSplit(6, 0) 0 8
vertex: 4 [] NoSplit() 8 10
vertex: 5 [9 10] ThresholdSplit(1, 171.459) 10 18
vertex: 6 [] NoSplit() 18 20
vertex: 7 [] NoSplit() 0 1
vertex: 8 [11 12] ThresholdSplit(0, 100.76) 1 8
vertex: 9 [13 14] ThresholdSplit(2, 157.214) 10 16
vertex: 10 [] NoSplit() 16 18
vertex: 11 [15 16] ThresholdSplit(0, 99.3751) 1 3
vertex: 12 [] NoSplit() 3 8
vertex: 13 [] NoSplit() 10 15
vertex: 14 [] NoSplit() 15 16
vertex: 15 [] NoSplit() 1 2
vertex: 16 [] NoSplit() 2 3
)";
  random_forest forest = parse_random_forest(text);

  std::ostringstream out;
  out << forest;
  std::string text1 = out.str();
  std::cout << "\n" << text1 << "\n";
  CHECK(utilities::trim_copy(text) == utilities::trim_copy(text1));
}