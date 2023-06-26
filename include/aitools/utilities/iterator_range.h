// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/utilities/iterator_range.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_ITERATOR_RANGE_H
#define AITOOLS_UTILITIES_ITERATOR_RANGE_H

#include <cstdlib>
#include <iterator>

namespace aitools::utilities {

/// \brief Hand-crafted iterator range.
template <typename Iterator>
class iterator_range
{
  protected:
    Iterator m_begin;
    Iterator m_end;

  public:
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;

    iterator_range() = default;

    iterator_range(Iterator begin, Iterator end)
    : m_begin(begin), m_end(end)
    {}

    template <typename Container>
    iterator_range(Container& x)
    : m_begin(std::begin(x)), m_end(std::end(x))
    {}

    template <typename Container>
    iterator_range(const Container& x)
    : m_begin(std::begin(x)), m_end(std::end(x))
    {}

    Iterator begin()
    {
      return m_begin;
    }

    Iterator begin() const
    {
      return m_begin;
    }

    Iterator end()
    {
      return m_end;
    }

    Iterator end() const
    {
      return m_end;
    }

    const reference front() const
    {
      assert(!empty());
      return *m_begin;
    }

    reference front()
    {
      assert(!empty());
      return *m_begin;
    }

    const reference back() const
    {
      assert(!empty());
      return *std::prev(m_end);
    }

    reference back()
    {
      assert(!empty());
      return *std::prev(m_end);
    }

    std::size_t size() const
    {
      return std::distance(m_begin, m_end);
    }

    bool empty() const
    {
      return m_begin == m_end;
    }
};

template <typename Iterator>
iterator_range<Iterator> make_iterator_range(Iterator first, Iterator last)
{
  return iterator_range<Iterator>(first, last);
}

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_ITERATOR_RANGE_H
