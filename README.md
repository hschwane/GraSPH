# myPersonalUtilities
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

- highly customisable thread safe logger
- ini-Style configuration files
- different timers
    - Timer (a simple coutdown timer you have to poll)
    - AsyncTimer (like Timer but running in a seperate thread)
    - Stopwatch (a timer that works like a stopwatch)
    - DeltaTimer (timer that calculates the time since it was last polled)
    - all timers can use the cpu_clock or std::chrono defined clocks
    
- documentation in doxygen style comments
------------------------
## planned features

- wrappers for basic openGL / window management and imGUI
- code for network communication
- code for serial communication
- jason and/or xml (probably external library)
- a argument parser for command line apps
- maybe windows and mac support
