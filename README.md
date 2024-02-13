# t-SNE & HSNE Analysis  [![Actions Status](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions)

t-SNE and HSNE analyis plugins for [ManiVault](https://github.com/ManiVaultStudio/core).

```bash
git clone git@github.com:ManiVaultStudio/t-SNE-Analysis.git
```

This project builds two plugins which wrap the functionality of the [HDILib](https://github.com/biovault/HDILib).

<p align="center">
  <img src="https://github.com/ManiVaultStudio/t-SNE-Analysis/assets/58806453/b179dffb-8222-4431-96a3-162c579fc149" alt="t-SNE and HSNE embeddings">
  Left: t-SNE embedding of 10k MNIST test data. Center: (top) HSNE top scale embedding of the same data, (bottom) two refinements of overlapping top-level selections. Right: HSNE setting panels.
</p>

## Compilation with a locally built HDILib
By default, during the cmake configuration, a pre-built version of HDILib will be downloaded.
You might want to compile the [HDILib](https://github.com/biovault/HDILib) locally instead. 
To use this locally compiled library, set the cmake variable `USE_ARTIFACTORY_LIBS` to `OFF` and provide `HDILIB_ROOT`, e.g. `PATH_TO_HDILib_install\lib\cmake\HDILib` for cmake to find the HDILib binaries.

On Windows, in order to manage the HDILib dependency [flann](https://github.com/flann-lib/flann), we recommend using [vcpkg](https://github.com/microsoft/vcpkg/). Set up cmake to find pacakges with vcpkg by providing the variables CMAKE_TOOLCHAIN_FILE (`PATH_TO/vcpkg/scripts/buildsystems/vcpkg.cmake`) and VCPKG_TARGET_TRIPLET `x64-windows-static`.

Tested with Ubuntu 22.10, gcc 12.2.0:
```bash
# In your local t-SNE analysis plugin folder
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DUSE_ARTIFACTORY_LIBS=OFF -DHDILIB_ROOT=/PATH/TO/YOUR/LOCALHDILIB -DMV_INSTALL_DIR=/PATH/TO/MANIVAULT
cmake --build build --config Release --target install
```
