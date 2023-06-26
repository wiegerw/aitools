// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/datasets/sampling.h
/// \brief add your file description here.

#ifndef AITOOLS_DATASETS_SAMPLING_H
#define AITOOLS_DATASETS_SAMPLING_H

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <vector>
#include <utility>
#include "aitools/datasets/dataset.h"
#include "aitools/utilities/random.h"

namespace aitools {

enum class sample_technique
{
    without_replacement,
    with_replacement,
    stratified
};

inline
std::ostream& operator<<(std::ostream& out, const sample_technique& criterion)
{
  switch(criterion)
  {
    case sample_technique::without_replacement: out << "without_replacement"; break;
    case sample_technique::with_replacement: out << "with_replacement"; break;
    case sample_technique::stratified: out << "stratified"; break;
  }
  return out;
}

inline
sample_technique parse_sample_technique(const std::string& text)
{
  if (text == "without-replacement")
  {
    return sample_technique::without_replacement;
  }
  else if (text == "with-replacement")
  {
    return sample_technique::with_replacement;
  }
  else if (text == "stratified")
  {
    return sample_technique::stratified;
  }
  throw std::runtime_error("Unknown sample selection criterion" + text);
}

class dataset_sampler
{
  private:
    // the indices from which the selection is taken
    const std::vector<std::uint32_t>& indices;

    sample_technique technique;

    // partitions the indices according to their class
    std::vector<std::vector<std::uint32_t>> classes;

    // a random number generator
    std::mt19937 rng;

    std::vector<std::uint32_t> select_samples_stratified(double sample_fraction)
    {
      std::vector<std::uint32_t> result;
      std::size_t N = static_cast<std::size_t>(std::round(sample_fraction * indices.size()));
      result.reserve(N);
      auto out = std::back_inserter(result);
      std::size_t K = classes.size();
      for (std::size_t k = 0; k < K; k++)
      {
        const auto& class_k = classes[k];
        std::size_t count = (N == indices.size()) ? class_k.size() : static_cast<std::size_t>(std::round(sample_fraction * class_k.size()));
        out = sample_with_replacement(class_k.begin(), class_k.end(), out, count, rng);
      }
      return result;
    }

    std::vector<std::uint32_t> select_samples_with_replacement(double sample_fraction)
    {
      std::vector<std::uint32_t> result;
      std::size_t N = static_cast<std::size_t>(std::round(sample_fraction * indices.size()));
      result.reserve(N);
      sample_with_replacement(indices.begin(), indices.end(), std::back_inserter(result), N, rng);
      return result;
    }

    std::vector<std::uint32_t> select_samples_without_replacement(double sample_fraction)
    {
      std::vector<std::uint32_t> result;
      std::size_t N = static_cast<std::size_t>(std::round(sample_fraction * indices.size()));
      result.reserve(N);
      std::sample(indices.begin(), indices.end(), std::back_inserter(result), N, rng);
      return result;
    }

  public:
    dataset_sampler(const dataset& D, const std::vector<std::uint32_t>& indices_, sample_technique technique_, std::size_t seed = std::random_device{}())
      : indices(indices_), technique(technique_), classes(D.class_count()), rng(seed)
    {
      if (technique == sample_technique::stratified)
      {
        const auto& y = D.y();
        std::size_t K = D.class_count();
        classes.reserve(K);
        for (std::size_t i: indices)
        {
          auto k = static_cast<std::size_t>(y[i]);
          classes[k].push_back(i);
        }
      }
    }

    std::vector<std::uint32_t> sample(double sample_fraction)
    {
      switch (technique)
      {
        case sample_technique::without_replacement: return select_samples_without_replacement(sample_fraction); break;
        case sample_technique::with_replacement: return select_samples_with_replacement(sample_fraction); break;
        case sample_technique::stratified: return select_samples_stratified(sample_fraction); break;
        default: return select_samples_stratified(sample_fraction); break;
      }
    }
};

class k_fold
{
  private:
    std::vector<std::uint32_t> I; // the indices of the fold
    std::size_t k; // the number of folds
    std::mt19937 rng;
    std::size_t fold_size;

  public:
    k_fold(std::vector<std::uint32_t> I_, std::size_t k_, std::size_t seed = std::random_device{}())
     : I(std::move(I_)), k(k_), rng(seed), fold_size(I.size() / k)
    {
      std::shuffle(I.begin(), I.end(), rng);
    }

    // returns the i-th test set and training set
    std::pair<std::vector<std::uint32_t>, std::vector<std::uint32_t>> folds(std::size_t i) const
    {
      assert(i < k);

      // compute the range [first, last) of the test set
      auto first = I.begin() + i * fold_size;
      auto last = (i == k - 1) ? I.end() : (I.begin() + (i+1) * fold_size);
      std::size_t n_test = last - first; // the size of the test set
      std::vector<std::uint32_t> test_set;
      test_set.reserve(n_test);
      std::copy(first, last, std::back_inserter(test_set));

      // put the other elements in the training set
      std::vector<std::uint32_t> training_set;
      training_set.reserve(I.size() - n_test);
      std::copy(I.begin(), first, std::back_inserter(training_set));
      std::copy(last, I.end(), std::back_inserter(training_set));

      return { test_set, training_set };
    }
};

} // namespace aitools

#endif // AITOOLS_DATASETS_SAMPLING_H
