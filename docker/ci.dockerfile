FROM ghcr.io/bython-talk/bython-base AS bython-ci
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -q -y update
RUN apt-get -q -y install --no-install-recommends lcov ninja-build clang-format-16 && apt-get clean all