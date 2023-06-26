// Copyright: Alvaro Correia on 23/09/2020.
// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/numerics/math.h
/// \brief add your file description here.

#ifndef AITOOLS_NUMERICS_MATH_FUNCTIONS_H
#define AITOOLS_NUMERICS_MATH_FUNCTIONS_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <vector>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/constants/constants.hpp>

constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = boost::math::double_constants::pi;
constexpr double one_div_root_two_pi = boost::math::double_constants::one_div_root_two_pi;

namespace aitools {

template<typename T>
int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

// Computes the probability density function at x of a normal distribution with
// mean 0 and standard deviation 1.
inline
double phi(double x)
{
  return one_div_root_two_pi * exp(-0.5 * x * x);
}

// Computes the log-probability density function at x of a normal distribution
// with mean 0 and standard deviation 1.
inline
double logphi(double x)
{
  static double P = -infinity;
  if (P <= -infinity)
  {
    P = -0.5 * std::log(2.0 * pi);
  }
  return P - 0.5 * x * x;
}

// Computes the cumulative distribution function at x of  a normal distribution
// with mean 0 and standard deviation 1.
inline
double c_phi(double x)
{
  static double RT2PI = -infinity;
  if (RT2PI <= -infinity)
  {
    RT2PI = sqrt(4.0 * acos(0.0));
  }

  constexpr double SPLIT = 7.07106781186547;
  constexpr double N0 = 220.206867912376;
  constexpr double N1 = 221.213596169931;
  constexpr double N2 = 112.079291497871;
  constexpr double N3 = 33.912866078383;
  constexpr double N4 = 6.37396220353165;
  constexpr double N5 = 0.700383064443688;
  constexpr double N6 = 3.52624965998911e-02;
  constexpr double M0 = 440.413735824752;
  constexpr double M1 = 793.826512519948;
  constexpr double M2 = 637.333633378831;
  constexpr double M3 = 296.564248779674;
  constexpr double M4 = 86.7807322029461;
  constexpr double M5 = 16.064177579207;
  constexpr double M6 = 1.75566716318264;
  constexpr double M7 = 8.83883476483184e-02;

  const double z = fabs(x);
  double c = 0.0;

  if (z <= 37.0)
  {
    const double e = exp(-z * z / 2.0);
    if (z < SPLIT)
    {
      const double n = (((((N6 * z + N5) * z + N4) * z + N3) * z + N2) * z + N1) * z + N0;
      const double d = ((((((M7 * z + M6) * z + M5) * z + M4) * z + M3) * z + M2) * z + M1) * z + M0;
      c = e * n / d;
    }
    else
    {
      const double f = z + 1.0 / (z + 2.0 / (z + 3.0 / (z + 4.0 / (z + 13.0 / 20.0))));
      c = e / (RT2PI * f);
    }
  }
  return x <= 0.0 ? c : 1 - c;
}

// Computes the probability density function at x of a normal distribution
// with mean 0 and standard deviation 1, and truncated between a and b.
inline
double tphi(double x, double mean, double sd, double a, double b)
{
  if (x < a || x > b)
  {
    return 0;
  }
  return (1.0 / sd) * phi((x - mean) / sd) / (c_phi((b - mean) / sd) - c_phi((a - mean) / sd));
}

// Computes the log-probability density function at x of a normal distribution
// with mean 0 and standard deviation 1, and truncated between a and b.
inline
double logtphi(double x, double mean, double sd, double a, double b)
{
  if (x < a || x > b)
  {
    return -infinity;
  }
  return -std::log(sd) + logphi((x - mean) / sd) - std::log(c_phi((b - mean) / sd) - c_phi((a - mean) / sd));
}

// Computes the probability density function at x of a normal distribution
// with mean \c mu and standard deviation \c sigma, and truncated between a and b.
// The area is normalized to 1.
inline double truncated_phi(double x, double mu, double sigma, double a, double b)
{
  using boost::math::normal;
  if (x < a || x > b)
  {
    return 0;
  }
  auto N = normal(mu, sigma);
  return pdf(N, x) / (cdf(N, b) - cdf(N, a));
}

// Computes the log probability density function at x of a normal distribution
// with mean \c mu and standard deviation \c sigma, and truncated between a and b.
// The area is normalized to 1.
inline double log_truncated_phi(double x, double mu, double sigma, double a, double b)
{
  using boost::math::normal;
  if (x < a || x > b)
  {
    return -infinity;
  }
  auto N = normal(mu, sigma);
  return std::log(pdf(N, x)) - std::log(cdf(N, b) - cdf(N, a));
}

template <typename Iter>
double log_sum_exp(Iter first, Iter last)
{
  if (first == last)
  {
    return 0.0;
  }

  double max_value = *std::max_element(first, last);
  if (max_value <= -infinity)
  {
    return -infinity;
  }

  // Takes the exponential, sums and computes the log.
  double sum = 0.0;
  for (auto i = first; i != last; ++i)
  {
    sum += exp(*i - max_value);
  }
  return std::log(sum) + max_value;
}

} // namespace aitools

#endif // AITOOLS_NUMERICS_MATH_FUNCTIONS_H
