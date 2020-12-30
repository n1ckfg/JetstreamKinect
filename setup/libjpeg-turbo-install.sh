TARGET=libjpeg-turbo-2.0.6

tar -xvf $TARGET.tar.gz
cd $TARGET
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..
rm -rf $TARGET

