#  -*- encoding: koi8-r -*-
import event
import block
import storage


# ����������� ����� �������� � ������ (������ ������)
# �����, ���������� �� �������� ����� �������� � ������.
# �������� ������������ ����� ������ ���������������� �������

# �������� ���� �������� �� ���� ��,
# ������� ����� �� �������� ����� ��� ����������.
# ������� �� ����� ������ �������� ����� ������� � ������� ��,
# � ������� ���� ���������� ������ ��� �������� ����� (��������, ������� �� ������ ������ �����)
# ������������������ �������� �����:
# - �������� �������
# - ���� ��������:
#   - ��������� ������ � ������
#   - ��������� ������� ���� � ������
#   - ���������� ����� ������ � ������.

class Checkpoint:

    def __init__(self, name, clear):
        self.name = name
        # ������ ������ - ���������
        self.blocks = storage.Storage()
        # ������� ����
        self.curr_bl = block.Block(name, 0)
        # ������������ ������ ����� � ��������
        self.max_bl_size = 32

    def CheckIn(self, time, player):
        player.CheckInAt(time, self)
        e = event.New(self, player)
        self.ProcessData(time, e)
        self.DoLog(time)

    def ProcessData(self, time, event):
        player = event.player
        # add event to block and register
        self.curr_bl.events += [event]
        if len(self.curr_bl.events) > self.max_bl_size:
          self.blocks.Store([self.curr_bl])
          self.curr_bl.events = []
          self.curr_bl.num+=1

        if player.IsCarrier():
	  print "getting data from player"
          self.blocks.Store(player.blocks)

          if len(self.curr_bl.events) > 0:
            self.blocks.Store([self.curr_bl])
            self.curr_bl.events = []
            self.curr_bl.num+=1
          events = 0
          player.blocks.Clear()
	  print "putting data to player"
          for i in range(len(self.blocks)):
            b = self.blocks[len(self.blocks)-1-i]
            events += len(b.events)
            if events > player.Capacity():
              break
            player.blocks.Store([b.Copy()]);

                
    def DoLog(self, time):
#        all = len(self.blocks)
#        print "+CP+\t%d\t%s\t%d\t%d" % (time, self.name, all, all - self.count)
	pass

    def LogHeader(self):
	pass
#        print "+CP+\t%s\t%s\t%s\t%s" % ("time", "name", "stored", "other_stored")
            
                        
                        

def New(name, clear):
    return Checkpoint(name, clear)
#    return StoreAndPass3Times(name)
#    return StoreOnPlayer(name)
#    return Checkpoint(name)

New("Header", False).LogHeader()
