#Future work and known Issues

## bugs / issues
- finding the new timestep has brobably issues
- gravitational softening needs to be improved
- find more stable way to adjust smoothing length

## physics / accuracy
- option to use 64bit floating point numbers
- fix energy/entropy conservation
- thermal energy / radiation
- viscous heating
- magnetic fields
- star feedback and model for internal evolution of the star (probably add plugin interface)
- handle soft and solid bodys, damage and fragmentation (see C. Schäfer)
- other time integration algorithms / other methods for adaptive timestep calculation

## performance
- enable pre-simulated simulation
- add datastructure / tree code
- have a list of neighbors for each particle
- individual timesteps
- allow for particles to merge (maybe even split)
- multi-GPU simulation
- have the CPU also do some work
- actually do reductions instead of iterative implementation in accumulator

## output and visualisation and interpretation
- find stars
- make star visualisation better
- make gas visualisation better
- have the camera follow a star/structure
- save simulation to usable data format (see "enable pre-simulated simulation")

## usability and debugging
- print particles to readable file for debug
- have code a gui or even an editor for initial conditions
- save / load initial conditions to file
- use key callbacks instead of polling

## other
- some classes for managing simulation settings, shader compillation and shader dispatch
- add gpu stopwatch for shader profiling
- 2D mode