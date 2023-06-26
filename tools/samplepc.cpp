// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file samplepc.cpp
/// \brief add your file description here.

#include <iostream>
#include <string>
#include <lyra/lyra.hpp>
#include "aitools/datasets/io.h"
#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/probabilistic_circuits/io.h"
#include "aitools/utilities/command_line_tool.h"
#include "aitools/utilities/print.h"

using namespace aitools;

class tool: public command_line_tool
{
  protected:
    std::string input_file{};
    std::string output_file{};
    unsigned int sample_count = 10;
    std::size_t seed = std::random_device{}();

    void add_options(lyra::cli& cli) override
    {
      cli |= lyra::opt(sample_count, "count")["--count"]("The number of samples.");
      cli |= lyra::opt(seed, "value")["--seed"]("A seed value for the random generator.");
      cli |= lyra::arg(input_file, "input-file").required()("A file containing a probabilistic circuit.");
      cli |= lyra::arg(output_file, "output-file").required()("A file where the generated dataset is written to.");
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Loading probabilistic circuit from " << input_file << std::endl;
      probabilistic_circuit pc = load_probabilistic_circuit(input_file);
      std::mt19937 rng{seed};
      AITOOLS_LOG(log::verbose) << "Drawing " << sample_count << " samples from the probabilistic circuit" << std::endl;
      dataset D = sample_pc(pc, sample_count, rng);
      AITOOLS_LOG(log::verbose) << "Saving dataset to " << output_file << std::endl;
      save_dataset(output_file, D);
      return true;
    }

    std::string description() const override
    {
      return "Draws random samples from a probabilistic circuit and saves them in a dataset.";
    }
};

int main(int argc, const char** argv)
{
  return tool().execute(argc, argv);
}
