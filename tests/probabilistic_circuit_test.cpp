// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file probabilistic_circuit_test.cpp
/// \brief Tests for probability distributions.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <random>
#include "aitools/datasets/algorithms.h"
#include "aitools/datasets/random.h"
#include "aitools/decision_trees/io.h"
#include "aitools/decision_trees/learning.h"
#include "aitools/statistics/distributions.h"
#include "aitools/numerics/math_utility.h"
#include "aitools/probabilistic_circuits/probabilistic_circuit.h"
#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/probabilistic_circuits/io.h"
#include "aitools/probabilistic_circuits/generative_forest.h"
#include "aitools/random_forests/learning.h"
#include "aitools/utilities/print.h"
#include "aitools/utilities/string_utility.h"

// Returns the expected size of the PC obtained from forest
std::size_t expected_pc_size(const aitools::random_forest& forest, std::size_t m)
{
  std::size_t result = 1;
  for (const auto& tree: forest.trees())
  {
    result = result + tree.vertices().size() + (m + 1) * aitools::leaf_count(tree);
  }
  return result;
}

TEST_CASE("test_decision_tree_to_pc")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::verbose);

  std::size_t n = 20;
  std::size_t m = 3;
  dataset D = make_random_dataset(n, m);
  std::cout << "D =\n" << D << std::endl;
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  decision_tree_options options;
  options.max_depth = 2;
  options.max_features = m;
  binary_decision_tree tree = learn_decision_tree(D, I, options, threshold_plus_single_split_family(D, options), gain(options.imp_measure), node_is_finished);
  std::cout << tree;
  random_forest forest;
  forest.trees().push_back(tree);
  probabilistic_circuit pc = build_generative_forest(forest, D);
  save_probabilistic_circuit(std::cout, pc);
  std::size_t pc_size = probabilistic_circuit_size(pc);
  std::size_t expected = expected_pc_size(forest, m);
  CHECK_EQ(pc_size, expected);

  const auto& X = D.X();
  for (std::size_t i = 0; i < n; i++)
  {
    std::cout << "i " << evi_query_recursive(pc, X[i]) << " " << evi_query_iterative(pc, X[i]) << std::endl;
  }

  expand_sum_split_nodes(pc);
  save_probabilistic_circuit(std::cout, pc);
}

TEST_CASE("test_forest")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::verbose);

  std::size_t n = 20;
  std::size_t m = 8;
  dataset D = make_random_dataset(n, m);
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  decision_tree_options tree_options;
  tree_options.max_depth = n;
  tree_options.max_features = m;
  random_forest_options forest_options;
  forest_options.sample_criterion = sample_technique::stratified;
  forest_options.forest_size = 10;
  forest_options.sample_fraction = 1;
  bool sequential = true;
  random_forest forest = learn_random_forest(D, I, forest_options, tree_options, threshold_plus_single_split_family(D, tree_options),
                                             gain1(tree_options.imp_measure), node_is_finished, sequential);
  probabilistic_circuit pc = build_generative_forest(forest, D);
  save_probabilistic_circuit(std::cout, pc);
}

TEST_CASE("test_intervals1")
{
  using namespace aitools;
  using vertex = binary_decision_tree::vertex;

  std::string text = R"(
binary_decision_tree: 1.0
indices: 0 1 2 3 4 5 6 7 8 9
tree_size: 7
vertex: 0 [1 2] ThresholdSplit(2, 5.0) 0 10
vertex: 1 [3 4] ThresholdSplit(0, 3.0) 0 5
vertex: 2 [5 6] ThresholdSplit(1, 4.0) 5 10
vertex: 3 [] NoSplit() 0 2
vertex: 4 [] NoSplit() 2 5
vertex: 5 [] NoSplit() 5 8
vertex: 6 [] NoSplit() 8 10
  )";
  binary_decision_tree tree = parse_decision_tree(text);
  enumerate_intervals(tree, 3, [&](const vertex& u, std::uint32_t ui, const std::vector<interval>& intervals)
  {
    std::cout << "node " << ui << " " << print_list(intervals) << std::endl;
  });
}

