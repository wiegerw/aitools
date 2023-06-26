// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/random_forests/algorithms.h
/// \brief add your file description here.

#ifndef AITOOLS_RANDOM_FORESTS_ALGORITHMS_H
#define AITOOLS_RANDOM_FORESTS_ALGORITHMS_H

#include <algorithm>
#include <execution>
#include "aitools/decision_trees/algorithms.h"
#include "aitools/random_forests/random_forest.h"
#include "aitools/utilities/stack_array.h"

namespace aitools {

/// \brief Executes the decision tree on input x, and returns the predicted class.
/// N.B. This implementation is not efficient for multiple executions.
inline
std::size_t predict(const random_forest& forest, const std::vector<double>& x)
{
  std::size_t K = forest.trees().front().class_count();
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, K);
  std::fill(counts.begin(), counts.end(), 0);

  for (const auto& tree: forest.trees())
  {
    std::size_t k = predict(tree, x);
    counts[k]++;
  }

  auto i = std::max_element(counts.begin(), counts.end());
  return i - counts.begin();
}

/// \brief Executes the random forest on input x, and returns the predicted class.
/// Should be used for multiple executions.
class random_forest_predictor
{
  private:
    const random_forest& m_forest;
    std::vector<decision_tree_predictor> m_predictors;

  public:
    explicit random_forest_predictor(const random_forest& forest)
      : m_forest(forest)
    {
      m_predictors.reserve(forest.trees().size());
      for (const auto& tree: forest.trees())
      {
        m_predictors.emplace_back(tree);
      }
    }

    std::size_t predict(const std::vector<double>& x)
    {
      std::size_t K = m_forest.trees().front().class_count();
      AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, K);
      std::fill(counts.begin(), counts.end(), 0);

      for (const auto& predictor: m_predictors)
      {
        std::size_t k = predictor.predict(x);
        counts[k]++;
      }

      auto i = std::max_element(counts.begin(), counts.end());
      return i - counts.begin();
    }
};

/// \brief Executes the decision tree for the samples in I, and returns the percentage of correct predictions.
inline
double accuracy(const random_forest& forest, const index_range& I, const dataset& D)
{
  const auto& X = D.X();
  const auto& y = D.y();

  random_forest_predictor predictor(forest);
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

/// \brief Executes the decision tree for the samples in I, and returns the percentage of correct predictions.
inline
double accuracy_parallel(const random_forest& forest, const index_range& I, const dataset& D)
{
  const auto& X = D.X();
  const auto& y = D.y();
  std::vector<std::size_t> correct(forest.trees().size());
  std::iota(correct.begin(), correct.end(), 0);

  random_forest_predictor predictor(forest);
  std::for_each(std::execution::par, correct.begin(), correct.end(), [&](std::size_t& c)
  {
    std::size_t i = *(I.begin() + c);
    std::size_t k = predictor.predict(X[i]);
    c = (k == static_cast<std::size_t>(y[i])) ? 1 : 0;
  });
  std::size_t correct_predictions = std::accumulate(correct.begin(), correct.end(), 0);

//  std::size_t correct_predictions = std::transform_reduce(std::execution::par, I.begin(), I.end(), 0ul, std::plus<>(), [&](std::uint32_t i)
//  {
//    std::size_t k = predictor.predict(X[i]);
//    return (k == static_cast<std::size_t>(y[i])) ? 1 : 0;
//  });
//

  return static_cast<double>(correct_predictions) / static_cast<double>(I.size());
}

} // namespace aitools

#endif // AITOOLS_RANDOM_FORESTS_ALGORITHMS_H
