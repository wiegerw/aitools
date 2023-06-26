// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file learndt.cpp
/// \brief add your file description here.

#include <iostream>
#include <string>
#include <lyra/lyra.hpp>
#include "aitools/datasets/io.h"
#include "aitools/decision_trees/algorithms.h"
#include "aitools/decision_trees/io.h"
#include "aitools/decision_trees/learning.h"
#include "aitools/utilities/command_line_tool.h"
#include "aitools/utilities/logger.h"

using namespace aitools;

namespace aitools {

template <typename Gain, typename StopCriterion>
binary_decision_tree learn_decision_tree(const aitools::dataset& D,
                                        const std::vector<std::uint32_t>& I,
                                        const aitools::decision_tree_options& tree_options,
                                        const std::string& split_family,
                                        Gain gain,
                                        StopCriterion node_finished,
                                        std::size_t seed
                                       )
{
  using namespace aitools;

  if (split_family == "threshold")
  {
    return learn_decision_tree(D, I, tree_options, threshold_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, seed);
  }
  else if (split_family == "threshold-single")
  {
    return learn_decision_tree(D, I, tree_options, threshold_plus_single_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, seed);
  }
  else if (split_family == "threshold-subset")
  {
    return learn_decision_tree(D, I, tree_options, threshold_plus_subset_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, seed);
  }
  throw std::runtime_error("unknown split family " + split_family);
}

} // namespace aitools

class tool: public command_line_tool
{
  protected:
    std::string input_file{};
    std::string output_file{};
    std::size_t seed = std::random_device{}();
    std::string split_family = "threshold";
    std::string impurity_measure = "gini";
    decision_tree_options tree_options;

    void add_options(lyra::cli& cli) override
    {
      cli |= lyra::opt(seed, "value")["--seed"]("A seed value for the random generator.");
      cli |= lyra::opt(split_family, "family")["--split-family"]("The split family").choices("threshold", "threshold-single", "threshold-subset");
      cli |= lyra::opt(tree_options.max_depth, "count")["--max-depth"]("The maximum depth of the tree");
      cli |= lyra::opt(tree_options.max_categorical_size, "size")["--max-categorical-size"]("The maximum number of classes for a categorical variable");
      cli |= lyra::opt(tree_options.min_samples_leaf, "count")["--min-samples-leaf"]("The minimum number of samples in a leaf");
      cli |= lyra::opt(tree_options.support_missing_values)["--missing"]["-m"]("Support missing values");
      cli |= lyra::opt(tree_options.optimization)["--optimized"]("Apply an optimization");
      cli |= lyra::arg(input_file, "input-file").required()("Load a dataset from the given file.");
      cli |= lyra::arg(output_file, "output-file").required()("Save a generative forest to the given file.");
    }

    std::string description() const override
    {
      return "Learn a decision tree from a dataset.";
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Reading dataset from " << input_file << std::endl;
      dataset D = load_dataset(input_file);

      tree_options.imp_measure = parse_impurity_measure(impurity_measure);
      tree_options.max_features = D.feature_count();
      tree_options.support_missing_values = tree_options.support_missing_values || D.has_missing_values();
      std::size_t n = D.X().row_count();
      std::vector<std::uint32_t> I(n);
      std::iota(I.begin(), I.end(), 0);

      AITOOLS_LOG(log::verbose) << "Creating decision tree" << std::endl;
      binary_decision_tree tree = learn_decision_tree(D, I, tree_options, split_family, gain1(tree_options.imp_measure), node_is_finished, seed);
      AITOOLS_LOG(log::verbose) << "Saving decision tree to " << output_file << std::endl;
      save_decision_tree(output_file, tree);
      return true;
    }
};

int main(int argc, const char** argv)
{
  return tool().execute(argc, argv);
}
