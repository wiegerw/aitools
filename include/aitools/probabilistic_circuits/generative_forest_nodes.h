// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/probabilistic_circuits/generative_forest_nodes.h
/// \brief add your file description here.

#ifndef AITOOLS_PROBABILISTIC_CIRCUITS_GENERATIVE_FOREST_NODES_H
#define AITOOLS_PROBABILISTIC_CIRCUITS_GENERATIVE_FOREST_NODES_H

#include "aitools/probabilistic_circuits/probabilistic_circuit_nodes.h"

namespace aitools {

class less_node : public terminal_node
{
  private:
    int m_value;

    bool contains(double x) const
    {
      return is_missing(x) || x < m_value;
    }

  public:
    less_node(int scope, int value)
      : terminal_node(scope), m_value(value)
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 1 : 0;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 0 : -infinity;
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "less", index, successors);
      out << ' ' << m_scope << ' ' << m_value << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      throw std::runtime_error("Less nodes do not support sampling.");
    }
};

class greater_equal_node : public terminal_node
{
  private:
    int m_value;

    bool contains(double x) const
    {
      return is_missing(x) || x >= m_value;
    }

  public:
    greater_equal_node(int scope, int value)
      : terminal_node(scope), m_value(value)
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 1 : 0;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 0 : -infinity;
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "greater_equal", index, successors);
      out << ' ' << m_scope << ' ' << m_value << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      throw std::runtime_error("GreaterEqual nodes do not support sampling.");
    }
};

class equal_node : public terminal_node
{
  private:
    double m_value;

    bool contains(double x) const
    {
      return is_missing(x) || x == m_value;
    }

  public:
    equal_node(int scope, double value)
      : terminal_node(scope), m_value(value)
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 1 : 0;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 0 : -infinity;
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "equal_to", index, successors);
      out << ' ' << m_scope << ' ' << m_value << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      throw std::runtime_error("Equal nodes do not support sampling.");
    }
};

class not_equal_node : public terminal_node
{
  private:
    double m_value;

    bool contains(double x) const
    {
      return !is_missing(x) && x != m_value;
    }

  public:
    not_equal_node(int scope, double value)
      : terminal_node(scope), m_value(value)
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 1 : 0;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 0 : -infinity;
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "not_equal_to", index, successors);
      out << ' ' << m_scope << ' ' << m_value << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      throw std::runtime_error("NotEqual nodes do not support sampling.");
    }
};

class subset_node : public terminal_node
{
  private:
    uint32_t m_mask;

    bool contains(double x) const
    {
      return is_missing(x) || utilities::is_bit_set(m_mask, static_cast<std::size_t>(x));
    }

  public:
    subset_node(int scope, uint32_t mask)
      : terminal_node(scope), m_mask(mask)
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 1 : 0;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      return contains(x[m_scope]) ? 0 : -infinity;
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "subset", index, successors);
      out << ' ' << m_scope << ' ' << std::bitset<32>(m_mask) << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      throw std::runtime_error("Subset nodes do not support sampling.");
    }
};

} // namespace aitools

#endif // AITOOLS_PROBABILISTIC_CIRCUITS_GENERATIVE_FOREST_NODES_H
