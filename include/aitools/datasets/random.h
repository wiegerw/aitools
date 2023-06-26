// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/datasets/random.h
/// \brief add your file description here.

#ifndef AITOOLS_DATASETS_RANDOM_H
#define AITOOLS_DATASETS_RANDOM_H

#include <random>
#include "aitools/statistics/distribution.h"
#include "aitools/datasets/dataset.h"
#include "aitools/utilities/random.h"

namespace aitools {

namespace detail {

template <typename Generator>
void fill_column(numerics::matrix<double>& X, std::size_t j, Generator& generate)
{
  std::size_t n = X.row_count();
  for (std::size_t i = 0; i < n; i++)
  {
    X[i][j] = generate();
  }
}

} // namespace detail

inline
dataset make_random_dataset(std::size_t n, std::size_t m)
{
  numerics::matrix<double> X(n, m + 1);
  std::vector<unsigned int> ncat(m + 1);

  std::mt19937 mt{std::random_device{}()};

  for (std::size_t j = 0; j < m; j++)
  {
    bool categorical = random_bool(mt);
    if (categorical)
    {
      auto low = 0;
      ncat[j] = random_integer<unsigned int>(2, 10, mt);
      auto high = ncat[j] - 1;
      auto rand = [&]() { return random_integer<unsigned int>(low, high, mt); };
      detail::fill_column(X, j, rand);
    }
    else
    {
      double low = random_real(10.0, 100.0, mt);
      double high = random_real(110.0, 200.0, mt);
      ncat[j] = 0;
      auto random_real_generator = [&low, &high, &mt]() { return random_real(low, high, mt); };
      detail::fill_column(X, j, random_real_generator);
    }
  }

  auto K = random_integer(2, 3);
  ncat[m] = K;
  auto rand = [&]() { return random_integer(0, K - 1, mt); };
  detail::fill_column(X, m, rand);

  return dataset(X, ncat);
}

/// \brief Creates a random dataset containing \c n samples.
/// \param random_variables The distributions of the random variables that are used to sample them.
/// \param rng The random number generator used for sampling.
inline
dataset make_random_dataset(const std::vector<distribution>& random_variables, std::size_t n, std::mt19937& rng)
{
  std::size_t m = random_variables.size();

  std::vector<unsigned int> ncat;
  ncat.reserve(m);
  numerics::matrix<double> X(n, m);

  for (std::size_t j = 0; j < m; j++)
  {
    const distribution& dist = random_variables[j];
    ncat.push_back(category_count(dist));
    for (std::size_t i = 0; i < n; i++)
    {
      X[i][j] = sample(dist, rng);
    }
  }

  return dataset(X, ncat);
}

} // namespace aitools

#endif // AITOOLS_DATASETS_RANDOM_H
