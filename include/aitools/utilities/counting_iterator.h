// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/utilities/counting_iterator.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_COUNTING_ITERATOR_H
#define AITOOLS_UTILITIES_COUNTING_ITERATOR_H

#include <iterator>
#include <boost/stl_interfaces/iterator_interface.hpp>

namespace aitools::utilities {

template<typename Number = std::size_t>
class counting_iterator : public boost::stl_interfaces::iterator_interface<
  counting_iterator<Number>,
  std::random_access_iterator_tag,
  Number>
{
  private:
    friend boost::stl_interfaces::access;

    Number m_value;

  public:
    counting_iterator() = default;

    counting_iterator(Number value)
      : m_value(value)
    {
    }

    bool operator==(const counting_iterator& other) const noexcept
    {
      return m_value == other.m_value;
    }

    bool operator!=(const counting_iterator& other) const noexcept
    {
      return !(*this == other);
    }

    counting_iterator& operator++()
    {
      ++m_value;
      return *this;
    }

    Number& operator*() noexcept
    {
      return m_value;
    }

    counting_iterator& operator+=(std::ptrdiff_t i) noexcept
    {
      m_value += i;
      return *this;
    }

    auto operator-(const counting_iterator& other) const noexcept
    {
      return m_value - other.m_value;
    }
};

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_COUNTING_ITERATOR_H