TEST_CASE("test_intervals2")
{
  using namespace aitools;
  using vertex = binary_decision_tree::vertex;

  std::string text = R"(
binary_decision_tree: 1.0
indices: 0 1 2 3 4 5 6 7 8 9
tree_size: 7
vertex: 0 [1 2] ThresholdSplit(2, 5.0) 0 10
vertex: 1 [3 4] ThresholdSplit(0, 3.0) 0 5
vertex: 2 [5 6] SingleSplit(1, 4) 5 10
vertex: 3 [] NoSplit() 0 2
vertex: 4 [] NoSplit() 2 5
vertex: 5 [] NoSplit() 5 8
vertex: 6 [] NoSplit() 8 10
  )";
  binary_decision_tree tree = parse_decision_tree(text);
  enumerate_intervals(tree, 3, [&](const vertex& u, std::uint32_t ui, const std::vector<interval>& intervals)
  {
    std::cout << "node " << ui << " " << print_list(intervals) << std::endl;
  });
}

inline
void test_parse_print(const std::string& text)
{
  using namespace aitools;
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  std::ostringstream out;
  save_probabilistic_circuit(out, pc);
  std::string text1 = out.str();
  std::cout << "\n" << text1 << "\n";
  CHECK(utilities::trim_copy(text) == utilities::trim_copy(text1));
}

TEST_CASE("test_io")
{
  using namespace aitools;

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 15
category_counts: 0 0 0
categorical: 9 [] 0 [0.0714286 0.142857 0.214286 0.142857 0.285714 0.142857]
truncated_normal: 10 [] 1 78.5493 14.0491 -1.79769e+308 109.296
truncated_normal: 11 [] 2 58.5521 23.5572 -1.79769e+308 107.415
product: 4 [9 10 11]
categorical: 12 [] 0 [0 0.25 0.5 0 0.25 0]
truncated_normal: 13 [] 1 50.5498 12.065 -1.79769e+308 109.296
truncated_normal: 14 [] 2 132.173 19.3011 107.415 1.79769e+308
product: 5 [12 13 14]
sum_split: 2 [4 5] [0.777778 0.222222] ThresholdSplit(2, 107.415)
categorical: 6 [] 0 [0.5 0 0 0 0 0.5]
truncated_normal: 7 [] 1 110.28 0.983799 109.296 1.79769e+308
normal: 8 [] 2 36.5643 4.59506
product: 3 [6 7 8]
sum_split: 1 [2 3] [0.9 0.1] ThresholdSplit(1, 109.296)
sum: 0 [1] [1]
  )";
  test_parse_print(text);
}

// The examples below are from "Probabilistic Circuits: A Unifying Framework for Tractable Probabilistic Models"
// by Choi, Vergari and Van den Broeck
TEST_CASE("test_example4")
{
  using namespace aitools;

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 1
category_counts: 0
normal: 0 [] 0 1 0.1
  )";
  test_parse_print(text);
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  std::vector<double> x = {1.1};
  double e1 = evi_query_recursive(pc, x);
  double e2 = evi_query_iterative(pc, x);
  double expected = 2.41;
  REQUIRE_LT(std::abs(e1 - expected), 0.01);
  REQUIRE_LT(std::abs(e2 - expected), 0.01);
  std::cout << "example 4 " << e1 << " " << e2 << std::endl;
}

TEST_CASE("test_example5")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  // N.B. Use the square root of 0.2, 0.5 and 0.5!
  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 4
category_counts: 0 0 0
normal: 1 [] 0 0 0.447214
normal: 2 [] 1 1 0.707107
normal: 3 [] 2 -2 0.547723
product: 0 [1 2 3]
  )";
  test_parse_print(text);
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  std::vector<double> x = {0.1, -0.1, -2.2};
  double e1 = evi_query_recursive(pc, x);
  double e2 = evi_query_iterative(pc, x);
  double expected = 0.147; // N.B. The second value in the paper appears to be wrong
  expected = 0.0997;
  REQUIRE_LT(std::abs(e1 - expected), 0.01);
  REQUIRE_LT(std::abs(e2 - expected), 0.01);
  std::cout << "example 5 " << e1 << " " << e2 << std::endl;
}

