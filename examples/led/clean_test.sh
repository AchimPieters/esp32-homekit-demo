#!/bin/bash

echo "cleaning..."
rm -rf ./build
rm -rf ./managed_components
rm  -f ./sdkconfig
rm  -f ./dependencies.lock

echo "building..."
idf.py build
