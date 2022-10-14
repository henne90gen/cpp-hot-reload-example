# C++ Hot Reload Example

## Build

```shell
mkdir build
cd build
cmake -G Ninja ..
ninja
```

## Usage

When running the application, it is possible to recompile the library code and the app will automatically pick up the new code.

```shell
$ ninja && ./test_executable
# ------ different terminal ------
$ ninja # -> will cause hot reloading
```
