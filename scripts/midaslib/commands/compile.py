import argparse
import logging
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.compiler import Targets, BuildTypes


@command("compile", "c")
class CompileCommand(Command):
    @parse_address
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("build", choices=BuildTypes.keys(), help="Build type")
        parser.add_argument("target", choices=Targets.keys(), help="Compile target")
        pass

    def execute(self, args):
        build_type = BuildTypes[args.build]
        target = Targets[args.target]
        logging.info(f"Compiling: {build_type.name} {target.name}")


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")
