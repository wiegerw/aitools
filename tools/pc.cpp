// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pc.cpp
/// \brief Utilities for probabilistic circuits.

#include <iomanip>
#include <iostream>
#include <string>
#include "aitools/probabilistic_circuits/io.h"
#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/utilities/command_line_group_tool.h"
#include "aitools/utilities/logger.h"

namespace aitools {

class expand_sum_split_nodes_command : public utilities::sub_command
{
  protected:
    std::string input_file;
    std::string output_file;
    probabilistic_circuit pc;

    void add_options(lyra::command& cmd) override
    {
      cmd.add_argument(lyra::arg(input_file, "input-file").required()("A file containing a generative forest."));
      cmd.add_argument(lyra::arg(output_file, "output-file").required()("The output file containing a probabilistic circuit."));
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Loading probabilistic circuit from " << input_file << std::endl;
      pc = load_probabilistic_circuit(input_file);
      AITOOLS_LOG(log::verbose) << "Saving probabilistic circuit to " << output_file << std::endl;
      save_probabilistic_circuit(output_file, pc);
      return true;
    }

  public:
    expand_sum_split_nodes_command()
      : utilities::sub_command("expand-sum-split-nodes", "Expand sum-split nodes in the PC.")
    {
    }
};

class is_decomposable_command : public utilities::sub_command
{
  protected:
    std::string input_file;
    probabilistic_circuit pc;

    void add_options(lyra::command& cmd) override
    {
      cmd.add_argument(lyra::arg(input_file, "input-file").required()("A file containing a probabilistic circuit."));
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Loading probabilistic circuit from " << input_file << std::endl;
      pc = load_probabilistic_circuit(input_file);
      bool result = is_decomposable(pc);
      std::cout << std::boolalpha << result << std::endl;
      return true;
    }

  public:
    is_decomposable_command()
    : utilities::sub_command("is-decomposable", "Determines if a probabilistic circuit is decomposable.")
    {
    }
};

class is_smooth_command : public utilities::sub_command
{
  protected:
    std::string input_file;
    probabilistic_circuit pc;

    void add_options(lyra::command& cmd) override
    {
      cmd.add_argument(lyra::arg(input_file, "input-file").required().help("A file containing a probabilistic circuit."));
    }

    bool run() override
    {
      AITOOLS_LOG(log::verbose) << "Loading probabilistic circuit from " << input_file << std::endl;
      pc = load_probabilistic_circuit(input_file);
      bool result = is_smooth(pc);
      std::cout << std::boolalpha << result << std::endl;
      return true;
    }

  public:
    is_smooth_command()
     : utilities::sub_command("is-smooth", "Determines if a probabilistic circuit is smooth.")
    {
    }
};

} // namespace aitools

int main(int argc, const char** argv)
{
  using namespace aitools;

  utilities::command_line_group_tool tool;
  expand_sum_split_nodes_command expand_sum_split_nodes;
  is_decomposable_command is_decomposable;
  is_smooth_command is_smooth;
  tool.add_command(expand_sum_split_nodes);
  tool.add_command(is_decomposable);
  tool.add_command(is_smooth);
  return tool.execute(argc, argv);
}
