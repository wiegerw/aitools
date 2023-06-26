// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/binary_decision_tree.h
/// \brief add your file description here.

#ifndef AITOOLS_DECISION_TREE_H
#define AITOOLS_DECISION_TREE_H

#include <iostream>
#include <limits>
#include <vector>
#include "aitools/datasets/dataset.h"
#include "aitools/decision_trees/splitters.h"

namespace aitools {

class binary_decision_tree
{
  public:
    static constexpr std::uint32_t undefined_index = std::numeric_limits<std::uint32_t>::max();

    struct vertex
    {
      index_range I;                         // The range of indices belonging to this vertex
      std::uint32_t left = undefined_index;  // Index of the left child vertex.
      std::uint32_t right = undefined_index; // Index of the right child vertex.
      splitting_criterion split = std::monostate();     // The splitter

      vertex() = default;

      explicit vertex(const index_range& I_)
       : I(I_)
      {}

      explicit vertex(const index_range& I_, std::uint32_t left_, std::uint32_t right_, const splitting_criterion& split_)
        : I(I_), left(left_), right(right_), split(split_)
      {}

      bool is_leaf() const
      {
        return left == undefined_index && right == undefined_index;
      }
    };

  private:
    std::vector<vertex> m_vertices;
    std::vector<std::uint32_t> m_indices; // indices in the dataset
    std::vector<std::uint32_t> m_classes; // the class values corresponding to the indices
    std::vector<unsigned int> m_category_counts;

    // N.B. The vertices cannot be copied as is, because the index ranges need to be recomputed
    void copy_vertices(const binary_decision_tree& other)
    {
      m_vertices.clear();
      m_vertices.reserve(other.m_vertices.size());
      for (const vertex& v: other.m_vertices)
      {
        m_vertices.emplace_back(convert_index_range(v.I, other.m_indices, m_indices), v.left, v.right, v.split);
      }
    }

  public:
    binary_decision_tree() = default;

    explicit binary_decision_tree(const dataset& D, std::vector<std::uint32_t> indices)
      : m_indices(std::move(indices)), m_classes(D.classes()), m_category_counts(D.category_counts())
    {
      m_vertices.reserve(16);
      index_range I(m_indices.begin(), m_indices.end());
      m_vertices.emplace_back(I); // add a root to the tree
    }

    binary_decision_tree(const binary_decision_tree& other)
     : m_indices(other.m_indices), m_classes(other.m_classes), m_category_counts(other.m_category_counts)
    {
      copy_vertices(other);
    }

    binary_decision_tree& operator=(const binary_decision_tree& other)
    {
      if (this != &other)
      {
        copy_vertices(other);
        m_indices = other.m_indices;
        m_classes = other.m_classes;
        m_category_counts = other.m_category_counts;
      }
      return *this;
    }

    binary_decision_tree& operator=(binary_decision_tree&& other) noexcept
    {
      if (this != &other)
      {
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_classes = std::move(other.m_classes);
        m_category_counts = std::move(other.m_category_counts);
      }
      return *this;
    }

    binary_decision_tree(binary_decision_tree&& other) noexcept
    {
      m_vertices = std::move(other.m_vertices);
      m_indices = std::move(other.m_indices);
      m_classes = std::move(other.m_classes);
      m_category_counts = std::move(other.m_category_counts);
    }

    const std::vector<vertex>& vertices() const
    {
      return m_vertices;
    }

    std::vector<vertex>& vertices()
    {
      return m_vertices;
    }

    const std::vector<std::uint32_t>& indices() const
    {
      return m_indices;
    }

    std::vector<std::uint32_t>& indices()
    {
      return m_indices;
    }

    const std::vector<std::uint32_t>& classes() const
    {
      return m_classes;
    }

    std::vector<std::uint32_t>& classes()
    {
      return m_classes;
    }

    std::size_t feature_count() const
    {
      return m_category_counts.size() - 1;
    }

    const std::vector<unsigned int>& category_counts() const
    {
      return m_category_counts;
    }

    std::vector<unsigned int>& category_counts()
    {
      return m_category_counts;
    }

    const vertex& root() const
    {
      return m_vertices.front();
    }

    std::size_t class_count() const
    {
      return m_category_counts.back();
    }

    vertex& root()
    {
      return m_vertices.front();
    }

    std::uint32_t add_vertex(const vertex& u)
    {
      std::uint32_t index = m_vertices.size();
      m_vertices.push_back(u);
      return index;
    }

    const vertex& find_vertex(std::uint32_t i) const
    {
      return m_vertices[i];
    }

    vertex& find_vertex(std::uint32_t i)
    {
      return m_vertices[i];
    }

    void swap(binary_decision_tree& other)
    {
      m_vertices.swap(other.m_vertices);
      m_indices.swap(other.m_indices);
      m_classes.swap(other.m_classes);
      m_category_counts.swap(other.m_category_counts);
    }
};

inline
void swap(binary_decision_tree& tree1, binary_decision_tree& tree2)
{
  tree1.swap(tree2);
}

std::ostream& operator<<(std::ostream& to, const binary_decision_tree& tree);

std::ostream& operator<<(std::ostream& out, const binary_decision_tree::vertex& u);

} // namespace aitools

#endif // AITOOLS_DECISION_TREE_H
