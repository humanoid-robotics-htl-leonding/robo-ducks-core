import abc
import logging
import os
import shutil
import subprocess
from abc import ABC
from enum import Enum
from typing import List, Dict, Type

project_root_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))


class CompileTarget(ABC):
    name = None
    id = None
    foldername = None

    def check_toolchain_installed(self):
        pass


class Nao5CompileTarget(CompileTarget):
    name = "Nao V5"
    id = "nao5"
    foldername = "nao5"
    # -DNAO_V5=ON -DNAOLIB=ON -DTOOLCHAIN_DIR="${TOOLCHAIN_DIR}"
    setup_args = ["-DNAO_V5=ON", "-DNAOLIB=ON", f"-DTOOLCHAIN_DIR='{project_root_dir}/toolchain'"]

    def check_toolchain_installed(self):
        hulks_toolchain = os.path.join(project_root_dir, "toolchain", "hulks-v5")
        naoqi_toolchain = os.path.join(project_root_dir, "toolchain", "naoqi-v5")
        if not os.path.exists(hulks_toolchain):
            raise RuntimeError("For the compilation of v5 the hulks-toolchain"
                               " is needed but missing (has to be located in"
                               "'[project]/toolchain/hulks-v5')")
        if not os.path.exists(naoqi_toolchain):
            raise RuntimeError("For the compilation of v5 the naoqi-toolchain"
                               " is needed but missing (has to be located in"
                               "'[project]/toolchain/naoqi-v5')")


class Nao6CompileTarget(CompileTarget):
    name = "Nao V6"
    id = "nao6"
    foldername = "nao6"
    setup_args = ["-DNAO_V6=ON",  f"-DTOOLCHAIN_DIR='{project_root_dir}/toolchain'"]

    def check_toolchain_installed(self):
        hulks_toolchain = os.path.join(project_root_dir, "toolchain", "hulks-v5")
        if not os.path.exists(hulks_toolchain):
            raise RuntimeError("For the compilation of v6 the hulks-toolchain"
                               " is needed but missing (has to be located in"
                               "'[project]/toolchain/hulks-v6')")


class SimrobotCompileTarget(CompileTarget):
    name = "Simrobot"
    id = "simrobot"
    foldername = "simrobot"
    setup_args = ["-DREPLAY=ON"]


class BuildType:
    name = None
    id = None
    foldername = None


class DebugBuildType(BuildType):
    name = "Debug"
    id = "debug"
    foldername = "Debug"


class DevelopBuildType(BuildType):
    name = "Develop"
    id = "develop"
    foldername = "Develop"


class ReleaseBuildType(BuildType):
    name = "Release"
    id = "release"
    foldername = "Release"


Targets: Dict[str, Type[CompileTarget]] = {
    Nao5CompileTarget.id:  Nao5CompileTarget,
    Nao6CompileTarget.id:  Nao6CompileTarget,
    SimrobotCompileTarget.id:  SimrobotCompileTarget,
}

BuildTypes: Dict[str, Type[BuildType]] = {
    DevelopBuildType.id: DevelopBuildType,
    DebugBuildType.id: DebugBuildType,
    ReleaseBuildType.id: ReleaseBuildType
}


def get_build_dir(target: CompileTarget, build: BuildType):
    return os.path.join(project_root_dir, "build", target.foldername, build.foldername)


class Compiler:
    def __init__(self, target: CompileTarget, build: BuildType = None):
        if build is not None:
            self.build_type = build()
        else:
            self.build_type = None
        self.target = target()

    def compile(self):
        build_dir = os.path.join(project_root_dir, "build", self.target.foldername, self.build_type.foldername)
        cmake_cache_path = os.path.join(build_dir, "CMakeCache.txt")
        if not os.path.exists(cmake_cache_path):
            raise RuntimeError("Please execute setup before compile. (Missing CMakeCache.txt for target)")

        used_cpus = os.cpu_count() - 1
        logging.info(f"Using {used_cpus} Threads (-j{used_cpus})")
        subprocess.run(["make", f"-j{used_cpus}"], cwd=build_dir)

    def setup(self):
        self.target.check_toolchain_installed()
        logging.info("Toolchains seem to be installed.")

        for _, build_type in BuildTypes.items():
            logging.info(f"Setting up '{build_type.name}' for {self.target.name}")
            build_dir = get_build_dir(self.target, build_type())

            if os.path.exists(build_dir):
                shutil.rmtree(build_dir)
            os.makedirs(build_dir)

            # cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" "$@" "${BASEDIR}"

            other_args = self.target.setup_args

            #"-G Unix Makefiles",

            subprocess.run(["cmake", f"-DCMAKE_BUILD_TYPE={build_type.name}", *other_args, project_root_dir], cwd=build_dir)
