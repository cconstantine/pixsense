FROM nvidia/cuda:10.1-cudnn7-devel

#get deps
RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
  python3-dev python3-pip git g++ wget make libprotobuf-dev protobuf-compiler libopencv-dev \
  libgoogle-glog-dev libboost-all-dev libhdf5-dev libatlas-base-dev \
  libssl-dev

#for python api
RUN pip3 install numpy opencv-python 

RUN wget https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3.tar.gz
RUN tar xzf cmake-3.17.3.tar.gz 
RUN cd cmake-3.17.3 && ./configure && make -j $(nproc) && make install

#get openpose
WORKDIR /openpose
RUN git clone --branch v1.6.0 --depth 1 https://github.com/CMU-Perceptual-Computing-Lab/openpose.git .

#build it
WORKDIR /openpose/build
RUN cmake -DBUILD_PYTHON=ON .. && make -j `nproc`
RUN make install
RUN mkdir -p /usr/share/openpose
RUN mv /openpose/models /usr/share/openpose/

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends xorg-dev libglu1-mesa-dev libusb-1.0-0-dev
WORKDIR /librealsense
RUN git clone --branch v2.32.1 --depth 1 --recursive --shallow-submodules https://github.com/IntelRealSense/librealsense.git .
WORKDIR /librealsense/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GRAPHICAL_EXAMPLES=OFF -DBUILD_WITH_TM2=OFF .. && make -j `nproc` && make install

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends libglm-dev

WORKDIR /app
ENV HOME="/app"
# RUN rm -rf /openpose /librealsense

#This user schenanigans allows for local development
ARG USER=pixsense
ARG USER_ID=1000
ARG GROUP_ID=1000

RUN groupadd -g ${GROUP_ID} ${USER} && \
    useradd -l -u ${USER_ID} -g ${USER} ${USER}

RUN chown ${USER}:${USER} /app
USER ${USER}

# #get openpose
# WORKDIR /app
# RUN git clone https://github.com/CMU-Perceptual-Computing-Lab/openpose.git .

# #build it
# WORKDIR /openpose/build
# RUN cmake -DBUILD_PYTHON=ON .. && make -j `nproc`
# WORKDIR /openpose