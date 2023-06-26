// Author(s): Maurice Laveaux
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/stack_array.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_STACK_ARRAY_H
#define AITOOLS_UTILITIES_STACK_ARRAY_H

/// \brief Define macros to conditionally enable platform specific code.
#ifdef _WIN32
#define AITOOLS_PLATFORM_WINDOWS 1
#endif

#ifdef __linux
#define AITOOLS_PLATFORM_LINUX 1
#endif

#ifdef __FreeBSD__
#define AITOOLS_PLATFORM_FREEBSD 1
#endif

#ifdef __APPLE__
#define AITOOLS_PLATFORM_MAC 1
#endif

// Reserve a local array of type TYPE and SIZE elements (where SIZE
// is not necessarily a constant value). These will be allocated on the stack,
// but the memory will not be initialised, nor will a destructor be called
// on these memory positions when the reserved data is freed.

#ifdef AITOOLS_PLATFORM_WINDOWS
#include <malloc.h>
#define AITOOLS_SPECIFIC_STACK_ALLOCATOR(TYPE, SIZE) reinterpret_cast<TYPE*>(_alloca((SIZE)*sizeof(TYPE)))
#elif (AITOOLS_PLATFORM_LINUX || AITOOLS_PLATFORM_MAC)

#include <alloca.h>

#define AITOOLS_SPECIFIC_STACK_ALLOCATOR(TYPE, SIZE) reinterpret_cast<TYPE*>(alloca((SIZE)*sizeof(TYPE)))
#elif AITOOLS_PLATFORM_FREEBSD
#include <cstdlib>
#define AITOOLS_UTILITIES_SPECIFIC_STACK_ALLOCATOR(TYPE, SIZE) reinterpret_cast<TYPE*>(alloca((SIZE)*sizeof(TYPE)))
#else
static_assert(false, "AITOOLS_SPECIFICATION_STACK_ALLOCATOR has not yet been defined for your platform.");
#endif

#include <cstddef>
#include <iterator>

namespace aitools {

/// \brief Inherit from this class to prevent it from being copyable.
/// \details Declares the copy (assignment) constructors as deleted.
class noncopyable
{
  public:
    noncopyable() = default;

    noncopyable(const noncopyable&) = delete;

    noncopyable& operator=(const noncopyable&) = delete;
};

/// \brief Provides (a subset of) the interface of std::array<T> for a portion of preallocated memory. Can be used to
///        interface with a portion of memory allocated on the stack, \see AITOOLS_DECLARE_STACK_ARRAY. The advantage over
///        AITOOLS_SPECIFIC_STACK_ALLOCATOR is that the lifetime of the underlying objects is bounded by the lifetime of the
///        stack_array.
template<typename T>
class stack_array : public noncopyable
{
  public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using value_type = T;

    /// \brief The given pointer should be able to hold N element of sizeof(T) bytes.
    stack_array<T>(T* reserved_memory, std::size_t N)
      : m_reserved_memory(reserved_memory),
        m_size(N)
    {
      for (T& element : *this)
      {
        new(&element) T();
      }
    }

    ~stack_array()
    {
      for (T& element : *this)
      {
        element.~T();
      }
    }

    // The remaining interface of std::array

    iterator begin()
    {
      return data();
    }

    const_iterator begin() const
    {
      return data();
    }

    iterator end()
    {
      return data() + size();
    }

    const_iterator end() const
    {
      return data() + size();
    }

    T* data()
    {
      return m_reserved_memory;
    }

    const T* data() const
    {
      return m_reserved_memory;
    }

    bool empty() const
    {
      return size() != 0;
    }

    reverse_iterator rbegin()
    {
      return reverse_iterator(data() + size());
    }

    const_reverse_iterator rbegin() const
    {
      return const_reverse_iterator(data() + size());
    }

    reverse_iterator rend()
    {
      return reverse_iterator(data());
    }

    const_reverse_iterator rend() const
    {
      return const_reverse_iterator(data());
    }

    std::size_t size() const
    {
      return m_size;
    }

    std::size_t max_size() const
    {
      return m_size;
    }

    T& operator[](std::size_t index)
    {
      assert(index < size());
      return m_reserved_memory[index];
    }

    const T& operator[](std::size_t index) const
    {
      assert(index < size());
      return m_reserved_memory[index];
    }

    const T& front() const
    {
      return m_reserved_memory[0];
    }

    T& front()
    {
      return m_reserved_memory[0];
    }

  private:
    T* m_reserved_memory;
    std::size_t m_size;
};

} // namespace aitools

/// \brief Define a (hopefully) unique name for the underlying reserved stack memory.
#define AITOOLS_STACK_ARRAY_NAME(NAME) \
  NAME ## _reserved_stack_memory

/// \brief Declares a stack_array<TYPE> with the specified NAME that stores SIZE elements type TYPE.
#define AITOOLS_DECLARE_STACK_ARRAY(NAME, TYPE, SIZE) \
  TYPE* AITOOLS_STACK_ARRAY_NAME(NAME) = AITOOLS_SPECIFIC_STACK_ALLOCATOR(TYPE, SIZE); \
  aitools::stack_array<TYPE> NAME (AITOOLS_STACK_ARRAY_NAME(NAME), SIZE)

#endif // AITOOLS_UTILITIES_STACK_ARRAY_H
