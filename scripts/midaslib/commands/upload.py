import logging
import os
import tempfile
from argparse import ArgumentParser

from midaslib.command import command, Command
from midaslib.commonparsers import parse_address
from midaslib.compiler import Targets, Compiler, BuildTypes, Nao6CompileTarget


import midaslib.naocom as nc
from midaslib.uploader import Uploader


@command("upload", "u")
class UploadCommand(Command):
    help = "Upload code to nao."

    @parse_address(True)
    def define_parser(self, parser: ArgumentParser):
        # parser.add_argument("target", choices=Targets.keys(), help="Compile target")
        parser.add_argument("build", choices=BuildTypes.keys(), help="Build type")
        parser.add_argument("target", choices=Targets.keys(), help="Compile target")
        parser.add_argument("--config", "-c", action="store_true", help="Upload Config")
        parser.add_argument("--execute", "-e", action="store_true", help="Start after upload")
        parser.add_argument("--no-compile", "-n", action="store_true", help="Don't recompile, just upload.")

    def execute(self, args):
        build_type = BuildTypes[args.build]
        target = Targets[args.target]
        logging.info(f"Uploading '{build_type.name}' to '{target.name}'")

        if not args.no_compile:
            comp = Compiler(target, build_type)
            comp.compile()

        upl = Uploader(args.config, args.address, target, build_type)
        upl.upload()

        if args.execute:
            nao = nc.Nao(args.address)
            nao.execute(["/home/nao/naoqi/bin/tuhhNao"])
