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


FROM llvm16 AS llvm16-catch3
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q update
RUN apt-get -q -y install --no-install-recommends gcc unzip cmake make && apt-get clean all
RUN wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.3.2.zip -O /catch2.zip && unzip /catch2.zip -d /catch2

WORKDIR /catch2/Catch2-3.3.2
RUN cmake -E make_directory build

WORKDIR /catch2/Catch2-3.3.2/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_LIBDIR=/usr/lib/ /catch2/Catch2-3.3.2
RUN cmake --build . -j4
RUN cmake --install .



FROM llvm16-catch3 AS llvm16-catch3-lexy
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /
RUN wget https://github.com/foonathan/lexy/archive/refs/tags/v2022.12.1.zip -O /lexy.zip && unzip /lexy.zip -d /lexy

WORKDIR /lexy/lexy-2022.12.1/
RUN cmake -E make_directory build

WORKDIR /lexy/lexy-2022.12.1/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_LIBDIR=/usr/lib/ \
    -DLEXY_BUILD_BENCHMARKS=OFF \
    -DLEXY_BUILD_EXAMPLES=OFF \
    -DLEXY_BUILD_TESTS=OFF \
    -DLEXY_BUILD_DOCS=OFF \
    -DLEXY_BUILD_PACKAGE=OFF \
    -DLEXY_ENABLE_INSTALL=ON \
    /lexy/lexy-2022.12.1

RUN cmake --build . -j4
RUN cmake --install .

WORKDIR /


FROM llvm16-catch3-lexy AS bython-base

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends pipx && apt-get clean all
RUN pipx ensurepath && pipx install lit

ENTRYPOINT [ "/bin/bash" ]