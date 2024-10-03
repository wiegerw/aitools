// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file dataset_test.cpp
/// \brief Tests for dataset.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <cmath>
#include <random>
#include "aitools/datasets/algorithms.h"
#include "aitools/datasets/random.h"
#include "aitools/decision_trees/learning.h"
#include "aitools/statistics/distributions.h"
#include "aitools/numerics/math_utility.h"
#include "aitools/utilities/print.h"

inline
std::pair<double, double> compute_mean_standard_deviation(const std::vector<double>& x)
{
  std::size_t n = x.size();
  double sum = 0.0;
  for (auto x_i: x)
  {
    sum += x_i;
  }
  double mean = sum / n;

  double variance = 0.0;
  for (auto x_i: x)
  {
    variance += std::pow(x_i - mean, 2);
  }
  double stddev = std::sqrt(variance / n);
  return {mean, stddev};
}


// Draw n random samples of w1 * N(mu1, sigma1) + w2 * N(mu2, sigma2)
inline
aitools::dataset sample_mixed_gaussian(double w1, double mu1, double sigma1, double w2, double mu2, double sigma2, std::size_t n)
{
  using namespace aitools;

  auto seed = std::random_device{}();
  std::mt19937 rng{static_cast<unsigned int>(seed)};

  std::normal_distribution<double> dist1(mu1, sigma1);
  std::normal_distribution<double> dist2(mu2, sigma2);
  std::uniform_real_distribution<double> uni(0, 1);
  std::vector<double> w = {w1, w2};

  std::vector<std::vector<double>> X;
  for (std::size_t i = 0; i < n; i++)
  {
    double p = uni(rng);
    std::size_t j = detail::find_categorical_section(w, p);
    double value = (j == 0) ? dist1(rng) : dist2(rng);
    X.push_back({value});
  }
  std::vector<unsigned int> ncat = {0};
  return dataset(numerics::matrix<double>(X), ncat);
}

TEST_CASE("test_mixed_gaussian")
{
  using namespace aitools;

  double w1 = 0.25;
  double mu1 = 0;
  double sigma1 = 1;
  double w2 = 0.75;
  double mu2 = 1;
  double sigma2 = 2;
  std::size_t n = 100000;

  dataset D = sample_mixed_gaussian(w1, mu1, sigma1, w2, mu2, sigma2, n);
  auto [mu, sigma] = mean_standard_deviation(D, xrange(n), 0);
  auto [mu_expected, sigma_expected] = mean_standard_deviation_mixture(w1, mu1, sigma1, w2, mu2, sigma2);
  REQUIRE_LT(std::abs(mu - mu_expected), 0.05);
  REQUIRE_LT(std::abs(sigma - sigma_expected), 0.05);
}

TEST_CASE("test_random_dataset")
{
  using namespace aitools;
  log::aitools_logger::set_reporting_level(log::quiet);

  // create a random dataset
  std::vector<distribution> distributions = { normal_distribution(1, 2), normal_distribution(3, 1), categorical_distribution({0.2, 0.3, 0.5})};
  std::size_t n = 10000;
  auto seed = std::random_device{}();
  std::mt19937 rng{static_cast<unsigned int>(seed)};
  dataset D = make_random_dataset(distributions, n, rng);
  std::cout << "D =\n" << D << std::endl;

  double mu1_expected = 1;
  double sigma1_expected = 2;
  double mu2_expected = 3;
  double sigma2_expected = 1;

  auto [mu1, sigma1] = mean_standard_deviation(D, xrange(n), 0);
  auto [mu2, sigma2] = mean_standard_deviation(D, xrange(n), 1);
  std::cout << "mu1 = " << mu1 << " sigma1 = " << sigma1 << std::endl;
  std::cout << "mu2 = " << mu2 << " sigma2 = " << sigma2 << std::endl;
  REQUIRE_LT(std::abs(mu1 - mu1_expected), 0.05);
  REQUIRE_LT(std::abs(sigma1 - sigma1_expected), 0.05);
  REQUIRE_LT(std::abs(mu2 - mu2_expected), 0.05);
  REQUIRE_LT(std::abs(sigma2 - sigma2_expected), 0.05);

  std::vector<std::size_t> counts(3);
  D.compute_categorical_counts(xrange(n), 2, counts);
  double p0 = static_cast<double>(counts[0]) / n;
  double p1 = static_cast<double>(counts[1]) / n;
  double p2 = static_cast<double>(counts[2]) / n;
  std::cout << "p0 = " << p0 << " p1 = " << p1 << " p2 = " << p2 << std::endl;
  REQUIRE_LT(std::abs(p0 - 0.2), 0.05);
  REQUIRE_LT(std::abs(p1 - 0.3), 0.05);
  REQUIRE_LT(std::abs(p2 - 0.5), 0.05);
}
