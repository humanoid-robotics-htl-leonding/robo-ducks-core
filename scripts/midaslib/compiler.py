import logging
import os
import subprocess
from enum import Enum


class CompileTarget:
    name = None
    id = None
    foldername = None


class Nao5CompileTarget(CompileTarget):
    name = "Nao V5"
    id = "nao5"
    foldername = "nao5"


class Nao6CompileTarget(CompileTarget):
    name = "Nao V6"
    id = "nao6"
    foldername = "nao6"


class SimrobotCompileTarget(CompileTarget):
    name = "Simrobot"
    id = "simrobot"
    foldername = "simrobot"


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


Targets = {
    Nao5CompileTarget.id:  Nao5CompileTarget,
    Nao6CompileTarget.id:  Nao6CompileTarget,
    SimrobotCompileTarget.id:  SimrobotCompileTarget,
}

BuildTypes = {
    DevelopBuildType.id: DevelopBuildType,
    DebugBuildType.id: DebugBuildType,
    ReleaseBuildType.id: ReleaseBuildType
}

project_root_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "../.."))


class Compiler:
    def __init__(self, build: BuildType, target: CompileTarget):
        self.build_type = build
        self.target = target
        self.build_dir = os.path.join(project_root_dir, "build", self.target.foldername, self.build_type.foldername)

    def compile(self):
        cmake_cache_path = os.path.join(self.build_dir, "CMakeCache.txt")
        if not os.path.exists(cmake_cache_path):
            raise RuntimeError("Please execute setup before compile. (Missing CMakeCache.txt for target)")

        used_cpus = os.cpu_count() - 1
        logging.info(f"Using {used_cpus} Threads (-j{used_cpus})")
        subprocess.run(["make", f"-j{used_cpus}"], cwd=self.build_dir)