TEST_CASE("test_example9")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 3
category_counts: 0
normal: 1 [] 0 -2 2
normal: 2 [] 0 2 1.5
sum: 0 [1 2] [0.8 0.2]
  )";
  test_parse_print(text);
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  std::vector<double> x = {1, 1};
  double e1 = evi_query_recursive(pc, x);
  double e2 = evi_query_iterative(pc, x);
  double expected = 0.09;
  REQUIRE_LT(std::abs(e1 - expected), 0.01);
  REQUIRE_LT(std::abs(e2 - expected), 0.01);
  std::cout << "example 9 " << e1 << " " << e2 << std::endl;
}

TEST_CASE("test_example11")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 23
category_counts: 0 0 0 0
normal: 1 [] 0  -1 2
normal: 2 [] 0  -2 0.1
normal: 3 [] 1 0.6 0.1
normal: 4 [] 1 0 1
normal: 5 [] 2 -1.5 0.2
normal: 6 [] 2 -1 0.5
normal: 7 [] 3 0 1
normal: 8 [] 3 0 0.1
sum: 9 [3 4] [0.8 0.2]
sum: 10 [3 4] [0.7 0.3]
sum: 11 [1 2] [0.5 0.5]
sum: 12 [1 2] [0.1 0.9]
product: 13 [9 11]
product: 14 [10 12]
sum: 15 [13 14] [0.4 0.6]
sum: 16 [13 14] [0.5 0.5]
product: 17 [5 15]
product: 18 [6 16]
sum: 19 [17 18] [0.5 0.5]
sum: 20 [17 18] [0.2 0.8]
product: 21 [7 19]
product: 22 [8 20]
sum: 0 [21 22] [0.2 0.8]
  )";
  // test_parse_print(text);
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  std::vector<double> x = {-1.85, 0.5, -1.3, 0.2};
  double e1 = evi_query_recursive(pc, x);
  double e2 = evi_query_iterative(pc, x);
  double expected = 0.75;
  REQUIRE_LT(std::abs(e1 - expected), 0.01);
  REQUIRE_LT(std::abs(e2 - expected), 0.01);
  std::cout << "example 11 " << e1 << " " << e2 << std::endl;
}

inline
void test_sampling(const aitools::probabilistic_circuit& pc, std::size_t j, double mu_expected, double sigma_expected, std::size_t n = 100000)
{
  using namespace aitools;

  auto seed = std::random_device{}();
  std::mt19937 rng{static_cast<unsigned int>(seed)};
  dataset D = sample_pc(pc, n, rng);
  auto [mu, sigma] = mean_standard_deviation(D, xrange(n), j);
  std::cout << "mu = " << mu << " sigma = " << sigma << std::endl;
  REQUIRE_LT(std::abs(mu - mu_expected), 0.01);
  REQUIRE_LT(std::abs(sigma - sigma_expected), 0.01);
}

TEST_CASE("test_sample1")
{
  using namespace aitools;

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 1
category_counts: 0
normal: 0 [] 0 0 1
  )";
  test_parse_print(text);
  double mu = 0;
  double sigma = 1;
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  test_sampling(pc, 0, mu, sigma, 500000);
}

TEST_CASE("test_sample2")
{
  using namespace aitools;

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 1
category_counts: 0
normal: 0 [] 0 1 2
  )";
  test_parse_print(text);
  double mu = 1;
  double sigma = 2;
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  test_sampling(pc, 0, mu, sigma, 500000);
}

TEST_CASE("test_sample3")
{
  using namespace aitools;

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 3
category_counts: 0
normal: 1 [] 0 0 1
normal: 2 [] 0 1 2
sum: 0 [1 2] [0.25 0.75]
  )";
  test_parse_print(text);
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  double w1 = 0.25;
  double mu1 = 0;
  double sigma1 = 1;
  double w2 = 0.75;
  double mu2 = 1;
  double sigma2 = 2;
  auto [mu, sigma] = mean_standard_deviation_mixture(w1, mu1, sigma1, w2, mu2, sigma2);
  test_sampling(pc, 0, mu, sigma, 500000);
}

