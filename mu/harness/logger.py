from collections import deque
import queue

class Subscriber:
    def __init__(self, id, conds):
        self.id = id
        self.conds = conds
        self.q = queue.Queue()

    def put(self, msg):
        self.q.put(msg)

    def get(self):
        while True:
            try:
                return self.q.get(timeout=0.1)
            except queue.Empty:
                continue


class Cond:
    def __init__(self, name, func):
        self.name = name
        self.func = func

    def check(self, msg):
        return self.name in msg and self.func(msg[self.name])

    @classmethod
    def checkList(cls, conds, msg):
        for cond in conds:
            if not cond.check(msg):
                return False
        return True


class Logger:
    def __init__(self, max_logs=1000):
        self.max_logs = max_logs
        self.logs = deque()
        self.subs = {}

    def log(self, msg):
        while len(self.logs) >= self.max_logs:
            self.logs.popleft()
        self.logs.append(msg)
        for sub in self.subs:
            if Cond.checkList(sub.conds, msg):
                sub.put(msg)

    def query(self, conds):
        ret = []
        for msg in self.logs:
            if Cond.checkList(conds, msg):
                ret.append(msg)

        return ret

    def subscribe(self, sub):
        self.subs[sub.id] = sub

    def unsubscribe(self, sub):
        del self.subs[sub.id]
