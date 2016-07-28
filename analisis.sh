#!/bin/bash


ETC=$(cat  times.txt | grep "End Time Cold" | awk '{print $4}')
STH=$(cat  times.txt | grep "Start Time Hot" | awk '{print $4}')
ETH=$(cat  times.txt | grep "End Time Hot Time" | awk '{print $5}')
Tc=$(cat times.txt | grep "Hot Temperature" | awk '{print $3}') 



cat > gnuplottemp << EOF

f(x) = a*x+b
g(x) = c*x+d
h(x) = f*x+g

fit f(x) "lneal-A2" u 1:2.3 via a,b
fit g(x) "lneal-A1" u 1:2:3 via c,d
fit h(x) "lneal-A3" u 1:2:3 via f,g


EOF

rm fit.log
gnuplot gnuplottemp  
clear
a=$(cat fit.log  | grep "a               =" | awk '{print $3}')
a=$(echo $a | sed -e 's/[eE]+*/\*10\^/')
a=$(echo "scale=10; $a" | bc)
erra=$(cat fit.log  | grep "a               =" | awk '{print $5}')
erra=$(echo $erra | sed -e 's/[eE]+*/\*10\^/')
erra=$(echo "scale=10; $erra" | bc)

b=$(cat fit.log  | grep "b               =" | awk '{print $3}')
b=$(echo $b | sed -e 's/[eE]+*/\*10\^/')
b=$(echo "scale=10; $b" | bc) 
errb=$(cat fit.log  | grep "b               =" | awk '{print $5}')
errb=$(echo $errb | sed -e 's/[eE]+*/\*10\^/')
errb=$(echo "scale=10; $errb" | bc) 

c=$(cat fit.log  | grep "c               =" | awk '{print $3}')
c=$(echo $c | sed -e 's/[eE]+*/\*10\^/')
c=$(echo "scale=10; $c" | bc) 
errc=$(cat fit.log  | grep "c               =" | awk '{print $5}')
errc=$(echo $errc | sed -e 's/[eE]+*/\*10\^/')
errc=$(echo "scale=10; $errc" | bc) 

d=$(cat fit.log  | grep "d               =" | awk '{print $3}')
d=$(echo $d | sed -e 's/[eE]+*/\*10\^/')
d=$(echo "scale=10; $d" | bc) 
errd=$(cat fit.log  | grep "d               =" | awk '{print $5}')
errd=$(echo $errd | sed -e 's/[eE]+*/\*10\^/')
errd=$(echo "scale=10; $errd" | bc) 

f=$(cat fit.log  | grep "f               =" | awk '{print $3}')
f=$(echo $f | sed -e 's/[eE]+*/\*10\^/')
f=$(echo "scale=10; $f" | bc) 
errf=$(cat fit.log  | grep "f               =" | awk '{print $5}')
errf=$(echo $errf | sed -e 's/[eE]+*/\*10\^/')
errf=$(echo "scale=10; $errf" | bc) 

g=$(cat fit.log  | grep "g               =" | awk '{print $3}')
g=$(echo $g | sed -e 's/[eE]+*/\*10\^/')
g=$(echo "scale=10; $g" | bc) 
errg=$(cat fit.log  | grep "g               =" | awk '{print $5}')
errg=$(echo $errg | sed -e 's/[eE]+*/\*10\^/')
errg=$(echo "scale=10; $errg" | bc) 


#Caliente-Medio
TCM=$(echo "scale=5; ($g - $b)/($a-$f)" | bc)
DCM=$(echo "scale=5; $a*$TCM + $b" | bc)


#Fria-Medio
TFM=$(echo "scale=5; ($g - $d)/($c-$f)" | bc)
DFM=$(echo "scale=5; $c*$TFM + $d" | bc)


cat > gnuplottemp << EOF
set term png
set output 'pics/grafica1.png'

