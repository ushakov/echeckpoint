import storage

max_mem = 0

class Player:
    def __init__(self, name, carrier):
        self.blocks = storage.Storage()
        self.name = name
        self.carrier = carrier
        if self.carrier == 1:
            self.capacity = 56
        elif self.carrier == 2:
            self.capacity = 896
        self.cps = []

    def Id(self):
        return self.name

    def IsCarrier(self):
        return self.carrier != 0

    def Capacity(self):
        return self.capacity

    def CheckInAt(self, time, cp):
        self.cps += [cp]
        self.DoLog(time, cp)

    def Checkpoints(self):
        return self.cps

    def Destroy(self):
         self.blocks.Clear()

    def DoLog(self, time, cp):
        if self.IsCarrier():
            s = len(self.blocks)
        else:
            s = "-"
        print "+PL+\t%d\t%s\t%s\t%d" % (time, self.Id(), str(s), len(self.cps))

    def LogHeader(self):
        pass
        print "+PL+\t%s\t%s\t%s\t%s" % ("time", "id", "storage", "CPs")



def New(name, carrier):
    return Player(name, carrier)

New("Header", False).LogHeader()
