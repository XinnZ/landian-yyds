DIR=$(pwd)

EXTRA_ARGS=""

if [[ ! -d OpenCV3 ]]; then
  git clone --depth 1 --branch 3.4.3 https://github.com/opencv/opencv.git OpenCV3
fi

## optional add-on with
# git clone --depth 1 --branch 3.4.3 https://github.com/opencv/opencv_contrib.git opencv_contrib
if [[ -d opencv_contrib ]]; then
  CONTRIB_PATH="$DIR/opencv_contrib/modules"
  EXTRA_ARGS="$EXTRA_ARGS -DOPENCV_EXTRA_MODULES_PATH=$CONTRIB_PATH"
fi

cd OpenCV3

# comment out the define for LIBV4L
sed -i -e 's/\(.*HAVE_LIBV4L\)/\/\/\1/' cmake/templates/cvconfig.h.in
sed -i -e "s/HAVE_LIBV4L YES/HAVE_LIBV4L NO/" cmake/OpenCVFindLibsVideo.cmake

mkdir -p build
cd build

cmake  -D CMAKE_BUILD_TYPE=RELEASE \
       -D CMAKE_INSTALL_PREFIX=/usr/local \
       -D ENABLE_PRECOMPILED_HEADERS=OFF \
       -D WITH_CUDA=ON \
       -D CUDA_ARCH_BIN="6.2" \
       -D CUDA_ARCH_PTX="" \
       -D WITH_CUBLAS=ON \
       -D ENABLE_FAST_MATH=ON \
       -D CUDA_FAST_MATH=ON \
       -D ENABLE_NEON=ON \
       -D WITH_LIBV4L=ON \
       -D BUILD_TESTS=OFF \
       -D BUILD_PERF_TESTS=OFF \
       -D BUILD_EXAMPLES=OFF \
       -D WITH_OPENGL=ON \
       $EXTRA_ARGS \
       ..

make -j3

sudo make install

echo "OpenCV_DIR=$DIR/OpenCV3/build"
