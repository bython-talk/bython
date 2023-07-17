rm -rf build/
rm CMakeCache.txt
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./source
make
make install
make clean