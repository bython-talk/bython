FROM ghcr.io/bython-talk/bython-base AS bython-dev
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends gdb clang-format cppcheck ninja-build libxml2-dev gzip && apt-get clean all

WORKDIR /