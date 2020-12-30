# https://gist.github.com/madelinegannon/c212dbf24fc42c1f36776342754d81bc

if [ ${MACHINE_TYPE} == 'aarch64' ]; then
  curl https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
  sudo apt-add-repository https://packages.microsoft.com/ubuntu/18.04/multiarch/prod
  sudo apt-get update

  sudo apt-get install libk4a1.4 libk4a1.4-dev k4a-tools
else
  # body tracking SDK requires an older version of the base SDK and must go first
  sudo apt-get install libk4abt1.0 libk4abt1.0-dev

  # k4a-tools and the newer base sdk have a dependency conflict with the body tracking sdk 
  #sudo apt-get install libk4a1.4 libk4a1.4-dev k4a-tools
fi

sudo cp 99-k4a.rules /etc/udev/rules.d/

