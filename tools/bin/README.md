# Tools Binaries

In order to build the example projects, a file need to be copied into tools\bin.

A version of GNU _sed.exe_ for Windows. Version 4.7 is known to work. For arm64 platforms, an x64 build will work without issue. A pre-built version of GNU sed 4.7 can be found by following the links in point 5 [here](https://lists.gnu.org/archive/html/sed-devel/2018-12/msg00031.html). **Note:** The binary has been deployed successfully, but no information is known about its provenance beyond the fact that it is hosted on gnu.org. The binary needs to be renamed to _sed.exe_.

Alternatively, sed from mingw-w64 is known to work.