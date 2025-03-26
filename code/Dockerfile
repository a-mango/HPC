FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update && \
    apt-get install -y \
      gcc-13 g++-13 \
      curl \
      build-essential \
      git \
      cmake \
      gcovr \
      ffmpeg \
      libsndfile-dev libfftw3-dev && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 60 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 60
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
    apt-get install -y nodejs
