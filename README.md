<div align="center">
    <img src="assets/LOGO.png", width="200"/>
    <h1>NCT</h1>
</div>

NCT (NiCeTy or Nice C Tea) is a lightweight command-line project manager for C projects. It helps you quickly initialize, build, and test C projects using a simple, standardized structure and C-based build recipes.

## Features
- Quick project initialization with boilerplate files & directories
- Intuitive C-based build recipes that helps you write your build logic in C, not complex DSLs or shell scripts
- A built-in build.h provides dependency fetching, incremental builds, file operations, make-like build system
- Cross platform (Windows and Unix)
- Automatic recompilation of build recipe
- Argument passing to build and test commands
- Meta build configuration via `.nct/.nct` config file
- A dedicated command for running tests

## Why NCT?
NCT is a project manager, not a build system. Its goal is to provide a simple, standard, and convenient workflow for C projects.

- Standardization: Every NCT project has the same structure and the same commands (nct build, nct test). No more deciphering custom Makefile rules or README build instructions for every new project.
- Convenience: It provides a full workflow, from project initialization with nct init to a built-in utility library (build.h).
- Cross Platform: make is not a standard part of Windows development. NCT provides a consistent command-line experience on any platform.
- Flexibility: While NCT provides build.h as a starting point, your build script is just C. You have the full power of the language and are free to use any library you want, including more advanced build systems like [nob.h](https://github.com/tsoding/nob.h). The .nct config files allow you to integrate any toolchain or workflow.
- Familiar, Make-like Rules: Use the familiar and powerful rules to define your build graph.

NCT's purpose is to bring the simple "one command" experience to the C ecosystem without sacrificing power or flexibility.

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

