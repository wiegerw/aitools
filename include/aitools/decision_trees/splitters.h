// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_tree_splitters.h
/// \brief add your file description here.

#ifndef AITOOLS_SPLITTERS_H
#define AITOOLS_SPLITTERS_H

#include <bitset>
#include <variant>
#include "aitools/datasets/dataset.h"
#include "aitools/decision_trees/decision_tree_options.h"
#include "aitools/decision_trees/impurity.h"
#include "aitools/numerics/math_utility.h"
#include "aitools/utilities/bit_utility.h"
#include "aitools/utilities/logger.h"
#include "aitools/utilities/stack_array.h"

namespace aitools {

inline
std::size_t select(const std::monostate& split, const std::vector<double>& x)
{
  return 0ul;
}

/// \brief Models the splitting criterion SingleSplit(variable, value)
struct single_split
{
  std::size_t variable; // the index of the variable
  double value;         // the split value

  single_split(std::size_t variable_, double value_)
    : variable(variable_), value(value_)
  {}

  bool operator==(const single_split& other) const
  {
    return std::tie(variable, value) == std::tie(other.variable, other.value);
  }

  bool operator!=(const single_split& other) const
  {
    return !(*this == other);
  }

  bool operator<(const single_split& other) const
  {
    return std::tie(variable, value) < std::tie(other.variable, other.value);
  }
};

inline
std::ostream& operator<<(std::ostream& out, const single_split& split)
{
  return out << "SingleSplit(" << split.variable << ", " << split.value << ")";
}

inline
std::size_t select(const single_split& split, const std::vector<double>& x)
{
  double x_i = x[split.variable];
  return (x_i == split.value) ? 0ul : 1ul;
}

/// \brief Models the splitting criterion SubsetSplit(variable, mask)
struct subset_split
{
  std::size_t variable; // the index of the variable
  std::uint32_t mask;   // the bit mask of the values in the first partition

  static constexpr std::size_t max_subset_size = 8 * sizeof(variable);

  subset_split(std::size_t variable_, std::uint32_t mask_)
    : variable(variable_), mask(mask_)
  {}

  /// \brief Returns true if value is contained in the first partition
  bool contains(std::size_t value) const
  {
    return utilities::is_bit_set(mask, value);
  }

  bool operator==(const subset_split& other) const
  {
    return std::tie(variable, mask) == std::tie(other.variable, other.mask);
  }

  bool operator!=(const subset_split& other) const
  {
    return !(*this == other);
  }

  bool operator<(const subset_split& other) const
  {
    return std::tie(variable, mask) < std::tie(other.variable, other.mask);
  }
};

inline
std::ostream& operator<<(std::ostream& out, const subset_split& split)
{
  return out << "SubsetSplit(" << split.variable << ", " << std::bitset<32>(split.mask) << ")";
}

inline
std::size_t select(const subset_split& split, const std::vector<double>& x)
{
  double x_i = x[split.variable];
  return (split.contains(x_i)) ? 0ul : 1ul;
}

/// \brief Models the splitting criterion ThresholdSplit(variable, value)
struct threshold_split
{
  std::size_t variable; // the index of the variable
  double value;         // the split value

  threshold_split(std::size_t variable_, double value_)
    : variable(variable_), value(value_)
  {}

  bool operator==(const threshold_split& other) const
  {
    return std::tie(variable, value) == std::tie(other.variable, other.value);
  }

  bool operator!=(const threshold_split& other) const
  {
    return !(*this == other);
  }

