// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file decision_tree_test.cpp
/// \brief Tests for probability distributions.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <random>
#include <set>
#include "aitools/datasets/io.h"
#include "aitools/datasets/random.h"
#include "aitools/decision_trees/algorithms.h"
#include "aitools/decision_trees/decision_tree.h"
#include "aitools/decision_trees/io.h"
#include "aitools/decision_trees/learning.h"
#include "aitools/random_forests/learning.h"
#include "aitools/utilities/string_utility.h"
#include "aitools/utilities/container_utility.h"

namespace aitools {

inline
void check_decision_tree(const binary_decision_tree& tree, const std::vector<std::uint32_t>& I, const decision_tree_options& options)
{
  AITOOLS_LOG(log::debug) << "--- checking decision tree ---" << std::endl;
  AITOOLS_LOG(log::debug) << "root = " << tree.root() << std::endl;

  auto indices = [](const binary_decision_tree::vertex& u)
  {
    return std::set<std::uint32_t>(u.I.begin(), u.I.end());
  };

  std::set<uint32_t> tree_indices(I.begin(), I.end());

  visit_nodes_bfs(tree, [&](const binary_decision_tree::vertex& u, std::size_t ui, std::size_t depth) {
    CHECK(depth <= options.max_depth);
    CHECK(u.I.size() >= options.min_samples_leaf);

    if (ui == 0) // u is the root of the tree
    {
      CHECK(indices(u) == tree_indices);
    }

    if (!u.is_leaf())
    {
      auto v1 = tree.find_vertex(u.left);
      auto v2 = tree.find_vertex(u.right);
      CHECK(indices(u) == utilities::set_union(indices(v1), indices(v2)));
    }
  });
}

} // namespace aitools

TEST_CASE("test_tree")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  std::size_t n = 20;
  std::size_t m = 5;
  dataset D = make_random_dataset(n, m);
  CHECK_EQ(D.feature_count(), m);

  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  AITOOLS_LOG(log::debug) << "I = " << aitools::print_list(I) << std::endl;
  decision_tree_options options;
  options.max_depth = 4;
  std::size_t seed = 12345;
  binary_decision_tree tree;

  AITOOLS_LOG(log::debug) << "split options\n" << options << std::endl;

  AITOOLS_LOG(log::debug) << "---------------------------------------------------------\n";
  AITOOLS_LOG(log::debug) << "  threshold I = " << print_list(I) << "\n";
  AITOOLS_LOG(log::debug) << "---------------------------------------------------------\n";
  tree = learn_decision_tree(D, I, options, threshold_split_family(D, options), gain(options.imp_measure), node_is_finished, seed);
  print_decision_tree(tree);
  check_decision_tree(tree, I, options);
  AITOOLS_LOG(log::debug) << "Accuracy threshold: " << accuracy(tree, I, D) << std::endl;

  AITOOLS_LOG(log::debug) << "---------------------------------------------------------" << std::endl;
  AITOOLS_LOG(log::debug) << "  threshold-single I = " << print_list(I) << std::endl;
  AITOOLS_LOG(log::debug) << "---------------------------------------------------------" << std::endl;
  tree = learn_decision_tree(D, I, options, threshold_plus_single_split_family(D, options), gain(options.imp_measure), node_is_finished, seed);
  check_decision_tree(tree, I, options);
  print_decision_tree(tree);
  AITOOLS_LOG(log::debug) << "Accuracy threshold-single: " << accuracy(tree, I, D) << std::endl;

  AITOOLS_LOG(log::debug) << "---------------------------------------------------------\n";
  AITOOLS_LOG(log::debug) << "  threshold-subset I = " << print_list(I) << "\n";
  AITOOLS_LOG(log::debug) << "---------------------------------------------------------\n";
  tree = learn_decision_tree(D, I, options, threshold_plus_subset_split_family(D, options), gain(options.imp_measure), node_is_finished, seed);
  print_decision_tree(tree);
  check_decision_tree(tree, I, options);
  AITOOLS_LOG(log::debug) << "Accuracy: threshold-subset " << accuracy(tree, I, D) << std::endl;
}

