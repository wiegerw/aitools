// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/random_forests/io.h
/// \brief add your file description here.

#ifndef AITOOLS_RANDOM_FORESTS_IO_H
#define AITOOLS_RANDOM_FORESTS_IO_H

#include "aitools/decision_trees/io.h"
#include "aitools/random_forests/random_forest.h"
#include "aitools/utilities/string_utility.h"
#include "aitools/utilities/text_utility.h"

namespace aitools {

class random_forest_parser
  {
    protected:
      random_forest forest{};
      decision_tree_parser dt_parser;
      std::regex re_forest_size {R"(forest_size:\s*(\d+)\s*)"};

      void parse_random_forest(const std::string& line)
      {
        // TODO
      }

      void parse_forest_size(const std::string& line)
      {
        std::smatch m;
        std::regex_match(line, m, re_forest_size);
        const auto& m1 = m[1];
        std::size_t N = parse_natural_number(m1.first, m1.second);
        forest.trees().reserve(N);
      }

    public:
      void parse_line(const std::string& line)
      {
        if (utilities::starts_with(line, "random_forest:"))
        {
          parse_random_forest(line);
        }
        else if (utilities::starts_with(line, "forest_size:"))
        {
          parse_forest_size(line);
        }
        else if (utilities::starts_with(line, "binary_decision_tree:"))
        {
          if (dt_parser.has_tree())
          {
            forest.trees().push_back(dt_parser.get_result());
          }
          dt_parser.parse_line(line);
        }
        else
        {
          // all unhandled lines are forwarded to the decision tree parser
          dt_parser.parse_line(line);
        }
      }

      void parse(std::istream& from)
      {
        for (std::string line; std::getline(from, line); )
        {
          AITOOLS_LOG(log::debug) << "LINE: " << line << std::endl;
          parse_line(line);
        }
      }

      void parse(const std::string& filename)
      {
        for (const std::string& line: utilities::split_lines(read_text_fast(filename)))
        {
          AITOOLS_LOG(log::debug) << "LINE: " << line << std::endl;
          parse_line(line);
        }
      }

      random_forest get_result()
      {
        if (dt_parser.has_tree())
        {
          forest.trees().push_back(dt_parser.get_result());
        }
        random_forest result;
        std::swap(forest, result);
        return result;
      }
  };

inline
random_forest parse_random_forest(std::istream& from)
{
  random_forest_parser parser;
  parser.parse(from);
  return parser.get_result();
}

inline
random_forest parse_random_forest(const std::string& text)
{
  std::istringstream stream(text);
  return parse_random_forest(stream);
}

inline
random_forest load_random_forest(const std::string& filename)
{
  random_forest_parser parser;
  parser.parse(filename);
  return parser.get_result();
}

inline
void save_random_forest(const std::string& filename, const random_forest& forest)
{
  std::ofstream to(filename);
  if (!to)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for writing.");
  }
  to << forest;
}

} // namespace aitools

#endif // AITOOLS_RANDOM_FORESTS_IO_H
