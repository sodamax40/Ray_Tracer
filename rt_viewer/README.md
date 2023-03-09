# Ray Tracing Viewer (1TD388/1MD150)

This code skeleton is used as starting point for the ray tracing project in the computer graphics courses 1TD388 and 1MD150 at Uppsala University. The project loosly follows the structure of the "Ray Tracing in a Weekend" [book](https://raytracing.github.io/books/RayTracingInOneWeekend.html), but starts from Chapter 6 in the book. To solve the tasks for the project (see course page in Studium), you mainly need to modify the files main.cpp and rt_raytracing.h/cpp, but you might have to modify some of the other .h files as well.


## Build instructions for Linux

You need the following tools, which can be installed via the package manager of your Linux distribution:

    gcc/g++, CMake 3.5 (or higher), make, git

In addition, you also need to install libraries for OpenGL. On for example Ubuntu 18.04, you can install them with

    sudo apt install libgl1-mesa-dev libxmu-dev libxi-dev libxrandr-dev libxcursor-dev libxinerama-dev


### Environment variables

Before building and running the scene viewer, the environment variable `RT_VIEWER_ROOT` must be set to point to the directory of this `README.md` file. Example:

    export RT_VIEWER_ROOT=$HOME/rt_viewer


### Building and running the program

To build the program, open a terminal, navigate to
the path in `RT_VIEWER_ROOT`, and type

    mkdir build
    cd build
    cmake ../
    make

To run the program, navigate to the resulting executable (rt_viewer), and type

    ./rt_viewer

Alternatively, run the attached Bash script `build.sh` that will perform all these steps for you:

    ./build.sh

Note: You do not have to run CMake every time you change something in the source files. Just use the generated makefile (or the `build.sh` script) to rebuild the program.


## Build instructions for Windows

We recommend using Visual Studio Community 2019 and CMake GUI 3.14 (or higher) for building the code on Windows. Using WSL on Windows is currently not recommended, because of poor support for graphical applications.

Note: When installing Visual Studio, remember to check the box for "Desktop development with C++" under Workloads, otherwise no C++ tools will be installed (and CMake will complain about not finding any compiler). 


### Environment variables

Define a user environment variable `RT_VIEWER_ROOT` that points to the directory of this `README.md` file. On Windows 10, you can create and change environment variables by going to the start menu and searching for `Edit environment variables for your account`.


### Building and running the program

Before you start CMake or Visual Studio or do anything else, set the `RT_VIEWER_ROOT` variable as described above.

1. Open the CMake GUI.
2. Specify the search path to the scene viewer root directory in the "Where is the source code:" text box (or select that directory from the GUI).
3. Create a subdirectory `RT_VIEWER_ROOT\build`, and specify the search path to this directory in the "Where to build the binaries:" text box.
4. Press the "Configure" button and choose the compiler (Visual Studio XX 20XX Win64).
5. Press the "Configure" button again (you need to do this twice).
6. Press the "Generate" button to generate Visual Studio project
   files.
7. Close the CMake GUI.
8. Navigate to the `RT_VIEWER_ROOT\build` directory and
   open the `rt_viewer.sln` file in Visual Studio.
9. Right-click on the Part1 project and choose Set as StartUp Project
10. Press F5 to compile and run the program. The resulting executable (`rt_viewer.exe`) will be placed in `RT_VIEWER_ROOT\build\Debug`

Note 1: The CMake GUI cannot expand environment variables in the text boxes, so you need to specify the full absolute path in the text boxes even if you have set `RT_VIEWER_ROOT`.

Note 2: Every time you change the `RT_VIEWER_ROOT` variable you have to close all open command prompts and restart CMake (since CMake will not reload the values of environment variables automatically). In rare cases, it might be necessary to reboot the system to update the environment variable.

Note 3: CMake can have problem handling path names that contain non-ASCII (international) characters. Assuming that you don't want to change your language settings, a simple fix is to move the gltf_viewer folder to a folder with only ASCII symbols in it's full path.


## Build instructions for macOS

Should be roughly the same procedure as for Linux. You can use [brew](https://brew.sh/) or some other package manager to install the required tools (gcc, CMake, git, etc.) and then use CMake to generate Unix makefiles.

Since none of the lab instructors in the course are Mac users, we can only provide limited instructions for this platform.


## Usage from the command line

Linux/macOS:

    ./rt_viewer

Windows:

    rt_viewer.exe


## Third-party dependencies

The application depends on the following third-party libraries, which are included in the `external` folder and built from source code during compilation:

- GLEW (https://github.com/nigels-com/glew)
- GLFW v3.1.1 (https://github.com/glfw/glfw)
- GLM v0.9.5.4 (https://github.com/g-truc/glm)
- ImGUI v1.50 (https://github.com/ocornut/imgui)
- lodepng v20130128 (https://github.com/lvandeve/lodepng)


### Code style

This code uses the WebKit C++ style (with minor modifications) and clang-format (version 6.0) for automatic formatting.
