class SimIo:
    def __init__(self, sim, name, getter, setter):
        self.sim = sim
        self.name = name
        self.getter = getter
        self.setter = setter

    def get(self):
        if self.getter is None:
            raise NotImplementedError
        return self.getter()

    def set(self, val):
        if self.setter is None:
            raise NotImplementedError
        self.setter(val)
