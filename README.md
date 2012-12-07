## gaxb Overview

gaxb is one cool cookie.

## Installation

Please make sure your system has git, CMake 2.8, libxml2 and a compiler tool-chain available.

For cmake may we recommend the following on Mac OS X:

```bash
brew install cmake
```

For cmake, a tool-chain, and libxml2 may we recommend the following on Linux:

```bash
apt-get install build-essential
apt-get install cmake
apt-get install libxml2-dev
```

Now to get the code:

```bash
git clone -b cmake git://github.com/SmallPlanet/gaxb.git
cd gaxb
git submodule update --init lua
```

Add build it (out-of-source using cmake)

```bash
mkdir build && cd build
cmake ..
make
```

The executable (gaxb) will be waiting for you in the current directory!

## Usage

If you want to give it a whirl straight away (after building), try the following from the gaxb directory:

```bash
mkdir Generated
build/gaxb objc test/sample.xsd -t test/gaxb.templates/ -o Generated
```

This will generate some quick classes in Objective-C based on the schema at test/sample.xsd.

## Enjoy!