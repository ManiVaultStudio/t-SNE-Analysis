from conans import ConanFile
from conan.tools.cmake import CMakeDeps, CMake, CMakeToolchain
from conans.tools import save, load
from conans.tools import os_info, SystemPackageTool
#from conan.tools.system.package_manager import Brew
import os
import shutil
import pathlib
import subprocess
from rules_support import PluginBranchInfo
from conans import tools
import shutil

class SNEAnalysesConan(ConanFile):
    """Class to package SNE-Analyses using conan

    Packages both RELEASE and RELWITHDEBINFO.
    Uses rules_support (github.com/ManiVaultStudio/rulessupport) to derive
    versioninfo based on the branch naming convention
    as described in https://github.com/ManiVaultStudio/core/wiki/Branch-naming-rules
    """

    name = "SNEAnalyses"
    description = (
        "ManiVault plugins for dimensionality reduction of data. "
        "t-SNE and HSNE plugins are produced. "
    )
    topics = ("hdps", "ManiVault", "plugin", "data", "dimensionality reduction")
    url = "https://github.com/ManiVaultStudio/t-SNE-Analysis"
    author = "B. van Lew b.van_lew@lumc.nl"  # conan recipe author
    license = "MIT"

    short_paths = True
    # generators = "cmake"
    generators = "CMakeDeps"

    # Options may need to change depending on the packaged library
    settings = {"os": None, "build_type": None, "compiler": None, "arch": None}
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": True, "fPIC": True}

    scm = {
        "type": "git",
        "subfolder": "hdps/t-SNE-Analysis",
        "url": "auto",
        "revision": "auto",
    }

    def __get_git_path(self):
        path = load(
            pathlib.Path(pathlib.Path(__file__).parent.resolve(), "__gitpath.txt")
        )
        print(f"git info from {path}")
        return path

    def export(self):
        print("In export")
        # save the original source path to the directory used to build the package
        save(
            pathlib.Path(self.export_folder, "__gitpath.txt"),
            str(pathlib.Path(__file__).parent.resolve()),
        )

    def set_version(self):
        # print("In setversion")
        # Assign a version from the branch name
        branch_info = PluginBranchInfo(self.recipe_folder)
        self.version = branch_info.version

    def requirements(self):
        #if os.environ.get("CONAN_REQUIRE_HDILIB", None) is not None:
        #    self.requires("HDILib/1.2.6@biovault/stable")
        branch_info = PluginBranchInfo(self.__get_git_path())
        print(f"Core requirement {branch_info.core_requirement}")
        self.requires(branch_info.core_requirement)

    # Remove runtime and use always default (MD/MDd)
    def configure(self):
        # if self.settings.compiler == "Visual Studio":
        #    del self.settings.compiler.runtime
        pass

    def system_requirements(self):
        if os_info.is_macos:
            installer = SystemPackageTool()
            installer.install("libomp")
            proc = subprocess.run("brew --prefix libomp",  shell=True, capture_output=True)
            subprocess.run(f"ln {proc.stdout.decode('UTF-8').strip()}/lib/libomp.dylib /usr/local/lib/libomp.dylib", shell=True)
        if os_info.is_linux:
            self.run("sudo apt update && sudo apt install -y libtbb-dev")
            self.run("sudo ./cmake/install-lkeb-artifactory-cert.sh")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def generate(self):
        generator = None
        if self.settings.os == "Macos":
            generator = "Xcode"
        if self.settings.os == "Linux":
            generator = "Ninja Multi-Config"

        tc = CMakeToolchain(self, generator=generator)

        tc.variables["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"

        # Use the Qt provided .cmake files
        qt_path = pathlib.Path(self.deps_cpp_info["qt"].rootpath)
        qt_cfg = list(qt_path.glob("**/Qt6Config.cmake"))[0]
        qt_dir = qt_cfg.parents[0].as_posix()

        tc.variables["Qt6_DIR"] = qt_dir

        # Use the ManiVault .cmake file to find ManiVault with find_package
        mv_core_root = self.deps_cpp_info["hdps-core"].rootpath
        manivault_dir = pathlib.Path(mv_core_root, "cmake", "mv").as_posix()
        print("ManiVault_DIR: ", manivault_dir)
        tc.variables["ManiVault_DIR"] = manivault_dir

        # Set some build options
        tc.variables["MV_UNITY_BUILD"] = "ON"
        
        if os_info.is_macos:
            proc = subprocess.run("brew --prefix libomp", shell=True, capture_output=True)
            prefix_path = f"{proc.stdout.decode('UTF-8').strip()}"
            tc.variables["OpenMP_ROOT"] = prefix_path
            
        tc.generate()

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder="hdps/t-SNE-Analysis")
        cmake.verbose = True
        return cmake

    def build(self):
        print("Build OS is: ", self.settings.os)

        cmake = self._configure_cmake()
        cmake.build(build_type="RelWithDebInfo")
        cmake.build(build_type="Release")

    def package(self):
        package_dir = pathlib.Path(self.build_folder, "package")
        relWithDebInfo_dir = package_dir / "RelWithDebInfo"
        release_dir = package_dir / "Release"
        print("Packaging install dir: ", package_dir)
        subprocess.run(
            [
                "cmake",
                "--install",
                self.build_folder,
                "--config",
                "RelWithDebInfo",
                "--prefix",
                relWithDebInfo_dir,
            ]
        )
        subprocess.run(
            [
                "cmake",
                "--install",
                self.build_folder,
                "--config",
                "Release",
                "--prefix",
                release_dir,
            ]
        )

        # Add the PDB files to the Conan package
        if self.settings.os == "Windows":
            print("Copying PDBs...")
            pdb_dest = pathlib.Path(package_dir, "RelWithDebInfo", "PDBs")
            pdb_dest.mkdir()
            pdb_files = pdb_files = [p for p in pathlib.Path(self.build_folder).rglob('*') if p.is_file() and p.suffix.lower() == '.pdb']
            print("PDB(s): ", pdb_files)
            for pfile in pdb_files:
                shutil.copy(pfile, pdb_dest)
        
        self.copy(pattern="*", src=package_dir)

    def package_info(self):
        self.cpp_info.relwithdebinfo.libdirs = ["RelWithDebInfo/lib"]
        self.cpp_info.relwithdebinfo.bindirs = ["RelWithDebInfo/Plugins", "RelWithDebInfo"]
        self.cpp_info.relwithdebinfo.includedirs = ["RelWithDebInfo/include", "RelWithDebInfo"]
        self.cpp_info.release.libdirs = ["Release/lib"]
        self.cpp_info.release.bindirs = ["Release/Plugins", "Release"]
        self.cpp_info.release.includedirs = ["Release/include", "Release"]
