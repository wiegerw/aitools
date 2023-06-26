#!/bin/bash

print_header() {
  local title="$1"
  local line="================================================================================"
  local padding="                                                                               "
  local title_length=${#title}
  local padding_length=$(( 74 - title_length ))
  local left_padding_length=$(( padding_length / 2 ))
  local left_padding="${padding:0:left_padding_length}"
  local right_padding_length=$(( 74 - title_length - left_padding_length ))
  local right_padding="${padding:0:right_padding_length}"
  echo "$line"
  echo "===${left_padding}${title}${right_padding}==="
  echo "$line"
}

# Convert the .csv files into .data files
python3 preprocess.py

# The location of the tools. Change this if necessary.
tooldir=../tools/dist

for data_file in output/*.data ; do
    name=${data_file%.*}
    print_header $name
    random_forest_file="${name}.rf"

    # create random forest
    $tooldir/learnrf --verbose \
                     --forest-size=100 \
                     --min-samples-leaf=5 \
                     --max-depth=1000 \
                     --impurity-measure="gini" \
                     --sample-fraction=1 \
                     --sample-technique=stratified \
                     --execution-mode=parallel \
                     --seed=123456 \
                     --split-family=threshold-subset \
                     ${data_file} ${random_forest_file}
    echo ""

    # create probabilistic circuit / generative forest
    generative_forest_file="${name}.gef"
    $tooldir/buildgef -v ${random_forest_file} ${data_file} ${generative_forest_file}
    echo ""

    # draw 100000 samples of the probabilistic circuit
    sample_file="${name}.samples"
    $tooldir/samplepc -v --count=100000 ${generative_forest_file} ${sample_file}
    echo ""

    # print some information about the data sets + samples
    $tooldir/datasetinfo ${data_file}
    echo ""
    $tooldir/datasetinfo ${sample_file}
    echo ""
done
