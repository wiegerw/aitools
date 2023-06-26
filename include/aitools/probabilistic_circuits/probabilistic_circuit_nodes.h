// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/probabilistic_circuits/probabilistic_circuit_nodes.h
/// \brief add your file description here.

#ifndef AITOOLS_PROBABILISTIC_CIRCUITS_PROBABILISTIC_CIRCUIT_NODES_H
#define AITOOLS_PROBABILISTIC_CIRCUITS_PROBABILISTIC_CIRCUIT_NODES_H

#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>
#include "aitools/decision_trees/splitters.h"
#include "aitools/numerics/math_functions.h"
#include "aitools/statistics/distributions.h"
#include "aitools/statistics/sampling.h"
#include "aitools/utilities/logger.h"
#include "aitools/utilities/print.h"
#include "aitools/utilities/stack_array.h"

namespace aitools {

class pc_node;

using pc_node_ptr = std::shared_ptr<pc_node>;

/// \brief Abstract base class for nodes in a generative forest
class pc_node : public std::enable_shared_from_this<pc_node>
{
  protected:
    std::vector<pc_node_ptr> m_successors;

    // saves the common part of all PC nodes
    void save_node(std::ostream& out, const std::string& name, std::size_t index, const std::vector <std::size_t>& successors) const
    {
      out << name << ": " << index << ' ';
      print_container(out, successors, "[", "]", " ");
    }

  public:
    virtual ~pc_node() = default;

    // A scratch value used for inference computations
    mutable double value{};

    bool is_leaf() const
    {
      return m_successors.empty();
    }

    const std::vector<pc_node_ptr>& successors() const
    {
      return m_successors;
    }

    std::vector<pc_node_ptr>& successors()
    {
      return m_successors;
    }

    /// \brief Compute an EVI query recursively.
    virtual double evi(const std::vector<double>& x) const = 0;

    /// \brief Compute a log EVI query recursively.
    virtual double log_evi(const std::vector<double>& x) const = 0;

    /// \brief Compute an EVI query iteratively.
    virtual void evi_iterative(const std::vector<double>& x) const = 0;

    /// \brief Compute a log EVI query iteratively.
    virtual void log_evi_iterative(const std::vector<double>& x) const = 0;

    /// \brief Draw a random sample. This is a generic implementation for PCs.
    virtual void sample(std::vector<double>& x, std::mt19937& rng) const = 0;

    /// \brief Saves the node in a simple textual format to the stream \c out.
    /// \param i The index of the node.
    /// \param successors The successors of the nodes: <tt>successors[i]</tt> contains the indices of the successors
    /// of the node with index \c i.
    virtual void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const = 0;
};

class sum_node : public pc_node
{
  protected:
    std::vector<double> m_weights;

  public:
    explicit sum_node(std::vector<double> weights)
      : m_weights(std::move(weights))
    {
    }

    const std::vector<double>& weights() const
    {
      return m_weights;
    };

    std::vector<double>& weights()
    {
      return m_weights;
    };

    double evi(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      double result = 0.;
      for (std::size_t i = 0; i < p; i++)
      {
        result += m_weights[i] * m_successors[i]->evi(x);
      }
      return result;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      AITOOLS_DECLARE_STACK_ARRAY(result, double, p);
      for (std::size_t i = 0; i < p; i++)
      {
        result[i] = std::log(m_weights[i]) + m_successors[i]->log_evi(x);
      }
      return log_sum_exp(result.begin(), result.end());
    }

    void evi_iterative(const std::vector<double>& x) const override
    {
      value = 0;
      std::size_t p = m_successors.size();
      for (std::size_t i = 0; i < p; i++)
      {
        value += m_weights[i] * m_successors[i]->value;
      }
    }