  bool operator<(const threshold_split& other) const
  {
    return std::tie(variable, value) < std::tie(other.variable, other.value);
  }
};

inline
std::ostream& operator<<(std::ostream& out, const threshold_split& split)
{
  return out << "ThresholdSplit(" << split.variable << ", " << split.value << ")";
}

inline
std::size_t select(const threshold_split& split, const std::vector<double>& x)
{
  double x_i = x[split.variable];
  return (x_i < split.value) ? 0ul : 1ul;
}

/// \brief Typesafe union for splitting criteria. The class std::monostate is used to model no value.
using splitting_criterion = std::variant<std::monostate, single_split, subset_split, threshold_split>;

/// Stream operator for splitters
/// \param out An output stream
/// \param split A splitter
/// \return The output stream \c out
inline
std::ostream& operator<<(std::ostream& out, const splitting_criterion& split)
{
  struct print_split_visitor
  {
    std::ostream& out;

    explicit print_split_visitor(std::ostream& out_)
      : out(out_)
    {
    }

    void operator()(const single_split& split)
    {
      out << split;
    }

    void operator()(const subset_split& split)
    {
      out << split;
    }

    void operator()(const threshold_split& split)
    {
      out << split;
    }

    void operator()(const std::monostate&)
    {
      out << "NoSplit()";
    }
  };

  print_split_visitor visitor(out);
  std::visit(visitor, split);
  return out;
}

/// \brief Partitions the indices in the range I into I1 and I2 using the given splitter.
/// \param split A splitter
/// \param D A data set
/// \param I An index range
/// \param rng A random number generator that is used to assign samples with missing values to a random partition
/// \param support_missing_values Indicates whether the data set may contain missing values
/// \return A partition <tt>I1, I2</tt> of the range \c I. The indices in the range \c I are reordered accordingly.
inline
std::pair<index_range, index_range> apply_split(const splitting_criterion& split, const dataset& D, const index_range& I, std::mt19937& rng, bool support_missing_values)
{
  struct apply_split_visitor
  {
    const dataset& D;
    const index_range& I;
    std::vector<std::uint32_t>::iterator mid;
    std::mt19937& rng;
    bool support_missing_values;

    apply_split_visitor(const dataset& D_, const index_range& I_, std::mt19937& rng_, bool support_missing_values_)
      : D(D_), I(I_), rng(rng_), support_missing_values(support_missing_values_)
    {}

    void operator()(const single_split& split)
    {
      const auto& X = D.X();
      mid = std::partition(I.begin(), I.end(), [&](std::size_t i)
      {
        double x_i = X[i][split.variable];
        return (x_i == split.value) || (support_missing_values && is_missing(x_i) && random_bool(rng));
      });
    }

    void operator()(const subset_split& split)
    {
      const auto& X = D.X();
      mid = std::partition(I.begin(), I.end(), [&](std::size_t i)
      {
        auto x_i = static_cast<std::size_t>(X[i][split.variable]);
        return split.contains(x_i) || (support_missing_values && is_missing(x_i) && random_bool(rng));
      });
    }

    void operator()(const threshold_split& split)
    {
      const auto& X = D.X();
      mid = std::partition(I.begin(), I.end(), [&](std::size_t i)
      {
        double x_i = X[i][split.variable];
        return (x_i < split.value) || (support_missing_values && is_missing(x_i) && random_bool(rng));
      });
    }

    void operator()(const std::monostate&)
    {
      throw std::runtime_error("cannot apply an undefined split");
    }
  };

  apply_split_visitor visitor(D, I, rng, support_missing_values);
  std::visit(visitor, split);
  index_range I1(I.begin(), visitor.mid);
  index_range I2(visitor.mid, I.end());
  assert(is_partition(I, I1, I2));
  return { I1, I2 };
}

/// \brief Returns the variable of a splitting criterion
inline
std::size_t split_variable(const splitting_criterion& split)
{
  struct split_variable_visitor
  {
    std::size_t operator()(const single_split& split)
    {
      return split.variable;
    }

    std::size_t operator()(const subset_split& split)
    {
      return split.variable;
    }

    std::size_t operator()(const threshold_split& split)
    {
      return split.variable;
    }

    std::size_t operator()(const std::monostate& split)
    {
      return std::numeric_limits<std::size_t>::max();
    }
  };

  return std::visit(split_variable_visitor(), split);
};

/// \brief Determines to which partition an element of the feature space belongs.
/// \param split A splitter
/// \param x An element of the feature space
/// \return The index of the partition to which \c x belongs.
inline
std::size_t select(const splitting_criterion& split, const std::vector<double>& x)
{
  return std::visit([&x](auto& val) { return select(val, x); }, split);
}

/// \brief Enumerates all possible single splits for a given variable.
/// \tparam ReportSplit
/// \param D A data set
/// \param I The index range of the samples that will be split
/// \param v The index of the split variable
/// \param options The split options
/// \param D1_counts A container that will hold the class counts of the first partition. It must have size at least <tt>D.class_count()</tt>.
/// \param D2_counts A container that will hold the class counts of the second partition. It must have size at least <tt>D.class_count()</tt>.
/// \param report_split A callback function that will be called for every splitter \c split using <tt>report_split(split, D1_counts, D2_counts)</tt>.
template <typename ReportSplit>
void enumerate_single_splits(const dataset& D,
                             const index_range& I,
                             std::size_t v,
                             const decision_tree_options& options,
                             std::vector<std::size_t>& D1_counts,
                             std::vector<std::size_t>& D2_counts,
                             ReportSplit report_split)
{
  assert(is_valid_range(I, D.X().row_count()));

  AITOOLS_LOG(log::debug) << "=== enumerate_single_splits v = " << v << " I = " << I << std::endl;
  const auto& X = D.X();
  const auto& y = D.y();

  auto Iend = I.end();

  // move samples with missing values to the back, and ignore them
  if (options.support_missing_values)
  {
    Iend = std::partition(I.begin(), I.end(), [&X, v](std::uint32_t i) { return !is_missing(X[i][v]); });
  }

  std::sort(I.begin(), Iend, [&X, v](std::size_t i1, std::size_t i2) { return X[i1][v] < X[i2][v]; });

  std::fill(D1_counts.begin(), D1_counts.end(), 0ul);
  D.compute_class_counts(I, D2_counts);
  std::size_t D_sum = sum(D2_counts);

  // determine the range of samples [first, ..., last) with equal values for variable v
  auto first = I.begin();
  while (first != I.end())
  {
    auto value = X[*first][v];
    auto last = first;
    ++last;
    while (last != I.end() && X[*last][v] == value)
    {
      ++last;
    }

    std::size_t count = last - first;
    if (count < options.min_samples_leaf || (D_sum - count) < options.min_samples_leaf)
    {
      first = last;
      continue;
    }

    // update counts
    for (auto i = first; i != last; ++i)
    {
      auto k = static_cast<std::size_t>(y[*i]);
      D1_counts[k]++;
      D2_counts[k]--;
    }

    report_split(single_split(v, value), D1_counts, D2_counts);

    // restore counts
    for (auto i = first; i != last; ++i)
    {
      auto k = static_cast<std::size_t>(y[*i]);
      D1_counts[k]--;
      D2_counts[k]++;
    }

    first = last;
  }
}

/// \brief Enumerates all possible subset splits for a given variable.
/// \tparam ReportSplit
/// \param D A data set
/// \param I The index range of the samples that will be split
/// \param v The index of the split variable
/// \param options The split options
/// \param D1_counts A container that will hold the class counts of the first partition. It must have size at least <tt>D.class_count()</tt>.
/// \param D2_counts A container that will hold the class counts of the second partition. It must have size at least <tt>D.class_count()</tt>.
/// \param report_split A callback function that will be called for every splitter \c split using <tt>report_split(split, D1_counts, D2_counts)</tt>.
template <typename ReportSplit>
void enumerate_subset_splits(const dataset& D,
                             const index_range& I,
                             std::size_t v,
                             const decision_tree_options& options,
                             std::vector<std::size_t>& D1_counts,
                             std::vector<std::size_t>& D2_counts,
                             ReportSplit report_split)
{
  assert(is_valid_range(I, D.X().row_count()));
  const auto& X = D.X();
  const auto& y = D.y();
  const auto& ncat = D.category_counts();
  std::size_t K = D.class_count();
  std::size_t ncat_v = ncat[v];

  AITOOLS_LOG(log::debug) << "=== enumerate_subset_splits v = " << v << " I = " << I << std::endl;

  if (ncat_v > subset_split::max_subset_size)
  {
    throw std::runtime_error("subset splits can handle at most " + std::to_string(subset_split::max_subset_size) + " categories");
  }

  auto Iend = I.end();

  // move samples with missing values to the back, and ignore them
  if (options.support_missing_values)
  {
    Iend = std::partition(I.begin(), I.end(), [&X, v](std::uint32_t i) { return !is_missing(X[i][v]); });
  }

  if (Iend == I.begin())
  {
    return;
  }

  std::sort(I.begin(), Iend, [&X, v](std::size_t i1, std::size_t i2) { return X[i1][v] < X[i2][v]; });

  // make a table of the class counts
  std::vector<std::size_t> W(ncat_v * K, 0);
  index_range I1(I.begin(), Iend);
  for (auto i: I1)
  {
    auto x_i = static_cast<std::size_t>(X[i][v]);
    auto y_i = static_cast<std::size_t>(y[i]);
    W[x_i * K + y_i]++;
  }
  AITOOLS_LOG(log::debug) << "W = " << print_list(W) << std::endl;

  std::vector<std::size_t> D_counts(K);
  D.compute_class_counts(I, D_counts);

  std::vector<std::size_t> pos; // { j | exists i: X[i][v] = j }
  for (std::size_t i = 0; i < ncat_v; i++)
  {
    for (std::size_t j = 0; j < K; j++)
    {
      if (W[i * K + j] > 0)
      {
        pos.push_back(i);
        break;
      }
    }
  }
  AITOOLS_LOG(log::debug) << "pos = " << print_list(pos) << std::endl;

  std::size_t mask{0};
  utilities::set_bit(mask, pos[0], true); // The first non-empty class is in the first partition

  // add the counts of { i in I | X[i][v] = j } to D1_counts
  auto add = [&](std::size_t j)
  {
    for (std::size_t k = 0; k < K; k++)
    {
      D1_counts[k] += W[j * K + k];
    }
  };

  std::size_t p = pos.size() - 1;
  std::size_t N = (1 << p) - 1;

  // each value of i is the bitmask of a subset of the range pos[1:]
  for (std::size_t i = 0; i < N; i++)
  {
    std::fill(D1_counts.begin(), D1_counts.end(), 0);
    add(0); // 0 is always in the first partition

    for (std::size_t j = 0; j < p; j++)
    {
      bool in_first_partition = utilities::is_bit_set(i, j);
      utilities::set_bit(mask, pos[j + 1], in_first_partition);
      if (in_first_partition)
      {
        add(j+1);
      }
    }

    for (std::size_t k = 0; k < K; k++)
    {
      D2_counts[k] = D_counts[k] - D1_counts[k];
    }
    std::size_t D1_sum = sum(D1_counts);
    std::size_t D2_sum = sum(D2_counts);
    if (D1_sum >= options.min_samples_leaf && D2_sum >= options.min_samples_leaf)
    {
      report_split(subset_split(v, mask), D1_counts, D2_counts);
    }
  }
}

/// \brief Enumerates all possible threshold splits for a given variable.
/// \tparam ReportSplit
/// \param D A data set
/// \param I An index range of samples belonging to the decision tree node that will be split
/// \param v The index of the split variable
/// \param options The split options
/// \param D1_counts A container that will hold the class counts of the first partition. It must have size at least <tt>D.class_count()</tt>.
/// \param D2_counts A container that will hold the class counts of the second partition. It must have size at least <tt>D.class_count()</tt>.
/// \param report_split A callback function that will be called for every splitter \c split using <tt>report_split(split, D1_counts, D2_counts)</tt>.
template <typename ReportSplit>
void enumerate_threshold_splits(const dataset& D,
                                const index_range& I,
                                std::size_t v,
                                const decision_tree_options& options,
                                std::vector<std::size_t>& D1_counts,
                                std::vector<std::size_t>& D2_counts,
                                ReportSplit report_split)
{
  assert(is_valid_range(I, D.X().row_count()));

  AITOOLS_LOG(log::debug) << "=== enumerate_threshold_splits v = " << v << " I = " << I << std::endl;
  const auto& X = D.X();
  const auto& y = D.y();

  auto Iend = I.end();

  // move samples with missing values to the back, and ignore them
  if (options.support_missing_values)
  {
    Iend = std::partition(I.begin(), I.end(), [&X, v](std::uint32_t i) { return !is_missing(X[i][v]); });
  }

  std::sort(I.begin(), Iend, [&X, v](std::size_t i1, std::size_t i2) { return X[i1][v] < X[i2][v]; });

  auto first = I.begin() + options.min_samples_leaf;
  auto last = Iend - options.min_samples_leaf + 1;
  if (last <= first)
  {
    return;
  }

  index_range I1(I.begin(), first);
  index_range I2(first, Iend);

  assert(is_valid_range(I1, D.X().row_count()));
  assert(is_valid_range(I2, D.X().row_count()));

  D.compute_class_counts(I1, D1_counts);
  D.compute_class_counts(I2, D2_counts);

  bool same_y = false;

  for (auto i = first; i != last; ++i)
  {
    auto value = X[*i][v];

    // update the counts
    if (i != first)
    {
      auto k = static_cast<std::size_t>(y[*(i-1)]);
      D1_counts[k]++;
      D2_counts[k]--;
    }

    // there cannot be a split between two equal values
    if (X[*(i-1)][v] == value)
    {
      if (y[*(i - 1)] != y[*i])
      {
        same_y = false;
      }
      continue;
    }

    // a split between two samples in the same class is not optimal
    if (options.optimization)
    {
      bool next_same_y = (i + 1 != last) && (y[*i] == y[*(i+1)]) && (value != X[*(i+1)][v]);
      if (same_y && next_same_y)
      {
        AITOOLS_LOG(log::debug1) << "skipping " << threshold_split(v, value) << " counts = " << print_list(D1_counts) << " " << print_list(D2_counts) << " y = " << y[*i] << " gain_gini = " << gain(impurity_measure::gini)(D1_counts, D2_counts) << " gain_entropy = " << gain(impurity_measure::entropy)(D1_counts, D2_counts) << std::endl;
        continue;
      }
      else
      {
        AITOOLS_LOG(log::debug1) << "keeping  " << threshold_split(v, value) << " counts = " << print_list(D1_counts) << " " << print_list(D2_counts) << " y = " << y[*i] << " gain_gini = " << gain(impurity_measure::gini)(D1_counts, D2_counts) << " gain_entropy = " << gain(impurity_measure::entropy)(D1_counts, D2_counts) << std::endl;
        same_y = next_same_y;
      }
    }
    else
    {
      AITOOLS_LOG(log::debug1) << "keeping  " << threshold_split(v, value) << " counts = " << print_list(D1_counts) << " " << print_list(D2_counts) << " y = " << y[*i] << " gain_gini = " << gain(impurity_measure::gini)(D1_counts, D2_counts) << " gain_entropy = " << gain(impurity_measure::entropy)(D1_counts, D2_counts) << std::endl;
    }

    report_split(threshold_split(v, value), D1_counts, D2_counts);
  }
}

/// \brief A family of decision tree splits.
///
/// It uses threshold splits only.
struct threshold_split_family
{
  const dataset& D;
  const decision_tree_options& options;

