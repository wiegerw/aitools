// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/probabilistic_circuit_io.h
/// \brief add your file description here.

#ifndef AITOOLS_PROBABILISTIC_CIRCUITS_IO_H
#define AITOOLS_PROBABILISTIC_CIRCUITS_IO_H

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include "aitools/decision_trees/splitters_io.h"
#include "aitools/utilities/parse_numbers.h"
#include "aitools/probabilistic_circuits/probabilistic_circuit.h"
#include "aitools/utilities/print.h"
#include "aitools/utilities/string_utility.h"
#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/probabilistic_circuits/generative_forest.h"
#include "aitools/utilities/stopwatch.h"

namespace aitools {

namespace detail {

inline
std::unordered_map<std::shared_ptr<pc_node>, std::size_t> make_node_index(const probabilistic_circuit& pc)
{
  std::unordered_map<std::shared_ptr<pc_node>, std::size_t> node_index;
  std::size_t index = 0;
  visit_nodes_bfs(pc, [&index, &node_index](const std::shared_ptr<pc_node>& u, std::size_t depth)
  {
    node_index[u] = index++;
  });
  return node_index;
}

} // namespace detail

/// \brief Saves a probabilistic circuit in a simple textual file format
inline
void save_probabilistic_circuit(std::ostream& to, const probabilistic_circuit& pc)
{
  std::vector<std::size_t> successors;

  to << "probabilistic_circuit: 1.0\n";

  std::unordered_map<std::shared_ptr<pc_node>, std::size_t> node_index = detail::make_node_index(pc);

  std::size_t N = node_index.size();
  to << "pc_size: " << N << "\n";
  to << "category_counts: " << print_container(pc.category_counts()) << "\n";

  auto compute_successors = [&node_index, &successors](const std::shared_ptr<pc_node>& u)
  {
    successors.clear();
    for (const auto& v: u->successors())
    {
      successors.push_back(node_index[v]);
    }
  };

  std::vector<std::shared_ptr<pc_node>> order = topological_ordering(pc);
  // utilities::stopwatch watch;
  for (const auto& u: order)
  {
    std::size_t ui = node_index[u];
    compute_successors(u);
    u->save(to, ui, successors);
  }
  // AITOOLS_LOG(log::verbose) << "successor computation " << watch.seconds() << " seconds" << std::endl;
}

class probabilistic_circuit_parser
{
  protected:
    probabilistic_circuit pc;
    std::vector<std::shared_ptr<pc_node>> vertices;
    std::regex re_pc_size{R"(pc_size:\s*(\d+)\s*)"};
    std::regex re_category_counts {R"(category_counts:(.*))"};
    std::regex re_sum{R"(sum:\s*(\d+)\s*\[([^\]]*)\]\s*\[([^\]]*)\]\s*)"};
    std::regex re_sum_split{R"(sum_split:\s*(\d+)\s*\[([^\]]*)\]\s*\[([^\]]*)\]\s*(\w+\([^)]*\))\s*)"};
    std::regex re_product{R"(product:\s*(\d+)\s*\[([^\]]*)\]\s*)"};
    std::regex re_categorical{R"(categorical:\s*(\d+)\s*\[\]\s*(\d+)\s*\[([^\]]*)\]\s*)"};
    std::regex re_normal{R"(normal:\s*(\d+)\s*\[\]\s*(\d+)\s*([^\s]+)\s*([^\s]+)\s*)"};
    std::regex re_truncated_normal{R"(truncated_normal:\s*(\d+)\s*\[\]\s*(\d+)\s*([^\s]+)\s*([^\s]+)\s*([^\s]+)\s*([^\s]+)\s*)"};
    std::regex re_less{R"(less:\s*(\d+)\s*\[\]\s*(\d+)\s*(.*)\s*)"};
    std::regex re_greater_equal{R"(greater_equal:\s*(\d+)\s*\[\]\s*(\d+)\s*(.*)\s*)"};
    std::regex re_equal{R"(equal:\s*(\d+)\s*\[\]\s*(\d+)\s*(.*)\s*)"};
    std::regex re_not_equal{R"(not_equal:\s*(\d+)\s*\[\]\s*(\d+)\s*(.*)\s*)"};
    std::regex re_subset{R"(subset:\s*(\d+)\s*\[\]\s*(\d+)\s*([01]+)\s*)"};

    std::vector<std::shared_ptr<pc_node>> parse_successors(const std::string& text) const
    {
      std::vector<std::size_t> S = parse_natural_number_sequence(text);
      std::vector<std::shared_ptr<pc_node>> result;
      result.reserve(S.size());
      for (auto s: S)
      {
        result.push_back(vertices[s]);
      }
      return result;
    };

    void parse_probabilistic_circuit(const std::string& line)
    {
      // TODO
    }

    void parse_pc_size(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_pc_size);
      const auto& m1 = match[1];
      std::size_t N = parse_natural_number(m1.first, m1.second);
      vertices.resize(N);
    }

