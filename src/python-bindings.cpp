// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file src/python-bindings.cpp
/// \brief add your file description here.

#include "aitools/datasets/io.h"
#include "aitools/decision_trees/decision_tree_options.h"
#include "aitools/probabilistic_circuits/generative_forest.h"
#include "aitools/probabilistic_circuits/probabilistic_circuit.h"
#include "aitools/random_forests/learning.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <sstream>

namespace py = pybind11;
using namespace aitools;

template <typename T>
std::string print(const T& t)
{
  std::ostringstream out;
  out << t;
  return out.str();
}

random_forest learn_rf(const aitools::dataset& D,
                       const std::vector<std::uint32_t>& I,
                       aitools::random_forest_options forest_options,
                       const aitools::decision_tree_options& tree_options,
                       const std::string& split_family,
                       bool sequential,
                       std::size_t seed)
{
  using namespace aitools;

  if (split_family == "threshold")
  {
    return aitools::learn_random_forest(D, I, forest_options, tree_options, threshold_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, sequential, seed);
  }
  else if (split_family == "threshold-single")
  {
    return aitools::learn_random_forest(D, I, forest_options, tree_options, threshold_plus_single_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, sequential, seed);
  }
  else if (split_family == "threshold-subset")
  {
    return aitools::learn_random_forest(D, I, forest_options, tree_options, threshold_plus_subset_split_family(D, tree_options), gain1(tree_options.imp_measure), node_is_finished, sequential, seed);
  }
  else
  {
    throw std::runtime_error("unknown split family " + split_family);
  }
}

