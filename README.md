# C++ Hot Reload Example

## Build

```shell
mkdir build
cd build
cmake -G Ninja ..
ninja
```

## Usage

When running the application, it is possible to recompile the library code.
This will cause the app to automatically pick up the new code and run it.

```shell
$ ninja && ./test_executable
# ------ different terminal ------
$ ninja # -> will cause hot reloading
```
