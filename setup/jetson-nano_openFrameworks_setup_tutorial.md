### How to Set Up the NVIDIA Jetson Nano for openFrameworks
A Step-By-Step Guide from Unboxing to Creative Coding

[Getting Started](#getting-started)    
1. [Set up and Configure the Nano](#1-set-up-and-configure-the-nano)
2. [Set up and Configure openFrameworks](#2-set-up-and-configure-openframeworks)
    - [Getting the Nightly Build](#getting-the-nightly-build)
    - [Editing MAKE files](#editing-make-files)
    - [Recompile with New Configs](#recompile-with-new-configs)
    - [Finish openFrameworks Linux Setup](#finish-openframeworks-linux-setup)
    - [Test openFrameworks](#test-openframeworks)    

[Working with Addons](#working-with-addons)
1. [ofxKinect](#1-ofxkinect)
2. [ofxKinectV2](#2-ofxkinectv2)
    - [Install libfreenect2](#install-libfreenect2)
    - [Configure OpenCL](#configure-opencl)
    - [Build the Shared Library](#building-the-shared-library)
    - [Which ofxKinectV2?](#which-ofxkinectv2)
    - [Build the example app](#build-the-example-app)
    - [Troubleshooting](#troubleshooting)
    
[Additional Info](#additional-info)
- [Helpful Resources](#helpful-resources)
- [Tutorials](#tutorials)


# Getting Started

#### Hardware Requirements

1. NVIDIA Jetson Nano [Developer Kit](https://www.seeedstudio.com/NVIDIA-Jetson-Nano-Development-Kit-p-2916.html)
2. [64 GB MicroSD Card](https://www.amazon.com/Samsung-MicroSDXC-Adapter-MB-ME64GA-AM/dp/B06XX29S9Q/)
3. [5V, 4A Power Supply](https://www.adafruit.com/product/1466)
    - The J25 power jack is 9.5 mm deep, and accepts positive polarity plugs with 2.1 mm inner diameter and 5.5 mm outer diameter.
4. Keyboard and Mouse
5. Monitor and HDMI or DP cable

## 1. Set up and Configure the Nano
Follow NVIDIA's official [Getting Started With Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit) to setup and boot the Jetson Nano. 

- There's also a [Troubleshooting](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit#troubleshooting) section if you run into any issues.
- Remeber that if you're powering the Nano from the barrel jack, you need to add a jumper to the J48 Power Select Header pins to disable power supply via Micro-USB and enable 5V⎓4A via the J25 power jack.
- Full hardware specs for the Jetson Nano can be found [here](https://elinux.org/Jetson_Nano).

After setup, there are few optional, but useful settings you might want to configure for your Nano:

- [Disable password](https://askubuntu.com/questions/147241/execute-sudo-without-password) when using `sudo`
- [Enable SSH](https://linuxhint.com/enable_ssh_server_ubuntu_1804/) so we can remotely connect to the Nano from another computer

## 2. Set up and Configure openFrameworks
Next, we need to download, configure, and build openFrameworks on the Nano.

Hat tip to [Jason Van Cleave](https://twitter.com/jvcleave/status/1118172916582113282?s=20) for his [thorough instructions](https://gist.github.com/jvcleave/e49c0b52085d040a5cd8a3385121cb91) (expounded on below):

#### Getting the Nightly Build
openFrameworks for linuxarmv7 is currently supported in the nightly build found at the bottom of the [Downloads](https://openframeworks.cc/download/) page. 

- Get the package name that corresponds with `of_v2019XXXX_linuxarmv7l_nightly.tar.gz`.
- At the time of this tutorial, the package name was `of_v20190324_linuxarmv7l_nightly.tar.gz`.

To download and unpack openFrameworks:

```
> cd ~
> wget https://openframeworks.cc/ci_server/versions/nightly/of_v20190324_linuxarmv7l_nightly.tar.gz
> tar -zxvf of_v20190324_linuxarmv7l_nightly.tar.gz
```

#### Editing MAKE files
Next, we need to edit some of openFrameworks' `make` files to work with the Jetson Nano.

Edit the `config.shared.mk` file:
```
> cd ~
> gedit of_v20190324_linuxarmv7l_nightly/libs/openFrameworksCompiled/project/makefileCommon/config.shared.mk
```
Change the line:
```
else ifeq ($(PLATFORM_ARCH),armv7l)
```
to
```
else ifeq ($(PLATFORM_ARCH),aarch64)
```
Then save and close.


Next edit the `config.linuxarmv7l.default.mk` file:
```
> gedit of_v20190324_linuxarmv7l_nightly/libs/openFrameworksCompiled/project/linuxarmv7l/config.linuxarmv7l.default.mk
```
Comment out the following flags:
```
#PLATFORM_CFLAGS += -march=armv7
#PLATFORM_CFLAGS += -mtune=cortex-a8
#PLATFORM_CFLAGS += -mfpu=neon
#PLATFORM_CFLAGS += -mfloat-abi=hard
PLATFORM_CFLAGS + = -fPIC
PLATFORM_CFLAGS + = -ftree-vectorize
PLATFORM_CFLAGS + = -Wno-psabi
PLATFORM_CFLAGS + = -pipe
```
and
```
#PLATFORM_PKG_CONFIG_LIBRARIES += glesv1_cm
#PLATFORM_PKG_CONFIG_LIBRARIES += glesv2
#PLATFORM_PKG_CONFIG_LIBRARIES += egl
```
Then save and close.

#### Recompile with New Configs
Next, we need to recompile and build the `kiss` and `tess2` libraries with these modified settings.  You can download oF's `apothecary` tool to recompile the libraries:

```
> git clone https://github.com/openframeworks/apothecary.git
> cd apothecary/apothecary
> ./apothecary -t linux download kiss
> ./apothecary -t linux prepare kiss
> ./apothecary -t linux build kiss
> ./apothecary -t linux download tess2
> ./apothecary -t linux prepare tess2
> ./apothecary -t linux build tess2
```

Replace oF's `kiss` and `tess2` libraries with these newly created static libraries:

```
> cd ~
> sudo cp apothecary/apothecary/build/kiss/lib/linux/libkiss.a of_v20190324_linuxarmv7l_release/libs/kiss/lib/linuxarmv7l/
> sudo cp apothecary/apothecary/build/tess2_patched/build/libtess2.a of_v20190324_linuxarmv7l_release/libs/tess2/lib/linuxarmv7l/
```

> You can also just download the libs [`libkiss.a`](https://www.dropbox.com/s/qbo3d5q8s7ugv9w/libkiss.a?dl=0) and [`libtess.a`](https://www.dropbox.com/s/o1v226cge438jx9/libtess2.a?dl=0) that [Jason von Cleave](https://twitter.com/jvcleave/status/1118172916582113282?s=20) properly recompiled.


#### Finish openFrameworks Linux Setup
Now we are all set up to do openFrameworks's normal [Linux Install Instructions](https://openframeworks.cc/setup/armv7/):
```
> cd of_v20190324_linuxarmv7l_release/scripts/linux/ubuntu
> sudo ./install_dependencies.sh
> cd ..
> ./compileOf.sh
```

#### Test openFrameworks
You can Build and Run the `allAddonsExample` to test whether the built-in addons are working out of the box:
```
> cd of_v20190324_linuxarmv7l_release/examples/templates/allAddonsExample
> make && make run
```

In my nightly build, `ofxSvg` fails with the following error:

`/home/nano/of_v20190324_linuxarmv7l_release/addons/ofxSvg/libs/svgtiny/lib/linuxarmv7l/libsvgtiny.a: error adding symbols: File in wrong format`

If you comment out lines relating to `ofxSvg` in `addons.make` and then in `ofApp.h`, then the app should run.

# Working with Addons
Here is a setup guide for working with some of the more popular addons and sensors for openFrameworks.

## 1. ofxKinect
openFramework's addon for the v1 Microsoft Kinect [should](https://forum.openframeworks.cc/t/ofxkinect-included-in-of-v0-9-8-on-raspberry-pi-3b-jessie-arm6/28287/10) build and run out-of-the-box (with root privilages):

```
> cd ~/of_v20190324_linuxarmv7l_release/examples/computer_vision/kinectExample
> make
> sudo make run
```
To run a ofxKinect app without root privilages, copy over libfreenect's udev rules for device access:
```
> cd ~/of_v20190324_linuxarmv7l_release/addons/ofxKinect
> sudo cp libs\libfreenect\platform\linux\udev\51-kinect.rules /etc/udev/rules.d
```
> You may need to restart your system for the rules to take effect.

> The Jetson Nano will run one Kinect, but it does not support running multiple Kinects.

## 2. ofxKinectV2
To work with the v2 Microsoft Kinect in openFrameworks, you need to first install `libfreenect2`. You can find their Linux Installation instructions [here](https://github.com/OpenKinect/libfreenect2/blob/master/README.md#linux).

> The Jetson Nano image already comes loaded with libusb >= 1.0.20, OpenGL version 4.6.0 NVIDIA 32.1.0, and CUDA 10.0.166

### Install libfreenect2
```
> cd ~
> git clone https://github.com/OpenKinect/libfreenect2.git
> cd libfreenect2
> sudo apt-get install build-essential cmake pkg-config
> sudo apt-get install libturbojpeg0-dev
```
### Configure OpenCL
By default, Jetson boards don't support OpenCL. So we need to add and reconfigure a few things to get `libfreenect2` (and ofxKinectV2) to build with OpenCL enabled.

Start by downloading and extracting Khronos Group's OpenCL Headers from [github](https://github.com/KhronosGroup/OpenCL-Headers).

Once extracted move the `CL/` directory into `/usr/include`:
```
> sudo mv ~/Downloads/CL /usr/include
```

Now if you go into `/usr/lib/aarch64-linux-gnu` you can find the OpenCL library as `libOpenCL.so.1`. We need to add a symbolic link from `libOpenCL.so.1` to `libOpenCL.so`:
```
> cd /usr/lib/aarch64-linux-gnu
> sudo ln -s libOpenCL.so.1 libOpenCL.so
```

Now we should be all set up to build `libfreenect2` with OpenCL enabled.

### Building the Shared Library
The following commands `make` and `install` libfreenect2 system-wide:
```
mkdir build && cd build
cmake ..
make
make install
```
> After running `cmake ..`, verify in the console that the Feature List has OpenCL enabled (you should see `-- OpenCL	yes` printed out).

Set up udev rules for device access:
```
sudo cp ../platform/linux/udev/90-kinect2.rules /etc/udev/rules.d/
```
Plug in the v2 Kinect, and then run the libfreect2's example app: 
```
./bin/Protonect
```
With the `Protonect` example, should now see RGB, Depth, and IR feeds streaming from the Kinect. If not, look through libfreenect2's [Troubleshooting](https://github.com/OpenKinect/libfreenect2/blob/master/README.md#troubleshooting-and-reporting-bugs) section. 

> This step needs to be working before moving forward.


### Which ofxKinectV2
There are a few addons out there for working with the KinectV2 in openFrameworks, but Linux support is rare. I modifed Theo Watson's `ofxKinectV2` to work for Linux. Download my [fork](https://github.com/madelinegannon/ofxKinectV2) to start:

```
> cd ~/of/addons
> cd git clone https://github.com/madelinegannon/ofxKinectV2.git
```
By default, when an openFrameworks app builds, it links to the libraries in its local `libs` folder. But we want `libfreenect2` and `libusb-1.0` to link against our system wide installations. To resolve this, move or delete these folders in `ofxKinectV2/libs`:
```
> cd ofxKinectV2/libs
> mv libfreenect2 ~/Desktop
> mv libusb ~/Desktop
```
Now you should only see a `protonect` folder in `libs`.

Next, we want to make sure our `LD_LIBRARY_PATH` is set properly, so openFrameworks knows where to search for ofKinectV2's linked libraries.

Add `CUDA` paths to the system environment (you should also to add these lines to your `~/.bashrc`):
```
> export LD_LIBRARY_PATH="/usr/local/cuda/lib64:${LD_LIBRARY_PATH}"
> export PATH="/usr/local/cuda/bin:${PATH}"
```
A system-wide configuration of the libary path can be created with the following commands:
```
> echo "/usr/local/cuda/lib64" | sudo tee /etc/ld.so.conf.d/cuda.conf
> sudo ldconfig
```

Also add the `libfreenect2` path to the system environment (you should also add this path to your `~/.bashrc`):
```
> export LD_LIBRARY_PATH="/usr/local/lib:${$LD_LIBRARY_PATH}"
```

### Build the example app
Everything _should_ be set up to build and run the example app included of ofxKinectV2. 

But first, just a quick note on how things get linked and built in openFrameworks on linuxarmv7:

- When building from command line, openFramework uses the `ofxKinectV2/addons_config.mk` file to add all the compiler flags for the project. 
    - You'll notice I've added `libfreenect2` and `cuda` package names to the variable `ADDON_PKG_CONFIG_LIBRARIES`.
    - The build process looks for `.pc` files with the corresponding names in `/usr/lib/pkgconfig`.

Now we can `make` and `run`:
```
> make -j
> make run
```
Now you should see something very similar to `libfreenect2` Protonect example, but with the addition of some sliders that let you easily do depth thresholding.

> The framerate was not great on this. I need to see if there are ways to optimize the example app for the Nano.

### Troubleshooting
I had one runtime error that really tripped me up for a bit. The ofxKinectV2 example app would build and run, and in the console it would acknowledge that it detected and opened the Kinect. However, the stream wouldn't start, and the following error would print out:

```
[Error] [protocol::CommandTransaction] bulk transfer failed: LIBUSB_ERROR_TIMEOUT Operation timed out
```
What was happening was that openFrameworks wasn't actually linking to `libfreenect2.so` at runtime. I fixed this by making sure that I removed the local `libfreenect2` directory in `ofxKinectV2/libs`, and that the system-wide path to `libfreenect2.so` (`/usr/local/lib`) was also added to the LD_LIBRARY_PATH in my `~/.bashrc`.

## Additional Info
### Helpful Resources

- Jason Van Cleave's [openFrameworks Jetson Nano Instructions](https://gist.github.com/jvcleave/e49c0b52085d040a5cd8a3385121cb91)
- Takesan's [Install openFrameworks 0.10.0 on JETSON TX2 Instructions](http://takesan.hatenablog.com/entry/2018/08/27/133926)
- [ofxLibfreenect2](https://github.com/pierrep/ofxLibFreenect2)
- [Build ofxKinectV2 on Linux](https://github.com/ofTheo/ofxKinectV2/issues/27)

### Tutorials
- [How to get the Kinect V2 working in openFrameworks on Linux](https://gist.github.com/madelinegannon/10f62caba7184b90ea43a734768e5147)
- Jetson Hacks: [Jetson Nano Power Settings](https://www.jetsonhacks.com/2019/04/10/jetson-nano-use-more-power/) Tutorial
- Jetson Hacks: [MS Kinect V2 on NVIDIA Jetson TX1](https://www.jetsonhacks.com/2016/07/11/ms-kinect-v2-nvidia-jetson-tx1/) Tutorial