f(x) = a*x+b
g(x) = c*x+d
h(x) = f*x+g

fit f(x) "lneal-A2" u 1:2.3 via a,b
fit g(x) "lneal-A1" u 1:2:3 via c,d
fit h(x) "lneal-A3" u 1:2:3 via f,g

set grid
set xlabel "Tiempo(s)"
set ylabel "Temperatura(C)"
plot "temperatura-A" t "", (x >= $TFM && x <= $TCM) ? h(x) : 1/0 t "", (x >= 0 && x <=$TFM) ? g(x) : 1/0 t "", (x >= $TCM && x <= $ETH) ? f(x) : 1/0 t ""

   
EOF
gnuplot gnuplottemp
clear
rm fit.log

cat > gnuplottemp << EOF
set term png
set output 'pics/grafica2.png'

f(x) = a*x+b
g(x) = c*x+d
h(x) = f*x+g

fit f(x) "lneal-A2" u 1:2.3 via a,b
fit g(x) "lneal-A1" u 1:2:3 via c,d
fit h(x) "lneal-A3" u 1:2:3 via f,g

set grid
set xlabel "Tiempo(s)"
set ylabel "Temperatura(C)"
plot "lneal-A3" t "", "lneal-A1" t "", "lneal-A2" t "", (x >= $TFM && x <= $TCM) ? h(x) : 1/0 t "", (x >= 0 && x <=$TFM) ? g(x) : 1/0 t "", (x >= $TCM && x <= $ETH) ? f(x) : 1/0 t ""

   
EOF


   
gnuplot gnuplottemp
clear   

#Capacidad calorifica
Ca=4.184
Mf=70
Mc=30
#Tc=74
DCF=$(echo "scale=5; $c*$ETH + $d" | bc)

Cc=$(echo "scale=5; -$Ca*($Mf+$Mc*(($DCM-$Tc)/($DCM-$DFM)))" | bc)

#ERRORES
ERRTCM=$( echo "define abs(x) {if (x<0) {return -x}; return x;} scale=5; abs(1/($a-$f))*( ($errg- $errb) + ( ($g-$b)/($a-$f) )*($errf-$erra) )" | bc)  
ERRDCM=$( echo "define abs(x) {if (x<0) {return -x}; return x;} scale=5; $TCM*$erra + $errb + abs($a)*$ERRTCM" | bc)
ERRTFM=$( echo "define abs(x) {if (x<0) {return -x}; return x;} scale=5; abs(1/($c-$f))*( ($errg- $errd) + ( ($g-$d)/($c-$f) )*($errf-$errc) )" | bc)  
ERRDFM=$( echo "define abs(x) {if (x<0) {return -x}; return x;} scale=5; $TFM*$errc + $errd + abs($a)*$ERRTFM" | bc)
ERRTc=1
ERRMf=1
ERRMc=1

ERRCc=$( echo "define abs(x) {if (x<0) {return -x}; return x;} scale=5; $Ca*( $ERRMf + $Mc*abs( ($DCM-$DFM)/($DCM-$Tc) )*( abs( ($ERRDCM + $ERRTc)/( $DCM -$Tc) ) + abs( ($ERRDCM  -$ERRDFM)/($DCM - $DFM) ) + abs($ERRMc/$Mc) ) )" | bc)

echo "Resultados: "
echo
echo "Masa de Agua Fría: "	$Mf " +/- " $ERRMf
echo "Masa de Agua Caliente: "	$Mc " +/- " $ERRMc
echo "Capacidad Calofífica del Agua: "	$Ca
echo "Temperatura Inicial Caliente: "	$Tc " +/- " $ERRTc
echo "Temperatura Final Fria:	"	$DFM " +/- " $ERRDFM
echo "Temperatura Final Caliente: "	$DCM " +/- " $ERRDCM
echo 
echo "***********Capacidad Calorifica Calorimetro: "	$Cc " +/- " $ERRCc





