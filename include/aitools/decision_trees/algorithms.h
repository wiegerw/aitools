// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_trees/algorithms.h
/// \brief add your file description here.

#ifndef AITOOLS_DECISION_TREES_ALGORITHMS_H
#define AITOOLS_DECISION_TREES_ALGORITHMS_H

#include <deque>
#include "aitools/decision_trees/decision_tree.h"

namespace aitools {

/// \brief Visits the nodes of the decision tree in breadth first order. It calls the function f on each vertex \c u
/// of the tree using <tt>f(u, ui, depth)</tt>, where \c ui is the index of the node in the node array of the tree
/// and \c depth is the depth of the node in the tree.
template<typename Function>
void visit_nodes_bfs(const binary_decision_tree& tree, Function f)
{
  using vertex = binary_decision_tree::vertex;
  auto undefined = binary_decision_tree::undefined_index;

  std::size_t depth = 0;
  std::deque<std::uint32_t> todo = {0};
  std::size_t level = 1; // the number of nodes in the current level
  while (!todo.empty())
  {
    auto ui = todo.front();
    todo.pop_front();
    level--;
    const vertex& u = tree.find_vertex(ui);
    if (u.left != undefined)
    {
      todo.push_back(u.left);
    }
    if (u.right != undefined)
    {
      todo.push_back(u.right);
    }
    f(u, ui, depth);
    if (level == 0)
    {
      depth++;
      level = todo.size();
    }
  }
}

template<typename NumberSequence>
double variance(const NumberSequence& classes, const binary_decision_tree::vertex& u)
{
  const auto& y = classes;

  double sum = 0;
  for (std::size_t i: u.I)
  {
    sum += y[i];
  }
  double average = sum / u.I.size();

  double total = 0;
  for (std::size_t i: u.I)
  {
    total += square(y[i] - average);
  }
  return total / u.I.size();
}

inline
double mis_classification(const dataset& D, const binary_decision_tree::vertex& u)
{
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, D.class_count());
  D.compute_class_counts(u.I, counts);
  return mis_classification(counts);
}

inline
double cross_entropy(const dataset& D, const binary_decision_tree::vertex& u)
{
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, D.class_count());
  D.compute_class_counts(u.I, counts);
  return cross_entropy(counts);
}

inline
double gini_index(const dataset& D, const binary_decision_tree::vertex& u)
{
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, D.class_count());
  D.compute_class_counts(u.I, counts);
  return gini_index(counts);
}

template<typename NumberSequence>
void compute_class_counts(const binary_decision_tree& tree, const index_range& I, NumberSequence& counts)
{
  const auto& y = tree.classes();
  std::fill(counts.begin(), counts.end(), 0);
  for (std::uint32_t i: I)
  {
    assert(!is_missing(y[i]));
    auto k = y[i];
    counts[k]++;
  }
}

inline
double mis_classification(const binary_decision_tree& tree, const binary_decision_tree::vertex& u)
{
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, tree.class_count());
  compute_class_counts(tree, u.I, counts);
  return mis_classification(counts);
}

inline
double cross_entropy(const binary_decision_tree& tree, const binary_decision_tree::vertex& u)
{
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, tree.class_count());
  compute_class_counts(tree, u.I, counts);
  return cross_entropy(counts);
}

inline
double gini_index(const binary_decision_tree& tree, const binary_decision_tree::vertex& u)
{
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, tree.class_count());
  compute_class_counts(tree, u.I, counts);
  return gini_index(counts);
}

inline
bool node_is_finished(const binary_decision_tree::vertex& u, const dataset& D, std::size_t depth, const decision_tree_options& options)
{
  return (u.I.size() <= options.min_samples_leaf) || (mis_classification(D, u) <= 0.01) || (depth >= options.max_depth);
}

void print_decision_tree(const binary_decision_tree& tree);

std::size_t leaf_count(const binary_decision_tree& tree);

/// \brief The \c topological_ordering algorithm creates a linear ordering of the vertices such that if
/// edge (u,v) appears in tree, then u comes before v in the ordering.
std::vector<std::uint32_t> topological_ordering(const binary_decision_tree& tree);

/// \brief Returns an array containing the depth of each node in the tree.
std::vector<std::size_t> decision_tree_depth(const binary_decision_tree& tree);

/// \brief Executes the decision tree on input x, and returns the predicted class.
/// N.B. This implementation is not efficient for multiple executions.
std::size_t predict(const binary_decision_tree& tree, const std::vector<double>& x);

/// \brief Executes the decision tree on input x, and returns the predicted class.
/// Should be used for multiple executions.
class decision_tree_predictor
{
  private:
    const binary_decision_tree& m_tree;
    std::vector<std::uint32_t> m_values;

  public:
    explicit decision_tree_predictor(const binary_decision_tree& tree);

    [[nodiscard]] std::size_t predict(const std::vector<double>& x) const;
};

/// \brief Executes the decision tree for the samples in I, and returns the percentage of correct predictions.
double accuracy(const binary_decision_tree& tree, const index_range& I, const dataset& D);

} // namespace aitools

#endif // AITOOLS_DECISION_TREES_ALGORITHMS_H
