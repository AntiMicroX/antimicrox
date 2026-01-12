FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

RUN apt-get update 

# Install basic tools
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install \
    g++ cmake extra-cmake-modules ninja-build \
    qt6-base-dev qt6-tools-dev-tools libqt6core5compat6-dev qt6-tools-dev \
    libsdl2-dev libxi-dev libxtst-dev libx11-dev itstool gettext

# Install xeyes for testing dev environment
RUN apt-get install -y --no-install-recommends x11-apps
# if xeyes returns error: "Error: Can't open display: :0"
# launch on host machine command xhost +local:
