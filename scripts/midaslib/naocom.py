import logging
import os
import signal
import subprocess
from enum import Enum


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
        popen.send_signal(sign)
        print(f"Signal {sign} was sent to running process.")

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

#  local SSH_CMD="ssh -q -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -l ${SSH_USERNAME} -i \"${SSH_KEY}\""
# 
# --exclude=*webots* --exclude=*.gitkeep --exclude=*.touch
# local RSYNC_PARAMETERS="-trzKLP ${RSYNC_EXCLUDE}"
# if ${DELETE_FILES}; then
#   RSYNC_PARAMETERS+=" --delete --delete-excluded"
# rsync ${RSYNC_PARAMETERS} --rsh="${SSH_CMD}" "${TMP_DIR}/naoqi" "${RSYNC_TARGET}:"


def ssh_command(address, user, command):
    command = [
        "ssh", *ssh_standard_args, "-l", user, address, command
    ]
    return subprocess_run(command)


class Nao:
    def __init__(self, address):
        self.address = address
        self.user = "nao"

    def execute(self, command):
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
        self.execute("systemctl reboot")

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
