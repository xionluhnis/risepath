# risepath
Expanding material path planning

# Build with

* [OpenFrameworks](http://http://openframeworks.cc) (version 0.8.4)
* [OpenVDB](http://http://www.openvdb.org) (version 3.0.0)

# Installation

First, install OpenVDB.
Then, install OpenFrameworks for your platform.
Finally, compile with make!

```
make && make run
```

# TODO

* Add voxel visualization
* Include drawing tools:
  * Pencil = draw directly voxels
  * Pen = polyline drawing by clicking => fill voxels
  * Eraser = remove directly voxels
  * Swirl = ???
* Visualize / specify expansion parameters (set of arrows for the directions of interest = on the side, and on the forefront)
* Generate expansion using the parameters
