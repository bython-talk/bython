FROM ubuntu:22.04

# Get bare minimum dependencies for llvm.sh
RUN apt-get update && apt-get install -y lsb-release wget software-properties-common gnupg

# Fetch llvm.sh and install all necessary tools
RUN wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 16 all

COPY update-alternatives-clang.sh /update-alternatives-clang.sh
RUN chmod +x /update-alternatives-clang.sh && ./update-alternatives-clang.sh 16 160

RUN apt-get install --no-install-recommends -y cmake ninja-build make cppcheck gdb git ssh
