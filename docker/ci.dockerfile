FROM ghcr.io/bython-talk/bython-base AS bython-ci
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends lcov ninja-build clang-format-16 && apt-get clean all
RUN update-alternatives \
    --install /usr/bin/clang          clang         /usr/bin/clang-16 160 \
    --slave   /usr/bin/clang++        clang++       /usr/bin/clang++-16 \
    --slave   /usr/bin/clang-format   clang-format  /usr/bin/clang-format-16