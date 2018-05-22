# mpUtils
a ~~platform independent~~ linux c++ library with some utility classes I use for my personal projects and university assignments

-------------------------

## usage / install

#### as a framework

You can create your own projects as subfolders inside the "exec"-folder. Then you only have to provide a CMakeLists.txt
in your subfolder which handles compilation and links with ``mpUtils``. See the existing executables for reference.
To build use any cmake compatible IDE or your console:
```
mkdir bin
cd bin
cmake ..
make
```
In ``bin/exec/yorExecutable`` should now wait your compiled project.



#### as a library

You can install this library to your systems lib and include directories:
```
mkdir bin
cd bin
cmake ..
make 
sudo make install
```
Now set up you own project and link mpUtils as a shared library

--------------------------
## features

- wrapper for some  openGl (using GLEW) and glfw functionality
- utilities that help with graphical programming
- highly customisable thread safe logger
- ini-Style configuration file parser
- different timers including asynchronous ones
- doxygen style documentation

------------------------
## planned features

- wrapper for the remaining openGL features
- more utilities for graphics programming (maybe game engine style functionality)
- include a GUI
- network communication
- serial communication
- jason and/or xml
- a argument parser for command line apps
- windows and mac support
- improvements to the log (lock-free-queue, more sinks with better format, different depths for massages)
