FROM nvidia/cuda:10.0-cudnn7-devel

#get deps
RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
  vim git g++ wget make libopencv-dev \
  caffe-tools-cuda libcaffe-cuda-dev\
  # libgrpc++-dev protobuf-compiler-grpc \
  protobuf-compiler  protobuf-c-compiler libprotoc-dev \
  libgoogle-glog-dev libboost-all-dev libhdf5-dev libatlas-base-dev \
  libssl-dev build-essential autoconf libtool pkg-config \
  xorg-dev libglu1-mesa-dev libusb-1.0-0-dev libglm-dev \
  libpq-dev

# cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3.tar.gz
RUN tar xzf cmake-3.17.3.tar.gz 
RUN cd cmake-3.17.3 && ./configure && make -j $(nproc) && make install

# # grpc
# WORKDIR /grpc
# RUN git clone --branch v1.28.1 --depth 1 --recursive --shallow-submodules https://github.com/grpc/grpc.git .
# WORKDIR /grpc/cmake/build
# RUN cmake -DgRPC_INSTALL=ON \
#       -DgRPC_BUILD_TESTS=OFF \
#       -DCMAKE_INSTALL_PREFIX=/usr \
#       -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
#       -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
#       -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
#       -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF \
#       -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
#       -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
#       ../..
# RUN make -j $(nproc)
# RUN make install

# # caffe
# WORKDIR /caffe
# RUN git clone --depth 1 https://github.com/BVLC/caffe.git .

# openpose
WORKDIR /openpose
RUN git clone --branch v1.6.0 --depth 1 https://github.com/CMU-Perceptual-Computing-Lab/openpose.git .
COPY models /openpose/models

WORKDIR /openpose/build
RUN cmake \
 -DBUILD_PYTHON=OFF \
 -DBUILD_CAFFE=OFF \
 -DCaffe_INCLUDE_DIRS=/usr/include \
 -DCMAKE_BUILD_TYPE=Release \
 -DCaffe_LIBS_RELEASE=/usr/lib/x86_64-linux-gnu/libcaffe.so \
 -DCaffe_LIBS=/usr/lib/x86_64-linux-gnu/libcaffe.so .. && make -j `nproc`
RUN make install

# librealsense
WORKDIR /librealsense
RUN git clone --branch v2.32.1 --depth 1 --recursive --shallow-submodules https://github.com/IntelRealSense/librealsense.git .
WORKDIR /librealsense/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GRAPHICAL_EXAMPLES=OFF -DBUILD_WITH_TM2=OFF .. && make -j `nproc` && make install

# dlib
WORKDIR /dlib
RUN git clone --branch v19.20 --depth 1 --recursive --shallow-submodules https://github.com/davisking/dlib.git .
WORKDIR /dlib/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
RUN make -j $(nproc)
RUN make install

# catch2 (test framework)
WORKDIR /catch2
RUN git clone --branch v2.12.3 --depth 1 --recursive --shallow-submodules https://github.com/catchorg/Catch2.git .
RUN cmake -Bbuild -H. -DBUILD_TESTING=OFF
RUN cmake --build build/ --target install

########### Start Pixsense
WORKDIR /app/build
ENV HOME="/app"

#This user schenanigans allows for local development
ARG USER=pixsense
ARG USER_ID=1000
ARG GROUP_ID=1000

RUN groupadd -g ${GROUP_ID} ${USER} && \
    useradd -l -u ${USER_ID} -g ${USER} ${USER}

RUN chown ${USER}:${USER} /app
USER ${USER}
