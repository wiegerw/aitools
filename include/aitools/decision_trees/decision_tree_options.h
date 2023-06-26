// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_tree_options.h
/// \brief add your file description here.

#ifndef AITOOLS_DECISION_TREE_OPTIONS_H
#define AITOOLS_DECISION_TREE_OPTIONS_H

#include <iostream>
#include "impurity.h"

namespace aitools {

struct decision_tree_options
{
  /// \brief The impurity measure
  impurity_measure imp_measure;

  /// \brief The minimum number of samples in a leaf
  std::size_t min_samples_leaf;

  /// \brief The maximum number of variables that is considered for splitting a node
  std::size_t max_features;

  /// \brief The maximum depth of the decision tree
  std::size_t max_depth;

  /// \brief The maximum class count for a categorical variable. If the class count of a categorical variable is greater
  /// or equal to this value, it will be treated as a continuous variable.
  std::size_t max_categorical_size;

  bool support_missing_values;

  bool optimization = false;

  explicit decision_tree_options(impurity_measure imp_measure_ = impurity_measure::gini,
                                 std::size_t min_samples_leaf_ = 1,
                                 std::size_t max_features_ = 1000000,
                                 std::size_t max_depth_ = 1000000,
                                 std::size_t max_categorical_size_ = 10,
                                 bool support_missing_values_ = false)
    : imp_measure(imp_measure_),
      min_samples_leaf(min_samples_leaf_),
      max_features(max_features_),
      max_depth(max_depth_),
      max_categorical_size(max_categorical_size_),
      support_missing_values(support_missing_values_)
  {}
};

inline
std::ostream& operator<<(std::ostream& out, const decision_tree_options& options)
{
  out << "imp_measure = " << options.imp_measure << '\n';
  out << "min_samples_leaf = " << options.min_samples_leaf <<  '\n';
  out << "max_features = " << options.max_features <<  '\n';
  out << "max_depth = " << options.max_depth <<  '\n';
  out << "max_categorical_size = " << options.max_categorical_size <<  '\n';
  out << "support_missing_values = " << options.support_missing_values <<  '\n';
  return out;
}

} // namespace aitools

#endif // AITOOLS_DECISION_TREE_OPTIONS_H
