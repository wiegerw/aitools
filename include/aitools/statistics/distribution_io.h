// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/statistics/distribution_io.h
/// \brief add your file description here.

#ifndef AITOOLS_STATISTICS_DISTRIBUTION_IO_H
#define AITOOLS_STATISTICS_DISTRIBUTION_IO_H

#include <fstream>
#include <sstream>
#include "aitools/statistics/distribution.h"
#include "aitools/statistics/distributions.h"
#include "aitools/utilities/parse_numbers.h"
#include "aitools/utilities/string_utility.h"

namespace aitools {

class distribution_parser
{
  protected:
    std::regex re_uniform_distribution{R"(UniformDistribution\(\s*([^\s]+),\s*([^\s]+)\s*\))"};
    std::regex re_normal_distribution{R"(NormalDistribution\(\s*([^\s]+),\s*([^\s]+)\s*\))"};
    std::regex re_truncated_normal_distribution{R"(TruncatedNormalDistribution\(\s*([^\s]+),\s*([^\s]+),\s*([^\s]+),\s*([^\s]+)\s*\))"};
    std::regex re_categorical_distribution{R"(CategoricalDistribution\(([^)]+)\))"};

    uniform_distribution parse_uniform_distribution(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_uniform_distribution);
      auto a = parse_double(match.str(1));
      auto b = parse_double(match.str(2));
      return uniform_distribution(a, b);
    }

    normal_distribution parse_normal_distribution(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_normal_distribution);
      auto mu = parse_double(match.str(1));
      auto sigma = parse_double(match.str(2));
      return normal_distribution(mu, sigma);
    }

    truncated_normal_distribution parse_truncated_normal_distribution(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_truncated_normal_distribution);
      auto mu = parse_double(match.str(1));
      auto sigma = parse_double(match.str(2));
      auto a = parse_double(match.str(3));
      auto b = parse_double(match.str(4));
      return truncated_normal_distribution(mu, sigma, a, b);
    }

    categorical_distribution parse_categorical_distribution(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_categorical_distribution);
      std::vector<double> probabilities;
      for (const std::string& s: utilities::regex_split(match.str(1), ","))
      {
        probabilities.push_back(parse_double(s));
      }
      return categorical_distribution(probabilities);
    }

  public:
    distribution parse_distribution(const std::string& line)
    {
      if (utilities::starts_with(line, "UniformDistribution("))
      {
        return parse_uniform_distribution(line);
      }
      else if (utilities::starts_with(line, "NormalDistribution("))
      {
        return parse_normal_distribution(line);
      }
      else if (utilities::starts_with(line, "TruncatedNormalDistribution("))
      {
        return parse_truncated_normal_distribution(line);
      }
      else if (utilities::starts_with(line, "CategoricalDistribution("))
      {
        return parse_categorical_distribution(line);
      }
      throw std::runtime_error("Could not parse a distribution from '" + line + "'");
    }

    std::vector<distribution> parse_distribution_list(std::istream& from)
    {
      std::vector<distribution> result;
      for (std::string line; std::getline(from, line);)
      {
        AITOOLS_LOG(log::debug) << "LINE: " << line << std::endl;
        result.push_back(parse_distribution(line));
      }
      return result;
    }
};

inline
distribution parse_distribution(std::istream& from)
{
  distribution_parser parser;
  std::string line;
  std::getline(from, line);
  return parser.parse_distribution(line);
}

inline
distribution parse_distribution(const std::string& line)
{
  std::istringstream stream(line);
  return parse_distribution(stream);
}

inline
std::vector<distribution> parse_distribution_list(std::istream& from)
{
  distribution_parser parser;
  return parser.parse_distribution_list(from);
}

inline
std::vector<distribution> parse_distribution_list(const std::string& text)
{
  std::istringstream stream(text);
  return parse_distribution_list(stream);
}

inline
std::vector<distribution> load_distribution_list(const std::string& filename)
{
  std::ifstream from(filename);
  if (!from)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for reading.");
  }
  return parse_distribution_list(from);
}

} // namespace aitools

#endif // AITOOLS_STATISTICS_DISTRIBUTION_IO_H
