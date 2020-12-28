# https://gist.github.com/madelinegannon/c212dbf24fc42c1f36776342754d81bc

curl https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
sudo apt-add-repository https://packages.microsoft.com/ubuntu/18.04/multiarch/prod
sudo apt-get update

sudo apt install k4a-tools
sudo apt install libk4a1.4-dev

sudo cp 99-k4a.rules /etc/udev/rules.d/
