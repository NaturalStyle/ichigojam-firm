10 ' ENV.IN Test Program
20 ENV.IN [0]
30 IF [4]<>0 THEN ? "ERR=";[4]:GOTO 20
40 T=[0]
50 ? "T=";
60 IF T<0 ? "-";:T=-T
70 ? T/10;".";T%10;"C";
80 ? " H=";[1]/10;".";[1]%10;"%";
90 IF [5]=3 THEN ? " P=";[2]/10;".";[2]%10;"hPa"
100 IF [5]=1 THEN ? " P=N/A"
110 WAIT 60
120 GOTO 20
