// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/probabilistic_circuits/algorithms.h
/// \brief add your file description here.

#ifndef AITOOLS_PROBABILISTIC_CIRCUITS_ALGORITHMS_H
#define AITOOLS_PROBABILISTIC_CIRCUITS_ALGORITHMS_H

#include <cmath>
#include "aitools/datasets/dataset.h"
#include "aitools/numerics/math_utility.h"
#include "aitools/probabilistic_circuits/probabilistic_circuit.h"
#include "aitools/utilities/container_utility.h"

namespace aitools {

/// \brief Visits the nodes of the probabilistic circuit in breadth first order. It calls the function f on each vertex
/// \c u of the PC using <tt>f(u, depth)</tt>, where \c depth is the depth of the node in the circuit.
template <typename Function>
void visit_nodes_bfs(const probabilistic_circuit& pc, Function f)
{
  std::size_t depth = 0;
  std::deque<pc_node_ptr> todo = { pc.root() };
  std::size_t level = 1; // the number of nodes in the current level
  while (!todo.empty())
  {
    auto u = todo.front();
    todo.pop_front();
    level--;
    if (!u->is_leaf())
    {
      for (const auto& v: u->successors())
      {
        todo.push_back(v);
      }
    }
    f(u, depth);
    if (level == 0)
    {
      depth++;
      level = todo.size();
    }
  }
}

/// \brief Returns the number of vertices in the probabilistic circuit \c pc.
std::size_t probabilistic_circuit_size(const probabilistic_circuit& pc);

/// \brief The \c topological_ordering algorithm creates a linear ordering of the vertices such that if
/// edge (u,v) appears in the graph, then v comes before u in the ordering.
/// \pre The graph must be a tree, with root u0
std::vector<pc_node_ptr> topological_ordering(const probabilistic_circuit& pc);

inline
double evi_query_recursive(const probabilistic_circuit& pc, const std::vector<double>& x)
{
  return pc.root()->evi(x);
}

inline
double evi_query_iterative(const probabilistic_circuit& pc, const std::vector<double>& x, const std::vector<pc_node_ptr>& U)
{
  for (const auto& u: U)
  {
    u->evi_iterative(x);
  }
  return pc.root()->value;
}

inline
double evi_query_iterative(const probabilistic_circuit& pc, const std::vector<double>& x)
{
  return evi_query_iterative(pc, x, topological_ordering(pc));
}

inline
double log_evi_query_iterative(const probabilistic_circuit& pc, const std::vector<double>& x, const std::vector<pc_node_ptr>& U)
{
  for (const auto& u: U)
  {
    u->log_evi_iterative(x);
  }
  return pc.root()->value;
}

inline
double log_evi_query_iterative(const probabilistic_circuit& pc, const std::vector<double>& x)
{
  return log_evi_query_iterative(pc, x, topological_ordering(pc));
}

/// \brief Draw one random sample of the probabilistic circuit \c pc
std::vector<double> sample_pc(const probabilistic_circuit& pc, std::mt19937& rng);

/// \brief Draw n random samples of the probabilistic circuit \c pc and put them in a dataset
dataset sample_pc(const probabilistic_circuit& pc, std::size_t n, std::mt19937& rng);

/// \brief Returns \c true if the sums of the weights of all sum nodes are in the interval [1-tolerance, 1+tolerance]
bool is_normalized(const probabilistic_circuit& pc, double tolerance = 1e-10);

/// \brief Returns \c true if all sum nodes in the probabilistic circuit \c pc are smooth.
bool is_smooth(const probabilistic_circuit& pc);

/// \brief Returns \c true if all product nodes in the probabilistic circuit \c pc are decomposable.
bool is_decomposable(const probabilistic_circuit& pc);

/// \brief Does same sanity checks on the probabilistic circuit \c pc.
bool is_valid(const probabilistic_circuit& pc);

} // namespace aitools

#endif // AITOOLS_PROBABILISTIC_CIRCUITS_ALGORITHMS_H
