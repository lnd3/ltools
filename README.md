# ltools

This repo consist of a collection of utilities and general purpose c++ tools useful for various applications related to opengl rendering, physics simulation, network tools, multithreading and a variety of general purpose functions for string manipulation and logging. It comes with a custom test suite and cmake build system (`bs`).
The repo utilizes dependencies from `ldeps`.

## Platform support
Built regurlarly with msvc 19 (VS2022) and gnu 11.4 (ubuntu & wsl). Targets only c++20 in `packages` but `ldeps` has both c and other version of c++.
