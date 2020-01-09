FROM ubuntu:bionic
ARG USER=docker
ARG UID=1000
ARG GID=1000
ARG PW=docker

MAINTAINER Jagoda Górska <juliagoda.pl@protonmail.com>


# Dependencies of the Qt offline installer
RUN apt-get -y update && apt-get install -y \
    g++ \
    make \
    build-essential \
    git \
    mesa-utils \
    libgl1-mesa-glx \
    libgl1-mesa-dri \
    gettext \
    autoconf \
    pkg-config \
    cmake \
    extra-cmake-modules \
    libtool \
    curl \
    libsdl2-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libxi-dev \
    libxtst-dev \
    libx11-dev \
    libxrender-dev \
    libxext-dev \
    itstool \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
    && apt-get autoremove -y



RUN git clone https://github.com/juliagoda/antimicroX.git && cd antimicroX

COPY . /opt

WORKDIR /opt

RUN cmake -DCMAKE_INSTALL_PREFIX=/usr . && make && make install

ENV PATH /usr/bin:$PATH

RUN useradd -m ${USER} --uid=${UID} && echo "${USER}:${PW}" | \
      chpasswd
      
USER ${UID}:${GID}

WORKDIR /home/${USER}

ENTRYPOINT ["./unix_x11_antimicrox", "latest-ubuntu18.04"]
