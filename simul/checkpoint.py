import storage
import event

class Checkpoint:
    def __init__(self, name):
        self.name = name
        self.storage = storage.New("ST_" + name)

    def CheckIn(self, time, player):
        player.CheckInAt(time, self)
        e = event.New(self, player)
        self.ProcessData(time, e)
        self.DoLog(time)

    def Storage(self):
        return self.storage

    def Id(self):
        return self.name

    def ProcessData(self, time, player, event):
        self.storage.Store(event)

    def DoLog(self, time):
        pass
#        print "+CP+\t%d\t%s\t%d" % (time, self.name, len(self.Storage()))

    def LogHeader(self):
        pass
#        print "+CP+\t%s\t%s\t%s" % ("time", "name", "storage")


class StoreOnPlayer(Checkpoint):
    def ProcessData(self, time, event):
        player = event.player
        if player.IsCarrier():
            player.Storage().Store(event)
        self.Storage().Store(event)

class StoreAndPass3Times(Checkpoint):
    def __init__(self, name):
        Checkpoint.__init__(self, name)
        self.events = {0:[], 1:[], 2:[], 3:[]}
        self.times = {}
    
    def ProcessData(self, time, event):
        player = event.player
        self.Storage().Store(event)
        self.events[0] += [event]
        self.times[event] = 0
        if player.IsCarrier():
            # store news from this player
            for ev in player.Storage():
                if self.Storage().Has(ev):
                    t = self.times[ev]
                    if t < 3:
                        self.events[t].remove(ev)
                        self.times[ev] = t+1
                        self.events[t+1] += [ev]
                else:
                    self.events[0] += [ev]
                    self.times[ev] = 0
                    self.Storage().Store(ev)
            # send events if memory permits
            player_mem = player.Capacity()
            temp_storage = []
            t = 0
            while len(player.Storage()) < player_mem and t < 3:
                # find events to send with this player at level t
                all = len(self.events[t])
                send = max(player_mem - len(player.Storage()), all)
                ev_send = self.events[t][0:send]
                for ev in ev_send:
                    player.Storage().Store(ev)
                    self.times[ev] += 1
                self.events[t] = self.events[t][send:] + temp_storage
                temp_storage = ev_send
                t += 1
            self.events[t] += temp_storage

class Block:
    CAPACITY = 56
    
    def __init__(self, name, num):
        self.name = name
        self.num = num
        self.transmits = 0
        self.events = []

    def Copy(self):
        t = Block(self.name, self.num)
        t.transmits = self.transmits
        t.events = self.events[:]
        return t

def spec_cmp(x, y):
    if x[0] == y[0]:
        return cmp(x[2], y[2])
    else:
        return cmp(x[0], y[0])

class StoreInBlocks(Checkpoint):
    def __init__(self, name, clear):
        Checkpoint.__init__(self, name)
        self.blocks = {}
        self.blockwrites = {}
        self.count = 0
        self.clear = clear
        self.maxwrites = 0

    def EnsureBlock(self, name, num):
        if not name in self.blocks:
            self.blocks[name] = []
            self.blockwrites[name] = []
            for i in range(20):
                self.blocks[name] += [Block(name, i)]
                self.blockwrites[name] += [0]
        
    def SetBlock(self, name, num, data):
        self.EnsureBlock(name, num)
        self.blocks[name][num] = data
        self.blockwrites[name][num] += 1
        if self.blockwrites[name][num] > self.maxwrites:
            self.maxwrites = self.blockwrites[name][num]

    def GetBlock(self, name, num):
        self.EnsureBlock(name, num)
        return self.blocks[name][num]
    
    def ProcessData(self, time, event):
        player = event.player
        # add event to block and register
        n = self.count / Block.CAPACITY
        b = self.GetBlock(self.name, n)
        b.events += [event]
        b.transmits = 0
        self.SetBlock(self.name, n, b)
        self.count += 1
        self.Storage().Store(event)

        if player.IsCarrier():
            # get all blocks from player
            if "blocks" in player.__dict__:
                for b in player.blocks:
                    my = self.GetBlock(b.name, b.num)
                    if len(my.events) < len(b.events):
                        # new version; replace
                        b.transmits = 0
                        self.SetBlock(b.name, b.num, b.Copy())
                        for e in my.events:
                            self.Storage().Remove(e)
                        for e in b.events:
                            self.Storage().Store(e)
                    # we haven't transmitted this block yet
                    if self.clear:
                        my.transmits = 0
                # clear player's storage
                player.Storage().Clear()
            player.blocks = []

            # send information with this player
            r = []
            for t in self.blocks:
                for b in self.blocks[t]:
                    if len(b.events) > 0:
                        r += [[b.transmits, b.name, b.num]]
            r = sorted(r, spec_cmp);
            on_player = 0
            t = 0
            while on_player < player.Capacity() and t < len(r):
                spec = r[t]
                b = self.GetBlock(spec[1], spec[2])
                if on_player + len(b.events) < player.Capacity():
                    player.blocks += [b.Copy()]
                    b.transmits += 1
                    for ev in b.events:
                        player.Storage().Store(ev)
                    on_player += len(b.events)
                t += 1
    def GetMaxWrites(self):
        return self.maxwrites
                
    def DoLog(self, time):
        all = len(self.Storage())
        print "+CP+\t%d\t%s\t%d\t%d" % (time, self.name, all, all - self.count)

    def LogHeader(self):
        print "+CP+\t%s\t%s\t%s\t%s" % ("time", "name", "stored", "other_stored")

