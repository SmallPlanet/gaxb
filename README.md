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

If you want to give it a whirl straight away (after building), try the following from the Examples/Sample directory:

```bash
mkdir Generated
../../build/gaxb objc sample.xsd -t ../../templates -o Generated
```

This will generate some quick Objective-C classes in Generated/ based on the schema at sample.xsd.

## Example Project

A sample XCode project for an iOS app using gaxb-generated classes is included in Examples/BigPlanet. Open BigPlanet.xcodeproj from that directory in XCode, select iPhone or iPad Simulator, then Build and Run. This simple example uses the schema located at XMLSchema/Planets.xsd and loads sample data in BigPlanets/sol.xml.  

## License

Copyright (c) 2012 Small Planet Digital, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Enjoy!
