// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file src/decision_trees.cpp
/// \brief add your file description here.

#include <algorithm>
#include <deque>
#include <utility>
#include "aitools/decision_trees/impurity.h"
#include "aitools/decision_trees/algorithms.h"
#include "aitools/datasets/missing.h"
#include "aitools/numerics/matrix.h"
#include "aitools/utilities/container_utility.h"
#include "aitools/utilities/iterator_range.h"
#include "aitools/utilities/logger.h"
#include "aitools/utilities/random.h"

namespace aitools {

namespace detail {

/// \brief Executes the decision tree on input x and returns the index of the leaf in which the execution ends.
inline
std::uint32_t execute_decision_tree(const binary_decision_tree& tree, const std::vector<double>& x)
{
  struct execute_split_visitor
  {
    const binary_decision_tree::vertex& u;
    const std::vector<double>& x;

    execute_split_visitor(const binary_decision_tree::vertex& u_, const std::vector<double>& x_)
    : u(u_), x(x_)
    {
    }

    std::size_t operator()(const threshold_split& split) const
    {
      return x[split.variable] < split.value ? u.left : u.right;
    }

    std::size_t operator()(const single_split& split) const
    {
      return x[split.variable] == split.value ? u.left : u.right;
    }

    std::size_t operator()(const subset_split& split) const
    {
      auto k = static_cast<std::size_t>(x[split.variable]);
      return utilities::is_bit_set(split.mask, k) ? u.left : u.right;
    }

    std::size_t operator()(const std::monostate&) const
    {
      throw std::runtime_error("cannot execute an undefined split");
    }
  };

  // AITOOLS_LOG(log::debug) << "execute " << aitools::print_list(x) << std::endl;
  std::size_t index = 0; // points to the root of the tree
  while (true)
  {
    const binary_decision_tree::vertex& u = tree.find_vertex(index);
    // AITOOLS_LOG(log::debug) << "node = " << index << " " << aitools::print_list(u.I) << std::endl;
    if (u.is_leaf())
    {
      break;
    }
    execute_split_visitor visitor(u, x);
    index = std::visit(visitor, u.split);
  }
  return index;
}

std::size_t majority_class(const binary_decision_tree& tree, const binary_decision_tree::vertex& u)
{
  const auto& y = tree.classes();

  if (u.I.size() == 1)
  {
    auto i = u.I.front();
    return static_cast<std::size_t>(y[i]);
  }

  std::size_t K = tree.class_count();
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, K);
  compute_class_counts(tree, u.I, counts);
  AITOOLS_DECLARE_STACK_ARRAY(max_counts, std::size_t, K);
  std::size_t N = 1; // the number of maximum values
  max_counts[0] = 0;
  for (std::size_t j = 1; j < K; j++)
  {
    if (counts[j] > counts[max_counts.front()])
    {
      max_counts[0] = j;
      N = 1;
    }
    else if (counts[j] == counts[max_counts.front()])
    {
      max_counts[N] = j;
      N++;
    }
  }
  auto k = random_integer<std::size_t>(0, N - 1);
  return max_counts[k];
}

} // namespace detail

std::size_t predict(const binary_decision_tree& tree, const std::vector<double>& x)
{
  std::uint32_t ui = detail::execute_decision_tree(tree, x);
  const binary_decision_tree::vertex& u = tree.find_vertex(ui);
  return detail::majority_class(tree, u);
}

decision_tree_predictor::decision_tree_predictor(const binary_decision_tree& tree)
: m_tree(tree), m_values(tree.vertices().size())
{
  visit_nodes_bfs(tree, [this, &tree](const binary_decision_tree::vertex& u, std::size_t ui, std::size_t depth) {
    m_values[ui] = detail::majority_class(tree, u);
  });
}

std::size_t decision_tree_predictor::predict(const std::vector<double>& x) const
{
  std::uint32_t ui = detail::execute_decision_tree(m_tree, x);
  return m_values[ui];
}

std::vector<std::size_t> decision_tree_depth(const binary_decision_tree& tree)
{
  using vertex = binary_decision_tree::vertex;

  std::size_t n = tree.vertices().size();
  std::vector<std::size_t> result(n);

  visit_nodes_bfs(tree, [&result](const vertex& u, std::size_t ui, std::size_t depth) {
    result[ui] = depth;
  });

  return result;
}

std::size_t leaf_count(const binary_decision_tree& tree)
{
  std::size_t result = 0;
  visit_nodes_bfs(tree, [&result](const binary_decision_tree::vertex& u, std::size_t ui, std::size_t depth) {
    if (u.is_leaf())
    {
      result++;
    }
  });
  return result;
}

std::vector<std::uint32_t> topological_ordering(const binary_decision_tree& tree)
{
  using vertex = binary_decision_tree::vertex;

  std::size_t n = tree.vertices().size();
  std::vector<std::uint32_t> result;
  result.reserve(n);

  visit_nodes_bfs(tree, [&result](const vertex& u, std::size_t ui, std::size_t depth) {
    result.push_back(ui);
  });

  return result;
}

void print_decision_tree(const binary_decision_tree& tree)
{
  visit_nodes_bfs(tree, [&](const binary_decision_tree::vertex& u, std::size_t ui, std::size_t depth) {
    std::cout << "node " << ui
    << ": depth = " << depth
    << " " << u
    << " split = " << u.split
    << " cross_entropy = " << cross_entropy(tree, u)
    << " gini_index = " << gini_index(tree, u)
    << " mis_classification = " << mis_classification(tree, u)
    << " variance = " << variance(tree.classes(), u)
    << std::endl;
  });
}

std::ostream& operator<<(std::ostream& out, const binary_decision_tree::vertex& u)
{
  auto print_index = [](std::uint32_t i)
  {
    return i == binary_decision_tree::undefined_index ? "u" : std::to_string(i);
  };

  return out << "left = " << print_index(u.left) << ", right = " << print_index(u.right) << ", I = " << u.I;
}

std::ostream& operator<<(std::ostream& to, const binary_decision_tree& tree)
{
  to << "binary_decision_tree: 1.0\n";
  std::size_t N = tree.vertices().size();
  to << "tree_size: " << N << "\n";
  to << "category_counts: " << print_container(tree.category_counts()) << "\n";
  to << "classes: " << print_container(tree.classes()) << "\n";
  to << "indices: " << print_container(tree.indices()) << "\n";
  auto Ibegin = tree.root().I.begin();
  for (std::size_t i = 0; i < N; i++)
  {
    const auto& u = tree.find_vertex(i);
    auto i0 = u.I.begin() - Ibegin;
    auto i1 = u.I.end() - Ibegin;
    if (u.is_leaf())
    {
      to << "vertex: " << i << " [] " << u.split << ' ' << i0 << ' ' << i1 << "\n";
    }
    else
    {
      to << "vertex: " << i << " [" << u.left << ' ' << u.right << "] " << u.split << ' ' << i0 << ' ' << i1 << "\n";
    }
  }
  return to;
}

double accuracy(const binary_decision_tree& tree, const index_range& I, const dataset& D)
{
  const auto& X = D.X();
  const auto& y = D.y();

  decision_tree_predictor predictor(tree);
  std::size_t correct_predictions = 0;
  for (auto i: I)
  {
    std::size_t k = predictor.predict(X[i]);
    if (k == static_cast<std::size_t>(y[i]))
    {
      correct_predictions++;
    }
  }
  return static_cast<double>(correct_predictions) / static_cast<double>(I.size());
}

} // namespace aitools