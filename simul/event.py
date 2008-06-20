import tick

events = []

class Event:
    def __init__(self, cp, player):
        self.cp = cp
        self.player = player
        self.storedAt = {}

    def Id(self):
        return "E_%s_%s" % (self.cp.Id(), self.player.Id())

    def StoreCallback(self, st):
        self.storedAt[st.Id()] = 1
	print "+EV+ %d %s %d %s" % (tick.Get(), self.Id(), len(self.storedAt), st.Id())

    def UnStoreCallback(self, st):
        del self.storedAt[st.Id()]
	print "+EV+ %d %s %d %s" % (tick.Get(), self.Id(), len(self.storedAt), st.Id())
        

def New(cp, player):
    global events
    e = Event(cp, player)
    events += [e]
    return e

