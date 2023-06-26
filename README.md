# The aitools library
The `aitools` library is a C++ library that contains some basic AI algorithms and data structures.

### History
The library was originally intended to become a fast C++ equivalent of the
[GeFS](https://github.com/AlCorreia/GeFs) Python library of Alvaro Correia. However,
it has never been used for this purpose. Since the library itself may be useful for
others, I have decided to make it open source.

## Contents
The `aitools` library currently supports binary decision trees, random forests and
probabilistic circuits. The main selling point of the `aitools` library is that the code
is based on precise mathematical specifications. This leads to clean, simple and efficient
code that is easy to generalize. The functionality that is offered is limited, so the
library may not be suitable as a drop-in for existing frameworks.

### binary decision trees
The binary decision tree implementation in `aitools` is both generic and fast. The vertex class is
very lightweight: it consists of an index range in the dataset, two references to child
nodes, and a generic splitter that is implemented using C++ variants.
Note that the decision tree generation of `aitools` is several orders of magnitude faster
than the Python/NumPy equivalent in the [GeFS](https://github.com/AlCorreia/GeFs) library.
Random forests have been implemented as simple containers of binary decision trees.

### probabilistic circuits
A straightforward implementation of probabilistic circuits is included. It stores
probabilistic circuits using a pointer structure, such that each node is allocated on the
heap. This may not scale well for really large circuits.

The probabilistic circuits come
with sum nodes, product nodes and terminal nodes with either a categorical, normal or
truncated normal distribution. Apart from that _sum split nodes_ are supported. These
are introduced to support the embedding of decision trees in probabilistic circuits,
as described in [Joints in Random Forests](https://proceedings.neurips.cc/paper/2020/file/8396b14c5dff55d13eea57487bf8ed26-Paper.pdf).
An algorithm for this transformation is included in the repository.

### input/output
All data structures can be stored to and loaded from disk using a simple
line based textual format. The parsing code is reasonably fast, but can be
improved by using manual parsers instead of regular expressions.
Of course for really large examples binary I/O needs to be added.
Datasets are stored in the simple format below:

```
dataset: 1.0
category_counts: 0 12 4 8 3 3 3 2 10 5 0 0 2 0 3 0 0 0 0 0 2
features: age;job;marital;education;default;housing;loan;contact;month;day_of_week;duration;campaign;pdays;previous;poutcome;emp.var.rate;cons.price.idx;cons.conf.idx;euribor3m;nr.employed;y
56 0 0 0 0 0 0 0 0 0 261 1 0 0 0 1.1 93.994 -36.4 4.857 5191 0
57 1 0 1 1 0 0 0 0 0 149 1 0 0 0 1.1 93.994 -36.4 4.857 5191 0
37 1 0 1 0 1 0 0 0 0 226 1 0 0 0 1.1 93.994 -36.4 4.857 5191 0
...
```

The first line is a header. The second line contains the category counts
of the features, where 0 corresponds with a continuous domain.
The third line contains a `;`-separated list of feature names.
Every subsequent line contains the numerical values of an example.

### command line tools
A number of command line tools is included.

* `buildgef` build a generative forest from a random forest
* `learndt` learn a binary decision tree from a dataset
* `learnrf` learn a random forest from a dataset
* `makedataset` generate a dataset with given distributions for the features
* `pc` contains some algorithms operating on probabilistic circuits
* `samplepc` generate samples of a probabilistic circuit

## Documentation
A detailed specification of the implementation can be found in [aitools.pdf](https://github.com/wiegerw/aitools/blob/main/doc/aitools.pdf).
Note that this is not a tutorial, but rather a precise description of the algorithms
that were implemented.

## Requirements
A C++17 compiler. Compilation has been tested with g++-12 and clang-14.

## Build
The following build systems are supported
* CMake 3.16+
* B2 (https://www.bfgroup.xyz/b2/)

## Dependencies
The `aitools` library uses the following third-party libraries. They are included in the repository.

* Boost (http://boost.org), already included in the repository
* fmt (https://github.com/fmtlib/fmt), already included in the repository
* Lyra (https://github.com/bfgroup/Lyra), already included in the repository
* Eigen (https://eigen.tuxfamily.org/)
* pybind11 (https://github.com/pybind/pybind11)


### Parallel STL
The `aitools` library uses parallel STL to parallelize some computations.
When using g++ or clang compilers, this may require installation of Intel's TBB library.

## Python bindings
The `aitools` library comes with python bindings. For this the package pybind11 is needed, see
https://github.com/pybind/pybind11. On Ubuntu pybind11 can be installed using

```
sudo apt-get install pybind11-dev
```

The python bindings can be installed using

```
pip install .
```
