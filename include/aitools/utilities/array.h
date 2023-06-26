// Copyright: Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/utilities/array.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_ARRAY_H
#define AITOOLS_UTILITIES_ARRAY_H

#include <cassert>
#include <algorithm>

namespace aitools::utilities {

template<typename T, unsigned int Size>
class array
{
  protected:
    unsigned int m_size{0};
    T m_data[Size];

  public:
    constexpr array() = default;

    constexpr array(const array<T, Size>& other)
    {
      m_size = other.m_size;
      std::copy(other.begin(), other.end(), begin());
    }

    constexpr array(std::initializer_list<T> data)
    {
      assert(data.size() <= Size);
      std::copy(data.begin(), data.end(), m_data);
    }

    constexpr array& operator=(const array<T, Size>& other)
    {
      if (&other != this)
      {
        m_size = other.m_size;
        std::copy(other.begin(), other.end(), begin());
      }
      return *this;
    }

    const T& front() const
    {
      assert(!empty());
      return m_data[0];
    }

    T& front()
    {
      assert(!empty());
      return m_data[0];
    }

    const T& back() const
    {
      assert(!empty());
      return m_data[m_size - 1];
    }

    T& back()
    {
      assert(!empty());
      return m_data[m_size - 1];
    }

    constexpr void clear()
    {
      m_size = 0;
    }

    constexpr void push_back(const T& elem)
    {
      assert(!full());
      m_data[m_size++] = elem;
    }

    constexpr void pop_back()
    {
      assert(!empty());
      --m_size;
    }

    constexpr void resize(unsigned int size)
    {
      assert(size <= Size);
      m_size = size;
    }

    constexpr bool empty() const
    {
      return m_size == 0;
    }

    constexpr bool full() const
    {
      return m_size == Size;
    }

    constexpr unsigned int size() const
    {
      return m_size;
    }

    constexpr const T& operator[](unsigned int i) const
    {
      assert(i >= 0 && i < m_size);
      return m_data[i];
    }

    constexpr T& operator[](unsigned int i)
    {
      assert(i >= 0 && i < m_size);
      return m_data[i];
    }

    constexpr const T* begin() const
    {
      return &m_data[0];
    }

    constexpr T* begin()
    {
      return &m_data[0];
    }

    constexpr const T* end() const
    {
      return &m_data[m_size];
    }

    constexpr T* end()
    {
      return &m_data[m_size];
    }

    constexpr bool operator==(const array<T, Size>& other) const
    {
      return std::equal(begin(), end(), other.begin());
    }

    constexpr bool operator!=(const array<T, Size>& other) const
    {
      return !(*this == other);
    }
};

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_ARRAY_H
