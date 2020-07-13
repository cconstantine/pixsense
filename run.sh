#!/bin/sh

set -e

exec docker run -v /tmp/.X11-unix:/tmp/.X11-unix \
 -e DISPLAY -it --rm --gpus all \
 --device /dev/video0:/dev/video0 \
 --device /dev/video1:/dev/video1 \
 --device /dev/video2:/dev/video2 \
 --device /dev/video6:/dev/video6 \
 --device /dev/video7:/dev/video7 \
 --device /dev/video8:/dev/video8 \
 -e NVIDIA_VISIBLE_DEVICES=0 \
 -v ${PWD}:/app/ \
 --network host \
 $*

