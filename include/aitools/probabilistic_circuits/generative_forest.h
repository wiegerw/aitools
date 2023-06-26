// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/probabilistic_circuits/generative_forest.h
/// \brief add your file description here.

#ifndef AITOOLS_PROBABILISTIC_CIRCUITS_GENERATIVE_FOREST_H
#define AITOOLS_PROBABILISTIC_CIRCUITS_GENERATIVE_FOREST_H

#include <algorithm>
#include <cmath>
#include "aitools/datasets/algorithms.h"
#include "aitools/decision_trees/algorithms.h"
#include "aitools/random_forests/random_forest.h"
#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/probabilistic_circuits/generative_forest_nodes.h"
#include "aitools/probabilistic_circuits/probabilistic_circuit.h"
#include "aitools/utilities/interval.h"

namespace aitools {

/// \brief Fits a normal distribution to random variable \c i using the samples in <tt>u.I</tt>.
/// \param D The data set that contains the samples in <tt>u.I</tt>.
/// \param ab An interval that contains the values of random variable \c i in the samples <tt>u.I</tt>.
/// \return A PC node containing the computed distribution.
std::shared_ptr<pc_node> fit_normal(const binary_decision_tree::vertex& u, const dataset& D, std::size_t i, const interval& ab);

/// \brief Fits a categorical distribution to random variable \c i using the samples in <tt>u.I</tt>.
/// \param D The data set that contains the samples in <tt>u.I</tt>.
/// \return A PC node containing the computed distribution.
std::shared_ptr<pc_node> fit_categorical(const binary_decision_tree::vertex& u, const dataset& D, std::size_t i);

/// \brief Enumerate the nodes in the tree, together with the intervals that constitute the partition of the
/// feature space corresponding to the node. The callback function \c report_node has the following signature:
/// <tt>report_node(vertex& u, std::size_t ui, std::vector<interval>& intervals)</tt>
template <typename ReportNode>
void enumerate_intervals(const binary_decision_tree& tree, std::size_t m, ReportNode report_node)
{
  using vertex = binary_decision_tree::vertex;

  std::vector<interval> intervals{m}; // intervals[i] is the interval for variable i
  std::vector<std::tuple<bool, double&, double>> undo_stack;
  std::vector<std::tuple<std::uint32_t, double&, double>> todo_stack;

  // Returns split.value if split is a threshold split, otherwise x
  auto threshold_value = [](const splitting_criterion& split, double x)
  {
    auto p = std::get_if<threshold_split>(&split);
    return p ? p->value : x;
  };

  auto add_left = [&](const vertex& u)
  {
    std::size_t i = split_variable(u.split);
    interval& I = intervals[i];
    double t = threshold_value(u.split, I.b);
    todo_stack.emplace_back(u.left, I.b, t);
    undo_stack.emplace_back(false, I.b, I.b);
  };

  auto add_right = [&](const vertex& u)
  {
    std::size_t i = split_variable(u.split);
    interval& I = intervals[i];
    double t = threshold_value(u.split, I.a);
    todo_stack.emplace_back(u.right, I.a, t);
    undo_stack.emplace_back(true, I.a, I.a);
  };

  auto root = tree.root();
  report_node(root, 0, intervals);

  if (root.is_leaf())
  {
    return;
  }

  add_right(root);
  add_left(root);
  while (!todo_stack.empty())
  {
    auto& [ui, var, value] = todo_stack.back();
    auto& u = tree.find_vertex(ui);
    var = value;
    report_node(u, ui, intervals);
    todo_stack.pop_back();
    if (u.is_leaf())
    {
      while (!undo_stack.empty())
      {
        auto [last_branch, var_, value_] = undo_stack.back();
        undo_stack.pop_back();
        var_ = value_;
        if (!last_branch)
        {
          break;
        }
      }
    }
    else
    {
      add_right(u);
      add_left(u);
    }
  }
}

/// \brief Assigns a univariate distribution to each leaf node in the vector \c pc_nodes.
void fit_leave_nodes(const binary_decision_tree& tree, std::vector<std::shared_ptr<pc_node>>& pc_nodes, const dataset& D);

/// \brief Converts a decision tree into a generative forest.
std::shared_ptr<pc_node> build_generative_tree(const binary_decision_tree& tree, const dataset& D);

/// \brief Converts a random forest into a generative forest.
probabilistic_circuit build_generative_forest(const random_forest& forest, const dataset& D);

/// \brief Converts a generative forest to a regular probabilistic circuit, by expanding the sum-split nodes.
void expand_sum_split_nodes(probabilistic_circuit& pc);

} // namespace aitools

#endif // AITOOLS_PROBABILISTIC_CIRCUITS_GENERATIVE_FOREST_H
