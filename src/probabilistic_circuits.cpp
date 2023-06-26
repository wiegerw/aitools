// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file src/probabilistic_circuits.cpp
/// \brief add your file description here.

#include "aitools/probabilistic_circuits/algorithms.h"
#include "aitools/probabilistic_circuits/generative_forest.h"
#include "aitools/utilities/container_utility.h"
#include "aitools/utilities/iterator_range.h"

namespace aitools {

namespace detail {

std::shared_ptr<pc_node> make_indicator_node(const splitting_criterion& split, std::size_t j)
{
  struct make_indicator_visitor
    {
    std::size_t j;

    explicit make_indicator_visitor(std::size_t j_)
    : j(j_)
    {}

    std::shared_ptr<pc_node> operator()(const single_split& split) const
    {
      if (j == 0)
      {
        return std::make_shared<equal_node>(split.variable, split.value);
      }
      else
      {
        return std::make_shared<not_equal_node>(split.variable, split.value);
      }
    }

    std::shared_ptr<pc_node> operator()(const subset_split& split) const
    {
      if (j == 0)
      {
        return std::make_shared<subset_node>(split.variable, split.mask);
      }
      else
      {
        return std::make_shared<subset_node>(split.variable, ~split.mask);
      }
    }

    std::shared_ptr<pc_node> operator()(const threshold_split& split) const
    {
      if (j == 0)
      {
        return std::make_shared<less_node>(split.variable, split.value);
      }
      else
      {
        return std::make_shared<greater_equal_node>(split.variable, split.value);
      }
    }

    std::shared_ptr<pc_node> operator()(const std::monostate& split) const
    {
      return nullptr;
    }
    };

  return std::visit(make_indicator_visitor(j), split);
}

} // namespace detail

std::size_t probabilistic_circuit_size(const probabilistic_circuit& pc)
{
  std::size_t result = 0;
  visit_nodes_bfs(pc, [&result](const pc_node_ptr& u, std::size_t depth)
  {
    result++;
  });
  return result;
}

std::vector<pc_node_ptr> topological_ordering(const probabilistic_circuit& pc)
{
  /// \brief An iterator range of vertices
  using vertex_range = utilities::iterator_range<std::vector<pc_node_ptr>::const_iterator>;
  using stack_element = std::pair<pc_node_ptr, vertex_range>;
  enum class colors { white, gray, black };

  std::size_t n = probabilistic_circuit_size(pc);
  std::unordered_map<pc_node_ptr, colors> color_map;

  auto succ = [](const pc_node_ptr& u)
  {
    const auto& E = u->successors();
    return vertex_range(E.begin(), E.end());
  };

  auto next = [](const vertex_range& E)
  {
    return vertex_range(E.begin() + 1, E.end());
  };

  auto color = [&](const pc_node_ptr& u) -> colors&
  {
    auto i = color_map.find(u);
    if (i == color_map.end())
    {
      i = color_map.insert({u, colors::white}).first;
    }
    return i->second;
  };

  std::vector<pc_node_ptr> result;
  result.reserve(n);

  std::stack<stack_element> dfs_stack;

  auto u0 = pc.root(); // the root of the PC
  color(u0) = colors::gray;
  dfs_stack.push({ u0, succ(u0) });
  while (!dfs_stack.empty())
  {
    auto [u, E] = dfs_stack.top();
    dfs_stack.pop();
    while (!E.empty())
    {
      auto v = E.front();
      if (color(v) == colors::white)
      {
        dfs_stack.push(stack_element(u, next(E)));
        u = v;
        color(u) = colors::gray;
        E = succ(u);
      }
      else
      {
        E = next(E);
      }
    }
    color(u) = colors::black;
    result.push_back(u);
  }
  return result;
}

bool is_normalized(const probabilistic_circuit& pc, double tolerance)
{
  bool result = true;
  visit_nodes_bfs(pc, [&result, tolerance](const pc_node_ptr& u, std::size_t depth)
  {
    if (auto u_ = std::dynamic_pointer_cast<sum_node>(u); u_) // This also covers sum_split_node
    {
      double w = aitools::sum(u_->weights());
      if (std::fabs(w - 1) > tolerance)
      {
        result = false;
      }
    }
  });
  return result;
}

struct smoothness_checker
{
  bool result = true;

