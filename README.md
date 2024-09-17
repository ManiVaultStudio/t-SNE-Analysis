# t-SNE & HSNE Analysis  [![Actions Status](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/t-SNE-Analysis/actions)

t-SNE and HSNE analysis plugins for [ManiVault](https://github.com/ManiVaultStudio/core).

```bash
git clone git@github.com:ManiVaultStudio/t-SNE-Analysis.git
```

This project builds two plugins which wrap the functionality of the [HDILib](https://github.com/biovault/HDILib):
- t-SNE: fast embedding computation with the GPU-based [A-tSNE](https://doi.org/10.1109/TVCG.2016.2570755)
- HSNE: hierarchical embeddings with [Hierarchical Stochastic Neighbor Embedding](https://doi.org/10.1111/cgf.12878)

<p align="center">
  <img src="https://github.com/ManiVaultStudio/t-SNE-Analysis/assets/58806453/b179dffb-8222-4431-96a3-162c579fc149" alt="t-SNE and HSNE embeddings">
  Left: t-SNE embedding of 10k MNIST test data. Center: (top) HSNE top scale embedding of the same data, (bottom) two refinements of overlapping top-level selections. Right: HSNE setting panels.
</p>

## Compilation with a locally built HDILib
By default, during the cmake configuration, a pre-built version of HDILib will be downloaded.
You might want to compile the [HDILib](https://github.com/biovault/HDILib) locally instead. 
To use this locally compiled library, set the cmake variable `MV_SNE_USE_ARTIFACTORY_LIBS` to `OFF` and provide `HDILIB_ROOT`, e.g. `PATH_TO_HDILib_install\lib\cmake\HDILib` for cmake to find the HDILib binaries.

On Windows, in order to manage the HDILib dependency [flann](https://github.com/flann-lib/flann), we recommend using [vcpkg](https://github.com/microsoft/vcpkg/). Set up cmake to find packages with vcpkg by providing the variables CMAKE_TOOLCHAIN_FILE (`PATH_TO/vcpkg/scripts/buildsystems/vcpkg.cmake`) and VCPKG_TARGET_TRIPLET `x64-windows-static-md`. See the [HDILib](https://github.com/biovault/HDILib) Readme for more details.

Tested with Ubuntu 22.10, gcc 12.2.0:
```bash
# In your local t-SNE analysis plugin folder
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMV_SNE_USE_ARTIFACTORY_LIBS=OFF -DHDILIB_ROOT=/PATH/TO/YOUR/LOCALHDILIB -DManiVault_DIR=/PATH/TO/MANIVAULT
cmake --build build --config Release --target install
```

## Notes on settings

- Exaggeration factor: Defaults to `4 + number of points / 60'000`
- Initialization:
  - Defaults to random. Optional: Use another data set as the initial embedding coordinates, e.g. the first two [PCA](https://github.com/ManiVaultStudio/PcaPlugin/) components.
  - Defaults to rescaling the initial coordinates such that the first embedding dimension has a standard deviation of 0.0001. If turned off, the random initialization will uniformly sample coordinates from a circle with radius 1.
  - See e.g. [The art of using t-SNE for single-cell transcriptomics](https://doi.org/10.1038/s41467-019-13056-x) for more details on recommended t-SNE settings
- Gradient Descent:
  - GPU-based implementation (default) requires OpenGL 3.3 and benefits from compute shaders (introduced in OpenGL 4.4 and not available on Apple devices)
  - CPU-based implementation of [Barnes-Hut t-SNE](https://jmlr.org/papers/v15/vandermaaten14a.html) automatically sets θ to `min(0.5, max(0.0, (numPoints - 1000.0) * 0.00005))`
  - Changes to gradient descent parameters are not taken into account when "continuing" the gradient descent, but when "reinitializing" they are
- kNN (specify search structure construction and query characteristics):
  - (Annoy) Trees & Checks: correspond to `n_trees` and `search_k`, see their [docs](https://github.com/spotify/annoy?tab=readme-ov-file#tradeoffs)
  - (HNSW): M & ef: are detailed in the respective [docs](https://github.com/nmslib/hnswlib/blob/master/ALGO_PARAMS.md#hnsw-algorithm-parameters)
- HSNE:
  - The number of scales includes the data scale, i.e., a setting of 2 scales indicates one abstraction scale above the data scale. Specifying 1 scale will not compute any abstraction level.
