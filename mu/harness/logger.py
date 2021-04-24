from collections import deque
import queue

class NoLog(queue.Empty):
    pass

class Log:
    def __init__(self, tag, msg):
        self.tag = tag
        self.msg = msg


class Subscriber:
    # Log consumers create a subscriber and add it to the logger
    # tags is an iterable with the tags to subscribe to
    def __init__(self, name, tags=None):
        self.name = name
        self.tags = tags
        self.q = queue.Queue()

    def put(self, log):
        if not self.tags or log.tags in self.tags:
            self.q.put(log)

    def get(self):
        try:
            return self.q.get(timeout=0.1)
        except queue.Empty as e:
            raise NoLog from e


class Logger:
    def __init__(self, max_logs=1000):
        self.max_logs = max_logs
        self.logs = deque()
        self.subs = {}

    def log(self, tag, msg):
        while len(self.logs) >= self.max_logs:
            self.logs.popleft()
        l = Log(tag, msg)
        self.logs.append(l)
        for sub in self.subs.values():
            sub.put(l)

    def query(self, tag):
        ret = []
        for log in self.logs:
            if log.tag == tag:
                ret.append(log)

        return ret

    def subscribe(self, sub):
        self.subs[sub.name] = sub

    def unsubscribe(self, sub):
        del self.subs[sub.name]
