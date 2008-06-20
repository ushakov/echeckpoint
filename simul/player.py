import storage

max_mem = 0

class Player:
    def __init__(self, name, carrier):
        self.name = name
        self.carrier = carrier
        if self.carrier == 1:
            self.storage = storage.New("ST_SM_" + name)
            self.capacity = 56
        elif self.carrier == 2:
            self.storage = storage.New("ST_BG_" + name)
            self.capacity = 896
        self.cps = []

    def Id(self):
        return self.name

    def IsCarrier(self):
        return self.carrier != 0

    def Capacity(self):
        return self.capacity

    def Storage(self):
        return self.storage

    def CheckInAt(self, time, cp):
        self.cps += [cp]
        self.DoLog(time, cp)

    def Checkpoints(self):
        return self.cps

    def Destroy(self):
        global max_mem
        if self.IsCarrier():
            max_mem = max(max_mem, self.Storage().MaxUsage())
            self.Storage().Clear()

    def DoLog(self, time, cp):
        if self.IsCarrier():
            s = len(self.Storage())
        else:
            s = "-"
        print "+PL+\t%d\t%s\t%s\t%s\t%d" % (time, self.Id(), str(s), cp.Id(), len(self.cps))

    def LogHeader(self):
        pass
        print "+PL+\t%s\t%s\t%s\t%s\t%s" % ("time", "id", "storage", "CP", "CPs")



def New(name, carrier):
    return Player(name, carrier)

New("Header", False).LogHeader()
