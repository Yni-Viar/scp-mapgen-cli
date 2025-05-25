# SCP Facility-like map generator

## About

SCP-like facility map generator, ported from Godot 4 edition (v8.1.0)

## Other editions

[Godot 4 version](https://github.com/Yni-Viar/scp-mapgen-godot)



## License?
MapGenCore.* -[MIT License](/LICENSE.MIT)
  - If your project is licensed under CC-BY-SA 3.0, CC-BY-SA 4.0 or GPL 3 (e.g. *SCP - Containment Breach* remake), the Author grants You permission to relicense the code under mentioned licenses.

astar.h is made by bensuperpc and is licensed under [MIT license](/thirdparty/LICENSE.txt).

Scons file have Godot Engine code, which is also licensed under MIT license (see file)

## What works:
- [x] Random generation (NOT Layout based)
- [x] 2D generator.
- [x] Checkpoint support (NEW!) *Note, that these checkpoints work differently from Containment Breach ones*
- [x] Many zone support (both in x and y directions) (currently, there is a limit of 512 rooms in a single generator node, you can increase it in code, but this may affect the performance (especcialy in 3D))

## Building

**C++ 20 is required**, because of astar.h .

if you have Scons and Python 3 installed (paste in your terminal):

`scons`

If you do not have these programs installed, there is quick install scripts (paste in your terminal)

|Windows|*nix|
|----|----|
|`python -m pip install scons`|`python3 -m pip install scons`|

Then launch in your terminal

`scons`