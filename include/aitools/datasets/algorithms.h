// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/datasets/algorithms.h
/// \brief add your file description here.

#ifndef AITOOLS_DATASETS_ALGORITHMS_H
#define AITOOLS_DATASETS_ALGORITHMS_H

#include <limits>
#include "aitools/datasets/dataset.h"
#include "aitools/datasets/missing.h"
#include "aitools/numerics/math_utility.h"
#include "aitools/utilities/counting_iterator.h"
#include "aitools/utilities/iterator_range.h"

namespace aitools {

template <class T>
utilities::iterator_range<utilities::counting_iterator<T>> xrange(T to)
{
  return utilities::make_iterator_range(utilities::counting_iterator<T>(0), utilities::counting_iterator<T>(to));
}

template <typename Range>
double mean(const dataset& D, const Range& I, std::size_t v)
{
  const auto& X = D.X();

  double total = 0.0;
  std::size_t count = 0ul;
  for (auto i: I)
  {
    auto x_i = X[i][v];
    if (!is_missing(x_i))
    {
      total += x_i;
      count++;
    }
  }
  return total / count;
}

template <typename IndexRange>
std::pair<double, double> mean_standard_deviation(const dataset& D, const IndexRange& I, std::size_t v)
{
  const auto& X = D.X();

  double mu = mean(D, I, v);
  double sigma;
  double variance = 0.0;
  std::size_t count = 0ul;
  for (auto i: I)
  {
    auto x_i = X[i][v];
    if (!is_missing(x_i))
    {
      variance += square(x_i - mu);
      count++;
    }
  }
  if (count == 0) // only missing values!
  {
    mu = 0;
    sigma = 1;
    AITOOLS_LOG(log::warning) << "Found only missing values in node with variable " << v << std::endl;
  }
  else
  {
    sigma = std::sqrt(variance / count);
    if (sigma == 0) // TODO: find a way to avoid this check
    {
      sigma = std::numeric_limits<double>::min();
    }
  }
  return { mu, sigma };
}

/// \brief Computes the fractions of values in an array \c x.
/// \param I The range of indices that is taken into account.
/// \param v The index of a categorical variable.
// counts[k] = |{ i in I | x[i] = k }| / |I|
template <typename Array, typename IndexRange>
void compute_fractions(const Array& x, const IndexRange& I, std::vector<double>& counts)
{
  std::fill(counts.begin(), counts.end(), 0);
  std::size_t n = 0;
  for (std::uint32_t i: I)
  {
    double x_i = x[i];
    if (!is_missing(x_i))
    {
      auto k = static_cast<std::size_t>(x_i);
      counts[k] += 1;
      n++;
    }
  }
  if (n == 0)
  {
    throw std::runtime_error("compute_fractions: all values are missing");
  }
  for (auto& c: counts)
  {
    c = c / n;
  }
}

template <typename Array>
std::size_t missing_value_count(const Array& x)
{
  std::size_t n = x.size();
  std::size_t result = 0;
  for (std::size_t i = 0; i < n; i++)
  {
    if (is_missing(x[i]))
    {
      result++;
    }
  }
  return result;
}

} // namespace aitools

#endif // AITOOLS_DATASETS_ALGORITHMS_H
