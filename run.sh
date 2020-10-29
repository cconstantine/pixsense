#!/bin/sh

set -e

exec docker run -v /tmp/.X11-unix:/tmp/.X11-unix \
 -e DISPLAY -it --rm --gpus all \
 $(for dev in /dev/video*; do echo -n "--device $dev:$dev "; done) \
 -e NVIDIA_VISIBLE_DEVICES=0 \
 -v ${PWD}:/app/ \
 --network host \
 -e PGPASSWORD=not_that_secret -e PGUSER=pixpq -e PGDATABASE=pixpq -e PGHOST=127.0.0.1 \
 $*