TEST_CASE("test_pc_properties")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  std::string text = R"(
probabilistic_circuit: 1.0
pc_size: 23
category_counts: 0 0 0 0
normal: 1 [] 0  -1 2
normal: 2 [] 0  -2 0.1
normal: 3 [] 1 0.6 0.1
normal: 4 [] 1 0 1
normal: 5 [] 2 -1.5 0.2
normal: 6 [] 2 -1 0.5
normal: 7 [] 3 0 1
normal: 8 [] 3 0 0.1
sum: 9 [3 4] [0.8 0.2]
sum: 10 [3 4] [0.7 0.3]
sum: 11 [1 2] [0.5 0.5]
sum: 12 [1 2] [0.1 0.9]
product: 13 [9 11]
product: 14 [10 12]
sum: 15 [13 14] [0.4 0.6]
sum: 16 [13 14] [0.5 0.5]
product: 17 [5 15]
product: 18 [6 16]
sum: 19 [17 18] [0.5 0.5]
sum: 20 [17 18] [0.2 0.8]
product: 21 [7 19]
product: 22 [8 20]
sum: 0 [21 22] [0.2 0.8]
  )";
  probabilistic_circuit pc = parse_probabilistic_circuit(text);
  CHECK(is_smooth(pc));
  CHECK(is_decomposable(pc));
  CHECK(is_normalized(pc));

  text = R"(
probabilistic_circuit: 1.0
pc_size: 23
category_counts: 0 0 0 0
normal: 1 [] 0  -1 2
normal: 2 [] 0  -2 0.1
normal: 3 [] 1 0.6 0.1
normal: 4 [] 1 0 1
normal: 5 [] 2 -1.5 0.2
normal: 6 [] 2 -1 0.5
normal: 7 [] 3 0 1
normal: 8 [] 3 0 0.1
sum: 9 [3 4] [0.8 0.2]
sum: 10 [3 4] [0.7 0.3]
sum: 11 [1 2] [0.5 0.5]
sum: 12 [1 2] [0.1 0.9]
product: 13 [9 11]
product: 14 [10 12]
sum: 15 [13 14] [0.4 0.6]
sum: 16 [13 14] [0.5 0.5]
product: 17 [5 15]
product: 18 [6 16]
sum: 19 [17 18] [0.5 1.5]
sum: 20 [4 17 18] [0.2 0.8]
product: 21 [7 19 1]
product: 22 [8 20]
sum: 0 [21 22] [0.2 0.8]
  )";
  pc = parse_probabilistic_circuit(text);
  CHECK(!is_smooth(pc));
  CHECK(!is_decomposable(pc));
  CHECK(!is_normalized(pc));
}

//TEST_CASE("test_sample4)
//{
//  using namespace aitools;
//  log::aitools_logger::set_reporting_level(log::quiet);
//
//  // create a random dataset
//  std::vector<distribution> distributions = { normal_distribution(1, 2), normal_distribution(3, 1), categorical_distribution({0.2, 0.3, 0.5})};
//  std::size_t n = 100;
//  std::size_t m = 2;
//  auto seed = std::random_device{}();
//  std::mt19937 rng{static_cast<unsigned int>(seed)};
//  dataset D = make_random_dataset(distributions, n, rng);
//  std::cout << "D =\n" << D << std::endl;
//
//  // create a generative forest
//  std::vector<std::uint32_t> I(n);
//  std::iota(I.begin(), I.end(), 0);
//  decision_tree_options tree_options;
//  tree_options.max_depth = n;
//  tree_options.max_features = m;
//  tree_options.min_samples_leaf = 5;
//  random_forest_options forest_options;
//  forest_options.sample_criterion = sample_technique::without_replacement;
//  forest_options.forest_size = 1;
//  forest_options.sample_fraction = 1;
//  bool sequential = true;
//  random_forest forest = learn_random_forest(D, I, forest_options, tree_options, threshold_split_family(D, tree_options),
//                                             gain1(tree_options.imp_measure), node_is_finished, sequential);
//  probabilistic_circuit gef = build_generative_forest(forest, D);
//
//  dataset D1 = sample_pc(gef, n, rng);
//  std::cout << "D1 =\n" << D1 << std::endl;
//  auto [mu1, sigma1] = mean_standard_deviation(D1, xrange(n), 0);
//  auto [mu2, sigma2] = mean_standard_deviation(D1, xrange(n), 1);
//  std::cout << "mu1 = " << mu1 << " sigma1 = " << sigma1 << std::endl;
//  std::cout << "mu2 = " << mu2 << " sigma2 = " << sigma2 << std::endl;
//}
