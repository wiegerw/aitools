// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_tree_impurity.h
/// \brief add your file description here.

#ifndef AITOOLS_IMPURITY_H
#define AITOOLS_IMPURITY_H

#include <cmath>
#include <iostream>
#include <vector>
#include "aitools/numerics/math_utility.h"

namespace aitools {

enum class impurity_measure
{
    gini,
    entropy,
    mis_classification
};

inline
std::ostream& operator<<(std::ostream& out, const impurity_measure& imp)
{
  switch(imp)
  {
    case impurity_measure::gini: out << "gini"; break;
    case impurity_measure::entropy: out << "entropy"; break;
    case impurity_measure::mis_classification: out << "misclassification"; break;
  }
  return out;
}

inline
impurity_measure parse_impurity_measure(const std::string& imp_measure)
{
  if (imp_measure == "gini")
  {
    return aitools::impurity_measure::gini;
  }
  else if (imp_measure == "entropy")
  {
    return aitools::impurity_measure::entropy;
  }
  else if (imp_measure == "misclassification")
  {
    return aitools::impurity_measure::mis_classification;
  }
  throw std::runtime_error("Unknown impurity measure " + imp_measure);
}

template <typename NumberSequence>
double gini_index(const NumberSequence& counts)
{
  double sum = aitools::sum(counts);
  double result = 0;
  for (std::size_t count_k: counts)
  {
    double p_k = count_k / sum;
    result += p_k * p_k;
  }
  return 1 - result;
}

template <typename NumberSequence>
double cross_entropy(const NumberSequence& counts)
{
  double sum = aitools::sum(counts);
  double result = 0;
  for (std::size_t count_k: counts)
  {
    double p_k = count_k / sum + 1e-12; // TODO: document this correction factor
    result += p_k * std::log2(p_k);
  }
  return -result;
}

template <typename NumberSequence>
double mis_classification(const NumberSequence& counts)
{
  double sum = aitools::sum(counts);
  double pk_max = max_value(counts) / sum;
  return 1 - pk_max;
}

template <typename NumberSequence>
double impurity(impurity_measure imp, const NumberSequence& counts)
{
  switch (imp)
  {
    case impurity_measure::gini: return gini_index(counts);
    case impurity_measure::entropy: return cross_entropy(counts);
    case impurity_measure::mis_classification: return mis_classification(counts);
  }
  throw std::runtime_error("unknown impurity measure");
}

struct gain
{
  impurity_measure imp;

  explicit gain(impurity_measure imp_)
   : imp(imp_)
  {}

  template <typename NumberSequence>
  double operator()(const NumberSequence& D1_counts, const NumberSequence& D2_counts) const
  {
    std::size_t K = D1_counts.size();
    std::vector<std::size_t> D_counts(K);
    for (std::size_t i = 0; i < K; i++)
    {
      D_counts[i] = D1_counts[i] + D2_counts[i];
    }

    double D_sum = sum(D_counts);
    double D1_sum = sum(D1_counts);
    double D2_sum = sum(D2_counts);

    return impurity(imp, D_counts) - (D1_sum / D_sum) * impurity(imp, D1_counts) - (D2_sum / D_sum) * impurity(imp, D2_counts);
  }
};

struct gain1
{
  impurity_measure imp;

  explicit gain1(impurity_measure imp_)
  : imp(imp_)
    {}

  template <typename NumberSequence>
  double operator()(const NumberSequence& D1_counts, const NumberSequence& D2_counts) const
  {
    std::size_t left_sum = sum(D1_counts);
    std::size_t right_sum = sum(D2_counts);
    return -(left_sum * impurity(imp, D1_counts) + right_sum * impurity(imp, D2_counts));
  }
};

} // namespace aitools

#endif // AITOOLS_IMPURITY_H