    void parse_category_counts(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_category_counts);
      const auto& m1 = match[1];
      pc.category_counts() = parse_natural_number_sequence<unsigned int>(m1.first, m1.second);
    }

    void parse_sum(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_sum);
      const auto& m1 = match[1];
      auto index = parse_natural_number(m1.first, m1.second);
      auto successors = parse_successors(match.str(2));
      auto weights = parse_double_sequence(match.str(3));
      vertices[index] = std::make_shared<sum_node>(std::move(weights));
      vertices[index]->successors() = std::move(successors);
    }

    void parse_sum_split(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_sum_split);
      const auto& m1 = match[1];
      auto index = parse_natural_number(m1.first, m1.second);
      auto successors = parse_successors(match.str(2));
      auto weights = parse_double_sequence(match.str(3));
      splitting_criterion split = parse_splitting_criterion(match.str(4));
      vertices[index] = std::make_shared<sum_split_node>(std::move(weights), split);
      vertices[index]->successors() = std::move(successors);
    }

    void parse_product(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_product);
      const auto& m1 = match[1];
      auto index = parse_natural_number(m1.first, m1.second);
      auto successors = parse_successors(match.str(2));
      vertices[index] = std::make_shared<product_node>();
      vertices[index]->successors() = std::move(successors);
    }

    void parse_categorical(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_categorical);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto probabilities = parse_double_sequence(match.str(3));
      vertices[index] = std::make_shared<categorical_node>(scope, std::move(probabilities));
    }

    void parse_normal(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_normal);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto mu = parse_double(match.str(3));
      auto sigma = parse_double(match.str(4));
      vertices[index] = std::make_shared<normal_node>(scope, mu, sigma);
    }

    void parse_truncated_normal(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_truncated_normal);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto mu = parse_double(match.str(3));
      auto sigma = parse_double(match.str(4));
      auto a = parse_double(match.str(5));
      auto b = parse_double(match.str(6));
      vertices[index] = std::make_shared<truncated_normal_node>(scope, mu, sigma, a, b);
    }

    void parse_less(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_less);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto value = parse_double(match.str(3));
      vertices[index] = std::make_shared<less_node>(scope, value);
    }

    void parse_greater_equal(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_greater_equal);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto value = parse_double(match.str(3));
      vertices[index] = std::make_shared<greater_equal_node>(scope, value);
    }

    void parse_equal(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_equal);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto value = parse_double(match.str(3));
      vertices[index] = std::make_shared<equal_node>(scope, value);
    }

    void parse_not_equal(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_not_equal);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      auto value = parse_double(match.str(3));
      vertices[index] = std::make_shared<not_equal_node>(scope, value);
    }

    void parse_subset(const std::string& line)
    {
      std::smatch match = utilities::regex_match(line, re_subset);
      const auto& m1 = match[1];
      const auto& m2 = match[2];
      auto index = parse_natural_number(m1.first, m1.second);
      auto scope = parse_natural_number(m2.first, m2.second);
      uint32_t mask = parse_binary_number(match.str(2));
      vertices[index] = std::make_shared<subset_node>(scope, mask);
    }

  public:
    void parse_line(const std::string& line)
    {
      if (utilities::starts_with(line, "probabilistic_circuit:"))
      {
        parse_probabilistic_circuit(line);
      }
      else if (utilities::starts_with(line, "pc_size:"))
      {
        parse_pc_size(line);
      }
      else if (utilities::starts_with(line, "category_counts:"))
      {
        parse_category_counts(line);
      }
      // sum: 0 [1 2 3 4 5 6 7 8 9 10] [0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1 0.1]
      else if (utilities::starts_with(line, "sum:"))
      {
        parse_sum(line);
      }
      // sum_split: 3 [7 8] [0.166667 0.833333] SingleSplit(1, 0)
      else if (utilities::starts_with(line, "sum_split:"))
      {
        parse_sum_split(line);
      }
      // product: 4 [9 10 11]
      else if (utilities::starts_with(line, "product:"))
      {
        parse_product(line);
      }
      // categorical: 11 [] 2 [0.333333 0.666667]
      else if (utilities::starts_with(line, "categorical:"))
      {
        parse_categorical(line);
      }
      // normal: 96 [] 17 100.181 1e-5
      else if (utilities::starts_with(line, "normal:"))
      {
        parse_normal(line);
      }
      else if (utilities::starts_with(line, "truncated_normal:"))
      {
        parse_truncated_normal(line);
      }
      else if (utilities::starts_with(line, "less:"))
      {
        parse_less(line);
      }
      else if (utilities::starts_with(line, "greater_equal:"))
      {
        parse_greater_equal(line);
      }
      else if (utilities::starts_with(line, "equal:"))
      {
        parse_equal(line);
      }
      else if (utilities::starts_with(line, "not_equal:"))
      {
        parse_not_equal(line);
      }
      else if (utilities::starts_with(line, "subset:"))
      {
        parse_subset(line);
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

    probabilistic_circuit get_result()
    {
      pc.root() = vertices.front();
      probabilistic_circuit result;
      std::swap(pc, result);
      vertices.clear();
      return result;
    }
};

inline
probabilistic_circuit parse_probabilistic_circuit(std::istream& from)
{
  probabilistic_circuit_parser parser;
  parser.parse(from);
  return parser.get_result();
}

inline
probabilistic_circuit parse_probabilistic_circuit(const std::string& text)
{
  std::istringstream stream(text);
  return parse_probabilistic_circuit(stream);
}

inline
probabilistic_circuit load_probabilistic_circuit(const std::string& filename)
{
  std::ifstream from(filename);
  if (!from)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for reading.");
  }
  return parse_probabilistic_circuit(from);
}

/// \brief Saves a probabilistic circuit in a simple textual file format
/// \pre The nodes are stored in feed_forward order
inline
void save_probabilistic_circuit(const std::string& filename, const probabilistic_circuit& pc)
{
  std::ofstream to(filename);
  if (!to)
  {
    throw std::runtime_error("Could not open file '" + filename + "' for writing.");
  }
  save_probabilistic_circuit(to, pc);
}

} // namespace aitools

#endif // AITOOLS_PROBABILISTIC_CIRCUITS_IO_H
