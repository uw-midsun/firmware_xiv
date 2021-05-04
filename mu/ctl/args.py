import argparse
from mu.ctl.log import logs

def get_args():
    parser = argparse.ArgumentParser(prog='muctl', description='interact with musrv')
    subparsers = parser.add_subparsers()

    parser_logs = subparsers.add_parser('logs', help='stream logs from musrv')
    parser_logs.set_defaults(func=logs)
    return parser.parse_args()
