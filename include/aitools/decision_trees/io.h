// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/decision_tree_io.h
/// \brief add your file description here.

#ifndef AITOOLS_IO_H
#define AITOOLS_IO_H

#include <fstream>
#include <string>
#include "aitools/decision_trees/decision_tree.h"
#include "aitools/decision_trees/splitters_io.h"
#include "aitools/utilities/print.h"
#include "aitools/utilities/parse_numbers.h"
#include "aitools/utilities/string_utility.h"
#include "aitools/utilities/text_utility.h"

namespace aitools {

class decision_tree_parser
{
  protected:
    binary_decision_tree tree;

    void parse_decision_tree(const std::string& line)
    {
      // TODO
    }

    void parse_tree_size(const std::string& line)
    {
      auto first = skip_string(line.begin(), line.end(), std::string("tree_size:"));
      unsigned int N;
      parse_integer(first, line.end(), N);
      tree.vertices().resize(N);
    }

    void parse_indices(const std::string& line)
    {
      auto first = skip_string(line.begin(), line.end(), std::string("indices:"));
      tree.indices() = parse_natural_number_sequence<std::uint32_t>(first, line.end());
    }

    void parse_classes(const std::string& line)
    {
      auto first = skip_string(line.begin(), line.end(), std::string("classes:"));
      tree.classes() = parse_natural_number_sequence<std::uint32_t>(first, line.end());
    }

    void parse_category_counts(const std::string& line)
    {
      auto first = skip_string(line.begin(), line.end(), std::string("category_counts:"));
      tree.category_counts() = parse_natural_number_sequence<unsigned int>(first, line.end());
    }

    // vertex: 23 [37 38] ThresholdSplit(0, 0.3292) 206 221
    void parse_vertex(const std::string& line)
    {
      unsigned int index;
      unsigned int left = binary_decision_tree::undefined_index;
      unsigned int right = binary_decision_tree::undefined_index;
      unsigned int i0;
      unsigned int i1;

      auto first = skip_string(line.begin(), line.end(), std::string("vertex:"));
      auto last = line.end();
      first = parse_integer(first, last, index);
      first = std::find(first, last, '[');
      ++first;
      first = parse_integer(first, last, left);
      first = parse_integer(first, last, right);
      first = std::find(first, last, ']');
      ++first;
      first = skip_spaces(first, last);
      auto end = std::find(first, last, ')');
      splitting_criterion split = parse_splitting_criterion(std::string(first, end));
      first = end;
      ++first;
      first = parse_integer(first, last, i0);
      parse_integer(first, last, i1);

      auto begin = tree.indices().begin();
      index_range I{begin + i0, begin + i1};
      tree.vertices()[index] = binary_decision_tree::vertex(I, left, right, split);
    }

  public:
    void parse_line(const std::string& line)
    {
      if (utilities::starts_with(line, "decision_tree:"))
      {
        parse_decision_tree(line);
      }
      else if (utilities::starts_with(line, "tree_size:"))
      {
        parse_tree_size(line);
      }
      else if (utilities::starts_with(line, "category_counts:"))
      {
        parse_category_counts(line);
      }
      else if (utilities::starts_with(line, "classes:"))
      {
        parse_classes(line);
      }
      else if (utilities::starts_with(line, "indices:"))
      {
        parse_indices(line);
      }
      else if (utilities::starts_with(line, "vertex:"))
      {
        parse_vertex(line);
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

    bool has_tree() const
    {
      return !tree.vertices().empty();
    }

    binary_decision_tree get_result()
    {
      binary_decision_tree result;
      std::swap(tree, result);
      return result;
    }
};

inline
binary_decision_tree parse_decision_tree(std::istream& from)
{
  decision_tree_parser parser;
  parser.parse(from);
  return parser.get_result();
}

inline
binary_decision_tree parse_decision_tree(const std::string& text)
{
  std::istringstream stream(text);
  return parse_decision_tree(stream);
}

inline
binary_decision_tree load_decision_tree(const std::string& filename)
{
  decision_tree_parser parser;
  parser.parse(filename);
  return parser.get_result();
}

inline
void save_decision_tree(const std::string& filename, const binary_decision_tree& tree)
{
  std::ofstream to(filename);
  if (!to)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for writing.");
  }
  to << tree;
}

} // namespace aitools

#endif // AITOOLS_IO_H
