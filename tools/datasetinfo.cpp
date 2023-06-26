// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file datasetinfo.cpp
/// \brief add your file description here.

#include <iostream>
#include <string>
#include <lyra/lyra.hpp>
#include "aitools/datasets/algorithms.h"
#include "aitools/datasets/io.h"
#include "aitools/utilities/command_line_tool.h"
#include "aitools/utilities/print.h"

using namespace aitools;

class tool: public command_line_tool
{
  protected:
    std::string input_file{};

    void add_options(lyra::cli& cli) override
    {
      cli |= lyra::arg(input_file, "filename").required()("Load a dataset from the given file.");
    }

    bool run() override
    {
      dataset D = load_dataset(input_file);
      std::size_t m = D.feature_count();
      std::size_t n = D.X().size();
      const auto& X = D.X();
      const auto& ncat = D.category_counts();

      auto print_feature = [&](std::size_t i)
      {
        if (D.is_categorical_variable(i))
        {
          std::size_t K = ncat[i];
          std::vector<double> fractions(K);
          compute_fractions(X.column(i), xrange(n), fractions);
          size_t missing = missing_value_count(X.column(i));
          std::cout << "feature " << i << ":"
                    << " ncat = " << K
                    << " fractions = " << aitools::print_list(fractions)
                    << (missing > 0 ? (" |missing| = " + std::to_string(missing)) : std::string(""))
                    << std::endl;
        }
        else
        {
          auto [mu, sigma] = mean_standard_deviation(D, xrange(n), i);
          size_t missing = missing_value_count(X.column(i));
          std::cout << "feature " << i << ":"
          << " ncat = 0"
          << " mean = " << mu
          << " stddev = " << sigma
          << (missing > 0 ? (" |missing| = " + std::to_string(missing)) : std::string(""))
          << std::endl;
        }
      };

      auto print_class = [&]()
      {
        std::size_t K = ncat.back();
        std::vector<double> fractions(K);
        compute_fractions(D.y(), xrange(n), fractions);
        size_t missing = missing_value_count(D.y());
        std::cout << "class:"
        << " ncat = " << K
        << " fractions = " << aitools::print_list(fractions)
        << (missing > 0 ? (" |missing| = " + std::to_string(missing)) : std::string(""))
        << std::endl;
      };

      std::cout << "dataset: " << input_file << '\n';
      std::cout << "number of samples: " << n << std::endl;
      std::cout << "number of features: " << m << std::endl;
      for (std::size_t i = 0; i < m; i++)
      {
        print_feature(i);
      }
      print_class();
      return true;
    }
};

int main(int argc, const char** argv)
{
  return tool().execute(argc, argv);
}