TEST_CASE("test_percentage")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  std::size_t n = 20;
  std::size_t m = 8;
  dataset D = make_random_dataset(n, m);
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  decision_tree_options options;
  options.max_depth = n;
  options.max_features = m;
  binary_decision_tree tree = learn_decision_tree(D, I, options, threshold_plus_single_split_family(D, options),
                                                  gain1(options.imp_measure), node_is_finished);
  double acc = accuracy(tree, I, D);
  AITOOLS_LOG(log::debug) << "Accuracy: " << acc << std::endl;
  CHECK_LT(std::abs(acc - 1.0), 0.0001);
  std::cout << tree;
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
  const auto& trees = forest.trees();
  for (std::size_t i = 0; i < trees.size(); i++)
  {
    AITOOLS_LOG(log::debug) << "=== tree " << i << "===" << std::endl;
    const auto& tree = trees[i];
    print_decision_tree(tree);
  }
}

TEST_CASE("test_apply_split")
{
  using namespace aitools;
  std::mt19937 mt{std::random_device{}()};

  std::string D_text =
    "dataset: 1.0\n"
    "category_counts: 4 5 2\n"
    "2 4 0\n"
    "1 3 0\n"
    "3 3 1\n"
    "2 3 1\n"
  ;
  dataset D = parse_dataset(D_text);
  std::vector<std::uint32_t> I = {0, 1, 2, 3};
  AITOOLS_LOG(log::debug) << D << std::endl;

  threshold_split split(0, 2);
  auto [I1, I2] = apply_split(split, D, I, mt, false);
  CHECK_EQ(I1.size(), 1);
  CHECK_EQ(I2.size(), 3);
  CHECK_EQ(I.front(), 1);
}

TEST_CASE("test_tree1")
{
  using namespace aitools;
  std::mt19937 mt{std::random_device{}()};
  decision_tree_options options;
  options.imp_measure = aitools::impurity_measure::entropy;
  options.max_depth = 100;

  std::string D_text =
    "dataset: 1.0\n"
    "category_counts: 2 2 2 2\n"
    "0 1 1 1\n"
    "0 1 1 1\n"
    "0 1 0 1\n"
    "0 0 1 1\n"
    "1 1 1 1\n"
    "1 1 1 1\n"
    "1 1 1 0\n"
    "1 1 0 0\n"
    "1 0 1 0\n"
    "1 0 1 0\n"
    "1 0 0 0\n"
  ;
  dataset D = parse_dataset(D_text);
  std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" << D << std::endl;
  std::size_t n = D.X().row_count();
  std::cout << "n = " << n << std::endl;
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  std::cout << "learning tree " << std::endl;
  binary_decision_tree tree = learn_decision_tree(D, I, options, threshold_plus_single_split_family(D, options), gain(options.imp_measure), node_is_finished);
  std::cout << "done " << std::endl;
}

TEST_CASE("test_gini")
{
  using namespace aitools;

  std::vector<std::size_t> D1_counts = { 4, 0 };
  std::vector<std::size_t> D2_counts = { 2, 5 };
  double gain_entropy = gain(aitools::impurity_measure::entropy)(D1_counts, D2_counts);
  double expected = 0.445;
  CHECK_LT(std::abs(gain_entropy - expected), 0.1);
}

TEST_CASE("test_optimization")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::debug);

  std::size_t n = 10;
  std::size_t m = 5;
  dataset D = make_random_dataset(n, m);
  AITOOLS_LOG(log::debug) << D << std::endl;
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  decision_tree_options options;
  options.max_depth = n;
  options.max_features = m;
  std::size_t seed = 123456;
  binary_decision_tree tree1 = learn_decision_tree(D, I, options, threshold_split_family(D, options), gain(options.imp_measure), node_is_finished, seed);
  options.optimization = true;
  binary_decision_tree tree2 = learn_decision_tree(D, I, options, threshold_split_family(D, options), gain(options.imp_measure), node_is_finished, seed);
  CHECK_EQ(tree1.vertices().size(), tree2.vertices().size());
}

