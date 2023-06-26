/// N.B. This code was derived from xtensor(!)
///
/// \file aitools/numerics/csv.h
/// \brief add your file description here.

#ifndef AITOOLS_NUMERICS_CSV_H
#define AITOOLS_NUMERICS_CSV_H

#include <fstream>
#include <string>
#include "aitools/numerics/matrix.h"

namespace aitools {

namespace detail {

template<class T>
inline T lexical_cast(const std::string& cell)
{
  T res;
  std::istringstream iss(cell);
  iss >> res;
  return res;
}

template<>
inline std::string lexical_cast(const std::string& cell)
{
  size_t first = cell.find_first_not_of(' ');
  if (first == std::string::npos)
  {
    return cell;
  }

  size_t last = cell.find_last_not_of(' ');
  return cell.substr(first, last == std::string::npos ? cell.size() : last + 1);
}

template<>
inline float lexical_cast<float>(const std::string& cell)
{
  return std::stof(cell);
}

template<>
inline double lexical_cast<double>(const std::string& cell)
{
  return std::stod(cell);
}

template<>
inline long double lexical_cast<long double>(const std::string& cell)
{
  return std::stold(cell);
}

template<>
inline int lexical_cast<int>(const std::string& cell)
{
  return std::stoi(cell);
}

template<>
inline long lexical_cast<long>(const std::string& cell)
{
  return std::stol(cell);
}

template<>
inline long long lexical_cast<long long>(const std::string& cell)
{
  return std::stoll(cell);
}

template<>
inline unsigned int lexical_cast<unsigned int>(const std::string& cell)
{
  return static_cast<unsigned int>(std::stoul(cell));
}

template<>
inline unsigned long lexical_cast<unsigned long>(const std::string& cell)
{
  return std::stoul(cell);
}

template<>
inline unsigned long long lexical_cast<unsigned long long>(const std::string& cell)
{
  return std::stoull(cell);
}

} // namespace detail

template<typename NumberType = double>
std::vector<NumberType> read_vector_csv(std::istream& row_stream, char delimiter = ' ')
{
  std::vector<NumberType> result;
  std::string cell;
  while (std::getline(row_stream, cell, delimiter))
  {
    result.push_back(detail::lexical_cast<NumberType>(cell));
  }
  return result;
}

template<typename NumberType = double>
std::vector<NumberType> read_vector_csv(const std::string& filename, char delimiter = ' ')
{
  std::ifstream from(filename);
  return read_vector_csv<NumberType>(from);
}

template<typename NumberType = double>
void write_vector_csv(std::ostream& out, const std::vector<NumberType>& v, char delimiter = ' ')
{
  for (std::size_t i = 0; i < v.size(); i++)
  {
    if (i > 0)
    {
      out << delimiter;
    }
    out << v[i];
  }
}

template<typename NumberType = double>
void write_vector_csv(const std::string& filename, const std::vector<NumberType>& v, char delimiter = ' ')
{
  std::ofstream out(filename);
  write_vector_csv(out, v, delimiter);
}

template<typename NumberType = double>
numerics::matrix<NumberType> read_matrix_csv(std::istream& stream,
                                   char delimiter = ' ',
                                   std::size_t skip_rows = 0,
                                   const std::string& comments = "#")
{
  std::vector<std::vector<NumberType>> rows;
  std::string line;

  for (std::size_t i = 0; i < skip_rows; i++)
  {
    std::getline(stream, line);
  }

  while (std::getline(stream, line))
  {
    if (std::equal(comments.begin(), comments.end(), line.begin()))
    {
      continue;
    }
    std::stringstream row_stream(line);
    rows.push_back(read_vector_csv<NumberType>(row_stream, delimiter));
  }

  return numerics::matrix<NumberType>(std::move(rows));
}

template<typename NumberType = double>
numerics::matrix<NumberType> read_matrix_csv(const std::string& filename,
                                   char delimiter = ' ',
                                   const std::size_t skip_rows = 0,
                                   const std::string& comments = "#")
{
  std::ifstream from(filename);
  return read_matrix_csv<NumberType>(from, delimiter, skip_rows, comments);
}

template<typename NumberType = double>
void write_matrix_csv(std::ostream& out, const numerics::matrix<NumberType>& m, char delimiter = ' ')
{
  for (std::size_t i = 0; i < m.row_count(); i++)
  {
    for (std::size_t j = 0; j < m.column_count(); j++)
    {
      if (j > 0)
      {
        out << delimiter;
      }
      out << m[i][j];
    }
    out << '\n';
  }
}

template<typename NumberType = double>
void write_matrix_csv(const std::string& filename, const numerics::matrix<NumberType>& m, char delimiter = ' ')
{
  std::ofstream out(filename);
  write_matrix_csv(out, m, delimiter);
}

} // namespace aitools

#endif // AITOOLS_NUMERICS_CSV_H
