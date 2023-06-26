// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/numerics/matrix.h
/// \brief add your file description here.

#ifndef AITOOLS_NUMERICS_MATRIX_H
#define AITOOLS_NUMERICS_MATRIX_H

#include <iostream>
#include <vector>
#include "aitools/utilities/print.h"
#include "aitools/utilities/random.h"

namespace aitools::numerics {

template <typename NumberType = double>
class vector
{
  private:
    std::vector<NumberType> m_elements;

  public:
    explicit vector(std::size_t size)
      : m_elements(size)
    {}

    NumberType& operator()(std::size_t j)
    {
      return m_elements[j];
    }

    NumberType operator()(std::size_t j) const
    {
      return m_elements[j];
    }

    const NumberType& operator[](std::size_t j) const
    {
      return m_elements[j];
    }

    NumberType& operator[](std::size_t j)
    {
      return m_elements[j];
    }

    std::size_t size() const
    {
      return m_elements.size();
    }

    std::size_t row_count() const
    {
      return 1;
    }

    std::size_t column_count() const
    {
      return m_elements.size();
    }

    auto begin()
    {
      return m_elements.begin();
    }

    auto begin() const
    {
      return m_elements.begin();
    }

    auto end()
    {
      return m_elements.end();
    }

    auto end() const
    {
      return m_elements.end();
    }
};

template <typename NumberType>
std::ostream& operator<<(std::ostream& out, const vector<NumberType>& x)
{
  return out << print_list(x);
}

template <typename NumberType>
std::size_t row_count(const vector<NumberType>& m)
{
  return m.row_count();
}

template <typename NumberType>
std::size_t column_count(const vector<NumberType>& m)
{
  return m.column_count();
}

/// \brief A very simple 2D matrix type
template <typename NumberType = double>
class matrix
{
  private:
    std::vector<std::vector<NumberType>> m_rows;
    std::size_t m_column_count{};

  public:
    matrix() = default;

    class column_type
    {
      private:
        const std::vector<std::vector<NumberType>>& rows;
        std::size_t j;

      public:
        explicit column_type(const std::vector<std::vector<NumberType>>& rows_, std::size_t j_)
         : rows(rows_), j(j_)
        {}

        NumberType& operator()(std::size_t i)
        {
          return rows[i][j];
        }

        NumberType operator()(std::size_t i) const
        {
          return rows[i][j];
        }

        NumberType& operator[](std::size_t i)
        {
          return rows[i][j];
        }

        NumberType operator[](std::size_t i) const
        {
          return rows[i][j];
        }

        std::size_t size() const
        {
          return rows.size();
        }
    };

    matrix(std::size_t rows, std::size_t columns)
      : m_rows(rows, std::vector<NumberType>(columns, NumberType())), m_column_count(columns)
    {}

    explicit matrix(const std::vector<std::vector<NumberType>>& rows) // TODO: add move constructor
     : m_rows(rows), m_column_count(rows.front().size())
    {}

    NumberType& operator()(std::size_t i, std::size_t j)
    {
      return m_rows[i][j];
    }

    NumberType operator()(std::size_t i, std::size_t j) const
    {
      return m_rows[i][j];
    }

    const std::vector<NumberType>& operator[](std::size_t i) const
    {
      return m_rows[i];
    }

    std::vector<NumberType>& operator[](std::size_t i)
    {
      return m_rows[i];
    }

    std::size_t row_count() const
    {
      return m_rows.size();
    }

    std::size_t column_count() const
    {
      return m_column_count;
    }

    std::size_t size() const
    {
      return m_rows.size();
    }

    column_type column(std::size_t j) const
    {
      return column_type(m_rows, j);
    }

    void add(std::vector<NumberType> row)
    {
      assert(row.size() == m_column_count);
      m_rows.push_back(std::move(row));
    }

    auto begin()
    {
      return m_rows.begin();
    }

    auto begin() const
    {
      return m_rows.begin();
    }

    auto end()
    {
      return m_rows.end();
    }

    auto end() const
    {
      return m_rows.end();
    }

    bool operator==(const matrix<NumberType>& other) const
    {
      return m_rows == other.m_rows;
    }

    bool operator!=(const matrix<NumberType>& other) const
    {
      return !(*this == other);
    }
};

template <typename NumberType>
std::ostream& operator<<(std::ostream& out, const matrix<NumberType>& x)
{
  return out << print_matrix(x);
}

template <typename NumberType>
std::size_t row_count(const matrix<NumberType>& m)
{
  return m.row_count();
}

template <typename NumberType>
std::size_t column_count(const matrix<NumberType>& m)
{
  return m.column_count();
}

} // namespace aitools

#endif // AITOOLS_NUMERICS_MATRIX_H
