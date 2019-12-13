import argparse
import logging
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.compiler import Targets, BuildTypes, Compiler


@command("compile", "c")
class CompileCommand(Command):
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("build", choices=BuildTypes.keys(), help="Build type")
        parser.add_argument("target", choices=Targets.keys(), help="Compile target")
        pass

    def execute(self, args):
        build_type = BuildTypes[args.build]
        target = Targets[args.target]
        logging.info(f"Compiling '{build_type.name}' for '{target.name}'")
        compiler = Compiler(build_type, target)
        compiler.compile()


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")
