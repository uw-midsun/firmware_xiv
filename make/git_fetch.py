"""Download and install script for github repos

Fetches the specified tag and unpacks it for the build system to pick up.
"""
#! /usr/bin/env python

import argparse
import subprocess
import os
import re
import zipfile
import shutil
import socket

# Fetch the latest release from the official GitHub API
URL = 'https://api.github.com/repos/{}/{}/releases/'

# Hash the file to prevent reinstalling the same file.
VERSION_FILE = 'version.txt'

# For validating there is an internet connection
REMOTE_SERVER = "www.google.com"


def connected():
    """Connected checks for a internet connection

    Tries to connect to Google over an HTTP socket and will fail if there is no
    connection available.
    """
    try:
        host = socket.gethostbyname(REMOTE_SERVER)
        _ = socket.create_connection((host, 80), 2)
        return True
    except socket.error:
        pass
    return False


def fetch_release(url, tag, output_file):
    """Fetches the release from the specified GitHub API URL

    Args:
        url: string that is the GitHub API URL
        tag: string of release tag
        output_file: string containing the release output file to fetch

    Returns:
        The return code object of the subprocess used to fetch the file.
    """
    popen = subprocess.Popen(
        [
            'curl -A "curl" -s {}{} | grep "{}" | cut -d : -f 2,3 |tr -d \\" | wget -qi -'.
            format(url, tag, output_file)
        ],
        shell=True,
        stderr=subprocess.PIPE,
        stdout=subprocess.PIPE)
    _, err = popen.communicate()
    return err


def check_hash(filename, hash_file):
    """Checks that the hash of the downloaded file against the previous hash.

    Args:
        filename: string containing the name of the downloaded file
        hash_file: string containing the name of the hash recording file

    Returns:
        A boolean value True if the hashes match of False if they don't
    """
    popen = subprocess.Popen(
        ['sha256sum {}'.format(filename)], shell=True, stdout=subprocess.PIPE)
    stdout, _ = popen.communicate()
    if hash_file in os.listdir(os.getcwd()):
        with open(hash_file, 'r+') as hashfp:
            if hashfp.readline() == stdout.decode('utf-8'):
                return True
    with open(hash_file, 'w+') as hashfp:
        hashfp.write(stdout.decode('utf-8'))
        hashfp.flush()
    return False


def clean_up(filename):
    """Cleans up by deleting the filename specified

    Also deletes files that may have been downloaded extra and not cleaned up
    properly.

    Args:
        filename: a string of the file to delete

    Returns:
        None
    """
    pattern = re.compile(filename + r'\.*[0-9]*')
    for fname in os.listdir(os.getcwd()):
        if re.search(pattern, fname):
            os.remove(os.path.join(os.getcwd(), fname))


def unpack(filename):
    """Unpacks the specified zip file

    Places the *.h files into inc/ and *.c into src/ and will create any missing
    directories. It will also delete any previous files in these folers.

    Args:
        filename: string containing the name of the zip file to unpack

    Returns:
        Boolean value indicating success or failure
    """
    _, zip_ext = os.path.splitext(os.path.join(os.getcwd(), filename))
    if zip_ext != '.zip':
        return False

    lsdir = os.listdir(os.getcwd())
    with zipfile.ZipFile(os.path.join(os.getcwd(), filename), 'r') as zip_ref:
        zip_ref.extractall(os.getcwd())
    lsdir_post = os.listdir(os.getcwd())
    newdir = list(set(lsdir_post) - set(lsdir))
    if len(newdir) != 1:
        return False

    if 'inc' in lsdir:
        clean_folder(os.path.join(os.getcwd(), 'inc'))
    else:
        os.mkdir(os.path.join(os.getcwd(), 'inc'))
    if 'src' in lsdir:
        clean_folder(os.path.join(os.getcwd(), 'src'))
    else:
        os.mkdir(os.path.join(os.getcwd(), 'src'))

    for file_path in os.listdir(os.path.join(os.getcwd(), newdir[0])):
        _, ext = os.path.splitext(file_path)
        if ext == '.h':
            os.rename(
                os.path.join(os.getcwd(), newdir[0], file_path),
                os.path.join(os.getcwd(), 'inc', os.path.basename(file_path)))
        if ext == '.c':
            os.rename(
                os.path.join(os.getcwd(), newdir[0], file_path),
                os.path.join(os.getcwd(), 'src', os.path.basename(file_path)))

    # Delete the unzipped dir
    shutil.rmtree(newdir[0])
    return True


def clean_folder(directory, ignore='tmp.c'):
    """Cleans up a folder by removing all files in it.

    Arg:
        directory: file path of the directory to delete
        ignore: string of space separated files to ignore

    Returns:
        Boolean value indicating success or failure
    """
    if not os.path.isdir(directory):
        return False

    ignore_list = ignore.split(' ')
    for file_path in os.listdir(directory):
        if os.path.basename(file_path) not in ignore_list:
            if os.path.isfile(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)

    return True


def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description='Execute a prebuild library update.')
    parser.add_argument(
        '-folder', type=str, help='the folder in which to run the hook')
    parser.add_argument(
        '-user', type=str, help='the github user who owns the repo')
    parser.add_argument('-repo', type=str, help='the github repo name')
    parser.add_argument(
        '-tag', type=str, help='the github tag for the release')
    parser.add_argument(
        '-file', type=str, help='the github release file to get')

    args = parser.parse_args()

    if not connected():
        print('No internet connection, skipping hook, will use local copy.')
        return

    os.chdir(os.path.join(os.getcwd(), args.folder))

    try:
        url = URL.format(args.user, args.repo)
        fetch_release(url, args.tag, args.file)
        if check_hash(args.file, VERSION_FILE):
            clean_up(args.file)
            return
        if not unpack(args.file):
            raise IOError('Failed to unpack file.')
        clean_up(args.file)
    except Exception as err:  # pylint: disable=W0703
        print('Failed to fetch release {}{}: {}'.format(url, args.tag, err))


if __name__ == '__main__':
    main()
