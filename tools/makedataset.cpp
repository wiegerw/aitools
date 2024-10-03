// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file makedataset.cpp
/// \brief add your file description here.

#include <string>
#include <lyra/lyra.hpp>
#include "aitools/datasets/algorithms.h"
#include "aitools/datasets/io.h"
#include "aitools/datasets/random.h"
#include "aitools/statistics/distribution_io.h"
#include "aitools/utilities/command_line_tool.h"
#include "aitools/utilities/print.h"

using namespace aitools;

class tool: public command_line_tool
{
  protected:
    std::string input_file{};
    std::string output_file{};
    unsigned int size{10000};
    std::size_t seed = std::random_device{}();

    void add_options(lyra::cli& cli) override
    {
      cli |= lyra::opt(size, "size")["--size"]("The number of samples.");
      cli |= lyra::arg(input_file, "input-file").required()("A file with distributions.");
      cli |= lyra::arg(output_file, "output-file").required()("The file in which the dataset is saved.");
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Reading distributions from " << input_file << std::endl;
      std::vector<distribution> random_variables = load_distribution_list(input_file);
      std::mt19937 rng{static_cast<unsigned int>(seed)};
      AITOOLS_LOG(log::verbose) << "Creating dataset" << std::endl;
      dataset D = make_random_dataset(random_variables, size, rng);
      AITOOLS_LOG(log::verbose) << "Saving dataset to " << output_file << std::endl;
      save_dataset(output_file, D);
      return true;
    }
};

int main(int argc, const char** argv)
{
  return tool().execute(argc, argv);
}
