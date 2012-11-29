## gaxb Overview

gaxb is one cool cookie.

## Installation

Please make sure your system has git, CMake 2.8  and a compiler tool-chain available.

For cmake may we recommend the following on Mac OS X:

```bash
brew install cmake
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