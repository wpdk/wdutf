# Design and Implementation

## Design Principles

The project conforms to the following design principles:

* A lightweight layer that runs as native Windows code, that can be tested independently.
* The primary focus is to enable unit testing of Windows kernel drivers in user space.

## Overview

The initial version of the project consisted of a user space library implementing portions of the kernel infrastructure. This allows existing kernel code to be compiled using the unit test framework and then tested.

This works well, but we wanted to do something better; we wanted to create a user space environment where the binary driver that is going to be used in production could be loaded, tested and examined with the Visual Studio debugger.

There are three major challenges associated with implementing a unit test framework like this:

* Providing the required definitions and symbols
* Handling low level implementation details
* Providing access to code in the driver under test

## Definitions and Symbols

The unit test framework depends upon the Windows Driver kit include files to provide definitions. Include paths are changed using _#pragma include_alias_ to redirect to a wrapper around each header file. This ensures that the correct definitions are used, without the need to copy or duplicate content. It also allows the code under test to work with the exact same binary structures that the kernel would provide (e.g. DEVICE and DRIVER objects).

One downside of this approach is that each release of the Windows Driver Kit may require updates to the wrappers, but in practice only minor adjustments have been necessary.

Kernel functionality has been implemented based on the Windows Driver Kit documentation.

## Low level Details

Windows device drivers are .sys files which cannot be loaded directly into a user space application. The framework includes a _load_ operation that will copy the driver, rename it to a _.dll_ and make minor changes in order for it to be loadable.

Various operations, such as privileged instructions, are then resolved at run-time through the use of exception handling. Fortunately, these tend to occur at low frequency.

## Access to Code

In general, a Windows driver has no externally visible symbols. This creates challenges accessing the code to be unit tested. This issue can be overcome in the following ways:

1. Compile and link the code as part of building the unit tests. This ensures that all symbols are available, but doesn't allow testing of the production images.
2. Relink the driver with an new _.def_ file to export the symbols required for unit testing. The unit tests can then be linked directly against this using _delay load_ to create an external reference that will be satisfied when the framework loads the driver.
3. Use the Microsoft Detours library to provide access to the private symbols of the driver under test. The macros TEST_FIND_FUNCTION, TEST_FIND_METHOD and TEST_FIND_VARIABLE defined in UnitTest.h implement this.

The Microsoft Detours library also allows the behaviour of functions in the driver to be changed by intercepting calls on a per-thread basis. The macro definitions for TEST_DEFINE_MOCK_* in UnitTest.h and test\DetoursTest.cpp provide examples.
