// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file number_test.cpp
/// \brief Tests for numbers.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iomanip>
#include <random>
#include "aitools/utilities/print.h"
#include "aitools/utilities/random.h"

TEST_CASE("test_nan")
{
  double d = std::stod("NaN");
  CHECK(std::isnan(d));

  constexpr double nan = std::numeric_limits<double>::quiet_NaN();

  std::array<double, 6> a = { 1, 2, nan, 3, nan, 4 };
  std::array<double, 6> b = { nan, nan, 1, 2, 3, 4 };
  auto i = std::stable_partition(a.begin(), a.end(), [](double x) { return std::isnan(x); });
  std::cout << aitools::print_list(a) << std::endl;
  CHECK_EQ(aitools::print_list(a), aitools::print_list(b));
  CHECK_EQ(i - a.begin(), 2);

  a = { 3, 2, nan, 4, nan, 1 };
  b = { 1, 2, 3, 4, nan, nan };
  auto less_nan = [](double x, double y) { return std::isnan(y) || x < y; };
  auto is_not_nan = [](double x) { return !std::isnan(x); };
  std::sort(a.begin(), a.end(), less_nan);
  auto j = std::find_if(a.rbegin(), a.rend(), is_not_nan);
  std::cout << aitools::print_list(a) << std::endl;
  CHECK_EQ(aitools::print_list(a), aitools::print_list(b));
  CHECK(j == a.rbegin() + 2);
  CHECK(j.base() == a.begin() + 4);

  CHECK(!(nan < 1));
  CHECK(!(nan == 1));
  CHECK(!(nan > 1));
  CHECK(!(nan < nan));
  CHECK(!(nan == nan));
  CHECK(!(nan > nan));
}

TEST_CASE("test_partition")
{
  std::array<int, 6> a = {1, 2, 3, 4, 5, 6};
  std::array<int, 6> b = {3, 1, 2, 4, 5, 6};
  std::stable_partition(a.begin(), a.end(), [&](int i) { return i == 3; });
  CHECK_EQ(aitools::print_list(a), aitools::print_list(b));

  constexpr double nan = std::numeric_limits<double>::quiet_NaN();
  std::array<double, 6> c = { 1, 2, nan, 3, nan, 4 };
  std::array<double, 6> d = { nan, nan, 1, 2, 3, 4 };
  std::cout << aitools::print_list(a) << std::endl;
  std::stable_partition(a.begin(), a.end(), [&](int i) { return std::isnan(i); });
  std::cout << aitools::print_list(a) << std::endl;
  CHECK_EQ(aitools::print_list(a), aitools::print_list(b));
}

TEST_CASE("test_random_sample")
{
  std::vector<int> a = {2, 3, 4, 5, 6};
  std::vector<int> b;
  aitools::sample_with_replacement(a.begin(), a.end(), std::back_inserter(b), 100, std::mt19937{std::random_device{}()});
  std::cout << "b = " << aitools::print_list(b) << std::endl;
}

TEST_CASE("test_strtod")
{
  std::string text_lowest = std::to_string(std::numeric_limits<double>::lowest());
  double result_lowest = std::strtod(text_lowest.c_str(), nullptr);
  CHECK(errno != ERANGE);

  std::string text_min = std::to_string(std::numeric_limits<double>::min());
  std::cout << "text_min = " << text_min << std::endl;
  double result_min = std::strtod(text_min.c_str(), nullptr);
  CHECK(errno != ERANGE);

  std::string text_max = std::to_string(std::numeric_limits<double>::max());
  double result_max = std::strtod(text_max.c_str(), nullptr);
  CHECK(errno != ERANGE);

  // N.B. Reading std::numeric_limits<double>::min() may lead to an ERANGE error value
  std::string text = "2.22507e-308";
  std::cout << "min = " << std::numeric_limits<double>::min() << std::endl;
  std::cout << "min = " << std::setprecision(20) << std::numeric_limits<double>::min() << std::endl;
  std::cout << "text = " << text << std::endl;
  double result = std::strtod(text.c_str(), nullptr);
  CHECK(errno == ERANGE);
}
