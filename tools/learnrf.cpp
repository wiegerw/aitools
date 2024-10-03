// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file learnrf.cpp
/// \brief add your file description here.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <lyra/lyra.hpp>
#include "aitools/datasets/io.h"
#include "aitools/decision_trees/algorithms.h"
#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/random_forests/algorithms.h"
#include "aitools/random_forests/io.h"
#include "aitools/random_forests/learning.h"
#include "aitools/utilities/file_utility.h"
#include "aitools/utilities/stopwatch.h"
#include "aitools/utilities/command_line_tool.h"

using namespace aitools;

inline
std::string add_number(const std::string& filename, int i)
{
  std::string stem = std::filesystem::path(filename).stem().string();
  std::string extension = std::filesystem::path(filename).extension().string();
  std::string result = stem + '-' + std::to_string(i);
  if (!extension.empty())
  {
    result = result + '.' + extension;
  }
  return result;
}

template <typename StopCriterion>
aitools::random_forest learn_random_forest(const aitools::dataset& D,
                                        const std::vector<std::uint32_t>& I,
                                        aitools::random_forest_options forest_options,
                                        const aitools::decision_tree_options& tree_options,
                                        const std::string& split_family,
                                        StopCriterion node_finished,
                                        bool sequential,
                                        std::size_t seed)
{
  using namespace aitools;

  if (split_family == "threshold")
  {
    return aitools::learn_random_forest(D, I, forest_options, tree_options, threshold_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, sequential, seed);
  }
  else if (split_family == "threshold-single")
  {
    return aitools::learn_random_forest(D, I, forest_options, tree_options, threshold_plus_single_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, sequential, seed);
  }
  else if (split_family == "threshold-subset")
  {
    return aitools::learn_random_forest(D, I, forest_options, tree_options, threshold_plus_subset_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, sequential, seed);
  }
  else
  {
    throw std::runtime_error("unknown split family " + split_family);
  }
}

class tool: public command_line_tool
{
  protected:
    std::string input_file;
    decision_tree_options tree_options;
    random_forest_options forest_options;
    std::string impurity_measure = "gini";
    std::string sample_technique = "stratified";
    double variable_fraction = 0.3;
    std::size_t max_rows = 1000000;
    std::string execution_mode = "sequential";
    std::size_t seed = std::random_device{}();
    std::string split_family = "threshold";
    std::size_t fold = 0;
    std::string output_file{};

    void add_options(lyra::cli& cli) override
    {
      cli |= lyra::opt(split_family, "family")["--split-family"]("The split family").choices("threshold", "threshold-single", "threshold-subset");
      cli |= lyra::opt(forest_options.forest_size, "count")["--forest-size"]["-t"]("The number of decision trees in the forest");
      cli |= lyra::opt(forest_options.sample_fraction, "fraction")["--sample-fraction"]["-s"]("The fraction of samples used for learning a decision tree");
      cli |= lyra::opt(sample_technique, "technique")["--sample-technique"]("The technique used for selecting samples").choices("without-replacement", "with-replacement", "stratified");
      cli |= lyra::opt(tree_options.max_depth, "count")["--max-depth"]("The maximum depth of the tree");
      cli |= lyra::opt(tree_options.max_categorical_size, "size")["--max-categorical-size"]("The maximum number of classes for a categorical variable");
      cli |= lyra::opt(tree_options.min_samples_leaf, "count")["--min-samples-leaf"]("The minimum number of samples in a leaf");
      cli |= lyra::opt(tree_options.support_missing_values)["--missing"]["-m"]("Support missing values");
      cli |= lyra::opt(tree_options.optimization)["--optimized"]("Apply an optimization");
      cli |= lyra::opt(max_rows, "count")["--max-rows"]("The maximum number of rows in the data set");
      cli |= lyra::opt(variable_fraction, "fraction")["--variable-fraction"]["-f"]("The fraction of variables used for learning a decision tree");
      cli |= lyra::opt(impurity_measure, "imp")["--impurity-measure"]["-i"]("The impurity measure").choices("gini", "entropy", "misclassification");
      cli |= lyra::opt(execution_mode, "mode")["--execution-mode"]("The execution mode").choices("sequential", "parallel");
      cli |= lyra::opt(seed, "value")["--seed"]("A seed value that can be used to make the algorithm deterministic. N.B. This does not work with parallel execution");
      cli |= lyra::opt(fold, "value")["--fold"]("Apply a k-fold cross validation");
      cli |= lyra::arg(input_file, "input-file").required()("The input file containing a data set");
      cli |= lyra::arg(output_file, "output-file").required()("Save the generated random forest to the given file");
    }

    std::string description() const override
    {
      return "Learn a random forest from a dataset.";
    }

    bool run() override
    {
      dataset D = load_dataset(input_file);

      tree_options.imp_measure = parse_impurity_measure(impurity_measure);
      forest_options.sample_criterion = parse_sample_technique(sample_technique);

      if (variable_fraction < 0.0 || variable_fraction > 1.0)
      {
        throw std::runtime_error("The variable fraction must be in the interval [0, 1]");
      }

      tree_options.max_features = std::max(static_cast<std::size_t>(1), static_cast<std::size_t>(std::round(variable_fraction * D.feature_count())));
      tree_options.support_missing_values = tree_options.support_missing_values || D.has_missing_values();

      AITOOLS_LOG(log::verbose) << "input_file = " << input_file << "\n";
      if (m_verbose)
      {
        print_info(D);
      }
      AITOOLS_LOG(log::verbose) << tree_options;
      AITOOLS_LOG(log::verbose) << forest_options;
      AITOOLS_LOG(log::verbose) << "execution mode = " << execution_mode << '\n';
      AITOOLS_LOG(log::verbose) << "variable fraction = " << variable_fraction << '\n';
      AITOOLS_LOG(log::verbose) << "seed = " << seed << '\n';

      std::size_t n = std::min(max_rows, D.X().row_count());
      std::vector<std::uint32_t> I(n);
      std::iota(I.begin(), I.end(), 0);
      bool sequential = execution_mode == "sequential";

      if (fold == 0)
      {
        utilities::stopwatch watch;
        random_forest forest = ::learn_random_forest(D, I, forest_options, tree_options, split_family, node_is_finished, sequential, seed);
        AITOOLS_LOG(log::verbose) << "elapsed time: " << watch.seconds() << "\n";
        save_random_forest(output_file, forest);
        const auto& trees = forest.trees();
        for (std::size_t i = 0; i < trees.size(); i++)
        {
          const auto& tree = trees[i];
          AITOOLS_LOG(log::debug) << "tree " << i << " #nodes = " << tree.vertices().size() << std::endl;
        }
      }
      else
      {
        k_fold f(I, fold, seed);
        for (unsigned int i = 0; i < fold; i++)
        {
          auto [test_set, training_set] = f.folds(i);
          random_forest forest = ::learn_random_forest(D, training_set, forest_options, tree_options, split_family, node_is_finished, sequential, seed);
          save_random_forest(add_number(output_file, i), forest);
          std::cout << "accuracy test set     " << i << " = " << accuracy(forest, test_set, D) << std::endl;
          std::cout << "accuracy training set " << i << " = " << accuracy(forest, training_set, D) << std::endl;
        }
      }
      return true;
    }
};

int main(int argc, const char** argv)
{
  return tool().execute(argc, argv);
}
