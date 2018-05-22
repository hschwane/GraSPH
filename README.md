# GraSHP
GraSPH is a Smoothed Partical Hydrodynamics code for developed for astrophysical simulation containing 
self gravitating fluid. The majority of the calculation is done on the GPU using openGL compute shader, 
which enables real time visualisation on most modern Graphics Cards. It was initially developed as part of 
my bachelor-thesis at the University of Koblenz in Germany which can be downloaded here (german): [opus](https://kola.opus.hbz-nrw.de/frontdoor/index/index/docId/1638)

You can see the code in action - simulating star formation from a molecular cloud - in the following video (which was created on a consumer notebook) [youtube](https://www.youtube.com/watch?v=PUyE3j0aoMw)

-------------------------

## usage / install

If you want to use the code yourself you will need a openGL 4.5 compatible graphics card. 
While it might work on different operating systems, I only work on Linux, so use anything else on your own risk.
To compile the code you will need openGL, GLEW, GLFW, GLM, cmake and a compiler that supports
C++17. After checking out the repository use the  following commands to compile.

```
mkdir bin
cd bin
cmake ..
make
```
The executable will be saved to ``bin/exec/GraSPH``.

The code is build around my [mpUtils](https://github.com/hschwane/mpUtils) framework, which is however included in this repository.
Files relevant to he simulation are contained in the ``exec/GraSPH`` subfolder. To change simulation settings
see the ``Settings.h`` file. To change initial conditions you have to change the code in ``main.cpp``.
More user friendly ways to change settings might be implemented in the future.