    void log_evi_iterative(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      AITOOLS_DECLARE_STACK_ARRAY(result, double, p);
      for (std::size_t i = 0; i < p; i++)
      {
        result[i] = std::log(m_weights[i]) + m_successors[i]->log_evi(x);
      }
      value = log_sum_exp(result.begin(), result.end());
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      std::uniform_real_distribution<double> uni(0, 1);
      double p = uni(rng);
      std::size_t j = detail::find_categorical_section(m_weights, p);
      const auto& v_j = m_successors[j];
      v_j->sample(x, rng);
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "sum", index, successors);
      out << ' ' << print_container(m_weights, "[", "]", " ") << "\n";
    }
};

class sum_split_node : public sum_node
{
  protected:
    splitting_criterion m_splitter;

  public:
    sum_split_node(std::vector<double> weights, const splitting_criterion& split)
      : sum_node(std::move(weights)), m_splitter(split)
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      std::size_t i = select(m_splitter, x);
      return m_weights[i] * m_successors[i]->evi(x);
    }

    double log_evi(const std::vector<double>& x) const override
    {
      std::size_t i = select(m_splitter, x);
      return std::log(m_weights[i]) + m_successors[i]->log_evi(x);
    }

    void evi_iterative(const std::vector<double>& x) const override
    {
      std::size_t i = select(m_splitter, x);
      value = m_weights[i] * m_successors[i]->value;
    }

    void log_evi_iterative(const std::vector<double>& x) const override
    {
      std::size_t i = select(m_splitter, x);
      value = std::log(m_weights[i]) + m_successors[i]->value;
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "sum_split", index, successors);
      out << ' ' << print_container(m_weights, "[", "]", " ") << ' ' << m_splitter << "\n";
    }

    const splitting_criterion& splitter() const
    {
      return m_splitter;
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      std::uniform_real_distribution<double> uni(0, 1);
      double p = uni(rng);
      std::size_t j = detail::find_categorical_section(m_weights, p);
      const auto& v_j = m_successors[j];
      v_j->sample(x, rng);
    }
};

class product_node : public pc_node
{
  public:
    double evi(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      double result = 1;
      for (std::size_t i = 0; i < p; i++)
      {
        result *= m_successors[i]->evi(x);
        if (result <= 0)
        {
          break;
        }
      }
      AITOOLS_LOG(log::debug) << "product: " << result << "\n";
      return result;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      double result = 0.;
      for (std::size_t i = 0; i < p; i++)
      {
        result += m_successors[i]->log_evi(x);
        if (result <= -infinity)
        {
          break;
        }
      }
      return result;
    }

    void evi_iterative(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      value = 1;
      for (std::size_t i = 0; i < p; i++)
      {
        value *= m_successors[i]->evi(x);
        if (value <= 0)
        {
          break;
        }
      }
    }

    void log_evi_iterative(const std::vector<double>& x) const override
    {
      std::size_t p = m_successors.size();
      value = 0;
      for (std::size_t i = 0; i < p; i++)
      {
        value += m_successors[i]->log_evi(x);
        if (value <= -infinity)
        {
          break;
        }
      }
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      for (const auto& v: m_successors)
      {
        v->sample(x, rng);
      }
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "product", index, successors);
      out << "\n";
    }
};

class terminal_node : public pc_node
{
  protected:
    unsigned int m_scope; // the index of the corresponding random variable

  public:
    explicit terminal_node(int scope)
      : m_scope(scope)
    {
    }

    unsigned int scope() const
    {
      return m_scope;
    };

    void evi_iterative(const std::vector<double>& x) const override
    {
      value = evi(x);
    }

    void log_evi_iterative(const std::vector<double>& x) const override
    {
      value = log_evi(x);
    }
};

/// \brief A terminal node that models a categorical distribution
class categorical_node : public terminal_node
{
  private:
    categorical_distribution m_dist;

  public:
    categorical_node(int scope, std::vector<double> probabilities)
      : terminal_node(scope), m_dist(std::move(probabilities))
    {
    }

