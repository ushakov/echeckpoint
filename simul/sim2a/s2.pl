#!/usr/bin/gnuplot

f(x) = A1-B1*exp(-x/C1)
A1=100
B1=50
C1=0.1

g(x) = A2-B2*exp(-x/C2)
A2=100
B2=50
C2=0.1

fit f(x) "RES" using 2:3 via A1, B1, C1
fit g(x) "../sim2/RES" using 2:3 via A2, B2, C2

plot \
  "../sim2/RES" using 2:3,\
  "RES" using 2:3,\
f(x),g(x),0,100

print A1, B1, C1
print A2, B2, C2

pause -1

