from conans import ConanFile, CMake
import os
import shutil
import pathlib
from rules_support import CoreBranchInfo


class SNEAnalysesConan(ConanFile):
    """Class to package SNE-Analyses using conan

    Packages both RELEASE and DEBUG.
    Uses rules_support (github.com/hdps/rulessupport) to derive
    versioninfo based on the branch naming convention
    as described in https://github.com/hdps/core/wiki/Branch-naming-rules
    """
    name = "SNEAnalyses"
    description = ("HDPS plugins for dimensionality reduction of data. "
                   "t-SNE and HSNE plugins are produced. ")
    topics = ("hdps", "plugin", "data", "dimensionality reduction")
    url = "https://github.com/hdps/t-SNE-Analysis"
    author = "B. van Lew b.van_lew@lumc.nl"   # conan recipe author
    license = "MIT"

    short_paths = True
    generators = ("cmake")

    # Options may need to change depending on the packaged library
    settings = {"os": None, "build_type": None, "compiler": None, "arch": None}
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": True, "fPIC": True}

    requires = (
        "qt/5.15.1@lkeb/stable",
        "hdps-core/latest@lkeb/stable"
    )

    scm = {
        "type": "git",
        "subfolder": "hdps/ImageLoaderPlugin",
        "url": "auto",
        "revision": "auto"
    }

    def set_version(self):
        # Assign a version from the branch name
        branch_info = CoreBranchInfo(self.recipe_folder)
        self.version = branch_info.version

    # Remove runtime and use always default (MD/MDd)
    def configure(self):
        if self.settings.compiler == "Visual Studio":
            del self.settings.compiler.runtime

    def system_requirements(self):
        #  May be needed for macOS or Linux
        pass

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def _configure_cmake(self, build_type):
        # locate Qt root to allow find_package to work
        qtpath = pathlib.Path(self.deps_cpp_info["qt"].rootpath)
        qt_root = str(list(qtpath.glob('**/Qt5Config.cmake'))[0].parents[3])
        print("Qt root ", qt_root)

        cmake = CMake(self, build_type=build_type)
        if self.settings.os == "Windows" and self.options.shared:
            cmake.definitions["CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS"] = True
        if self.settings.os == "Linux" or self.settings.os == "Macos":
            cmake.definitions["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"
        cmake.definitions["CMAKE_PREFIX_PATH"] = qt_root
        cmake.configure(source_folder="hdps/ImageLoaderPlugin")  # needed for scm
        cmake.verbose = True
        return cmake

    def build(self):
        print('Build OS is : ', self.settings.os)
        # If the user has no preference in HDPS_INSTALL_DIR simply set the install dir
        if not os.environ.get('HDPS_INSTALL_DIR', None):
            os.environ['HDPS_INSTALL_DIR'] = os.path.join(self.build_folder, "install")
        print('HDPS_INSTALL_DIR: ', os.environ['HDPS_INSTALL_DIR'])
        self.install_dir = os.environ['HDPS_INSTALL_DIR']

        # The ImageLoaderPlugin build expects the HDPS package to be in this install dir
        hdps_pkg_root = self.deps_cpp_info["hdps-core"].rootpath
        print("Install dir type: ", self.install_dir)
        shutil.copytree(hdps_pkg_root, self.install_dir)

        cmake_debug = self._configure_cmake('Debug')
        cmake_debug.build()

        cmake_release = self._configure_cmake('Release')
        cmake_release.build()

    def package(self):
        print('Packaging install dir: ', self.install_dir)
        self.copy(pattern="*", src=self.install_dir)

    def package_info(self):
        self.cpp_info.debug.libdirs = ['Debug/lib']
        self.cpp_info.debug.bindirs = ['Debug/Plugins', 'Debug']
        self.cpp_info.debug.includedirs = ['Debug/include', 'Debug']
        self.cpp_info.release.libdirs = ['Release/lib']
        self.cpp_info.release.bindirs = ['Release/Plugins', 'Release']
        self.cpp_info.release.includedirs = ['Release/include', 'Release']
