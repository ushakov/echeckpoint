class Block:
    def __init__(self, name, num):
        self.name = name
        self.num = num
        self.transmits = 0
        self.events = []

# при копировании transmits не копируется!
    def Copy(self):
        t = Block(self.name, self.num)
        t.events = self.events[:]
        return t
