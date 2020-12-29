# https://www.datatobiz.com/2019/10/03/fixing-wifi-connectivity-nvidia-jetson-nano/

sudo apt-get install git linux-headers-generic build-essential dkms

cd ../..
git clone https://github.com/pvaret/rtl8192cu-fixes
sudo dkms add ./rtl8192cu-fixes
sudo dkms install 8192cu/1.11
sudo depmod -a
sudo cp ./rtl8192cu-fixes/blacklist-native-rtl8192.conf /etc/modprobe.d/
sudo echo options rtl8xxxu ht40_2g=1 dma_aggregation=1 | sudo tee /etc/modprobe.d/rtl8xxxu.conf
sudo iw dev wlan0 set power_save off

# To enable ssh over wifi:
# sudo nano /etc/gdm3/custom.conf
# In this file, uncomment the following:
# I. AutomaticLoginEnable = true
# II. AutomaticLogin = user // put your user name here