class StoreInBlocks2(Checkpoint):
    def __init__(self, name, clear):
        Checkpoint.__init__(self, name)
        self.blocks = {}
        self.blockwrites = {}
        self.count = 0
        self.clear = clear
        self.maxwrites = 0
        self.order = []
        self.to_transmit = 0

    def EnsureBlock(self, name, num):
        if not name in self.blocks:
            self.blocks[name] = []
            self.blockwrites[name] = []
            for i in range(20):
                self.blocks[name] += [Block(name, i)]
                self.blockwrites[name] += [0]
        
    def SetBlock(self, name, num, data):
        self.EnsureBlock(name, num)
        self.blocks[name][num] = data
        self.blockwrites[name][num] += 1
        if self.blockwrites[name][num] > self.maxwrites:
            self.maxwrites = self.blockwrites[name][num]

    def GetBlock(self, name, num):
        self.EnsureBlock(name, num)
        return self.blocks[name][num]
    
    def ProcessData(self, time, event):
        player = event.player
        # add event to block and register
        n = self.count / Block.CAPACITY
        b = self.GetBlock(self.name, n)
        if len(b.events) == 0:
            self.order += [b]
        b.events += [event]
        b.transmits = 0
        self.SetBlock(self.name, n, b)
        self.count += 1
        self.Storage().Store(event)

        if player.IsCarrier():
            # get all blocks from player
            if "blocks" in player.__dict__:
                for b in player.blocks:
                    my = self.GetBlock(b.name, b.num)
                    if len(my.events) < len(b.events):
                        # new version; replace
                        b.transmits = 0
                        self.SetBlock(b.name, b.num, b.Copy())
                        if len(my.events) == 0:
                            self.order += [self.GetBlock(b.name, b.num)]
                        for e in my.events:
                            self.Storage().Remove(e)
                        for e in b.events:
                            self.Storage().Store(e)
                    # we haven't transmitted this block yet
                    if self.clear:
                        my.transmits = 0
                # clear player's storage
                player.Storage().Clear()
            player.blocks = []

            # send information with this player
            on_player = 0
            transmitted = 0
            while on_player < player.Capacity() and transmitted < len(self.order):
                index = (self.to_transmit + transmitted) % len(self.order)
                b = self.order[index]
                player.blocks += [b.Copy()]
                for ev in b.events:
                    player.Storage().Store(ev)
                b.transmits += 1
                on_player += len(b.events)
                transmitted += 1
            self.to_transmit += transmitted
            self.to_transmit %= len(self.order)

    def GetMaxWrites(self):
        return self.maxwrites
                
    def DoLog(self, time):
        all = len(self.Storage())
        print "+CP+\t%d\t%s\t%d\t%d" % (time, self.name, all, all - self.count)

    def LogHeader(self):
        print "+CP+\t%s\t%s\t%s\t%s" % ("time", "name", "stored", "other_stored")


def New(name, clear):
    return StoreInBlocks2(name, clear)
#    return StoreAndPass3Times(name)
#    return StoreOnPlayer(name)
#    return Checkpoint(name)

New("Header", False).LogHeader()
    
        
