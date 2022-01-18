10 poke 53272, 21 :rem *d018=15 setup vic memory
20 poke 53280, 7 :rem *d020=7 yellow border
30 poke 53281, 11:rem *d021=11 gray text bg
40 p = 1024 :rem $400
50 c = 0
60 poke p, c
70 c = c + 1
71 if c > 255 then c = 0
80 p = p + 1
90 if p < 2048 goto 60
100 p = 55296 :rem $d800
120 poke p, 5 :rem green
140 p = p + 1
150 if p < 56320 goto 120
160 goto 160