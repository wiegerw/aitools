# The aitools library
The `aitools` library is a C++ library that contains some basic AI data structures and algorithms.

### History
The library was originally intended to become a fast C++ equivalent of the
[GeFS](https://github.com/AlCorreia/GeFs) (Generative Forests) Python library. However,
it has never been used for this purpose. Since the library itself may be useful for
others, I have decided to make it open source.

## Contents

### main features
* support for binary decision trees, random forests and probabilistic circuits
* support for generative forests (an embedding of random forests in probabilistic circuits)
* the code is written in modern C++
* the code is based on precise mathematical specifications
* the code is clean, efficient and easy to generalize

### binary decision trees
The binary decision tree implementation in `aitools` is both generic and fast. Contrary to
many existing frameworks, the vertex class is very lightweight: it consists of an index range
in the dataset, two references to child nodes, and a generic splitter that is implemented
using C++ variants.
The decision tree generation of the `aitools` library is many orders of magnitude faster
than the Python/NumPy equivalent in the [GeFS](https://github.com/AlCorreia/GeFs) library.

Three different split criteria are currently supported:
* single split (a split on a single value of a discrete random variable)
* subset split (a split on a subset of values of a discrete random variable)
* threshold split (a split for arbitrary random variables, using a threshold value)

### probabilistic circuits
A straightforward implementation of probabilistic circuits is included. It stores
probabilistic circuits using a pointer structure, such that each node is allocated separately
on the  heap. This design was chosen to make it easy to build probabilistic circuits using the
Python interface. For really large circuits a different memory layout may be preferable.
The following node types are currently supported:
* sum nodes
* product nodes
* sum split nodes (an extension to efficiently encode random forests in probabilistic circuits)
* terminal nodes with a normal distribution
* terminal nodes with a truncated normal distribution
* terminal nodes with a categorical distribution

A number of algorithms on probabilistic circuits is supported:
* (logarithmic) EVI queries
* sampling
* a smoothness check
* a decomposability check
* a normalization check
* elimination of sum split nodes (at the cost of a larger PC)
* transformation of random forests into probabilistic circuits
* a generic breadth first traversal of probabilistic circuits

The generic breadth first traversal should make it fairly easy to add more algorithms. 

### generative forests
In the paper [Joints in Random Forests](https://proceedings.neurips.cc/paper/2020/file/8396b14c5dff55d13eea57487bf8ed26-Paper.pdf)
a method is described to transform random forests into probabilistic circuits. The transformed random forests
are called _generative forests_. The structure of the original random forest is exactly preserved in the
resulting probabilistic circuit. Since a probabilistic circuit models a probability distribution, this
adds new functionalities to a random forest. For example, using the probabilistic circuit it is possible
to generate new samples with approximately the same distribution as the original dataset that was used to
create the random forest.

### input/output
All data structures can be stored to and loaded from disk using a simple
line based textual format. The parsing code is reasonably fast, but can be
improved by using manual parsers instead of regular expressions.
Of course for really large examples binary I/O needs to be added.
Datasets are stored in a simple format:

```
dataset: 1.0
category_counts: 0 7 0 0 0 0 0 0 2
features: date day period nswprice nswdemand vicprice vicdemand transfer class
0 0 0 0.056443 0.439155 0.003467 0.422915 0.414912 0
0 0 0.021277 0.051699 0.415055 0.003467 0.422915 0.414912 0
0 0 0.042553 0.051489 0.385004 0.003467 0.422915 0.414912 0
0 0 0.06383 0.045485 0.314639 0.003467 0.422915 0.414912 0
0 0 0.085106 0.042482 0.251116 0.003467 0.422915 0.414912 1
...
```

The first line is a header. The second line contains the category counts
of the features, where 0 corresponds with a continuous domain.
The third line contains a list of feature names.
Every subsequent line contains the numerical values of an example.

### command line tools
A number of command line tools is included.

* `buildgef` build a generative forest from a random forest
* `datasetinfo` print information about a dataset
* `learndt` learn a binary decision tree from a dataset
* `learnrf` learn a random forest from a dataset
* `makedataset` generate an artificial dataset with given distributions for the features
* `pc` execute algorithms on probabilistic circuits
* `samplepc` generate samples of a probabilistic circuit

## Documentation
A detailed specification of the implementation can be found in [aitools.pdf](https://github.com/wiegerw/aitools/blob/main/doc/aitools.pdf).
Note that this is not a tutorial, but rather a precise description of the algorithms
that were implemented.

## Installation 

### Requirements
A C++17 compiler. Compilation has been tested with g++-12 and clang-14.

### Dependencies
The `aitools` library uses the following third-party libraries. They are included in the repository.

* Boost (http://boost.org), already included in the repository
* fmt (https://github.com/fmtlib/fmt), already included in the repository
* Lyra (https://github.com/bfgroup/Lyra), already included in the repository
* Eigen (https://eigen.tuxfamily.org/)
* pybind11 (https://github.com/pybind/pybind11)
* parallel STL (should be included in the C++ standard library)

#### Parallel STL
The `aitools` library uses parallel STL to parallelize some computations.
When using the g++ or clang compiler, this may require installation of Intel's TBB library.

### Build
The following build systems are supported
* CMake 3.16+
* B2 (https://www.bfgroup.xyz/b2/)

A CMake build on Ubuntu can for example be done using the commands

```
mkdir cmake
cd cmake
cmake .. -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=../install
make -j8
make install
```

A B2 build on Ubuntu can for example be done using
```
cd tools
b2 link=static release -j8
```
where we assume that a suitable default compiler has been configured.

## Experiments

The `data` subdirectory contains a number of example datasets. The bash script
`run_experiments.sh` can be used to run an experiment with the command line tools.
For each .csv file the following operations are performed:

* convert the .csv file into a dataset file with the extension .data
* generate a random forest from the dataset using the tool `learndf`
* transform the random forest into a generative forest using the tool `buildgef`
* generate 100000 samples from the generative forest using the tool `samplepc`
* compare the distributions of the dataset and the samples using the tool `datasetinfo` 

The distributions of the dataset and the generative forests are expected to be
approximately the same. Below the output for one of the datasets is displayed.
One can see that the mean and the standard deviation of all features are relatively
close.

```
================================================================================
===                         output/electricity.data                          ===
================================================================================
input_file = output/electricity.data
number of features 8
number of samples 45312
ncat    [0, 7, 0, 0, 0, 0, 0, 0, 2]
missing [0, 0, 0, 0, 0, 0, 0, 0]
imp_measure = gini
min_samples_leaf = 5
max_features = 2
max_depth = 1000
max_categorical_size = 10
support_missing_values = 0
forest_size = 100
sample_fraction = 1
sample_technique = stratified
execution mode = parallel
variable fraction = 0.3
seed = 123456
elapsed time: 1.58094

Parsing random forest output/electricity.rf
Elapsed time: 0.471078
Reading data file output/electricity.data
Building generative forest
Elapsed time: 0.890955
Saving generative forest to output/electricity.gef
Elapsed time: 8.13897

Loading probabilistic circuit from output/electricity.gef
Drawing 100000 samples from the probabilistic circuit
Saving dataset to output/electricity.samples

dataset: output/electricity.data
number of samples: 45312
number of features: 8
feature 0: ncat = 0 mean = 0.49908 stddev = 0.340304
feature 1: ncat = 7 fractions = [0.143008, 0.143008, 0.143008, 0.143008, 0.143008, 0.143008, 0.141949]
feature 2: ncat = 0 mean = 0.5 stddev = 0.294753
feature 3: ncat = 0 mean = 0.0578683 stddev = 0.0399903
feature 4: ncat = 0 mean = 0.425418 stddev = 0.163321
feature 5: ncat = 0 mean = 0.00346703 stddev = 0.0102129
feature 6: ncat = 0 mean = 0.422915 stddev = 0.120964
feature 7: ncat = 0 mean = 0.500526 stddev = 0.153372
class: ncat = 2 fractions = [0.424545, 0.575455]

dataset: output/electricity.samples
number of samples: 100000
number of features: 8
feature 0: ncat = 0 mean = 0.506153 stddev = 0.32698
feature 1: ncat = 7 fractions = [0.14176, 0.14407, 0.14362, 0.14337, 0.14377, 0.14263, 0.14078]
feature 2: ncat = 0 mean = 0.504278 stddev = 0.290965
feature 3: ncat = 0 mean = 0.0596604 stddev = 0.0431725
feature 4: ncat = 0 mean = 0.426836 stddev = 0.164153
feature 5: ncat = 0 mean = 0.00385123 stddev = 0.0108079
feature 6: ncat = 0 mean = 0.423188 stddev = 0.121483
feature 7: ncat = 0 mean = 0.500347 stddev = 0.152497
class: ncat = 2 fractions = [0.42349, 0.57651]
```

## Python bindings
The `aitools` library comes with python bindings. The python bindings can be installed using

```
pip install .
```

Unfortunately there is no documentation available yet for the Python bindings.