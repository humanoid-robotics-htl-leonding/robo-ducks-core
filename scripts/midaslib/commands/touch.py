import abc
import functools
import logging
import os
import stat
from argparse import ArgumentParser

from midaslib.command import Command, command
from midaslib.commonparsers import parse_address

import midaslib.naocom as nc


def touch_step(name):
    def touch_decoration(func):
        @functools.wraps(func)
        def touch_wrapper(self):
            logging.info(f" => Step: {name}")
            func(self)
        return touch_wrapper
    return touch_decoration


sysroot_file_names = {
    "nao5": "sysroot-7.3.0-1.tar.bz2",
    "nao6": "sysroot-9.2.0-1.tar.bz2"
}


@command("touch", "t")
class TouchCommand(Command):
    nao: nc.Nao = None
    args = None
    help = "(Former Gammaray) Setup NAO for usage with RoboDucks Code."

    @parse_address(True)
    def define_parser(self, parser: ArgumentParser):
        parser.add_argument("target", choices=["nao5", "nao6"], help="Nao Version to touch")
        parser.add_argument("--no-sysroot", "-n", action="store_true", default=False, help="Nao Version to touch")
        pass

    @touch_step("SSH-Key")
    def upload_ssh_key(self):
        self.nao.copy_ssh_key()

    def execute(self, args):
        self.args = args
        self.nao = nc.Nao(args.address)

        os.chmod(os.path.join(nc.base_dir, "files", "ssh_key"), stat.S_IRUSR)
        self.upload_ssh_key()

        if not args.no_sysroot:
            if args.target not in sysroot_file_names:
                raise RuntimeError(f"Unknown nao version: {args.target}.")

            sysroot_path = os.path.join(nc.project_base_dir, "toolchain", sysroot_file_names[args.target])

            if not os.path.exists(sysroot_path):
                raise RuntimeError(f"Please install sysroot for {args.target} as {sysroot_path}")

            print("Uploading Sysroot...")
            self.nao.upload(sysroot_path, "sysroot.tar.bz2")
            print("Extracting Sysroot...")
            self.nao.execute("tar xf sysroot.tar.bz2".split(' '))
            print("Deleting old Sysroot...")
            self.nao.execute("rm sysroot.tar.bz2".split(' '))
            print("Done.")

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
