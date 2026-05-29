10 'IR.IN Test Program
20 IR.IN 1,[0]
30 LED 0:'not busy
40 IF [5]<>0 GOTO 20
50 IF [4]<>0 GOTO 20
60 LED 1:'busy
70 ? HEX$([0],2);HEX$([1],2);HEX$([2],2);HEX$([3],2)
80 WAIT 5
90 GOTO 20
