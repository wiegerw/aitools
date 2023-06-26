// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/datasets/dataset.h
/// \brief add your file description here.

#ifndef AITOOLS_DATASETS_DATASET_H
#define AITOOLS_DATASETS_DATASET_H

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "aitools/datasets/missing.h"
#include "aitools/decision_trees/index_range.h"
#include "aitools/numerics/csv.h"
#include "aitools/numerics/matrix.h"
#include "aitools/utilities/logger.h"
#include "aitools/utilities/print.h"
#include "aitools/utilities/string_utility.h"

namespace aitools {

class dataset
{
  private:
    numerics::matrix<double> m_X;
    std::vector<unsigned int> m_category_counts;
    std::vector<std::string> m_features; // optional

    bool is_valid() const
    {
      return std::all_of(m_X.begin(), m_X.end(), [this](const auto& x) { return x.size() == m_category_counts.size(); });
    }

  public:
    dataset() = default;

    dataset(numerics::matrix<double> X, std::vector<unsigned int> ncat, std::vector<std::string> features = {})
      : m_X(std::move(X)), m_category_counts(std::move(ncat)), m_features(std::move(features))
    {
      assert(is_valid());
    }

    const numerics::matrix<double>& X() const
    {
      return m_X;
    }

    numerics::matrix<double>& X()
    {
      return m_X;
    }

    numerics::matrix<double>::column_type y() const
    {
      std::size_t j = m_X.column_count() - 1;
      return m_X.column(j);
    }

    void add(std::vector<double> row)
    {
      m_X.add(std::move(row));
    }

    const std::vector<unsigned int>& category_counts() const
    {
      return m_category_counts;
    }

    std::vector<unsigned int>& category_counts()
    {
      return m_category_counts;
    }

    const std::vector<std::string>& features() const
    {
      return m_features;
    }

    std::vector<std::string>& features()
    {
      return m_features;
    }

    std::size_t feature_count() const
    {
      return m_X.column_count() - 1;
    }

    std::size_t class_count() const
    {
      return m_category_counts.back();
    }

    bool is_continuous_variable(std::size_t v) const
    {
      return m_category_counts[v] <= 1;
    }

    bool is_categorical_variable(std::size_t v) const
    {
      return m_category_counts[v] > 0;
    }

    /// \brief Computes the class counts of samples in the range I.
    /// N.B. Does not take missing values into account.
    // counts[i] = |{ j | y[i] = j }|, with i in the range I
    template <typename NumberSequence>
    void compute_class_counts(const index_range& I, NumberSequence& counts) const
    {
      const auto& y = this->y();
      std::fill(counts.begin(), counts.end(), 0);
      for (std::uint32_t i: I)
      {
        assert(!is_missing(y[i]));
        auto k = static_cast<std::size_t>(y[i]);
        counts[k]++;
      }
    }

    /// \brief Computes the class counts of samples in the range I for a categorical variable.
    /// \param v The index of a categorical variable.
    /// \pre <tt>is_categorical_variable(v)</tt>
    // counts[k] = |{ i in I | X[i][v] = k }|
    template <typename IndexRange, typename NumberSequence>
    void compute_categorical_counts(const IndexRange& I, std::size_t v, NumberSequence& counts) const
    {
      assert(is_categorical_variable(v));
      const auto& X = m_X;
      std::fill(counts.begin(), counts.end(), 0);
      for (std::uint32_t i: I)
      {
        double X_iv = X[i][v];
        if (!is_missing(X_iv))
        {
          auto k = static_cast<std::size_t>(X_iv);
          counts[k]++;
        }
      }
    }

    bool has_missing_values() const
    {
      std::size_t m = feature_count();
      std::size_t n = m_X.row_count();

      std::vector<std::size_t> missing_counts(m, 0);
      for (std::size_t i = 0; i < n; i++)
      {
        for (std::size_t j = 0; j < m; j++)
        {
          if (is_missing(m_X[i][j]))
          {
            return true;
          }
        }
      }
      return false;
    }

    std::vector<uint32_t> classes() const
    {
      std::vector<uint32_t> result;
      std::size_t n = m_X.row_count();
      result.reserve(n);
      for (std::size_t i = 0; i < n; i++)
      {
        result.push_back(static_cast<uint32_t>(m_X[i].back()));
      }
      return result;
    }

    bool operator==(const dataset& other) const
    {
      return m_X == other.m_X && m_category_counts == other.m_category_counts;
    }
};

inline
void print_info(const dataset& D)
{
  const auto& X = D.X();
  const auto& ncat = D.category_counts();
  std::size_t m = D.feature_count();
  std::size_t n = X.row_count();

  std::vector<std::size_t> missing_counts(m, 0);
  for (std::size_t i = 0; i < n; i++)
  {
    for (std::size_t j = 0; j < m; j++)
    {
      if (is_missing(X[i][j]))
      {
        missing_counts[j]++;
      }
    }
  }
  AITOOLS_LOG(log::verbose) << "number of features " << m << '\n';
  AITOOLS_LOG(log::verbose) << "number of samples " << n << '\n';
  AITOOLS_LOG(log::verbose) << "ncat    " << print_list(ncat) << '\n';
  AITOOLS_LOG(log::verbose) << "missing " << print_list(missing_counts) << '\n';
}

/// \brief Saves a dataset D in a simple textual file format
inline
std::ostream& operator<<(std::ostream& to, const dataset& D)
{
  to << "dataset: 1.0\n";
  to << "category_counts: " << print_container(D.category_counts()) << '\n';
  if (!D.features().empty())
  {
    to << "features: " << utilities::string_join(D.features(), " ") << '\n';
  }
  const auto& X = D.X();
  for (const auto& Xi: X)
  {
    to << print_container(Xi) << '\n';
  }
  return to;
}

} // namespace aitools

#endif // AITOOLS_DATASETS_DATASET_H
