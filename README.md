# obs-text-slideshow

![CI Multiplatform Build](https://github.com/jbwong05/obs-text-slideshow/actions/workflows/main.yml/badge.svg)
![Clang Format](https://github.com/jbwong05/obs-text-slideshow/actions/workflows/clang-format.yml/badge.svg)

Inspired by the built in [image slideshow](https://github.com/obsproject/obs-studio/blob/master/plugins/image-source/obs-slideshow.c), except for text sources instead. Both Free Type 2 and GDI+ are supported. Useful for displaying song lyrics, captions, etc.

## Text file format
### Single text file format
New lines are expected in order to separate the text for the different text sources in the slideshow.
```
text for first
text source

text for second text source

text for third text
source

single line is supported

multiline is
supported

etc.
```

### Multiple text file format
For the multiple text file option, the text from each text file corresponds to the text for one text source in the slideshow.

## Installation
Installers can be found in the [Releases](https://github.com/jbwong05/obs-text-slideshow/releases) section.

Note: The macOS installer is currently unsigned because I don't have a subscription for the Apple Developer program.

## Building from Source

### Prerequisites
You'll need CMake and a minimal working development environment for OBS Studio installed on your computer. Only Qt and the standard `obs dependencies`(https://github.com/obsproject/obs-deps) are required. obs can be built with the `-DDISABLE_PLUGINS=true` flag which is sufficient. More specific details can be found [here](https://github.com/obsproject/obs-studio/wiki/Install-Instructions#windows-build-directions).

### Windows
#### Building with cmake-gui
In cmake-gui, you'll have to set these CMake variables:
- **QTDIR** (path) : location of the Qt environment suited for your compiler and architecture
- **LIBOBS_INCLUDE_DIR** (path) : location of the `libobs` subfolder in the source code of OBS Studio
- **LIBOBS_LIB** (filepath) : location of the obs.lib file
- **OBS_FRONTEND_LIB** (filepath) : location of the obs-frontend-api.lib file
- **LibObs_DIR** (filepath) : location of `libobs` folder within the obs build directory

#### Building from Command Line
```
git clone https://github.com/jbwong05/obs-text-slideshow.git
cd obs-text-slideshow
mkdir build && cd build
# Windows 64-bit
cmake -G"Visual Studio 16 2019" -A"x64" -DCMAKE_SYSTEM_VERSION="10.0.18363.657" -DQTDIR=<path to 64-bit Qt dir> -DLibObs_DIR=<path to libobs folder within obs build dir> -DLIBOBS_INCLUDE_DIR=<path to the libobs sub-folder in obs-studio's source code> -DLIBOBS_LIB=<path to obs.lib> -DOBS_FRONTEND_LIB=<path to obs-frontend-api.lib> ..
# or Windows 32-bit
cmake -G"Visual Studio 16 2019" -A"Win32" -DCMAKE_SYSTEM_VERSION="10.0.18363.657" -DQTDIR=<path to 32-bit Qt dir> -DLibObs_DIR=<path to libobs folder within obs build dir> -DLIBOBS_INCLUDE_DIR=<path to the libobs sub-folder in obs-studio's source code>  -DLIBOBS_LIB=<path to obs.lib> -DOBS_FRONTEND_LIB=<path to obs-frontend-api.lib> ..
# Open `obs-text-slideshow.sln` with Visual Studio and build
# Copy `obs-text-slideshow.dll` and `obs-text-slideshow.pdb` to the obs plugin directory
# Create a obs-text-slideshow folder in the obs data directory and copy the locale folder into the new obs-text-slideshow directory
```

### Linux
```
git clone https://github.com/jbwong05/obs-text-slideshow.git
cd obs-text-slideshow
mkdir build && cd build
# If you are on Ubuntu, add the `-DBUILD_UBUNTU=true` flag to your cmake command
cmake ..
# If dependencies are not on your path, you can manually specify their paths with the following:
cmake -DQTDIR=<path to Qt dir> -DLIBOBS_INCLUDE_DIR=<path to the libobs sub-folder in obs-studio's source code> -DLIBOBS_LIB=<path to libobs.so> -DOBS_FRONTEND_LIB=<path to libobs-frontend-api.so> ..
make
sudo make install
```

### OS X
```
git clone https://github.com/jbwong05/obs-text-slideshow.git
cd obs-text-slideshow
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR=<path to the libobs sub-folder in obs-studio's source code> -DLIBOBS_LIB=<path to libobs.0.dylib> -DOBS_FRONTEND_LIB=<path to libobs-frontend-api.dylib> -DQTDIR=<path to Qt dir> ..
make
# Copy libobs-text-slideshow.so to the obs-plugins folder
# Create a obs-text-slideshow folder in the obs data directory and copy the locale folder into the new obs-text-slideshow directory
```

## Possible future work
- [x] Text input from files (UTF-8)
- [ ] Individual text settings for each text source
- [x] GUI dock for easier transitioning between sources
  - [x] Find work around for the need for a `refresh sources` button; ~~because the obs-frontend-api doesn't have a source added or edited event~~ libobs handles the different signals for sources and not the frontend api