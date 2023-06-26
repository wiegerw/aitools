// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/random_forests/random_forest.h
/// \brief add your file description here.

#ifndef AITOOLS_RANDOM_FORESTS_RANDOM_FOREST_H
#define AITOOLS_RANDOM_FORESTS_RANDOM_FOREST_H

#include <algorithm>
#include <iterator>
#include <utility>
#include "aitools/decision_trees/decision_tree.h"
#include "aitools/datasets/sampling.h"

namespace aitools {

class random_forest
{
  private:
    std::vector<binary_decision_tree> m_trees{};

  public:
    random_forest() = default;

    explicit random_forest(std::vector<binary_decision_tree> trees)
     : m_trees(std::move(trees))
    {}

    const std::vector<binary_decision_tree>& trees() const
    {
      return m_trees;
    }

    std::vector<binary_decision_tree>& trees()
    {
      return m_trees;
    }

    void swap(random_forest& other)
    {
      m_trees.swap(other.m_trees);
    }
};

inline
void swap(random_forest& forest1, random_forest& forest2)
{
  forest1.swap(forest2);
}

/// \brief Saves a decision forest in a simple textual file format
inline
std::ostream& operator<<(std::ostream& out, const random_forest& forest)
{
  out << "random_forest: 1.0\n";
  out << "forest_size: " << forest.trees().size() << '\n';
  for (const auto& tree: forest.trees())
  {
    out << tree;
  }
  return out;
}

} // namespace aitools

#endif // AITOOLS_RANDOM_FORESTS_RANDOM_FOREST_H
