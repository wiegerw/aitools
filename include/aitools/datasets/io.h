// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/datasets/io.h
/// \brief add your file description here.

#ifndef AITOOLS_DATASETS_IO_H
#define AITOOLS_DATASETS_IO_H

#include <iostream>
#include "aitools/datasets/dataset.h"
#include "aitools/utilities/parse_numbers.h"

namespace aitools {

class dataset_parser
{
  protected:
    std::vector<std::vector<double>> X;
    std::vector<unsigned int> category_counts;
    std::vector<std::string> features;
    std::regex re_category_counts {R"(category_counts:(.*))"};

    void parse_dataset(const std::string& line)
    {
      // TODO
    }

    void parse_category_counts(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_category_counts);
      const auto& m1 = match[1];
      category_counts = parse_natural_number_sequence<unsigned int>(m1.first, m1.second);
    }

    void parse_features(const std::string& line)
    {
      std::size_t size = std::string("features:").size();
      features = utilities::regex_split(utilities::trim_copy(line.substr(size)), R"(\s+)");
    }

    void parse_row(const std::string& line)
    {
      std::vector<double> x = parse_double_sequence(line);
      if (!x.empty())
      {
        X.push_back(x);
      }
    }

  public:
    void parse_line(const std::string& line)
    {
      if (utilities::starts_with(line, "dataset:"))
      {
        parse_dataset(line);
      }
      else if (utilities::starts_with(line, "category_counts:"))
      {
        parse_category_counts(line);
      }
      else if (utilities::starts_with(line, "features:"))
      {
        parse_features(line);
      }
      else
      {
        parse_row(line);
      }
    }

    void parse(std::istream& from)
    {
      X.clear();
      category_counts.clear();
      for (std::string line; std::getline(from, line); )
      {
        AITOOLS_LOG(log::debug) << "LINE: " << line << std::endl;
        parse_line(line);
      }
    }

    dataset get_result()
    {
      // return {numerics::matrix<double>(std::move(X)), std::move(category_counts)}; TODO: use move constructor
      return dataset{numerics::matrix<double>(X), category_counts, features};
    }
};

inline
dataset parse_dataset(std::istream& from)
{
  dataset_parser parser;
  parser.parse(from);
  return parser.get_result();
}

inline
dataset parse_dataset(const std::string& text)
{
  std::istringstream stream(text);
  return parse_dataset(stream);
}

inline
dataset load_dataset(const std::string& filename)
{
  std::ifstream from(filename);
  if (!from)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for reading.");
  }
  return parse_dataset(from);
}

inline
void save_dataset(const std::string& filename, const dataset& D)
{
  std::ofstream to(filename);
  if (!to)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for writing.");
  }
  to << D;
}

} // namespace aitools

#endif // AITOOLS_DATASETS_IO_H
