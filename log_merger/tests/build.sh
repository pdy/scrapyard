#!/bin/sh

cd ../
make clean
make debug
cd -
cp ../bin/debug/log_merger_2 .
