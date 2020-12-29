# https://www.datatobiz.com/2019/10/03/fixing-wifi-connectivity-nvidia-jetson-nano/
# https://github.com/pvaret/rtl8192cu-fixes

sudo cp blacklist-native-rtl8192.conf /etc/modprobe.d/
sudo echo options rtl8xxxu ht40_2g=1 dma_aggregation=1 | sudo tee /etc/modprobe.d/rtl8xxxu.conf
sudo iw dev wlan0 set power_save off

# To enable ssh over wifi:
# sudo nano /etc/gdm3/custom.conf
# In this file, uncomment the following:
# I. AutomaticLoginEnable = true
# II. AutomaticLogin = user // put your user name here

