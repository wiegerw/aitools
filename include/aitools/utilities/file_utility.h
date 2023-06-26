// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/file_utility.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_FILE_UTILITY_H
#define AITOOLS_UTILITIES_FILE_UTILITY_H

#include <filesystem>
#include <stdexcept>
#include <string>

namespace aitools::utilities {

inline
std::string remove_extension(const std::string& filename)
{
  std::string extension = std::filesystem::path(filename).extension().string();
  return filename.substr(0, filename.size() - extension.size());
}

inline
void check_path_exists(const std::string& filename)
{
  if (!std::filesystem::exists(std::filesystem::path(filename)))
  {
    throw std::runtime_error("The file " + filename + " does not exist");
  }
}

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_FILE_UTILITY_H
