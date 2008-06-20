import event

total_stored = 0

class Storage:
    def __init__(self, name):
        self.index = {}
        self.name = name
        self.max_usage = 0

    def __len__(self):
        return len(self.index)

#     def __getitem__(self, key):
#         return self.items[key]

#     def __setitem__(self, key, value):
#         del self.index[self.items[key]]
#         self.items[key] = value
#         self.index[value] = key

    def __iter__(self):
        for i in self.index:
            yield i

    def Id(self):
        return self.name

    def Store(self, item):
        if not isinstance(item, event.Event):
            raise SystemError, "Not an Event in Store!"
        self.index[item] = 1
        item.StoreCallback(self)
        self.max_usage = max(self.max_usage, len(self))
        global total_stored
        total_stored += 1
        if self.name.startswith("ST_P"):
            if len(self) > 800:
                raise KeyError, "Aaaaa! player overfull! " + self.name;

    def Remove(self, item):
        if self.Has(item):
            del self.index[item]
            item.UnStoreCallback(self)
            global total_stored
            total_stored -= 1
        else:
            raise KeyError, "attempt to remove non-existing event: " + str(item)

    def Clear(self):
        for it in list(self):
            self.Remove(it)

    def Has(self, item):
        return item in self.index

    def MergeFrom(self, st):
        for item in st:
            if not self.Has(item):
                self.Store(item)

    def MaxUsage(self):
        return self.max_usage


def New(name):
    s = Storage(name)
    return s

    
