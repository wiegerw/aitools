// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/utilities/detail/command.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_DETAIL_COMMAND_H
#define AITOOLS_UTILITIES_DETAIL_COMMAND_H

#include <string>
#include <vector>

namespace aitools::utilities::detail {

struct command
{
  std::string name;
  const std::string& input_filename;
  const std::string& output_filename;
  const std::vector<std::string>& options;

  command(const std::string& name_,
          const std::string& input_filename_,
          const std::string& output_filename_,
          const std::vector<std::string>& options_
  )
    : name(name_),
      input_filename(input_filename_),
      output_filename(output_filename_),
      options(options_)
  {
  }

  virtual void execute() = 0;

  virtual ~command() = default;
};

} // namespace aitools::utilities::detail

#endif // AITOOLS_UTILITIES_DETAIL_COMMAND_H
