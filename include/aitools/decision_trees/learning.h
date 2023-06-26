// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_trees/learning.h
/// \brief add your file description here.

#ifndef AITOOLS_DECISION_TREES_LEARNING_H
#define AITOOLS_DECISION_TREES_LEARNING_H

#include "aitools/decision_trees/decision_tree.h"

namespace aitools {

/// \brief Algorithm for learning a binary decision tree from a dataset \c D. The vertices in the tree are guaranteed
/// to be in topological order.
/// \tparam SplitFamily A type that models a family of decision tree splits. An object \c split_family should have a method
/// \c enumerate that enumerates all possible splits for a given index range \c I and a set of variable indices \c V
/// with the following signature: <tt>const index_range& I, const std::vector<std::size_t>& V, ReportSplit report_split)</tt>.
/// \tparam Gain A function type for computing the gain of a binary split with the following signature:
/// <tt>gain(const NumberSequence& D1_counts, const NumberSequence& D2_counts)</tt>, where \c D1_counts and
/// \c D2_counts contain the class counts of the two partitions \c D1 and \c D2 of data set \c D.
/// \tparam StopCriterion A function type for determining whether a node does not need to be split any further, with the
/// following signature: <tt>node_finished(const binary_decision_tree::vertex& u, const dataset& D, std::size_t depth, const split_options& options)</tt>.
/// \param D A data set
/// \param I An index range of samples belonging to the decision tree
/// \param options The split options
/// \param split_family A family of decision tree splits
/// \param gain A gain function
/// \param stop A function that determines if a node does not need to be split any further
/// \param seed A seed value for the random generator
/// \return A decision tree for the samples in the index range \c I
template<typename SplitFamily, typename Gain, typename StopCriterion>
binary_decision_tree learn_decision_tree(const dataset& D,
                                         const std::vector<std::uint32_t>& I,
                                         const decision_tree_options& options,
                                         SplitFamily split_family,
                                         Gain gain,
                                         StopCriterion stop,
                                         std::size_t seed = std::random_device{}())
{
  using vertex = binary_decision_tree::vertex;

  binary_decision_tree tree(D, I);

  std::size_t m = D.feature_count();

  // a random number generator
  std::mt19937 rng{seed};

  // variables = [0, 1, ..., m), used for drawing a random sample to make a split
  std::vector<std::size_t> variables(m);
  std::iota(variables.begin(), variables.end(), 0);

  // Z contains indices of variables that are used for splitting a vertex
  std::size_t max_features = std::min(options.max_features, D.feature_count());
  std::vector<std::size_t> Z(max_features);

  std::deque<std::uint32_t> todo = {0};
  std::size_t depth = 0;
  std::size_t level_count = 1; // the number of nodes in the current level
  while (!todo.empty())
  {
    auto ui = todo.front();
    todo.pop_front();
    level_count--;
    vertex& u = tree.find_vertex(ui);
    AITOOLS_LOG(log::debug) << "visit node " << ui << " " << u << std::endl;
    if (!stop(u, D, depth, options))
    {
      // randomly select max_features split variables
      std::sample(variables.begin(), variables.end(), Z.begin(), max_features, rng);

      // enumerate all splits, and select the one with the highest gain
      double best_score = std::numeric_limits<double>::lowest();
      splitting_criterion best_split = std::monostate();
      split_family.enumerate(u.I, Z,
          [&](const splitting_criterion& split, const std::vector<std::size_t>& D1_counts, const std::vector<std::size_t>& D2_counts)
          {
            double score = gain(D1_counts, D2_counts);
            AITOOLS_LOG(log::debug) << split << " score = " << score << " counts = " << print_list(D1_counts) << " " << print_list(D2_counts) << std::endl;
            if (score > best_score)
            {
              best_score = score;
              best_split = split;
            }
          });
      AITOOLS_LOG(log::debug) << "--- best split: " << best_split << " best score = " << best_score << std::endl;

      if (best_split.index() != 0) // a valid split was found
      {
        u.split = best_split;
        auto[I1, I2] = apply_split(best_split, D, u.I, rng, options.support_missing_values);
        std::uint32_t left = tree.add_vertex(vertex(I1));  // N.B. This may invalidate reference u!
        std::uint32_t right = tree.add_vertex(vertex(I2));
        auto& u1 = tree.find_vertex(ui);
        u1.left = left;
        u1.right = right;
        if (depth < options.max_depth)
        {
          todo.push_back(left);
          todo.push_back(right);
        }
      }
    }
    if (level_count == 0)
    {
      depth++;
      level_count = todo.size();
      AITOOLS_LOG(log::debug) << "added " << level_count << " vertices at depth " << depth << std::endl;
    }
  }
  return tree;
}

} // namespace aitools

#endif // AITOOLS_DECISION_TREES_LEARNING_H
