C++ FMUs
========

This is a repository of FMUs written in plain C++ using the [CPPFMU] framework.


How to build
------------
**NOTE:** After cloning the repository for the first time, remember to
initialise and update the CPPFMU submodule:

    git submodule init
    git submodule update

The build procedure is fairly standard CMake.  First, create a build directory
somewhere.  Here, we'll assume that it is created in the top-level source
directory:

    mkdir build
    cd build

Then, run CMake twice, once to genereate the buildsystem and once to build the
code:

    cmake ..
    cmake --build .

Assuming everything went well, the generated FMUs will be located in the `build`
directory.


Checking FMU compliance
-----------------------
It's always a good idea to use the [FMU Compliance Checker] to verify that FMUs
are FMI compliant. To activate a set of tests that run the FMU checker on each
generated FMU, enable `FMU_CHECK_COMPLIANCE` option.  If the Compliance Checker
is not installed in a standard location, you may also have to specify where to
find it.  Here is an example:

    cmake -DFMU_CHECK_COMPLIANCE=ON -DCMAKE_PREFIX_PATH=/opt/fmu_checker ..
    cmake --build .
    ctest


Multi-platform FMUs
-------------------
The `tools/MergeFMUs.cmake` script is designed to merge FMUs compiled for
different platforms into one multi-platform FMU.

The most straightforward way to compile the FMUs for different platforms is to
actually perform the compilation on the platforms in question, and then gather
the FMUs in one place for merging. Cross compilation is another, albeit more
involved, possibility.

It is usually easy to compile for different processor architectures (i.e., 32-
and 64-bit processors) on a single platform, though.

With GCC and CMake's Makefile generator, the default target architecture is
usually the one the operating system was built for.  To compile for a different
one, ensure that you have installed the `g++-multilib` package, and then set
the CMake variable `CMAKE_CXX_FLAGS` to `-m32` or `-m64` in the generation
command in the previous section.  Here is how to build for both architectures
on a 64-bit machine:

    mkdir build64 build32
    cd build64
    cmake ..
    cmake --build .
    cd ../build32
    cmake -DCMAKE_CXX_FLAGS=-m32 ..
    cmake --build .

With the Visual Studio generators, the default is usually to build in 32-bit
mode regardless of OS architecture.  To switch to 64-bit mode, use the `-A`
option:

    mkdir build32 build64
    cd build32
    cmake ..
    cmake --build .
    cd ../build64
    cmake -A x64 ..
    cmake --build .

Once the various platform- and/or architecture-specific FMUs have been built,
they can be merged into one using the `MergeFMUs` script like in this example:

    cmake "-DINPUT_FMUS=build32/foo.fmu;build64/foo.fmu;/mnt/winbuild/foo.fmu" -DOUTPUT_FMU=foo.fmu -P tools/MergeFMUs.cmake


[CPPFMU]: https://github.com/viproma/cppfmu
[FMU Compliance Checker]: https://github.com/modelica-tools/FMUComplianceChecker
