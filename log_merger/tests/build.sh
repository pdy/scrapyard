#!/bin/sh

cd ../
make clean
make debug
cd -
cp ../bin/debug/log_merger_2 .
cp ../bin/debug/log_merger_3 .
cp ../bin/debug/log_merger_5 .
