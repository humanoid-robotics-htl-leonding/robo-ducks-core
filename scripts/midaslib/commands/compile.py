import argparse
import logging
import os
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.compiler import Targets, BuildTypes, Compiler, project_root_dir


@command("compile", "c")
class CompileCommand(Command):
    help = "Compile the code. (Run setup first)"

    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("build", choices=BuildTypes.keys(), help="Build type")
        parser.add_argument("target", choices=Targets.keys(), help="Compile target")

    def execute(self, args):
        build_type = BuildTypes[args.build]
        target = Targets[args.target]
        compiler = Compiler(target, build_type)
        compiler.compile()

        there = os.path.join(project_root_dir, "build", "simrobot", "current")
        if not os.path.exists(there):
            os.symlink(os.path.join(project_root_dir, "build", "simrobot", build_type.foldername), there)


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")
