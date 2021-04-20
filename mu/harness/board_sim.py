import threading

from mu.harness import project
from mu.harness import decoder
from mu.protogen import stores_pb2
from mu.protogen import gpio_pb2

from mu.harness.project import StoreUpdate

# Constants for stm32f0xx store keys
GPIO_KEY = (stores_pb2.MuStoreType.GPIO, 0)

class BoardSim:
    def __init__(self, pm, proj_name, sub_sim_classes=[], init_conds=[]):
        self.pm = pm
        self.sub_sims = {}
        for sub_sim_class in sub_sim_classes:
            self.sub_sims[sub_sim_class.__name__] = sub_sim_class(self)
        self.stores = {}
        self.timers = []
        self.proj = project.Project(pm, self, proj_name)
        self.fd = self.proj.popen.stdout.fileno()
        self.pm.register(self)
        for cond in init_conds:
            self.proj.write_store(cond)
        self.proj.send_command(stores_pb2.MuCmdType.FINISH_INIT_CONDS)

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
        gpio_msg = self.stores[(stores_pb2.MuStoreType.GPIO, 0)]
        return gpio_msg.state[(ord(port.capitalize()) - ord('A')) * 16 + pin]

    def set_gpio(self, port, pin, state):
        ind = (ord(port.capitalize()) - ord('A')) * 16 + pin
        gpio_msg = gpio_pb2.MuGpioStore()
        gpio_msg.state.extend([0] * 2 * 16)
        gpio_msg.state[ind] = state
        gpio_mask = gpio_pb2.MuGpioStore()
        gpio_mask.state.extend([0] * 2 * 16)
        gpio_mask.state[ind] = state
        gpio_update = StoreUpdate(gpio_msg, gpio_mask, stores_pb2.MuStoreType.GPIO, 0)

        self.proj.write_store(gpio_update)
    
    # TODO(SOFT-158): Implement set_adc(self, port, pin, val)
    
    # To be implemented by subclasses

    def handle_store(self, store, key):
        pass
