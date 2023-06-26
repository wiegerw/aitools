// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file buildgef.cpp
/// \brief add your file description here.

#include <filesystem>
#include <iostream>
#include <lyra/lyra.hpp>
#include "aitools/datasets/io.h"
#include "aitools/probabilistic_circuits/generative_forest.h"
#include "aitools/probabilistic_circuits/io.h"
#include "aitools/random_forests/io.h"
#include "aitools/utilities/command_line_tool.h"
#include "aitools/utilities/stopwatch.h"

using namespace aitools;

class tool: public command_line_tool
{
  protected:
    std::string input_file{};
    std::string dataset_file{};
    std::string output_file{};

    void add_options(lyra::cli& cli) override
    {
      cli |= lyra::arg(input_file, "random-forest-file").required()("A file containing a random forest");
      cli |= lyra::arg(dataset_file, "dataset-file").required()("A file containing the data set that corresponds to the forest");
      cli |= lyra::arg(output_file, "output-file").required()("The output file containing a generative forest");
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Parsing random forest " << input_file << std::endl;
      utilities::stopwatch watch;
      random_forest forest = load_random_forest(input_file);
      AITOOLS_LOG(log::verbose) << "Elapsed time: " << watch.seconds() << "\n";
      AITOOLS_LOG(log::verbose) << "Reading data file " << dataset_file << std::endl;
      dataset D = load_dataset(dataset_file);
      AITOOLS_LOG(log::verbose) << "Building generative forest" << std::endl;
      watch.reset();
      probabilistic_circuit pc = build_generative_forest(forest, D);
      AITOOLS_LOG(log::verbose) << "Elapsed time: " << watch.seconds() << "\n";
      AITOOLS_LOG(log::verbose) << "Saving generative forest to " << output_file << std::endl;
      watch.reset();
      save_probabilistic_circuit(output_file, pc);
      AITOOLS_LOG(log::verbose) << "Elapsed time: " << watch.seconds() << "\n";
      return true;
    }
};

int main(int argc, const char** argv)
{
  return tool().execute(argc, argv);
}