  /// \brief Default constructor
  threshold_split_family(const dataset& D_, const decision_tree_options& options_)
    : D(D_), options(options_)
  {}

  /// \brief Enumerates all possible splits.
  /// \param I The indices of the samples.
  /// \param V The indices of the split variables.
  /// \param report_split A callback function that is called for every possible split.
  template <typename ReportSplit>
  void enumerate(const index_range& I, const std::vector<std::size_t>& V, ReportSplit report_split) const
  {
    std::size_t K = D.class_count();
    std::vector<std::size_t> D1_counts(K);
    std::vector<std::size_t> D2_counts(K);
    for (std::size_t v: V)
    {
      enumerate_threshold_splits(D, I, v, options, D1_counts, D2_counts, report_split);
    }
  }
};

/// \brief A family of decision tree splits.
///
/// It uses single splits for categorical variables, and threshold splits for all others.
/// The splits can be enumerated using the function \c enumerate.
struct threshold_plus_single_split_family
{
  const dataset& D;
  const decision_tree_options& options;

  /// \brief Default constructor
  threshold_plus_single_split_family(const dataset& D_, const decision_tree_options& options_)
   : D(D_), options(options_)
  {}

  /// \brief Enumerates all possible splits.
  /// \param I The indices of the samples.
  /// \param V The indices of the split variables.
  /// \param report_split A callback function that is called for every possible split.
  template <typename ReportSplit>
  void enumerate(const index_range& I, const std::vector<std::size_t>& V, ReportSplit report_split) const
  {
    std::size_t K = D.class_count();
    std::vector<std::size_t> D1_counts(K);
    std::vector<std::size_t> D2_counts(K);
    const auto& ncat = D.category_counts();
    for (std::size_t v: V)
    {
      std::size_t ncat_v = ncat[v];
      if (2 <= ncat_v && ncat_v <= options.max_categorical_size)
      {
        enumerate_single_splits(D, I, v, options, D1_counts, D2_counts, report_split);
      }
      else
      {
        enumerate_threshold_splits(D, I, v, options, D1_counts, D2_counts, report_split);
      }
    }
  }
};

/// \brief A family of decision tree splits.
///
/// It uses subset splits for categorical variables, and threshold splits for all others.
/// The splits can be enumerated using the function \c enumerate.
struct threshold_plus_subset_split_family
{
  const dataset& D;
  const decision_tree_options& options;

  /// \brief Default constructor
  threshold_plus_subset_split_family(const dataset& D_, const decision_tree_options& options_)
    : D(D_), options(options_)
  {}

  /// \brief Enumerates all possible splits.
  /// \param I The indices of the samples.
  /// \param V The indices of the split variables.
  /// \param report_split A callback function that is called for every possible split.
  template <typename ReportSplit>
  void enumerate(const index_range& I, const std::vector<std::size_t>& V, ReportSplit report_split) const
  {
    std::size_t K = D.class_count();
    std::vector<std::size_t> D1_counts(K);
    std::vector<std::size_t> D2_counts(K);
    const auto& ncat = D.category_counts();
    for (std::size_t v: V)
    {
      std::size_t ncat_v = ncat[v];
      if (2 <= ncat_v && ncat_v <= options.max_categorical_size)
      {
        enumerate_subset_splits(D, I, v, options, D1_counts, D2_counts, report_split);
      }
      else
      {
        enumerate_threshold_splits(D, I, v, options, D1_counts, D2_counts, report_split);
      }
    }
  }
};

} // namespace aitools

#endif // AITOOLS_SPLITTERS_H
