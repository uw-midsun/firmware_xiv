from abc import ABC

class SubSim(ABC):
    def __init__(self, parent_sim):
        self.parent = parent_sim

    def handle_store(self, store, key):
        pass