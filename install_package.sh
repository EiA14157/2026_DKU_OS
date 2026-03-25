#!/bin/bash

WORKDIR=$(pwd)

sudo apt install -y libgtest-dev git vim g++ make cmake libssl-dev

cd /usr/src/googletest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib

cd "$WORKDIR" 