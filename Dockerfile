FROM ubuntu
LABEL Description="antimicroX run on Ubuntu with Qt 5.9.5 and SDL 2.0.8"

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
    libtool \
    curl \
    libsdl2-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libxi-dev \
    libxtst-dev \
    libx11-dev \
    itstool \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
    && apt-get autoremove -y

    
# cmake && extra-cmake-modules
RUN curl -L -O 'https://github.com/Kitware/CMake/releases/download/v3.12.2/cmake-3.12.2-Linux-x86_64.sh' && \
    chmod a+x cmake-3.12.2-Linux-x86_64.sh && \
    ./cmake-3.12.2-Linux-x86_64.sh --prefix=/usr --skip-license && \
    git clone git://anongit.kde.org/extra-cmake-modules && \
    cd extra-cmake-modules && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && \
    make && \
    make install && \
    cd ../../ && \
    rm -rf extra-cmake-modules


RUN git clone https://github.com/juliagoda/antimicroX.git

RUN cd antimicroX

COPY . /opt

WORKDIR /opt

RUN cmake -DCMAKE_INSTALL_PREFIX=/usr . && make && make install

WORKDIR /usr/bin

CMD ["./antimicroX"]
