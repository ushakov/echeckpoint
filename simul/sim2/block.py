class Block:
    def __init__(self, name, num):
        self.name = name
        self.num = num
        self.events = []

    def Copy(self):
        t = Block(self.name, self.num)
        t.events = self.events[:]
        return t
