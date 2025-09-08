# NiCeTy
NiCeTy is a lightweight command-line project manager for C projects.
It helps you quickly initialize, build, test, and run C projects with a simple configuration system.

## Features
- Project directory initialisation with boilerplate files & directories
- Intuitive build receipe in C
- Argument passing to build, test and run commands
- Command to initiate tests for the project
- Simple `.nct/.nct` config file stores build/test/run commands and timestamps
- Cross-platform friendly (non shell commands)
- Argument passing to compiled binaries

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
This compiles and runs `build.c` which, by default, compiles `src/main.c` into `bin/project`. 
Automatically recompiles if the build source was modified.
### Run the binary
```
nct run
```
This, by default, runs `bin/project`.
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
    │    ├──.nct
    │
    ├── bin
    │
    ├── src
    │    ├── common.h
    │    └── main.c
    │
    ├── build.c
    │
    └── test.c
```
