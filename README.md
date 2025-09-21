<center>
    <img src="assets/nct_logo.png" width="200">
    <h1>NCT</h1>
</center>

NCT (NiCeTy or Nice C Tea) is a lightweight command-line project manager for C projects. It helps you quickly initialize, build, and test C projects with a configuration system and build utilities.

## Features
- Quick project initialization with boilerplate files & directories
- Intuitive C-based build recipes that helps you write your build logic in C, not shell scripts
- Built-in build utilities via build.h including:
    - Dependency fetching from URLs
    - Incremental compilation (compile only when source changes)
    - File operations (copy, exists, directory creation)
- Automatic recompilation of build recipe
- Simple configuration via `.nct/.nct` config file
- Command to initiate tests for the project
- Argument passing to build and test commands

## Why NCT?
- Simple: No complex build systems or DSLs to learn
- Flexible: Build scripts are just C code. Opportunity to use better C build recipe systems like [nob.h](https://github.com/tsoding/nob.h)
- Fast: Incremental builds and minimal overhead
- Self-contained: Each project includes its build utilities

## User guide
### Initialize a new project
```
nct init project
cd project
```
This creates a folder `project/` with boilerplate source, header, build and test files.
### Build the project
```
nct build
```
This compiles and runs `build.c` which, by default, compiles `src/main.c` into `build/project`. 
Automatically recompiles if the build source was modified.
### Run tests
```
nct test
```
Compiles and runs `test.c`. Automatically recompiles if the test source was modified.

## Project Structure:
```
<project_name>
    │
    ├──.nct
    │    └──.nct
    ├── build
    ├── lib
    ├── src
    │    └── main.c
    ├── build.c
    ├── build.h
    └── test.c
```

## Todo
- Add windows support