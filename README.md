# Windows Driver Unit Test Framework

The Windows Driver Unit Test Framework (WDUTF) enables the unit testing of Windows kernel drivers using the [Microsoft Unit Testing Framework for C++](https://learn.microsoft.com/en-us/visualstudio/test/how-to-use-microsoft-test-framework-for-cpp?view=vs-2022), which runs in user space.

The project grew out of work being done by [DataCore](https://datacore.com/) to develop and test a world class, high-performance, storage stack running on Windows.

The scope of the project is currently limited to supporting the kernel features used by [DataCore](https://datacore.com/), but it is being made available to the community as a resource that can be developed and further extended.

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

<a id="source"></a>
## Source Code

The source code can be obtained using:
~~~{.sh}
git clone https://github.com/wpdk/wdutf
~~~

<a id="start"></a>
## Getting Started

The project is designed to be used in conjunction with [Visual Studio](https://visualstudio.microsoft.com/)
and the [Windows Driver Kit](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk).


<a id="prereq"></a>
## Runtime Prerequisites

The Windows Driver Unit Test Framework has been tested with:

* Visual Studio 2019
* Windows Driver Kit (10.0.22000.1)

It is known to work with earlier versions of the Windows Driver Kit and Visual Studio.

<a id="status"></a>
## Current Status

The project has been in use at [DataCore](https://datacore.com/) for many years. It is considered to be stable, but is currently limited to the kernel features required by DataCore's own driver stack.

<a id="limitations"></a>
## Limitations

* Minimal support for the Kernel Mode Driver Framework.

* Currently only x64 builds are supported.

<a id="issues"></a>
## Known Issues

* None known.

<a id="unit"></a>
## Unit Tests

The Windows Driver Unit Test Framework includes a set of unit test. These can be found in [test](https://github.com/wpdk/wdutf/blob/main/test).

<a id="contrib"></a>
## Contributing

Contributions are welcome and needed! In these initial stages of the project, please email the [maintainers](https://github.com/wpdk/wpdk/blob/master/MAINTAINERS.md) directly.

<a id="depend"></a>
## Dependencies

There are currently no external dependencies.

<a id="acknowledge"></a>
## Acknowledments
The Windows Driver Unit Test Framework has been developed and contributed by [DataCore](https://datacore.com/).

<a id="core"></a>
## Core Maintainers

The [core maintainers](https://github.com/wpdk/wdutf/blob/main/MAINTAINERS.md) primary responsibility is to provide technical oversight for the WDUTF Project. The current list includes:
* [Nick Connolly](https://github.com/nconnolly1), [DataCore](https://datacore.com/)
* [Robert Bassett](https://github.com/datacore-rbassett), [DataCore](https://datacore.com/)
