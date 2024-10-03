// Software License for MTL
// 
// Copyright (c) 2007 The Trustees of Indiana University.
//               2008 Dresden University of Technology and the Trustees of Indiana University.
//               2010 SimuNova UG (haftungsbeschränkt), www.simunova.com.
// All rights reserved.
// Authors: Peter Gottschling and Andrew Lumsdaine
// 
// This file is part of the Matrix Template Library
// 
// See also license.mtl.txt in the distribution.

#ifndef MTL_RAW_COPY_INCLUDE
#define MTL_RAW_COPY_INCLUDE
#include <boost/numeric/mtl/interface/vpt.hpp>

namespace mtl {

// Copies range of values into elements
// As name says it is a raw operation and to used with uttermost care
template <typename Matrix, typename InputIterator> 
void raw_copy(InputIterator first, InputIterator last, Matrix& matrix)
{
    using std::copy;
    vampir_trace<2> tracer;
    copy(first, last, matrix.elements());
}

// Temporary solution
// will be replaced by sequences and cursors generated by begin<all>(ma) and end<all>(ma)
// Using segmented cursors, matrices with non-contigous element storing can be handled

} // namespace mtl

#endif // MTL_RAW_COPY_INCLUDE