#! /bin/bash 

host +local:docker
docker run -it -e DISPLAY=unix$DISPLAY --mount type=bind,source=/dev/input,target=/dev/input --device /dev/input --mount type=bind,source=/home/$USER,target=/home/$USER --net=host -e HOME=$HOME --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" --device /dev/dri:/dev/dri --workdir=$HOME juliagoda/antimicrox:$1
