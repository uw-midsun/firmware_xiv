import argparse
from mu.ctl.log import logs
from mu.ctl import sims

def get_args():
    parser = argparse.ArgumentParser(prog='muctl', description='interact with musrv')
    parser.set_defaults(func=lambda _: parser.print_usage())
    subparsers = parser.add_subparsers()

    parser_logs = subparsers.add_parser('logs', help='stream logs from musrv')
    parser_logs.add_argument('sim', nargs='?', default='')
    parser_logs.set_defaults(func=logs)

    parser_reset = subparsers.add_parser('reset', help='reset musrv pm')
    parser_reset.set_defaults(func=sims.reset)

    sim_start = subparsers.add_parser('start', help='start sim')
    sim_start.add_argument('sim')
    sim_start.add_argument('proj', nargs='?', default='')
    sim_start.set_defaults(func=sims.start)

    sim_stop = subparsers.add_parser('stop', help='stop sim')
    sim_stop.add_argument('sim')
    sim_stop.set_defaults(func=sims.stop)

    sim_cat = subparsers.add_parser('sims', help='catalog of sims')
    sim_cat.set_defaults(func=sims.sims)

    sim_list = subparsers.add_parser('list', help='list running sims')
    sim_list.set_defaults(func=sims.sim_list)

    return parser.parse_args()
