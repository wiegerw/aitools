// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/statistics/sampling.h
/// \brief add your file description here.

#ifndef AITOOLS_STATISTICS_SAMPLING_H
#define AITOOLS_STATISTICS_SAMPLING_H

#include "aitools/statistics/distributions.h"

namespace aitools {

inline
double sample(const uniform_distribution& d, std::mt19937& rng)
{
  std::uniform_real_distribution<double> dist(d.lower(), d.upper());
  return dist(rng);
}

inline
double sample(const normal_distribution& d, std::mt19937& rng)
{
  std::normal_distribution<double> dist(d.mean(), d.standard_deviation());
  return dist(rng);
}

inline
double sample_truncated_normal_ab(double mu, double sigma, double a, double b, std::mt19937& rng)
{
  std::uniform_real_distribution<double> Unif(0, 1);
  double alpha = (a - mu) / sigma;
  double beta = (b - mu) / sigma;
  double Phi_alpha = Phi(alpha);
  double Phi_beta = Phi(beta);
  double p = Unif(rng);
  double x = Phi_inverse(Phi_alpha + p * (Phi_beta - Phi_alpha));
  return mu + sigma * x;
}

// special case for b == infinity
inline
double sample_truncated_normal_a(double mu, double sigma, double a, std::mt19937& rng)
{
  std::uniform_real_distribution<double> Unif(0, 1);
  double alpha = (a - mu) / sigma;
  double Phi_alpha = Phi(alpha);
  double p = Unif(rng);
  double x = Phi_inverse(Phi_alpha + p * (1.0 - Phi_alpha));
  return mu + sigma * x;
}

// special case for a == -infinity
inline
double sample_truncated_normal_b(double mu, double sigma, double b, std::mt19937& rng)
{
  std::uniform_real_distribution<double> Unif(0, 1);
  double beta = (b - mu) / sigma;
  double Phi_beta = Phi(beta);
  double p = Unif(rng);
  double x = Phi_inverse(p * Phi_beta);
  return mu + sigma * x;
}

inline
double sample(const truncated_normal_distribution& d, std::mt19937& rng)
{
  double a = d.a();
  double b = d.b();
  double mu = d.normal().mean();
  double sigma = d.normal().standard_deviation();
  if (a == truncated_normal_distribution::min)
  {
    return b == truncated_normal_distribution::max ? sample(d.normal(), rng) : sample_truncated_normal_b(mu, sigma, b, rng);
  }
  else
  {
    return b == truncated_normal_distribution::max ? sample_truncated_normal_a(mu, sigma, a, rng) : sample_truncated_normal_ab(mu, sigma, a, b, rng);
  }
}

inline
std::size_t sample(const categorical_distribution& d, std::mt19937& rng)
{
  double p = std::uniform_real_distribution<double>(0, 1)(rng);
  return d.inverse_cdf(p);
}

} // namespace aitools

#endif // AITOOLS_STATISTICS_SAMPLING_H
