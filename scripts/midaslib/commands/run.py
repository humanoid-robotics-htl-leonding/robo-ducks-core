from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address
from midaslib.naocom import Nao


@command("run", "r")  # Define aliases for this command
class RunCommand(Command):
    help = "Run a bash-command on the nao"  # Will be shown when midas -h is run

    @parse_address()
    def define_parser(self, parser: ArgumentParser):  # Here you can register the needed arguments
        parser.add_argument("shellcommand", nargs="*", help="Shell command to execute on the nao")

    def execute(self, args):
        nao = Nao(args.address)  # Nao Connector
        cmd = " ".join(args.shellcommand)
        nao.execute(cmd)
