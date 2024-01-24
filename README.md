# t-SNE & HSNE Analysis  [![Actions Status](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions)

### Build locally on Linux

Tested with Ubuntu 22.10, gcc 12.2.0

Compile the [HDILib](https://github.com/biovault/HDILib) first, and set the position independent code flag. Then link the t-SNE and HSNE plugin to the HDILib.
```
# In your local HDILib folder
apt install libflann-dev
mkdir build 
cd build
cmake -DCMAKE_BUILD_TYPE=Release -Dflann_INCLUDE_DIR=/urs/include/ -DENABLE_PID="ON" -DCMAKE_INSTALL_PREFIX=/PATHTOYOUR/LOCALHDILIB/INSTALLFOLDER ..
make && make install
```

```
# In your local t-SNE analysis plugin folder
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_ARTIFACTORY_LIBS=OFF -DHDILIB_ROOT=/PATHTOYOUR/LOCALHDILIB/INSTALLFOLDER
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release
```
