## A front shell in miniob

open in codespace and type:

```sh
mkdir build && cd build

vcpkg integrate install

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake ..

make -j2
```