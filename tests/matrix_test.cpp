// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file probability_test.cpp
/// \brief Tests for probability distributions.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstdio>
#include <random>
#include "aitools/datasets/dataset.h"
#include "aitools/datasets/io.h"
#include "aitools/numerics/csv.h"
#include "aitools/numerics/matrix.h"
#include "aitools/utilities/random.h"

inline
aitools::numerics::matrix<int> random_int_matrix(std::size_t rows, std::size_t columns, int low = 0, int high = 100)
{
  aitools::numerics::matrix<int> result(rows, columns);
  std::mt19937 mt{std::random_device{}()};
  auto random_int = [&low, &high, &mt]() { return aitools::random_integer(low, high, mt); };
  for (std::size_t i = 0; i < rows; i++)
  {
    for (std::size_t j = 0; j < columns; j++)
    {
      result[i][j] = random_int();
    }
  }
  return result;
}

inline
aitools::numerics::matrix<double> random_double_matrix(std::size_t rows, std::size_t columns, double low = 0, double high = 100)
{
  aitools::numerics::matrix<double> result(rows, columns);
  std::mt19937 mt{std::random_device{}()};
  auto random_double = [&low, &high, &mt]() { return aitools::random_real(low, high, mt); };
  for (std::size_t i = 0; i < rows; i++)
  {
    for (std::size_t j = 0; j < columns; j++)
    {
      result[i][j] = random_double();
    }
  }
  return result;
}

TEST_CASE("test1")
{
  using namespace aitools;

  numerics::matrix<> M(5, 10);
  CHECK(M(2, 3) == 0.0);
  CHECK(M[2][3] == 0.0);
  M[2][3] = 1;
  M(2, 4) = 2;
  CHECK(M(2, 3) == 1.0);
  CHECK(M[2][4] == 2.0);

  numerics::matrix<int> A = random_int_matrix(2, 3, 0, 10);
  std::cout << "A =\n" << A << std::endl;

  std::string filename = "test1.csv";
  write_matrix_csv(filename, A);
  numerics::matrix<int> B = read_matrix_csv<int>(filename);
  std::cout << "B = " << B << std::endl;
  CHECK_EQ(A, B);
  std::remove(filename.c_str());
}

TEST_CASE("csv_test1")
{
  using namespace aitools;

  std::string text = "0 1 2 3";
  std::stringstream stext(text);
  std::vector<int> v = read_vector_csv<int>(stext);
  std::vector<int> expected = { 0, 1, 2, 3 };
  CHECK(v == expected);
}

TEST_CASE("csv_test2")
{
  using namespace aitools;

  std::string text = "0, 1, 2, 3";
  std::stringstream stext(text);
  std::vector<int> v = read_vector_csv<int>(stext, ',');
  std::vector<int> expected = { 0, 1, 2, 3 };
  CHECK(v == expected);
}

TEST_CASE("datatest_test1")
{
  using namespace aitools;

  std::string text = "0, 1, 2, 3";
  std::stringstream stext(text);
  std::vector<int> v = read_vector_csv<int>(stext, ',');
  std::vector<int> expected = { 0, 1, 2, 3 };
  CHECK(v == expected);
}

// TODO: fix this test case; dataset::is_valid() fails
//TEST_CASE("datatest_test2")
//{
//  using namespace aitools;
//
//  std::string text =
//    "0 1 1 1\n"
//    "1 1 1 0\n"
//    "\n"
//    "2 2 2 2"
//  ;
//
//  dataset D = parse_dataset(text);
//  std::cout << "D = " << D << std::endl;
//}

