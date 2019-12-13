from enum import Enum


class CompileTarget:
    name = "Undefined"
    id = "undefined"


class Nao5CompileTarget(CompileTarget):
    name = "Nao V5"
    id = "nao5"


class Nao6CompileTarget(CompileTarget):
    name = "Nao V6"
    id = "nao6"


class SimrobotCompileTarget(CompileTarget):
    name = "Simrobot"
    id = "simrobot"


class BuildType:
    name = "Undefined"
    id = "undefined"


class DebugBuildType(BuildType):
    name = "Debug"
    id = "debug"


class DevelopBuildType(BuildType):
    name = "Develop"
    id = "develop"


class ReleaseBuildType(BuildType):
    name = "Release"
    id = "release"


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


class Compiler:
    pass