import logging
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.compiler import Targets, Compiler
from midaslib.naocom import Nao


@command("setup", "s")
class SetupCommand(Command):
    help = "Setup for compilation."

    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("target", choices=Targets.keys(), help="Compile target")

    def execute(self, args):
        target = Targets[args.target]
        logging.info(f"Setting up '{target.name}'")
        compiler = Compiler(target)
        compiler.setup()