PYBIND11_MODULE(aitools, m)
{
  m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: python_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

  py::class_<numerics::matrix<double>, std::shared_ptr<numerics::matrix<double>>>(m, "DoubleMatrix")
    .def(py::init<const std::vector<std::vector<double>>&>(), py::return_value_policy::copy)
    .def("__getitem__", [](numerics::matrix<double>& A, unsigned int i) { return A[i]; })
    ;

  py::class_<dataset, std::shared_ptr<dataset>>(m, "DataSet1")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("has_missing_values", [](const dataset& D) { return D.has_missing_values(); })
    .def("feature_count", [](const dataset& D) { return D.feature_count(); })
    .def("row_count", [](const dataset& D) { return D.X().row_count(); })
    .def("load", [](dataset& D, const std::string& filename) { D = load_dataset(filename); })
    .def("save", [](const dataset& D, const std::string& filename) { save_dataset(filename, D); })
    .def_property("X",
                  [](const dataset& D) { return D.X(); },
                  [](dataset& D, const numerics::matrix<double>& X) { D.X() = X; }
    )
    .def_property("category_counts",
                  [](const dataset& D) { return D.category_counts(); },
                  [](dataset& D, const std::vector<unsigned int>& ncat) { D.category_counts() = ncat; }
    )
    .def_property("features",
                  [](const dataset& D) { return D.features(); },
                  [](dataset& D, const std::vector<std::string>& features) { D.features() = features; }
                 )
    .def("__str__", [](const dataset& D) { return print(D); })
    .def("__eq__", [](const dataset& D1, const dataset& D2) { return D1 == D2; })
  ;

  py::class_<decision_tree_options, std::shared_ptr<decision_tree_options>>(m, "DecisionTreeOptions")
    .def(py::init<>(), py::return_value_policy::copy)
    .def_readwrite("min_samples_leaf", &decision_tree_options::min_samples_leaf)
    .def_readwrite("max_features", &decision_tree_options::max_features)
    .def_readwrite("max_depth", &decision_tree_options::max_depth)
    .def_readwrite("min_samples_leaf", &decision_tree_options::min_samples_leaf)
    .def_readwrite("max_categorical_size", &decision_tree_options::max_categorical_size)
    .def_readwrite("imp_measure", &decision_tree_options::imp_measure)
    .def_readwrite("support_missing_values", &decision_tree_options::support_missing_values)
    .def("__str__", [](const decision_tree_options& options) { return print(options); })
  ;

  py::class_<random_forest_options, std::shared_ptr<random_forest_options>>(m, "RandomForestOptions")
    .def(py::init<>(), py::return_value_policy::copy)
    .def_readwrite("forest_size", &random_forest_options::forest_size)
    .def_readwrite("sample_fraction", &random_forest_options::sample_fraction)
    .def_readwrite("sample_technique", &random_forest_options::sample_criterion)
  ;

  py::class_<pc_node, pc_node_ptr>(m, "PCNode")
    .def_property_readonly("is_leaf", &pc_node::is_leaf)
    .def_property_readonly("successors", static_cast<const std::vector<pc_node_ptr>& (pc_node::*)() const>(&pc_node::successors))
    ;

  py::class_<sum_node, pc_node, std::shared_ptr<sum_node>>(m, "SumNode")
    .def(py::init<std::vector<double>>(), py::return_value_policy::copy)
    .def_property_readonly("weights", static_cast<const std::vector<double>& (sum_node::*)() const>(&sum_node::weights))
    .def("evi", &sum_node::evi)
    .def("log_evi", &sum_node::log_evi)
    ;

  py::class_<product_node, pc_node, std::shared_ptr<product_node>>(m, "ProductNode")
    .def(py::init<>(), py::return_value_policy::copy)
    .def("evi", &product_node::evi)
    .def("log_evi", &product_node::log_evi)
    ;

  py::class_<normal_node, pc_node, std::shared_ptr<normal_node>>(m, "NormalNode")
    .def(py::init<int, double, double>(), py::return_value_policy::copy)
    .def_property_readonly("mean", &normal_node::mean)
    .def_property_readonly("std", &normal_node::standard_deviation)
    .def("evi", &normal_node::evi)
    .def("log_evi", &normal_node::log_evi)
    ;

  py::class_<truncated_normal_node, pc_node, std::shared_ptr<truncated_normal_node>>(m, "TruncatedNormalNode")
    .def(py::init<int, double, double>(), py::return_value_policy::copy)
    .def_property_readonly("mean", &truncated_normal_node::mean)
    .def_property_readonly("std", &truncated_normal_node::standard_deviation)
    .def_property_readonly("a", &truncated_normal_node::a)
    .def_property_readonly("b", &truncated_normal_node::b)
    .def("evi", &truncated_normal_node::evi)
    .def("log_evi", &truncated_normal_node::log_evi)
    ;

  py::class_<categorical_node, pc_node, std::shared_ptr<categorical_node>>(m, "MultinomialNode")
    .def(py::init<int, std::vector<double>>(), py::return_value_policy::copy)
    .def_property_readonly("params", &categorical_node::probabilities)
    .def_property_readonly("scope", &categorical_node::scope)
    .def("evi", &categorical_node::evi)
    .def("log_evi", &categorical_node::log_evi)
    ;

  py::class_<less_node, pc_node, std::shared_ptr<less_node>>(m, "LessNode")
    .def(py::init<int, double>(), py::return_value_policy::copy)
    .def("evi", &less_node::evi)
    .def("log_evi", &less_node::log_evi)
    ;

  py::class_<greater_equal_node, pc_node, std::shared_ptr<greater_equal_node>>(m, "GreaterEqualNode")
    .def(py::init<int, double>(), py::return_value_policy::copy)
    .def("evi", &greater_equal_node::evi)
    .def("log_evi", &greater_equal_node::log_evi)
    ;

  py::class_<equal_node, pc_node, std::shared_ptr<equal_node>>(m, "EqualNode")
    .def(py::init<int, double>(), py::return_value_policy::copy)
    .def("evi", &equal_node::evi)
    .def("log_evi", &equal_node::log_evi)
    ;

  py::class_<not_equal_node, pc_node, std::shared_ptr<not_equal_node>>(m, "NotEqualNode")
    .def(py::init<int, double>(), py::return_value_policy::copy)
    .def("evi", &not_equal_node::evi)
    .def("log_evi", &not_equal_node::log_evi)
    ;

  py::class_<subset_node, pc_node, std::shared_ptr<subset_node>>(m, "SubsetNode")
    .def(py::init<int, int>(), py::return_value_policy::copy)
    .def("evi", &subset_node::evi)
    .def("log_evi", &subset_node::log_evi)
    ;

  py::class_<binary_decision_tree, std::shared_ptr<binary_decision_tree>>(m, "DecisionTree")
    .def(py::init<>(), py::return_value_policy::copy)
    .def(py::init<const dataset&, std::vector<std::uint32_t>>())
  ;

  py::class_<random_forest, std::shared_ptr<random_forest>>(m, "RandomForest")
    .def(py::init<>(), py::return_value_policy::copy)
    .def(py::init<std::vector<binary_decision_tree>>())
    .def("trees", [](const random_forest& rf) { return rf.trees(); })
  ;

  py::class_<probabilistic_circuit, std::shared_ptr<probabilistic_circuit>>(m, "ProbabilisticCircuit")
    .def(py::init<pc_node_ptr, std::vector<unsigned int>>())
    ;

  py::enum_<impurity_measure>(m, "ImpurityMeasure", "Criterion to measure the quality of a split")
    .value("gini", impurity_measure::gini, "Gini impurity")
    .value("entropy", impurity_measure::entropy, "Entropy")
    .value("mis_classification", impurity_measure::mis_classification, "Misclassification")
    ;

  py::enum_<sample_technique>(m, "SampleTechnique", "Techniques for selecting samples of a dataset")
    .value("without_replacement", sample_technique::without_replacement, "the selected samples are unique")
    .value("with_replacement", sample_technique::with_replacement, "the selected samples may contain duplicates")
    .value("stratified", sample_technique::stratified, "the samples are partitioned according to their class. For each partition approximately the same fraction of samples is selected, and duplicates are possible")
    ;

  m.def("learn_random_forest", &learn_rf);
  m.def("parse_impurity_measure", &parse_impurity_measure);
  m.def("parse_sample_technique", &parse_sample_technique);
  m.def("build_generative_forest", &build_generative_forest);

  py::class_<std::mt19937>(m, "RandomNumberGenerator")
    .def(py::init<std::uint32_t>(), py::return_value_policy::copy)
    ;

  m.attr("__version__") = "0.1.0";
}
