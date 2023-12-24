FROM debian:trixie-slim as llvm16
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /
RUN apt-get -q update

RUN apt-get -q -y install --no-install-recommends \
    build-essential wget gpg software-properties-common gnupg \
    libedit-dev libzstd-dev zlib1g-dev libcurl4-openssl-dev libxml2-dev

RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x /llvm.sh
RUN /llvm.sh 16 && apt-get clean all

RUN update-alternatives \
    --install /usr/bin/clang          clang         /usr/bin/clang-16 160 \
    --slave   /usr/bin/clang++        clang++       /usr/bin/clang++-16 \
    --slave   /usr/bin/clang-format   clang-format  /usr/bin/clang-format-16 \
    --slave   /usr/bin/lldb           lldb          /usr/bin/lldb-16  \
    --slave   /usr/bin/FileCheck      FileCheck     /usr/bin/FileCheck-16

FROM llvm16 AS llvm16-cmake-deps
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -q -y install --no-install-recommends gcc unzip cmake make pkg-config libboost-dev


### Catch2
RUN wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.3.2.zip -O /catch2.zip && unzip /catch2.zip -d /catch2
ARG CATCH2_BUILD_DIR=/catch2/Catch2-3.3.2/build

RUN cmake -DCMAKE_BUILD_TYPE=Release -S /catch2/Catch2-3.3.2 -B ${CATCH2_BUILD_DIR}
RUN cmake --build ${CATCH2_BUILD_DIR} -j4 && cmake --install ${CATCH2_BUILD_DIR} --config Release
RUN rm -r /catch2 /catch2.zip


### lexy
RUN wget https://github.com/foonathan/lexy/archive/refs/heads/main.zip -O /lexy.zip && unzip /lexy.zip -d /lexy
ARG LEXY_BUILD_DIR=/lexy/lexy-main/build

RUN cmake -DCMAKE_BUILD_TYPE=Release \
    -DLEXY_BUILD_BENCHMARKS=OFF \
    -DLEXY_BUILD_EXAMPLES=OFF \
    -DLEXY_BUILD_TESTS=OFF \
    -DLEXY_BUILD_DOCS=OFF \
    -DLEXY_BUILD_PACKAGE=OFF \
    -DLEXY_ENABLE_INSTALL=ON \ 
    -S /lexy/lexy-main -B ${LEXY_BUILD_DIR}
RUN cmake --build ${LEXY_BUILD_DIR} -j4 && cmake --install ${LEXY_BUILD_DIR} --config RELEASE
RUN rm -r /lexy /lexy.zip

FROM llvm16-cmake-deps AS bython-base

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends pipx && apt-get clean all

RUN pipx install lit
ENV PATH="${PATH}:/root/.local/bin"

ENTRYPOINT [ "/bin/bash" ]