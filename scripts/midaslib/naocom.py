import logging
import os
import signal
import subprocess
import sys
import time
from enum import Enum
from typing import List

import paramiko
import spur


class NaoVersion(Enum):
    V5 = 0
    V6 = 1


base_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
project_base_dir = os.path.abspath(os.path.join(base_dir, ".."))
ssh_standard_args = [
    "-o", "UserKnownHostsFile=/dev/null",
    "-o", "StrictHostKeyChecking=no",
    "-o", "LogLevel=quiet",
    "-o", "ConnectTimeout=5",
    "-i", base_dir + "/files/ssh_key"
]


def subprocess_run(command):
    # logging.info(" ".join(command))
    # return subprocess.run(command, shell=False).returncode
    popen = subprocess.Popen(command)

    def signal_handler(sign, frame):
        print(f"Signal {sign} was sent to running process.")
        popen.send_signal(sign)
        try:  # TODO Not communicating here
            popen.communicate(timeout=2)
            pass
        except subprocess.TimeoutExpired:
            popen.send_signal(signal.SIGTERM)
            pass

    signal.signal(signal.SIGINT, signal_handler)
    popen.communicate()
    return_code = popen.returncode
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    return return_code


def copy_ssh_id(password, user, address):
    ssh_copy_id_command = [
        "sshpass", "-p", password, "ssh-copy-id", *ssh_standard_args, user + "@" + address
    ]
    return subprocess_run(ssh_copy_id_command)


def scp(sources, destination):
    if type(sources) != list:
        sources = [sources]

    scp_command = [
        "scp", *ssh_standard_args, "-r", *sources, destination
    ]
    return subprocess_run(scp_command)


def rsync(source, destination, delete=False):
    delete = [
        "--delete", "--delete-excluded"
    ] if delete else []
    ssh_cmd = " ".join(ssh_standard_args)
    command = [
        "rsync", "-trzKLP",
        "--exclude=*webots*",
        "--exclude=*.gitkeep",
        "--exclude=*.touch",
        *delete,
        "-e", f"ssh {ssh_cmd}",
        source,
        destination
    ]
    print(" ".join(command))
    return subprocess_run(command)


def ssh_command_old(address, user, command):
    command = [
        "ssh", *ssh_standard_args, "-l", user, address, command
    ]
    return subprocess_run(command)


def ssh_command(address, user, command: List[str]):
    ssh = spur.SshShell(
        hostname=address,
        username=user,
        private_key_file=os.path.join(base_dir, 'files', 'ssh_key')
    )

    obj = ssh.spawn(command, store_pid=True, stdout=sys.stdout, stderr=sys.stderr, encoding='ASCII')

    was_signal_sent = 0

    def signal_handler(sign, frame):
        print(f"Interrupt was caught. Stopping Process.")
        if was_signal_sent == 0:
            print("Sending Interrupt.")
            obj.send_signal(signal.SIGINT)
        elif was_signal_sent == 1:
            print("Sending Terminate.")
            obj.send_signal(signal.SIGTERM)
        else:
            print("Sending Kill Signal.")
            obj.send_signal(signal.SIGKILL)

    signal.signal(signal.SIGINT, signal_handler)

    result = obj.wait_for_result()

    signal.signal(signal.SIGINT, signal.SIG_DFL)

    return result.return_code


class Nao:
    def __init__(self, address):
        self.address = address
        self.user = "nao"

    def execute(self, command: List[str]):
        """
        Executes an ssh-command remotely
        """
        return ssh_command(self.address, self.user, command)

    def upload(self, source, destination):
        """
        Uploads a single file via scp
        """
        return scp(source, f"{self.user}@{self.address}:{destination}")

    def reboot(self):
        """
        Reboots the nao
        """
        self.execute(["systemctl", "reboot"])

    def copy_ssh_key(self):
        """
        Copies the local ssh key in scripts/files/ssh_key.pub to the nao. Useful for login without password
        """
        return copy_ssh_id("nao", self.user, self.address)

    def rsync(self, source, delete):
        """
        Uploads a file or a folder via rsync
        """
        return rsync(source, f"{self.user}@{self.address}:", delete)