  // Returns the scope of u; sets result to false if a sum node was found that is not smooth
  std::set<unsigned int> scope(const pc_node_ptr& u)
  {
    using utilities::set_union;

    if (!result)
    {
      return {};
    }

    if (auto u_ = std::dynamic_pointer_cast<terminal_node>(u); u_)
    {
      return {u_->scope()};
    }

    if (auto u_ = std::dynamic_pointer_cast<sum_node>(u); u_)
    {
      std::set<unsigned int> scope_u;
      for (const pc_node_ptr& v: u->successors())
      {
        std::set<unsigned int> scope_v = scope(v);
        if (scope_u.empty())
        {
          std::swap(scope_u, scope_v);
        }
        else
        {
          if (scope_v != scope_u)
          {
            result = false;
          }
        }
      }
      return scope_u;
    }

    if (auto u_ = std::dynamic_pointer_cast<product_node>(u); u_)
    {
      std::set<unsigned int> scope_u;
      for (const pc_node_ptr& v: u->successors())
      {
        scope_u = set_union(scope_u, scope(v));
      }
      return scope_u;
    }

    throw std::runtime_error("smoothness_checker: unexpected node");
  };

  bool check(const probabilistic_circuit& pc)
  {
    scope(pc.root());
    return result;
  }
};

bool is_smooth(const probabilistic_circuit& pc)
{
  return smoothness_checker().check(pc);
}

struct decomposability_checker
{
  bool result = true;

  // Returns the scope of u; sets result to false if a product node was found that is not decomposable
  std::set<unsigned int> scope(const pc_node_ptr& u)
  {
    using utilities::set_union;

    if (!result)
    {
      return {};
    }

    if (auto u_ = std::dynamic_pointer_cast<terminal_node>(u); u_)
    {
      return {u_->scope()};
    }

    if (auto u_ = std::dynamic_pointer_cast<sum_node>(u); u_)
    {
      std::set<unsigned int> scope_u;
      for (const pc_node_ptr& v: u->successors())
      {
        scope_u = set_union(scope_u, scope(v));
      }
      return scope_u;
    }

    if (auto u_ = std::dynamic_pointer_cast<product_node>(u); u_)
    {
      std::set<unsigned int> scope_u;
      for (const pc_node_ptr& v: u->successors())
      {
        for (auto x: scope(v))
        {
          auto [iter, inserted] = scope_u.insert(x);
          if (!inserted)
          {
            result = false;
          }
        }
      }
      return scope_u;
    }

    throw std::runtime_error("decomposability_checker: unexpected node");
  };

