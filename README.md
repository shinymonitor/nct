# NiCeTy
NiCeTy is a lightweight command-line project manager for C projects.
It helps you quickly initialize, build, test, and run C projects with a simple configuration system.

## Features
- **init** : create a new project with 
boilerplate files & directories
- **build** : compile your project using `build.c`
- **run** : run the compiled binary
- **test** : compile & run `test.c`
- **run** : run the compiled binary
- Simple `.nct/.nct` config file stores build/test/run commands and timestamps
- Cross-platform friendly (non shell commands)

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
nct run
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

## Requirements

- GCC (default, but configurable in build.c)
- POSIX-like environment (Linux/macOS). Windows may need adjustments.

## Build
```
gcc nct.c -o nct -Wall -Wextra -Werror
```

