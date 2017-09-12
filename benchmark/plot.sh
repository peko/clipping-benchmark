#!/bin/bash
echo CLIPPER
../build/clipper_benchmark | gnuplot -p -e "plot '<cat' using 1:2 title 'clipper' with lines"

echo GPC
../build/gpc_benchmark     | gnuplot -p -e "plot '<cat' using 1:2 title 'gpc'     with lines"
