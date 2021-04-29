import argparse
from mu.ctl.log import pm_logs

def get_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    parser_pm_logs = subparsers.add_parser('pm_logs')
    parser_pm_logs.set_defaults(func=pm_logs)
    return parser.parse_args()
