# How to build

## Prerequisite

- Linux or Mac is recommended, however, you can build on Windows, too.
- recent x86_64 CPU that has AVX instruction is necessary.

## Install dependencies.

### All platforms

Install [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up).
Don't forget to set PATH.

### Linux

Also install the following.

```shell
$ sudo apt-get install git clang
$ sudo apt-get install libprotobuf-dev libcurl4-nss-dev
$ sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
$ sudo apt-get install libmicrohttpd-dev libffms2-dev libusb-1.0-0-dev
```

### Mac (with homebrew)

Also install Xcode and the following.

```shell
$ brew install pkg-config
$ brew install sdl2 SDL2_ttf SDL2_image ffmpeg libusb protobuf
```

### Windows

Install Install Visual Studio 2017. Community Edition works.

TODO: How to install SDL2?

## Download repository

This downloads puyoai repository and dependent repositories.
We assume you made an empty directory `~/repos/puyoai` or `%HOME%\repos\puyoai`.

On Linux or Mac:

```shell
$ cd ~/repos/puyoai
$ gclient config --unmanaged https://github.com/puyoai/puyoai
$ gclient sync
```

On Windows

```
$ cd %HOME%\repos\puyoai
$ gclient config --unmanaged https://github.com/puyoai/puyoai
$ gclient sync
```

This command will take several minutes to download resources. 

## Build

### Linux/Mac

```shell
$ cd ~/repos/puyoai/puyoai
$ gn gen --args="is_debug=false" out/Release
$ ninja -C out/Release
```

This only generates minimal set. If you want to build everything, 

```shell
$ gn args out/Release
```

and set the following (as of 2019-02-13).

```
is_debug = false
use_capture = true
use_usb = true
use_gui = true
use_httpd = true
use_libcurl = true
use_tcp = true
use_curl = true
```

See declare_args in [build/config/BUILDCONFIG.gn](build/config/BUILDCONFIG.gn) to understand what options can be specified.

### Windows

First, run vcvars64.bat.

```
> "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
```

and in the same command promot, run the following.

```
> cd %HOME%\repos\puyoai\puyoai
> gn gen --args="is_debug=false" out/Release
> ninja -C out/Release
```

If you'd like to generate solution file of Visual Studio, specify `--ide=vs` in gn.

```
> cd %HOME%\repos\puyoai\puyoai
> gn gen --args="is_debug=false" --ide=vs out/Release
```

Then `all.sln` is generated.
