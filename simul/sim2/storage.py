import block

class Storage:
    def __init__(self):
        self.blocks = []
        self.max_usage = 0

    def __len__(self):
        return len(self.blocks)

    def __iter__(self):
        for i in self.blocks:
            yield i

    def __getitem__(self, key):
        return self.blocks[key]

    def Exists(self, name, num):
      for my_b in self.blocks:
        if name == my_b.name and num == my_b.num:
          return True
      return False


    def Store(self, bls):
      for b in bls:
        if not self.Exists(b.name, b.num):
          self.blocks += [b.Copy()]

    def Events(self):
      leng=0
      for b in self.blocks:
        leng += len(b.events)
      return leng

    def Clear(self):
        self.blocks = []


    def MaxUsage(self):
        return self.max_usage

    