    double evi(const std::vector<double>& x) const override
    {
      double x_i = x[m_scope];
      if (is_missing(x_i))
      {
        return 1;
      }
      double result = m_dist.pdf(x_i);
      AITOOLS_LOG(log::debug) << "categorical: " << result << "\n";
      return result;
    }

    double log_evi(const std::vector<double>& x) const override
    {
      return std::log(evi(x));
    }

    std::vector<double> probabilities() const
    {
      return m_dist.probabilities();
    };

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "categorical", index, successors);
      out << ' ' << m_scope << ' ' << print_container(m_dist.probabilities(), "[", "]", " ") << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      x[m_scope] = static_cast<double>(aitools::sample(m_dist, rng));
    }
};

/// \brief A terminal node that models a normal distribution
class normal_node : public terminal_node
{
  private:
    normal_distribution m_dist;

  public:
    normal_node(int scope, double mean, double standard_deviation)
      : terminal_node(scope), m_dist(mean, standard_deviation)
    {
      if (standard_deviation < 0)
      {
        throw std::invalid_argument("Standard deviation should be positive.");
      }
    }

    double mean() const
    {
      return m_dist.mean();
    };

    double standard_deviation() const
    {
      return m_dist.standard_deviation();
    };

    /// \brief The probability density function
    double evi(const std::vector<double>& x) const override
    {
      double x_i = x[m_scope];
      if (is_missing(x_i))
      {
        return 1;
      }
      double result = m_dist.pdf(x_i);
      AITOOLS_LOG(log::debug) << "gauss: " << result << "\n";
      return result;
    }

    /// \brief The log probability density function
    double log_evi(const std::vector<double>& x) const override
    {
      return std::log(evi(x));
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "normal", index, successors);
      out << ' ' << m_scope << ' ' << m_dist.mean() << ' ' << m_dist.standard_deviation() << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      x[m_scope] = aitools::sample(m_dist, rng);
    }
};

/// \brief A terminal node that models a truncated normal distribution
class truncated_normal_node : public terminal_node
{
  private:
    truncated_normal_distribution m_dist;

  public:
    truncated_normal_node(int scope,
                          double mean,
                          double standard_deviation,
                          double a = std::numeric_limits<double>::lowest(),
                          double b = std::numeric_limits<double>::max())
      : terminal_node(scope), m_dist(mean, standard_deviation, a, b)
    {
      if (standard_deviation < 0)
      {
        throw std::invalid_argument("Standard deviation should be positive.");
      }
      if (a > b)
      {
        throw std::invalid_argument("Lower bound larger than b bound in TruncatedNormal node.");
      }
    }

    double mean() const
    {
      return m_dist.normal().mean();
    };

    double standard_deviation() const
    {
      return m_dist.normal().standard_deviation();
    };

    double a() const
    {
      return m_dist.a();
    };

    double b() const
    {
      return m_dist.b();
    };

    /// \brief The probability density function
    double evi(const std::vector<double>& x) const override
    {
      double x_i = x[m_scope];
      if (is_missing(x_i))
      {
        return 1;
      }
      double result = m_dist.pdf(x_i);
      AITOOLS_LOG(log::debug) << "truncated_normal: " << result << "\n";
      return result;
    }

    /// \brief The log probability density function
    double log_evi(const std::vector<double>& x) const override
    {
      return std::log(evi(x));
    }

    void save(std::ostream& out, std::size_t index, const std::vector <std::size_t>& successors) const override
    {
      save_node(out, "truncated_normal", index, successors);
      out << ' ' << m_scope << ' ' << m_dist.normal().mean() << ' ' << m_dist.normal().standard_deviation() << ' '
          << m_dist.a() << ' ' << m_dist.b() << "\n";
    }

    void sample(std::vector<double>& x, std::mt19937& rng) const override
    {
      x[m_scope] = aitools::sample(m_dist, rng);
    }
};

} // namespace aitools

#endif // AITOOLS_PROBABILISTIC_CIRCUITS_PROBABILISTIC_CIRCUIT_NODES_H
