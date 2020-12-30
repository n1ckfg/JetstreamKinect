## Azure Kinect on Ubuntu 18.04
#### Notes on Installing Microsoft Azure Kinect Sensor and Body Tracking SDKs on Linux PC and NVIDIA Jetson Xavier NX
_06.12.2020_

Jump to:
- [Installing Sensor SDK on Linux PC](#installing-sensor-sdk-on-linux-pc)
- [Installing on Jetson Xavier NX](#installing-sensor-sdk-on-jetson-xavier-nx)
- [Installing Body Tracking SDK](#installing-body-tracking-sdk)
- [Updating Firmware for Azure Kinect](#updating-firmware-for-azure-kinect)

---

## Installing Sensor SDK on Linux PC

> See [Microsoft's Installation Guide](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/blob/develop/docs/usage.md#debian-package)

### 1. Add link to Microsoft's Linux Software Repository for AMD64
By adding microsoft's code repository to your system so that you can call `sudo apt install xxx` to install system wide.
> [Guide for Configuring the Repository](https://docs.microsoft.com/en-us/windows-server/administration/linux-package-repository-for-microsoft-software#ubuntu)

Here are Microsoft's Instructions:
```
curl https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
sudo apt-add-repository https://packages.microsoft.com/ubuntu/18.04/prod
sudo apt-get update
```
I ran into errors with these instructions. Here's how I solved them:

1. Install `curl` if you haven't already.
```
sudo apt update && upgrade
sudo apt install curl
```

2. Specify the architecture of the repository.
Running the repository command, I got this error on running`sudo apt-get update`: `N: Skipping acquire of configured file 'main/binary-i386/Packages' as repository 'https://packages.microsoft.com/ubuntu/18.04/prod' doesn't support support architecture 'i386'`.

Turns out you need specify the architecture of the repository to `[arch=amd64]`:

  - Log in as root:
  ```
  sudo -i
  ```
  - Modify `etc/apt/sources.list`. At the bottom of the file, change from:
  ```
deb https://packages.microsoft.com/ubuntu/18.04/prod bionic main
# deb-src https://packages.microsoft.com/ubuntu/18.04/prod bionic main
```
  to:
```
deb [arch=amd64] https://packages.microsoft.com/ubuntu/18.04/prod bionic main
# deb-src [arch=amd64] https://packages.microsoft.com/ubuntu/18.04/prod bionic main
```
and save.

  - Log out of root:
  ```
  exit
  ```
3. Rerun `sudo apt-get update`

### 2. Install Kinect Packages
If there are no more errors, you should be able to install the Kinect packages `install libk4a1.X libk4a1.X-dev k4a-tools`
> If you install `k4a-tools` first, it automatically downloads the latest version of `libk4a1.X`. Take note of the version for installing `libk4a1.X-dev` (it must be the same version as `libk4a1.X`).
```
sudo apt install k4a-tools
sudo apt install libk4a1.4-dev
```
> If you mess up, you can always uninstall and try again using `sudo apt remove k4a-tools`

### 3. Finish Device Setup

[Finish device setup](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/blob/develop/docs/usage.md#linux-device-setup) by setting up udev rules:

- Copy '[scripts/99-k4a.rules](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/blob/develop/scripts/99-k4a.rules)' into '/etc/udev/rules.d/'.
- Detach and reattach Azure Kinect devices if attached during this process.

### 4. Run an App

To run an app, you just call it directly in the terminal:
```
k4aviewer
```
> You may get a warning to upgrade your firmware when running `k4aviewer` the first time. See [instructions](#updating-firmware-for-azure-kinect) below.

> The `k4aviewer`, `k4arecorder`, and `AzureKinectFirmwareTool` downloaded through `k4a-tools` are all located in `/usr/bin`.


---

## Installing Sensor SDK on Jetson Xavier NX
At the time of writing (June 2020), Microsoft has debian packages for Ubuntu 18.04 for AMD64 and ARM64. However, this is the first release for ARM: there are some bugs and the Body Tracking SDK is not yet supported. More details on that below.

### 1. Add Microsoft's product repository for ARM64
```
curl https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
sudo apt-add-repository https://packages.microsoft.com/ubuntu/18.04/multiarch/prod
sudo apt-get update
```
### 2. Install Kinect Package
Same as instructions [above](#2-install-kinect-packages).

### 3. Finish Device Setup
Same as instructions [above](#3-finish-device-setup).

### 4. Increase default USB memory for Ubuntu
By default, Linux-based host computers allocate the USB controller only 16 MB of kernel memory to handle USB transfers. This amount is supposed to be enough to support a single Azure Kinect ... so this step _might not_ be necessary. But I went ahead and did it anyway just to be safe.

Microsoft has [instructions](https://docs.microsoft.com/en-us/azure/kinect-dk/multi-camera-sync#linux-computers-usb-memory-on-ubuntu) for this, however Jetson boards don't have grub. So instead, I used these [instructions from FLIR](https://www.flir.com/support-center/iis/machine-vision/application-note/getting-started-with-the-nvidia-jetson-platform/) for the Jetson TX2.
```
To acquire images greater than 2 MB in resolution, add the following to the APPEND line:

usbcore.usbfs_memory_mb=1000

to this file:

/boot/extlinux/extlinux.conf

If this method fails to set the memory limit, run the following command:

sudo sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
```
> Here's a good overview of why USBFS matters for usb camera on Linux, also from [FLIR](https://www.flir.com/support-center/iis/machine-vision/application-note/understanding-usbfs-on-linux/)

#### Notes on Running on Jetson Xavier NX
1. The Jetson Xavier NX Developer Kit seems compatible with [minimum system requirements](https://docs.microsoft.com/en-us/azure/kinect-dk/system-requirements#minimum-host-pc-hardware-requirements) of the Azure Kinect.
2. I'm running my X NX `15W 6CORE` mode with 250 GB NVMe SSD added to the DK.
    - See this [Jetson Hacks repo](https://github.com/jetsonhacks/rootOnNVMe) for a walkthrough of adding NVMe storage to the Jetson X NX.
3. Testing with `k4aviewer`, I got the following warnings and errors:

This warning is constant, but harmless:
```
[ warning  ] : depth_engine_thread(). Depth image processing is too slow at 1000ms (this may be transient).
```

This is a bug that crashes the data stream when the color camera is enabled:
```
[ error    ] : DecodeMJPEGtoBGRA32(). MJPEG decode failed: -1
[ warning  ] : capturesync_add_capture(). Capture Error Detected, Color 
[ error    ] : capturesync_get_capture(device->capturesync, capture_handle, timeout_in_ms) returned failure in k4a_device_get_capture()
```

Disabling the Color Camera avoids the errors, however [there's a fix](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/1249#issuecomment-642245118) in the next release, 1.4.1. But you can download the beta with the fix now:

- Click this nuget download link https://www.nuget.org/api/v2/package/Microsoft.Azure.Kinect.Sensor/1.4.1-beta.0, rename from `.nupkg` to `.zip`, and unzip for access to the AMD64 and ARM64 libraries.

4. From from the `k4aviewer` app, I consistently got 30FPS from the 3 different camera streams and 60FPS. 
    - 3-4 of the 6 CPUs were maxing out, with the other 2-3 running at 50% (with a few chrome tabs open).
    - You can max out all the CPUs just by opening a lot of tabs.
    - Disabling a camera frees up a CPU.
    - Microphone Array and IMU also worked reliably.

5. The Body Tracking SDK does not run on ARM yet ... supposedly that is [coming soon](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/1093#issuecomment-594313079) though.

---

## Installing Body Tracking SDK

> See [Microsoft's Instructions](https://docs.microsoft.com/en-us/azure/kinect-dk/body-sdk-download#linux-installation-instructions)

> The Body Tracking SDK is not currently supported on ARM ... supposedly that is [coming soon](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/1093#issuecomment-594313079) though.

At the time of writing, the latest Body Tracking SDK (version 1.0.0) has a [dependency on  version 1.3](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/1248) of the Sensor SDK. However, `k4a-tools` is dependent on version 1.4. It's currently [not possible to have multiple SDKs installed](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/836), so you have to pick one.

To downgrade from SDK 1.4 to 1.3:

1. Remove the 1.4 SDK.
```
sudo apt remove k4a-tools
sudo apt remove libk4a1.4
sudo apt remove libk4a1.4-dev
```
2. Install the 1.3 Sensor and Body Tracking SDKs.
```
sudo apt install k4a-tools=1.3.0 k4a1.3-dev k4abt1.0 k4abt1.0-dev
```
3. Run the apps
```
k4abt_simple_3d_viewer
```

---

## Updating Firmware for Azure Kinect

> See [Microsoft's Instructions](https://docs.microsoft.com/en-us/azure/kinect-dk/update-device-firmware)

1. Download the latest firmware's .bin file (at the time it was [here](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/issues/1093)).
  - You can check the firmware of your connected device by running `$AzureKinectFirmwareTool -q` in the terminal.
2. In the terminal, go to the directory with the firmware and update with the AzureKinectFirmwareTool: `AzureKinectFirmwareTool.exe -u <device_firmware_file.bin>`
```
AzureKinectFirmwareTool -u AzureKinectDK_Fw_1.6.108079014.bin
```
I got the following error when running the firmware tool:
```
ERROR: The device failed to reset after an update. Please manually power cycle the device.
[2020-06-10 11:33:27.424] [error] [t=19238] /__w/1/s/extern/Azure-Kinect-Sensor-SDK/src/firmware/firmware.c (61): firmware_t_get_context(). Invalid firmware_t (nil)
[2020-06-10 11:33:27.425] [error] [t=19238] /__w/1/s/extern/Azure-Kinect-Sensor-SDK/src/firmware/firmware.c (313): Invalid argument to firmware_get_device_version(). firmware_handle ((nil)) is not a valid handle of type firmware_t
ERROR: Failed to get updated versions
```
After power cylcing and unplugging from USB, the firmware did in fact update.
