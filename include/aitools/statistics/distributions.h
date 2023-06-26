// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/statistics/distributions.h
/// \brief Probability distributions.

#ifndef AITOOLS_STATISTICS_DISTRIBUTIONS_H
#define AITOOLS_STATISTICS_DISTRIBUTIONS_H

#include <limits>
#include <iostream>
#include <random>
#include <variant>
#include <boost/math/distributions/uniform.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/special_functions/erf.hpp>
#include "aitools/numerics/math_utility.h"
#include "aitools/utilities/logger.h"
#include "aitools/utilities/print.h"

namespace aitools {

namespace detail {

/// \param x A value in the interval [0, 1]
/// \return The smallest positive value i such that x <= p[0] + ... + p[i]
template <typename NumberSequence>
std::size_t find_categorical_section(const NumberSequence& p, double x)
{
  // TODO: implement a more efficient solution?
  double sum = 0;
  for (std::size_t i = 0; i < p.size(); i++)
  {
    sum += p[i];
    if (x <= sum)
    {
      return i;
    }
  }
  return p.size() - 1;
}

} // namespace detail

class uniform_distribution
{
  private:
    boost::math::uniform_distribution<double> m_dist;

  public:
    uniform_distribution(double lower, double upper)
      : m_dist(lower, upper)
    {
    }

    double lower() const
    {
      return m_dist.lower();
    }

    double upper() const
    {
      return m_dist.upper();
    }

    double pdf(double x) const
    {
      return boost::math::pdf(m_dist, x);
    }

    double cdf(double x) const
    {
      return boost::math::cdf(m_dist, x);
    }

    unsigned int category_count() const
    {
      return 0;
    }
};

inline
std::ostream& operator<<(std::ostream& out, const uniform_distribution& dist)
{
  return out << "UniformDistribution(" << dist.lower() << "," << dist.upper() << ")";
}

/// \brief CDF of the standard normal distribution.
inline
double Phi(double x)
{
  double mu = 0;
  double sigma = 1;
  boost::math::normal_distribution<double> dist(mu, sigma);
  return boost::math::cdf(dist, x);
}

/// \brief Inverse CDF of the standard normal distribution.
/// \pre 0 < x < 1
inline
double Phi_inverse(double x)
{
  return std::sqrt(2) * boost::math::erf_inv(2*x - 1);
}

class normal_distribution
{
  private:
    boost::math::normal_distribution<double> m_dist;

  public:
    explicit normal_distribution(double mu = 0, double sigma = 1)
    : m_dist(mu, sigma)
    {
    }

    double mean() const
    {
      return m_dist.mean();
    }

    double standard_deviation() const
    {
      return m_dist.standard_deviation();
    }

    double pdf(double x) const
    {
      return boost::math::pdf(m_dist, x);
    }

    double cdf(double x) const
    {
      return boost::math::cdf(m_dist, x);
    }

    double inverse_cdf(double x) const
    {
      auto mu = m_dist.mean();
      auto sigma = m_dist.standard_deviation();
      double arg = 2*x - 1;
      // prevent overflow; TODO: can this be done more elegantly?
      if (arg == -1)
      {
        return std::numeric_limits<double>::lowest();
      }
      if (arg == 1)
      {
        return std::numeric_limits<double>::max();
      }
      return mu + sigma * std::sqrt(2) * boost::math::erf_inv(arg);
    }

    unsigned int category_count() const
    {
      return 0;
    }
};

inline
std::ostream& operator<<(std::ostream& out, const normal_distribution& dist)
{
  return out << "NormalDistribution(" << dist.mean() << "," << dist.standard_deviation() << ")";
}

struct truncated_normal_distribution
{
  private:
    normal_distribution m_normal;
    double m_a;
    double m_b;

  public:
    static constexpr double min = std::numeric_limits<double>::lowest();
    static constexpr double max = std::numeric_limits<double>::max();
    const double Phi_a;
    const double Phi_b;
    const double Phi_inv_a;

  public:
    explicit truncated_normal_distribution(double mu = 0, double sigma = 1, double a = min, double b = max)
      : m_normal(mu, sigma), m_a(a), m_b(b),
        Phi_a(m_normal.cdf(a)),
        Phi_b(m_normal.cdf(b)),
        Phi_inv_a(m_normal.inverse_cdf(Phi_a))
    {}

    const normal_distribution& normal() const
    {
      return m_normal;
    }

    double a() const
    {
      return m_a;
    }

    double b() const
    {
      return m_b;
    }

    double pdf(double x) const
    {
      if (x < m_a)
      {
        return 0;
      }
      if (x > m_b)
      {
        return 1;
      }
      return m_normal.pdf(x) / (Phi_b - Phi_a);
    }

    double cdf(double x) const
    {
      if (x < m_a)
      {
        return 0;
      }
      if (x > m_b)
      {
        return 1;
      }
      return (m_normal.cdf(x) - Phi_a) / (Phi_b - Phi_a);
    }

    double inverse_cdf(double x) const
    {
      return Phi_inv_a + x * (Phi_b - Phi_a);
    }

    unsigned int category_count() const
    {
      return 0;
    }
};

inline
std::ostream& operator<<(std::ostream& out, const truncated_normal_distribution& dist)
{
  return out << "TruncatedNormalDistribution("
             << dist.normal().mean() << ","
             << dist.normal().standard_deviation() << ","
             << dist.a() << ","
             << dist.b()
             << ")";
}

class categorical_distribution
{
  private:
    std::vector<double> p;

    std::size_t val(double x) const
    {
      double intpart;
      if (std::modf(x, &intpart) != 0.0)
      {
        throw std::invalid_argument(
          "Non-integer observation " + std::to_string(x) + " observed for a categorical distribution.");
      }
      std::size_t i = std::lround(x); // TODO: check if this is the correct way of rounding
      if (i >= p.size())
      {
        throw std::out_of_range("Illegal category " + std::to_string(x) + " observed for a categorical distribution.");
      }
      return i;
    }

  public:
    explicit categorical_distribution(std::vector<double> p_)
      : p(std::move(p_))
    {
    }

    const std::vector<double>& probabilities() const
    {
      return p;
    }

    double pdf(double x) const
    {
      std::size_t i = val(x);
      return p[i];
    }

    double cdf(double x) const
    {
      std::size_t i = val(x);
      return std::accumulate(p.begin(), p.begin() + i + 1, 0.0);
    }

    std::size_t inverse_cdf(double x) const
    {
      return detail::find_categorical_section(p, x);
    }

    unsigned int category_count() const
    {
      return p.size();
    }
};

inline
std::ostream& operator<<(std::ostream& out, const categorical_distribution& dist)
{
  return out << "CategoricalDistribution(" << aitools::print_container(dist.probabilities(), "", "", ",") << ")";
}

/// \brief Returns the mean and standard deviation of a mixture of two one-dimensional distributions.
// See https://en.wikipedia.org/wiki/Mixture_distribution
inline
std::pair<double, double> mean_standard_deviation_mixture(double w1, double mu1, double sigma1, double w2, double mu2, double sigma2)
{
  using aitools::square;

  double mu = w1 * mu1 + w2 * mu2;
  double sigma = std::sqrt(w1 * (square(sigma1) + square(mu1)) + w2 * (square(sigma2) + square(mu2)) - square(mu));
  return {mu, sigma};
}

} // namespace aitools

#endif // AITOOLS_STATISTICS_DISTRIBUTIONS_H
