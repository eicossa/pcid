# PCID
[![Build Status](https://travis-ci.org/open-license-manager/open-license-manager.svg?branch=develop)](https://travis-ci.org/eicossa/pcid)
[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

A pc signature generator that can be used to implement copy protection or licensing software to node-lock your delivered software.

## License
The project is donated to the community. It comes with a very large freedom of use for everyone, and it will always be. 
It uses a [BSD 3 clauses](https://opensource.org/licenses/BSD-3-Clause) licensing schema, that allows free modification and inclusion in commercial software.

# How to build

make should work.

## prerequisites
 * compilsers: GCC (Linux) MINGW (Linux cross compile for Windows), MINGW or MSVC (Windows) 
 * tools/libs: cmake, boost. If target is linux openssl is necessary.

```
git clone https://github.com/eicossa/pcid.git
cd pcid/
make
cd bin
./pcid
```

# How to contribute
New ways of obtaining serial nos from motherboard, RAM, chassis etc (a la dmidecode) would be welcome. 
