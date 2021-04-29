from mu.ctl.args import get_args

if __name__ == '__main__':
    args = get_args()
    args.func(args)