  bool check(const probabilistic_circuit& pc)
  {
    scope(pc.root());
    return result;
  }
};

bool is_decomposable(const probabilistic_circuit& pc)
{
  return decomposability_checker().check(pc);
}

std::vector<double> sample_pc(const probabilistic_circuit& pc, std::mt19937& rng)
{
  std::size_t m = pc.feature_count();
  std::vector<double> result;
  result.resize(m);
  pc.root()->sample(result, rng);
  return result;
}

dataset sample_pc(const probabilistic_circuit& pc, std::size_t n, std::mt19937& rng)
{
  std::size_t m = pc.feature_count();

  std::vector<std::vector<double>> X;
  X.reserve(n);

  std::vector<double> x;
  x.resize(m);
  for (std::size_t i = 0; i < n; i++)
  {
    pc.root()->sample(x, rng);
    X.push_back(x);
  }

  return {numerics::matrix<double>(X), pc.category_counts()};
}

bool is_valid(const probabilistic_circuit& pc)
{
  bool valid = true;

  auto contains = [](const auto& map, const auto& key)
  {
    return map.find(key) != map.end();
  };

  // map the nodes to numbers
  std::unordered_map<pc_node_ptr, std::size_t> index;
  visit_nodes_bfs(pc, [&index](const pc_node_ptr& u, std::size_t depth)
  {
    std::size_t k = index.size();
    index[u] = k;
  });

  // check the successors
  visit_nodes_bfs(pc, [&index, &contains, &valid](const pc_node_ptr& u, std::size_t depth)
  {
    for (const auto& v: u->successors())
    {
      if (!contains(index, v))
      {
        AITOOLS_LOG(log::verbose) << "Found a node with an unknown successor" << std::endl;
        valid = false;
        break;
      }
    }
  });

  return valid;
}

std::shared_ptr<pc_node> fit_normal(const binary_decision_tree::vertex& u, const dataset& D, std::size_t i, const interval& ab)
{
  auto [mu, sigma] = u.I.empty() ? std::make_pair(0.0, 1.0) : mean_standard_deviation(D, u.I, i);
  if (ab.is_maximal())
  {
    return std::make_shared<normal_node>(i, mu, sigma);
  }
  else
  {
    return std::make_shared<truncated_normal_node>(i, mu, sigma, ab.a, ab.b);
  }
}

std::shared_ptr<pc_node> fit_categorical(const binary_decision_tree::vertex& u, const dataset& D, std::size_t i)
{
  std::size_t K = D.category_counts()[i];
  AITOOLS_DECLARE_STACK_ARRAY(counts, std::size_t, K);
  D.compute_categorical_counts(u.I, i, counts);
  std::size_t total = sum(counts);
  // double alpha = 1e-10; // TODO set the right value and add smoothing using alpha
  std::vector<double> probabilities;
  probabilities.reserve(K);
  for (std::size_t k: counts)
  {
    probabilities.push_back(static_cast<double>(k) / total);
  }
  return std::make_shared<categorical_node>(i, probabilities);
}

void fit_leave_nodes(const binary_decision_tree& tree, std::vector<std::shared_ptr<pc_node>>& pc_nodes, const dataset& D)
{
  using vertex = binary_decision_tree::vertex;

  const auto& ncat = tree.category_counts();
  std::size_t m = tree.feature_count();

  enumerate_intervals(tree, m, [&](const vertex& u, std::uint32_t ui, const std::vector<interval>& intervals)
  {
    if (u.is_leaf())
    {
      // construct a product node with an outgoing edge for each variable
      auto u_ = std::make_shared<product_node>();
      for (std::size_t i = 0; i < m; i++)
      {
        if (ncat[i] < 2) // continuous variable
        {
          auto v_i = fit_normal(u, D, i, intervals[i]);
          u_->successors().push_back(v_i);
        }
        else
        {
          auto v_i = fit_categorical(u, D, i);
          u_->successors().push_back(v_i);
        }
      }
      // add an outgoing edge for the class variable
      auto v = fit_categorical(u, D, m);
      u_->successors().push_back(v);
      pc_nodes[ui] = u_;
    }
  });
}

std::shared_ptr<pc_node> build_generative_tree(const binary_decision_tree& tree, const dataset& D)
{
  std::size_t n = tree.vertices().size();
  std::vector<std::shared_ptr<pc_node>> pc_nodes{n};

  // First construct the leaf nodes. This has to be done in a separate step, since the Gaussian leaf nodes need
  // to be truncated to an interval [a,b].
  fit_leave_nodes(tree, pc_nodes, D);

  std::vector<std::uint32_t> order = topological_ordering(tree);
  std::reverse(order.begin(), order.end());
  for (std::size_t ui: order) // Construct the non-terminal nodes in reverse topological order.
  {
    const auto& u = tree.find_vertex(ui);
    if (!u.is_leaf())
    {
      const auto& v1 = tree.find_vertex(u.left);
      const auto& v2 = tree.find_vertex(u.right);
      auto u_size = static_cast<double>(u.I.size());
      std::vector<double> m_weights = { v1.I.size() / u_size, v2.I.size() / u_size };
      auto u_ = std::make_shared<sum_split_node>(std::move(m_weights), u.split);
      u_->successors().push_back(pc_nodes[u.left]);
      u_->successors().push_back(pc_nodes[u.right]);
      pc_nodes[ui] = u_;
    }
  }
  return pc_nodes.front();
}

probabilistic_circuit build_generative_forest(const random_forest& forest, const dataset& D)
{
  std::size_t N = forest.trees().size();
  double weight = 1.0 / N;
  std::vector<double> weights(N, weight);
  auto root = std::make_shared<sum_node>(weights);
  for (const auto& tree: forest.trees())
  {
    root->successors().push_back(build_generative_tree(tree, D));
  }
  return probabilistic_circuit(root, D.category_counts());
}

void expand_sum_split_nodes(probabilistic_circuit& pc)
{
  std::deque<std::shared_ptr<pc_node>> todo = { pc.root() };

  auto expand_sum_split_node = [](std::shared_ptr<sum_split_node>& u) -> std::shared_ptr<pc_node>
  {
    auto& u_successors = u->successors();
    for (std::size_t j = 0; j < u_successors.size(); j++)
    {
      auto v_j = u_successors[j];
      auto y_j = std::make_shared<product_node>();
      auto z_j = detail::make_indicator_node(u->splitter(), j);
      u_successors[j] = y_j;
      y_j->successors() = {v_j, z_j};
    }
    auto result = std::make_shared<sum_node>(std::move(u->weights()));
    result->successors() = std::move(u->successors());
    return result;
  };

  while (!todo.empty())
  {
    auto u = todo.front();
    todo.pop_front();
    for (auto& v: u->successors())
    {
      auto v_ = std::dynamic_pointer_cast<sum_split_node>(v);
      if (v_)
      {
        v = expand_sum_split_node(v_);
      }
      todo.push_back(v);
    }
  }
}

} // namespace aitools