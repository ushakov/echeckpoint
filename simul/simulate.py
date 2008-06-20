import random, sys

import checkpoint as CP
import storage
import player
import event
import tick

if len(sys.argv) > 1 and sys.argv[1] == "--clear":
    print "Doing clear"
    clear = True
else:
    print "Doing dirty"
    clear = False


num_players = 500
num_cps = 20

small_carriers_ratio = 0.1
big_carriers_ratio = 0.02
giveup_prob = 0.005
skip_prob = 0.2
avg_time = 10

if __name__ == "__main__":
    random.seed()
    players = []
    for i in range(num_players):
        t = random.random()
        if t < small_carriers_ratio:
            p = player.New("P_%d" % i, 1)
        elif t < small_carriers_ratio + big_carriers_ratio:
            p = player.New("P_%d" % i, 2)
        else:
            p = player.New("P_%d" % i, 0)
        players += [p]

    checkpoints = []
    for i in range(num_cps):
        checkpoints += [CP.New("CP_%d" % i, clear)]

    finish = storage.New("Finish")

    in_game = range(num_players)     # players in game
    positions = [0] * num_players    # i means "before CP i"

    finished = []
    tick.Reset()
    while len(in_game) > 0:
        for pi in list(in_game):
            p = players[pi]
            if random.random() < giveup_prob:
                # player gives up, losing all info
                in_game.remove(pi)
                if players[pi].IsCarrier():
                    players[pi].Destroy()
                continue
            if random.random() < 1.0 / avg_time:
                # player arrives somewhere
                if random.random() < skip_prob:
                    # player misses the CP
                    positions[pi] += 1
                else:
                    # player arrives at CP
                    cp = checkpoints[positions[pi]]
                    cp.CheckIn(tick.Get(), p)
                    positions[pi] += 1
                if positions[pi] == num_cps:
                    # finished!
                    in_game.remove(pi)
                    finished += [pi]
                    if p.IsCarrier():
                        finish.MergeFrom(p.Storage())
                        p.Destroy()
        tick.Advance()
        print "tick %d: in_game %d; finished %d, player max_mem=%d, total stored=%d" % (tick.Get(), len(in_game), len(finished), player.max_mem, storage.total_stored)

    print "All done ======================="

    copies = {}
    for ev in event.events:
        nc = len(ev.storedAt)
        if nc in copies:
            copies[nc] += 1
        else:
            copies[nc] = 1
            
    total = len(event.events)
    for t in sorted(copies.keys()):
        print "FT: %d: %d (%2.2f%%)" % (t, copies[t], copies[t] * 100.0 / total)

    print "At Finish: %d (%2.2f%%)" % (len(finish), len(finish) * 100.0 / total)

    for cp in checkpoints:
        print "At %s: %d (%2.2f%%)" % (cp.Id(), len(cp.Storage()), len(cp.Storage()) * 100.0 / total)

    print "Max Mem Usage:"
    print "Players: %d" % (player.max_mem)
    print "Checkpoints:"
    for cp in checkpoints:
        print "%s: %d, %d maxwrites" % (cp.Id(), cp.Storage().MaxUsage(), cp.GetMaxWrites())

    for b_cp in enumerate(checkpoints):
        for b_n in range(20):
            s = "B_%d_%d" % (b_cp[0], b_n)
            for cp in enumerate(checkpoints):
                B = cp[1].GetBlock(b_cp[1].Id(), b_n)
                s += "\t%d/%d" % (len(B.events), B.transmits)
            print s

       
