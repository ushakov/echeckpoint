import random, sys

import checkpoint as CP
import storage
import player
import event
import tick

if len(sys.argv) > 2:
    small_carriers_ratio = float(sys.argv[1])
    big_carriers_ratio = float(sys.argv[2])
    clear = True
else:
    small_carriers_ratio = 0.1
    big_carriers_ratio = 0.02


num_players = 500
num_cps = 20

giveup_prob = 0.005
skip_prob = 0.2
avg_time = 10

clear = True

log_ticks = open('LOG_ticks', 'w')
log_res   = open('LOG_res', 'w')

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

    finish = storage.Storage()

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
                        finish.Store(p.blocks)
                        p.Destroy()
        tick.Advance()
        log_ticks.write("tick %d: in_game %d; finished %d, player max_mem=%d\n" % (tick.Get(), len(in_game), len(finished), player.max_mem))



    copies = {}
    for ev in event.events:
        nc = len(ev.storedAt)
        if nc in copies:
            copies[nc] += 1
        else:
            copies[nc] = 1
            
    total = len(event.events)
    for t in sorted(copies.keys()):
        log_res.write( "FT: %d: %d (%2.2f%%)\n" % (t, copies[t], copies[t] * 100.0 / total))

    log_res.write( "At Finish: %d (%2.2f%%)\n" % (finish.Events(), finish.Events() * 100.0 / total))

    for cp in checkpoints:
        log_res.write("At %s: %d (%2.2f%%)\n" % (cp.name, cp.blocks.Events(), cp.blocks.Events() * 100.0 / total))

    log_res.write("Max Mem Usage:\n")
    log_res.write("Players: %d\n" % (player.max_mem))
    log_res.write("Checkpoints:\n")
    for cp in checkpoints:
        log_res.write("%s: %d\n" % (cp.name, cp.blocks.Events()))

    for b_cp in checkpoints:
        for b_n in range(1000):
            s = "Block %s-%d" % (b_cp.name, b_n)
            empty=True
            for cp in checkpoints:
                if cp.blocks.Exists(b_cp.name, b_n):
                  s += "+"
                  empty=False
                else:
                  s += "-"
            if empty:
              break
            log_res.write(s+'\n')

#    f = open("RES-%f" % big_carriers_ratio, "a")
    f = open("RES", "a")
    f.write("%s %s %f\n" % (small_carriers_ratio, big_carriers_ratio, finish.Events() * 100.0 / total))
       
