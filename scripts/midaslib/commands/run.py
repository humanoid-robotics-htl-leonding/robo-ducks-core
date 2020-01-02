from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.naocom import Nao


@command("run", "r")
class RunCommand(Command):
    help = "Run a bash-command on the nao"
    @parse_address
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("shellcommand", nargs="*", help="Shell command to execute on the nao")

    def execute(self, args):
        nao = Nao(args.address)
        cmd = " ".join(args.shellcommand)
        nao.execute(cmd)
