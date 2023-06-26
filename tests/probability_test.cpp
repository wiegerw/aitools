// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file probability_test.cpp
/// \brief Tests for probability distributions.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <random>
#include "aitools/numerics/math_functions.h"

// returns a random double value: low <= value <= high
double random_double(double low, double high)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(low, high);
  return dist(mt);
}

// Computes the log-probability density function at x of a normal distribution
// with mean 0 and standard deviation 1, and truncated between a and b.
inline
double log_truncated_phi_imprecise(double x, double mean, double sd, double a, double b)
{
  using boost::math::normal;
  if (x < a || x > b)
  {
    return -infinity;
  }
  return std::log(cdf(normal(mean, sd), x));
}

TEST_CASE("test1")
{
  using namespace aitools;

  double mean = 140;
  double standard_deviation = 10;
  double a = 1;
  double b = 1000;

  for (auto i = 0; i < 1000; i++)
  {
    double x = random_double(a, b);
    CHECK_LT(std::abs(tphi(x, mean, standard_deviation, a, b) - truncated_phi(x, mean, standard_deviation, a, b)), 1e-10);
  }
}
