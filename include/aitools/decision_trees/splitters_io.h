// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_tree_splitters_io.h
/// \brief add your file description here.

#ifndef AITOOLS_SPLITTERS_IO_H
#define AITOOLS_SPLITTERS_IO_H

#include "splitters.h"
#include "aitools/utilities/parse_numbers.h"
#include "aitools/utilities/string_utility.h"

namespace aitools {

// ThresholdSplit(1, 5.2187)
inline
threshold_split parse_threshold_split(const std::string& line)
{
  unsigned int variable;
  double value;

  auto first = skip_string(line.begin(), line.end(), std::string("ThresholdSplit("));
  auto last = line.end();
  first = parse_integer(first, last, variable);
  first = std::find(first, last, ',');
  ++first;
  first = skip_spaces(first, last);
  parse_double(first, value);
  return { variable, value };
}

// SingleSplit(6, 0.5)
inline
single_split parse_single_split(const std::string& line)
{
  unsigned int variable;
  double value;

  auto first = skip_string(line.begin(), line.end(), std::string("SingleSplit("));
  auto last = line.end();
  first = parse_integer(first, last, variable);
  first = std::find(first, last, ',');
  ++first;
  first = skip_spaces(first, last);
  parse_double(first, value);
  return { variable, value };
}

// SubsetSplit(0, 00000000000000000000001111110001)
inline
subset_split parse_subset_split(const std::string& line)
{
  auto first = skip_string(line.begin(), line.end(), std::string("SubsetSplit("));
  auto last = line.end();
  unsigned int variable;
  first = parse_integer(first, last, variable);
  first = std::find(first, last, ',');
  ++first;
  first = skip_spaces(first, last);
  last = first;
  while (*last == '0' || *last == '1')
  {
    ++last;
  }
  std::uint32_t mask = std::stoi(std::string(first, last), nullptr, 2);
  return { variable, mask };
}

inline
splitting_criterion parse_splitting_criterion(const std::string& text)
{
  if (utilities::starts_with(text, "Si"))
  {
    return parse_single_split(text);
  }
  else if (utilities::starts_with(text, "Su"))
  {
    return parse_subset_split(text);
  }
  else if (utilities::starts_with(text, "Th"))
  {
    return parse_threshold_split(text);
  }
  return std::monostate();
}

} // namespace aitools

#endif // AITOOLS_SPLITTERS_IO_H
