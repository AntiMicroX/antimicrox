#! /bin/bash 

# $1 - first argument passed to script. It means a chosen tag of juliagoda/antimicrox docker image, for example latest-ubuntu18.04, latest here means from last commit of master branch 

host +local:docker
docker run -it -e DISPLAY=unix$DISPLAY --mount type=bind,source=/dev/input,target=/dev/input --device /dev/input --mount type=bind,source=/home/$USER,target=/home/$USER --net=host -e HOME=$HOME --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" --device /dev/dri:/dev/dri --workdir=$HOME juliagoda/antimicrox:$1
