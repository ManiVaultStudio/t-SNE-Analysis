# t-SNE & HSNE Analysis  [![Actions Status](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions)

### Using a locally built HDILib
By default, during the cmake configuration, a pre-build version of HDILib will be downloaded.
You might want to compile the [HDILib](https://github.com/biovault/HDILib) locally yourself instead. 
To use it, set the cmake variable `USE_ARTIFACTORY_LIBS` to OFF and provide `HDILIB_ROOT`, e.g. `PATH_TO_HDILib_install\lib\cmake\HDILib` for cmake to find the HDILib binaries.

On Windows, when in order to find install flann, we recommend using vcpkg. Set up cmake to find pacakges with vcpkg by providing the variables CMAKE_TOOLCHAIN_FILE (`PATH_TO/vcpkg/scripts/buildsystems/vcpkg.cmake`) and VCPKG_TARGET_TRIPLET `x64-windows-static`.


Tested with Ubuntu 22.10, gcc 12.2.0:
```bash
# In your local t-SNE analysis plugin folder
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_ARTIFACTORY_LIBS=OFF -DHDILIB_ROOT=/PATH/TO/YOUR/LOCALHDILIB -DMV_INSTALL_DIR=/PATH/TO/MANIVAULT
cmake --build build --config Release --target install
```
