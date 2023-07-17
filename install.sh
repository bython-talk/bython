mkdir build
cd build
rm ../CMakeCache.txt
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=./source
make
make install
make clean