class CanDump:
    def handle_update(self, pm, proj):
        pass

    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)
