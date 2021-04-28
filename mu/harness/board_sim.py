import signal
import threading

from mu.harness import project
from mu.harness import decoder
from mu.protogen import stores_pb2
from mu.protogen import gpio_pb2

from mu.harness.project import StoreUpdate

# Constants for stm32f0xx store keys
GPIO_KEY = (stores_pb2.MuStoreType.GPIO, 0)

POLL_LOCK_SIGNAL = signal.SIGUSR1


class BoardSim:
    def __init__(self, pm, proj_name, sub_sim_classes=None, init_conds=None):
        self.pm = pm
        self.sub_sims = {}
        if sub_sim_classes:
            for sub_sim_class in sub_sim_classes:
                self.sub_sims[sub_sim_class.__name__.lower()] = sub_sim_class(self)
        self.stores = {}
        self.timers = []
        self.proj = project.Project(pm, self, proj_name)
        self.fd = self.proj.popen.stdout.fileno()
        self.pm.register(self)
        if init_conds:
            for cond in init_conds:
                self.proj.write_store(cond)
        self.proj.send_command(stores_pb2.MuCmdType.FINISH_INIT_CONDS)

    def sub_sim(self, sim_name):
        return self.sub_sims[sim_name.lower()]

    def process_pipe(self):
        msg = self.proj.popen.stdout.read()
        store_info = decoder.decode_store_info(msg)
        if store_info.type == stores_pb2.LOG:
            mulog = stores_pb2.MuLog()
            mulog.ParseFromString(store_info.msg)
            log = mulog.log.decode().rstrip()
            self.pm.logger.log(type(self).__name__, log)
        else:
            self.handle_info(store_info)
        self.proj.popen.send_signal(POLL_LOCK_SIGNAL)

    def stop(self):
        self.proj.stop()
        for timer in self.timers:
            timer.cancel()

    def handle_info(self, store_info):
        key = (store_info.type, store_info.key)
        store = decoder.decode_store(store_info)
        self.stores[key] = store
        for sub_sim in self.sub_sims.values():
            sub_sim.handle_store(store, key)
        self.handle_store(store, key)

    # Utility methods for all boards
    def set_timer(self, time_s, func, args):
        timer = threading.Timer(time_s, func, args)
        self.timers.append(timer)
        timer.start()

    def get_gpio(self, port, pin):
        gpio_msg = self.stores[GPIO_KEY]
        return gpio_msg.state[(ord(port.capitalize()) - ord('A')) * 16 + pin]

    def set_gpio(self, port, pin, state):
        ind = (ord(port.capitalize()) - ord('A')) * 16 + pin
        gpio_msg = gpio_pb2.MuGpioStore()
        gpio_msg.state.extend([0] * 3 * 16)
        gpio_msg.state[ind] = state
        gpio_mask = gpio_pb2.MuGpioStore()
        gpio_mask.state.extend([0] * 3 * 16)
        gpio_mask.state[ind] = state
        gpio_update = StoreUpdate(gpio_msg, gpio_mask, GPIO_KEY)

        self.proj.write_store(gpio_update)

    # To be implemented by subclasses

    def handle_store(self, store, key):
        pass