TEST_CASE("test_subset_split")
{
  using namespace aitools;

  std::string dataset_text =
    "dataset: 1.0\n"
    "category_counts: 10 4\n"
    "0 0\n"
    "1 2\n"
    "2 1\n"
    "3 1\n"
    "4 2\n"
    "5 0\n"
    "6 2\n"
    "7 3\n"
    "8 0\n"
    "9 3\n"
    ;

  dataset D = parse_dataset(dataset_text);
  const auto& X = D.X();
  const auto& ncat = D.category_counts();
  std::size_t n = X.row_count();
  std::size_t K = D.class_count();
  decision_tree_options options;
  options.max_categorical_size = ncat[0];
  options.min_samples_leaf = 3;
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  std::vector<std::size_t> D1_counts(K);
  std::vector<std::size_t> D2_counts(K);
  std::vector<std::size_t> D_counts(K);
  D.compute_class_counts(I, D_counts);
  std::size_t v = 0;
  std::vector<subset_split> splits;
  enumerate_subset_splits(D, I, v, options, D1_counts, D2_counts, [&](const subset_split& split, const std::vector<std::size_t>& D1_counts, const std::vector<std::size_t>& D2_counts)
  {
    AITOOLS_LOG(log::debug) << split << " score = " << gain(aitools::impurity_measure::entropy)(D1_counts, D2_counts) << " counts = " << print_list(D1_counts) << " " << print_list(D2_counts) << std::endl;
    CHECK(sum(D1_counts) >= options.min_samples_leaf);
    CHECK(sum(D2_counts) >= options.min_samples_leaf);
    for (std::size_t k = 0; k < K; k++)
    {
      CHECK(D1_counts[k] + D2_counts[k] == D_counts[k]);
    }
    splits.push_back(split);
  });
  std::set<subset_split> splits1(splits.begin(), splits.end());
  CHECK(splits.size() == splits1.size());
}

TEST_CASE("test_topological_ordering")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::verbose);

  std::size_t n = 20;
  std::size_t m = 8;
  dataset D = make_random_dataset(n, m);
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  decision_tree_options options;
  options.max_depth = n;
  options.max_features = m;
  binary_decision_tree tree = learn_decision_tree(D, I, options, threshold_plus_single_split_family(D, options), gain(options.imp_measure), node_is_finished);
  std::vector<std::uint32_t> order = topological_ordering(tree);
  AITOOLS_LOG(log::verbose) << "topological ordering " << aitools::print_list(order) << std::endl;

  visit_nodes_bfs(tree, [&](const binary_decision_tree::vertex& u, std::size_t ui, std::size_t depth)
  {
    if (!u.is_leaf())
    {
      CHECK(order[ui] < order[u.left]);
      CHECK(order[ui] < order[u.right]);
    }
  });

  std::vector<std::uint32_t> expected_order(tree.vertices().size());
  std::iota(expected_order.begin(), expected_order.end(), 0);
  AITOOLS_LOG(log::verbose) << "expected topologigal ordering " << aitools::print_list(expected_order) << std::endl;
  CHECK(order == expected_order);
}

TEST_CASE("test_depth")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::verbose);

  std::size_t n = 20;
  std::size_t m = 8;
  dataset D = make_random_dataset(n, m);
  std::vector<std::uint32_t> I(n);
  std::iota(I.begin(), I.end(), 0);
  AITOOLS_LOG(log::debug) << D << std::endl;
  decision_tree_options options;
  options.max_depth = n;
  options.max_features = m;
  binary_decision_tree tree = learn_decision_tree(D, I, options, threshold_plus_single_split_family(D, options), gain(options.imp_measure), node_is_finished);
  std::vector<std::size_t> tree_depth = decision_tree_depth(tree);
  AITOOLS_LOG(log::verbose) << "depth " << aitools::print_list(tree_depth) << std::endl;

  visit_nodes_bfs(tree, [&tree_depth](const binary_decision_tree::vertex& u, std::size_t ui, std::size_t depth)
  {
    CHECK(tree_depth[ui] == depth);
  });
}

TEST_CASE("test_io")
{
  using namespace aitools;

  std::string text = R"(
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
vertex: 16 [] NoSplit() 2 3  )";
  binary_decision_tree tree = parse_decision_tree(text);

  std::ostringstream out;
  out << tree;
  std::string text1 = out.str();
  std::cout << "\n" << text1 << "\n";
  CHECK(utilities::trim_copy(text) == utilities::trim_copy(text1));
}