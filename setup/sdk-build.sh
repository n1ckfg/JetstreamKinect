MACHINE_TYPE=`uname -m`
if [ ${MACHINE_TYPE} == 'aarch64' ]; then
  
else
  sudo apt-get install uuid-dev libsoundio-dev xorg-dev
fi

mkdir build
cd build
cmake .. -GNinja
ninja
sudo ninja install


