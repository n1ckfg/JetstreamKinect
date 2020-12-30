sudo apt-get install uuid-dev libsoundio-dev xorg-dev

mkdir build
cd build
cmake .. -GNinja
ninja
sudo ninja install


