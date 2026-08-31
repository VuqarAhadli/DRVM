#!/bin/bash

set -e

if [ ! -d "build" ]; then
    mkdir build
fi


cd ./build
echo "Building project..."
cmake ..
make 

echo ""
echo "Starting Benchmark"
echo "=&$@#$%^&!@#$%(=&$@#$%^&!@#$%("
echo ""

echo "#&=*/!  DRVM - 50 runs  #&=*/!"
/usr/bin/time -l sh -c 'for i in {1..50}; do ./drvm ../extracted/i.class > /dev/null; done'

echo ""
echo "#&=*/!  JAVAP - 50 runs  #&=*/!"
/usr/bin/time -l sh -c 'for i in {1..50}; do javap -v ../extracted/i.class > /dev/null; done'

echo ""
echo "=&$@#$%^&!@#$%(=&$@#$%^&!@#$%("
echo "Done!"