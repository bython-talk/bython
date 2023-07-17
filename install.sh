rm -rf build/
rm CMakeCache.txt
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=./
make
make install
make clean