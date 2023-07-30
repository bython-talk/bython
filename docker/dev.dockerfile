FROM ghcr.io/bython-talk/bython-base AS bython-dev
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends \
    ninja-build gdb cppcheck \
    clang-format-16 \
    && apt-get clean all