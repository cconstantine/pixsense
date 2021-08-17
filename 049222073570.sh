#!/bin/bash

exec /usr/bin/python3 rs_cams.py --device 049222073570 -x 0.045 -y 0.373 -z 0.252 -r 270 --port 6543 "$@"
