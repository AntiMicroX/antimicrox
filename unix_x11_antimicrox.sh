#! /bin/bash 

host +local:docker
docker run -ti --rm -e DISPLAY=unix$DISPLAY --device /dev/input --device=/dev/dri:/dev/dri --net=host --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" juliagoda/antimicrox:$1
