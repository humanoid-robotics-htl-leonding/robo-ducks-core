import abc
import logging
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address

import midaslib.naocom as nc


class TouchStep:
    @abc.abstractmethod
    def run(self):
        pass

    def mayrun(self, args) -> bool:
        return True


@command("touch", "t")
class TouchCommand(Command):
    @parse_address
    def define_parser(self, parser: ArgumentParser):
        pass

    def execute(self, args):
        nao = nc.Nao(args.address)



        #
        #
        # if not args.sysroot_only:
        #     logging.info("Copy SSH-Key")
        #     nc.nao_copy_ssh_id(args.address)
        #
        # if not args.skip_sysroot:
        #     logging.info("Copy sysroot")
        #     nc.nao_cp(base_dir() + "/../toolchain/sysroot.tar.bz2",
        #            "nao@" + nao_address + ":")
        #     logging.info("Extracting sysroot")
        #     nc.nao_cmd(nao_address, "tar xf sysroot.tar.bz2")
        #     nc.nao_cmd(nao_address, "rm sysroot.tar.bz2")
        # else:
        #     logging.info("Skipping sysroot...")
        #
        # if args.sysroot_only:
        #     exit(0)
        #
        # # logging.info("Set Hostname")
        # # nao_cmd(nao_address,
        # #         "hostnamectl set-hostname YOUR_ROBOT_NAME_HERE" + str(args.nao_number))
        #
        # logging.info("Copy Files")
        # nao_cp(base_dir() + "/files/bin", "nao@" + nao_address + ":.local")
        # nao_cp(base_dir() + "/files/etc", "nao@" + nao_address + ":.local")
        # nao_cp(base_dir() + "/files/asound.state", "nao@" + nao_address + ":.config")
        # nao_cp(base_dir() + "/files/.asoundrc", "nao@" + nao_address + ":~")
        # nao_cp(base_dir() + "/files/.autoload.ini.*", "nao@" + nao_address + ":naoqi/preferences/")
        #
        # logging.info("Set executable bits")
        # nao_cmd(nao_address, "chmod u+x .local/bin/connman-hulks")
        # nao_cmd(nao_address, "chmod u+x .local/bin/setNetwork")
        # nao_cmd(nao_address, "chmod u+x .local/bin/alsa-restore")
        #
        # logging.info("Copy bashrc")
        # nao_cp(base_dir() + "/files/bashrc_v6", "nao@" + nao_address + ":.profile")
        #
        # # logging.info("Set nao ip")
        # # nao_cmd(
        # #     nao_address, "sed -e s/{wifi_ip}/" + get_nao_address(
        # #         args.nao_number, False) + "/ -e s/{eth_ip}/" + get_nao_address(
        # #             args.nao_number) + "/ -i /home/nao/.local/etc/connman-hulks.yaml")
        #
        # logging.info("Copy services")
        # nao_cmd(nao_address, "mkdir -p .config/systemd/user")
        # nao_cp([
        #     base_dir() + "/files/connman-hulks.service",
        #     base_dir() + "/files/lola-hulks.service",
        #     base_dir() + "/files/hulk.service",
        #     base_dir() + "/files/alsa-restore.service"
        # ], "nao@" + nao_address + ":.config/systemd/user/")
        #
        # logging.info("Activate services")
        # nao_cmd(nao_address, "systemctl --user enable lola-hulks.service")
        # # nao_cmd(nao_address, "systemctl --user enable connman-hulks.service")
        # nao_cmd(nao_address, "systemctl --user enable hulk.service")
        # nao_cmd(nao_address, "systemctl --user enable alsa-restore.service")
        #
        # logging.info("Add robocup.conf")
        # nao_cmd(nao_address, "touch robocup.conf")
        #
        # logging.info("Reboot NAO")
        # nao_reboot(nao_address)


if __name__ == '__main__':
    raise RuntimeError("Command class cannot be executed.")
