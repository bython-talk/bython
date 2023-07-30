FROM ghcr.io/bython-talk/bython-base AS bython-ci
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends lcov ninja-build clang-format-16 && apt-get clean all

RUN update-alternatives \
    --install /usr/bin/c++      c++       /usr/bin/g++    160
RUN update-alternatives \
    --install /usr/bin/cc       cc        /usr/bin/gcc    160
