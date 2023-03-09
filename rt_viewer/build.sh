#!/bin/bash

if [ -z "$RT_VIEWER_ROOT" ]
then
    echo "RT_VIEWER_ROOT is not set"
    exit 1
fi

# Generate build script
cd $RT_VIEWER_ROOT && \
if [ ! -d build ]; then
    mkdir build
fi
cd build && \
cmake ../ -DCMAKE_INSTALL_PREFIX=$RT_VIEWER_ROOT && \

# Build and install the program
make -j4 && \
make install && \

# Run the program
cd ../bin && \
./rt_viewer
