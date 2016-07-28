#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os 
import signal
import subprocess
import sys
import threading
import time
import serial
import numpy as np
import string
import struct
import math
import matplotlib
import matplotlib.pyplot as plt
import os.path

ARCH = sys.argv[1]

file = open( ARCH,'r' )

datoSPS = []
tiempoSPS = []
dato = []
tiempo = []
temp = []
pal=''
temporal = []


for line in file:
  for palabra in line:
    if palabra != '\n' and palabra != ' ' and palabra != '\r':    
      temp.append(palabra)
    if palabra == '\n' or palabra == ' ' or palabra == '\r':
      for i in temp:
        pal = pal+i
      temporal.append(pal)
      del temp[:]
      pal = ''
  tiempoSPS.append(int(temporal[0]))
  datoSPS.append(float(temporal[1]))
  del temporal[:]    

datoSPS[len(datoSPS)-1] = datoSPS[len(datoSPS)-2]

file.close()

for i in range(len(datoSPS)):
  if (i/2*2)==i:
#    print repr(i/2*2) + " " + repr(i) + " " + repr(i/2)
    tiempo.append(i/2)
    dato.append((datoSPS[i]+datoSPS[i-1])/2)

dato[0] = dato[1]

temperatura = []

a=0.0958513
b=6.23236
erra = 0.0009798
errb = 0.515
erri = 1.09833 

for i in dato:
  temperatura.append(i*a + b)
  
doc = './temperatura-A' 
 
file = open(doc,'w')

for i in range(len(dato)):
  file.write(repr(tiempo[i]) + " " +repr(temperatura[i]) + " " + repr(i*erra + errb + a*erri) + '\n')
  
file.close()



for i in range(350):
  temporal.append(dato[i])
#  print repr(tiempo[i]) + " " + repr(dato[i]) + " " + repr(dato[i+1]) + " " + repr(math.fabs(dato[i+1] - dato[i]))
  if tiempo[i] > int(sys.argv[2]):
#    print repr(tiempo[i]) + " es menor a " + repr(int(sys.argv[2]))
#  if math.fabs(dato[i+1] - dato[i]) >  5:
#  if tiempo[i] > sys.argv[2]:
#    print repr(tiempo[i]) + " " + repr(dato[i]) + " " + repr(dato[i+1]) + " " + repr(math.fabs(dato[i+1] - dato[i]))
    lineal = tiempo[i]
    break


#meancold = np.mean(temporal)
#stdcold = np.std(temporal)
endtimecold = lineal


del temporal[:]    

doc = './lneal-A1'

file = open(doc,'w')


for i in range(lineal):
  file.write(repr(tiempo[i]) + " " +repr(temperatura[i]) + " " + repr(dato[i]*erra + errb + a*erri) + '\n')
  temporal.append(temperatura[i])
  
file.close()  

meancold = np.mean(temporal)
stdcold = np.std(temporal)

del temporal[:]

    
  
for i in range(350):
  temporal.append(dato[len(dato)-1-i])
#  print repr(int(sys.argv[3])) + " " + repr(tiempo[len(dato)-1-i])
#  if math.fabs(dato[len(dato)-1-i-1] - dato[len(dato)-1-i]) > 5:
  if tiempo[len(dato)-1-i] < int(sys.argv[3]):
    lineal = i
#    print repr(tiempo[len(dato)-1-i]) + " " + repr(dato[len(dato)-2-i]) + " " + repr(dato[len(dato)-1-i]) + " " + repr(math.fabs(dato[len(dato)-2-i] - dato[len(dato)-1-i]))
#    print lineal
    break
    

#meanhot = np.mean(temporal)
#stdhot = np.std(temporal)  
starttimehot = len(dato)-lineal

del temporal[:]
    
doc = './lneal-A2'

file = open(doc,'w')


for i in range(lineal):
  file.write(repr(tiempo[len(dato)-1-i]) + " " +repr(temperatura[len(dato)-1-i]) +  " " + repr(dato[len(dato)-1-i]*erra + errb + a*erri) + '\n')
  temporal.append(temperatura[len(dato)-1-i])
 
file.close() 

meanhot = np.mean(temporal)
stdhot = np.std(temporal)

del temporal[:]

endtimecold = int(sys.argv[4])
starttimehot = int(sys.argv[5])

doc = './lneal-A3'
file = open(doc,'w')
#print repr(starttimehot - endtimecold)
for i in range( starttimehot - endtimecold ):
  file.write(repr(tiempo[endtimecold + i]) + " " +repr(temperatura[endtimecold + i]) +  " " + repr(dato[endtimecold + i]*erra + errb + a*erri) + '\n')
  
file.close()
  


doc = './times.txt'
file = open(doc,'w')


file.write("End Time Cold " +	repr(endtimecold) + '\n')
file.write("Start Time Hot " + repr(starttimehot) + '\n')
file.write("End Time Hot Data " + repr(temperatura[len(dato)-1]) + " ERR: " + repr(dato[len(dato)-1]*erra + errb + a*erri) + '\n' )
file.write("Start Time Cold Data " + repr(temperatura[0]) + " ERR: " + repr(dato[0]*erra + errb + a*erri) + '\n' )
file.write("End Time Hot Time " + repr(tiempo[len(dato)-1]) + '\n' )
file.write("Hot Temperature " + repr(int(sys.argv[6])) + '\n' )

file.close()  

subprocess.call(["./analisis.sh"])  

print "\n"
print "Promedios y Desviaciones sobre datos de temperatura individual"
print "\n"
print "Cold"
print "Mean: " + repr(meancold) + " Std Deviation: " + repr(stdcold)
print "Hot"  
print "Mean: " + repr(meanhot) + " Std Deviation: " + repr(stdhot)
print "\n"

  
  
  
  