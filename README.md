# Windows Driver Unit Test Framework

The Windows Driver Unit Test Framework (WDUTF) enables the unit testing of Windows kernel drivers using the [Microsoft Unit Testing Framework for C++](https://learn.microsoft.com/en-us/visualstudio/test/how-to-use-microsoft-test-framework-for-cpp?view=vs-2022), which runs in user space.

Running unit tests against code written for the Windows Kernel environment is hard. There have been a few projects that try to embed a unit test framework in the kernel, but they suffer from the compexity of having to load a driver, collect the output and handle crashes. We wanted to do something better; to create a user space environment where kernel code can run unmodified and tests can be examined using the Visual Studio debugger.

The initial version of the project consisted of a user space library implementing portions of the kernel infrastructure. This has now been extended to allow binary code built for the kernel to be loaded into the unit test environment and executed. Much of the kernel functionality is stubbed out, but the framework is sufficient to allow credible tests to be performed.

Our experience has been that bringing the simplicity and flexibility of user space development to kernel drivers can significantly reduce development time and improve project outcomes. The project is being contributed to the [Windows Platform Development Kit](https://wpdk.github.io/) to enable others to benefit from it.

[rtegrity](https://rtegrity.com/) is leading the ongoing development of the project, which grew out of work done by [DataCore](https://datacore.com/) to develop and test a world class, high-performance, storage stack running on Windows.

# In this ReadMe

* [Documentation](#documentation)
* [Source Code](#source)
* [Getting Started](#start)
* [Runtime Prerequisites](#prereq)
* [Current Status](#status)
* [Limitations](#limitations)
* [Known Issues](#issues)
* [Unit Tests](#unit)
* [Examples](#examples)
* [Contributing](#contrib)
* [Dependencies](#depend)
* [Acknowledgements](#acknowledge)
* [Core Maintainers](#core)

<a id="documentation"></a>
## Documentation

Further information about the [design and implementation](https://github.com/wpdk/wdutf/blob/main/doc/design.md)
can be found in the [project documentation](https://github.com/wpdk/wdutf/blob/main/doc). This is still a work in progress.

Basic sample code demonstrating the use of the unit test framework can be found in [examples](https://github.com/wpdk/wdutf/blob/main/examples). The unit tests in [test](https://github.com/wpdk/wdutf/blob/main/test) provide more advanced examples of using the framework.

<a id="source"></a>
## Source Code

The source code can be obtained using [git](https://git-scm.com/):
~~~{.sh}
git clone --recurse-submodules https://github.com/wpdk/wdutf
~~~

<a id="prereq"></a>
## Runtime Prerequisites

The Windows Driver Unit Test Framework has been tested on both x64 and arm64 with [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/). 

The following Visual Studio workloads need to be installed:

* Desktop development with C++
* Windows application development
* Linux and embedded development with C++

The following Visual Studio individual components are also needed (for either x64 or arm64):

* MSVC v143 - VS 2022 C++ with Spectre Mitigations
* C++ ATL for latest build tools
* C++ ATL for latest build tools with Spectre Mitigations
* C++ MFC for latest build tools
* C++ MFC for latest build tools with Spectre Mitigations
* MSVC v140 - VS 2015 build tools

The following are also required:

* [Windows Driver Kit](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
(10.0.26100.1)
* [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) (10.0.26100.1)
* [Microsoft Visual C++ 2022 Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170)

The Windows Driver Unit Test Framework is known to work with earlier versions of the Windows Driver Kit and Visual Studio.

In order to build the example projects, a couple of executables need to be copied into tools\bin. The [tools\bin\README](https://github.com/wpdk/wdutf/blob/main/tools/bin/README.md) contains details.

<a id="start"></a>
## Getting Started

The project is designed to be used in conjunction with [Visual Studio](https://visualstudio.microsoft.com/)
and the [Windows Driver Kit](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk).

Firstly, build the Detours library by opening a Developer Command Prompt for Visual Studio and running the following commands:

~~~{.sh}
# Only on arm64: Ensure that the target is correctly set
VsDevCmd -arch=arm64

cd detours\src
nmake
~~~

The Windows Driver Unit Test Framework project can then be built using
the Visual Studio solution WDUTF.sln.

<a id="status"></a>
## Current Status

The project has been in use for many years. It is considered to be stable, but is currently limited to the kernel features required by DataCore's own driver stack.

<a id="limitations"></a>
## Limitations

The scope of the project is currently limited to supporting the kernel features used by DataCore drivers, but it is being made available to the community as a resource that can be developed and further extended.

In particular:

* Minimal support for the Kernel Mode Driver Framework.

<a id="issues"></a>
## Known Issues

* None.

<a id="unit"></a>
## Unit Tests

The Windows Driver Unit Test Framework includes a set of unit test to validate the framework. These can be found in [test](https://github.com/wpdk/wdutf/blob/main/test).

<a id="contrib"></a>
## Contributing

Contributions are welcome and needed! In these initial stages of the project, please email the [maintainers](https://github.com/wpdk/wpdk/blob/master/MAINTAINERS.md) directly.

<a id="depend"></a>
## Dependencies

There are currently no external dependencies.

<a id="acknowledge"></a>
## Acknowledgements
[rtegrity](https://rtegrity.com/) is leading the ongoing development of the Windows Driver Unit Test Framework, which has been contributed by [DataCore](https://datacore.com/).

<a id="core"></a>
## Core Maintainers

The [core maintainers](https://github.com/wpdk/wdutf/blob/main/MAINTAINERS.md) primary responsibility is to provide technical oversight for the WDUTF Project. The current list includes:
* [Nick Connolly](https://github.com/nconnolly1), [rtegrity](https://rtegrity.com/)
* [Robert Bassett](https://github.com/datacore-rbassett), [DataCore](https://datacore.com/)
