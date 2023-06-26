// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/probabilistic_circuit.h
/// \brief add your file description here.

#ifndef AITOOLS_PROBABILISTIC_CIRCUITS_PROBABILISTIC_CIRCUIT_H
#define AITOOLS_PROBABILISTIC_CIRCUITS_PROBABILISTIC_CIRCUIT_H

#include <cmath>
#include <iostream>
#include <memory>
#include <stack>
#include <utility>
#include <vector>
#include "aitools/probabilistic_circuits/probabilistic_circuit_nodes.h"

namespace aitools {

class probabilistic_circuit
{
  private:
    pc_node_ptr m_root;
    std::vector<unsigned int> m_category_counts;

  public:
    probabilistic_circuit() = default;

    probabilistic_circuit(pc_node_ptr root, std::vector<unsigned int> category_counts)
     : m_root(std::move(root)), m_category_counts(std::move(category_counts))
    {}

    const pc_node_ptr& root() const
    {
      return m_root;
    }

    pc_node_ptr& root()
    {
      return m_root;
    }

    std::size_t feature_count() const
    {
      return m_category_counts.size();
    }

    const std::vector<unsigned int>& category_counts() const
    {
      return m_category_counts;
    }

    std::vector<unsigned int>& category_counts()
    {
      return m_category_counts;
    }

    void swap(probabilistic_circuit& other)
    {
      m_root.swap(other.m_root);
      m_category_counts.swap(other.m_category_counts);
    }
 };

inline
void swap(probabilistic_circuit& pc1, probabilistic_circuit& pc2)
{
  pc1.swap(pc2);
}

} // namespace aitools

#endif// AITOOLS_PROBABILISTIC_CIRCUITS_PROBABILISTIC_CIRCUIT_H
