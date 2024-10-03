// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/random_forests/learning.h
/// \brief add your file description here.

#ifndef AITOOLS_RANDOM_FORESTS_LEARNING_H
#define AITOOLS_RANDOM_FORESTS_LEARNING_H

#include <execution>
#include <iostream>
#include <random>
#include "aitools/datasets/sampling.h"
#include "aitools/decision_trees/learning.h"
#include "aitools/random_forests/random_forest.h"

namespace aitools {

struct random_forest_options
{
  /// \brief The number of trees in the forest
  std::size_t forest_size = 100;

  /// \brief The fraction of samples used for learning a tree in the forest
  double sample_fraction = 1;

  /// \brief The sample technique used for learning a tree in the forest
  sample_technique sample_criterion = sample_technique::stratified;
};

inline
std::ostream& operator<<(std::ostream& out, const random_forest_options& options)
{
  out << "forest_size = " << options.forest_size << '\n';
  out << "sample_fraction = " << options.sample_fraction << '\n';
  out << "sample_technique = " << options.sample_criterion << '\n';
  return out;
}

template<typename SplitFamily, typename Gain, typename StopCriterion>
random_forest learn_random_forest_sequential(const dataset& D,
                                             const std::vector <std::uint32_t>& indices,
                                             random_forest_options forest_options,
                                             const decision_tree_options& tree_options,
                                             SplitFamily split_family,
                                             Gain gain,
                                             StopCriterion node_finished,
                                             std::size_t seed = std::random_device{}())
{
  std::mt19937 rng{static_cast<unsigned int>(seed)};
  std::uniform_int_distribution <std::size_t> dist(std::numeric_limits<std::size_t>::min(),
                                                   std::numeric_limits<std::size_t>::max());

  std::vector <binary_decision_tree> trees;
  dataset_sampler sampler(D, indices, forest_options.sample_criterion, dist(rng));

  for (std::size_t i = 0; i < forest_options.forest_size; i++)
  {
    std::vector <std::uint32_t> I = sampler.sample(forest_options.sample_fraction);
    trees.push_back(learn_decision_tree(D, I, tree_options, split_family, gain, node_finished, dist(rng)));
  }
  return random_forest(trees);
}

template<typename SplitFamily, typename Gain, typename StopCriterion>
random_forest learn_random_forest_parallel(const dataset& D,
                                           const std::vector <std::uint32_t>& indices,
                                           random_forest_options forest_options,
                                           const decision_tree_options& tree_options,
                                           SplitFamily split_family,
                                           Gain gain,
                                           StopCriterion node_finished)
{
  std::vector <binary_decision_tree> trees(forest_options.forest_size);
  dataset_sampler sampler(D, indices, forest_options.sample_criterion);

  std::for_each(std::execution::par_unseq, trees.begin(), trees.end(), [&](binary_decision_tree& tree) {
    std::vector <std::uint32_t> I = sampler.sample(forest_options.sample_fraction);
    tree = learn_decision_tree(D, I, tree_options, split_family, gain, node_finished);
  });

  return random_forest(trees);
}

template<typename SplitFamily, typename Gain, typename StopCriterion>
random_forest learn_random_forest(const dataset& D,
                                  const std::vector <std::uint32_t>& indices,
                                  random_forest_options forest_options,
                                  const decision_tree_options& tree_options,
                                  SplitFamily split_family,
                                  Gain gain,
                                  StopCriterion node_finished,
                                  bool sequential,
                                  std::size_t seed = std::random_device{}())
{
  if (sequential)
  {
    return learn_random_forest_sequential(D, indices, forest_options, tree_options, split_family, gain, node_finished,
                                          seed);
  }
  else
  {
    return learn_random_forest_parallel(D, indices, forest_options, tree_options, split_family, gain, node_finished);
  }
}

} // namespace aitools

#endif // AITOOLS_RANDOM_FORESTS_LEARNING_H
