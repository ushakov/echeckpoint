#/bin/bash

rm res-a log-a-*
for i in 0 1 2; do
  for j in 0 1 2 3 4 5 6 7 8 9; do
    echo -n a: $i$j...
    python simulate.py | tee log-a-$i$j | grep '^FT: 1:' >> res-a
    echo "done"
  done
done

exit 1

rm res-clear log-clear-*
for i in 0 1 2; do
  for j in 0 1 2 3 4 5 6 7 8 9; do
    echo -n clear: $i$j...
    python simulate.py --clear | tee log-clear-$i$j | grep '^FT: 1:' >> res-clear
    echo "done"
  done
done

