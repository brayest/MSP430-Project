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


# Configuración del puerto serial

#SERIAL_PORT_NAME	= "/dev/ttyACM0"
SERIAL_PORT_NAME	= "/dev/ttyUSB0"
#SERIAL_PORT_BAUDRATE    = 9600
SERIAL_PORT_BAUDRATE	= 115200

# Posibles valores: FIVEBITS, SIXBITS, SEVENBITS, EIGHTBITS
SERIAL_PORT_BYTE_SIZE   = serial.EIGHTBITS


# Posibles valores: PARITY_NONE, PARITY_EVEN, PARITY_ODD PARITY_MARK, PARITY_SPACE
SERIAL_PORT_PARITY	= serial.PARITY_NONE
# Posibles valores: STOPBITS_ONE, STOPBITS_ONE_POINT_FIVE, STOPBITS_TWO
SERIAL_PORT_STOPBITS    = serial.STOPBITS_ONE
SERIAL_PORT_TIMEOUT     = 1
SERIAL_PORT_SOFT_CTRL   = False
# para otras configuraciones ver documentación de pySerial

 
#COMMAND = sys.argv[1] 

 
# Configuración de inicio de comunicación
INIT_CMDS = ['SM 2']

# Configuración de fin de comunicación
END_CMDS = ['CK 0', 'AD 0', 'CT 0' ]

# Código de caracter de fin de linea (tecla ENTER)
ENDLN_COD = 13
ENDLN_CHAR = chr( ENDLN_COD )


# Tiempo de espera entre comandos (segundos)
TIME_CMD_DLY = 1



# Do not change

Puerto =  serial.Serial( port=SERIAL_PORT_NAME, 
                                                baudrate=SERIAL_PORT_BAUDRATE,
                                                bytesize=SERIAL_PORT_BYTE_SIZE,
                                                parity	=SERIAL_PORT_PARITY,
                                                stopbits=SERIAL_PORT_STOPBITS,
                                                timeout =SERIAL_PORT_TIMEOUT,
                                                xonxoff =SERIAL_PORT_SOFT_CTRL	)

                                                

def SEND( CMDS ):
  for line in CMDS:
    Puerto.write(line + ENDLN_CHAR)  
    time.sleep( TIME_CMD_DLY )




def OBTENER_DATOS():
  doc = './DATOS/ADC-' + time.strftime("%y-%m-%d--%H-%M")
  file = open(doc,'w')
  valores = []
  valoresNUM = []
  cont = 0
  exp = 3
  hex = ['A','B','C','D','E','F']
  num = ['10','11','12','13','14','15']
  conv = 0
  while True:
    valor = Puerto.readline() 
    if len(valor) > 0:
      for letter in valor:
        if letter != '\n' and letter != '\r' and letter != ' ':
          for j in range(6):
            if letter == hex[j]:
              letter = num[j] 
          valores.append(int(letter))
    else: break
    for i in valores:
      conv += (i*math.pow(16,exp))
      exp = exp -1
    file.write(repr(cont) + " " +repr(conv) + '\n')
#    print repr(cont) + " " +repr(conv) + '\n'
    valoresNUM.append(conv)
    del valores[:]  
    conv = 0
    exp = 3
    cont +=1
  file.close()
  print 'Passing ' + doc + ' to analysis.'
  subprocess.call(["python","analysis.py", doc, "LINE"])
#  plt.plot(valoresNUM)
#  plt.show()
  
def CONTAR(MAX):
  doc = './DATOS/CONT-' + time.strftime("%y-%m-%d--%H-%M")
  file = open(doc,'w')
  palabra = False
  Numeros = {'0','1', '2', '3', '4', '5', '6', '7', '8', '9'}
  Datos = []
  Valores = []
  cont = 0.00
  Number = ''
  while True:
    print 'Completado:	'  + repr((cont*100.00)/float(MAX))
#    os.system("clear")
    valor = Puerto.readline()
    if len(valor) > 0:
      for letter in valor:
        if letter != '\n' and letter != '\r' and letter != ' ':
          if not letter in Numeros:
            palabra = True
            break
      if palabra == False:
        for letter in valor:
          if letter != '\n' and letter != '\r' and letter != ' ':
            Datos.append(letter)
        for dato in Datos:
          Number += dato
        if Number != '':
          Valores.append(int(Number))
#          print Number
          file.write(repr(int(Number)) + '\n')
          cont = cont +1
    else: break
    palabra = False
    Number = ''
    del Datos[:]
  file.close()
  Valores[0] = Valores[1]
  print 'Passing ' + doc + ' to analysis.'
  subprocess.call(["python","analysis.py", doc, "HIST"])
#  , bins, patches = plt.hist(Valores, bins= 30)
#  plt.show()

    

  
def HELP( NONE, NONE2 ):
      print '\n'
      print 'Usage: python msp430ecfm.py [OPTION] [ARGUMENT]'
      print '\n'
      print 'sudo command may be requiered in order to open /dev/tty[USB0]'
      print '\n'
      print 'OPTIONS ARGUMENT:'
      print '\n'
      print 'ADC limit sps				; ADC Function with sps(samples per second: limit 500) until limit(seconds), PIN: P1.3'
      print 'CLOCK limit frec				; Clock Function with Frecuency (Hz) max: 100Hz for limit(seconds), OFF: to turn off, PIN: P1.0'
      print 'COUNT limit 				; Count until limit(seconds), PIN: P1.4'
      print 'HELP					; Show commands help'
      print '\n'
      print 'Example:'
      print '		sudo python msp430ecfm.py ADC 10 100	; Activates ADC for 10 seconds with 100sps' 
      print '\n'

def COUNT( LIMIT, NONE ):
  S_CT = ['CT 1 '+ LIMIT]  
  E_CT = ['CT 0']
  SEND(S_CT)
  CONTAR(LIMIT)
  SEND(E_CT)
  
def ADC( LIMIT, SPS ):
  S_AD = ['AD 1 ' + LIMIT + ' ' + SPS]  
  E_AD = ['AD 0', 'CK 0']
  SEND(S_AD)
  OBTENER_DATOS()
  SEND(E_AD)
  
def CLOCK( LIMIT,FREC ):
  S_CK = ['CK 1 ' + LIMIT + ' ' + FREC]
  E_CK = ['CK 0'] 
  if LIMIT == 'OFF':
    SEND(E_CK)
  else:
    SEND(S_CK)

  
OPTIONS = {
  'HELP': HELP,
  'COUNT': COUNT,
  'ADC': ADC,
  'CLOCK': CLOCK,
}



POS = ['HELP','COUNT','ADC','CLOCK']

if len(sys.argv) == 3 or len(sys.argv) == 4:
  for i in POS:    
    if sys.argv[1] == i: 
      COMMAND = sys.argv[1]
      try:
        if len(sys.argv) == 3: 
          int(sys.argv[2])
          ARG = sys.argv[2]
          ARG2 = 'NONE'
        if len(sys.argv) == 4:
          int(sys.argv[2])
          ARG = sys.argv[2]
          int(sys.argv[3])
          ARG2 = sys.argv[3]    
      except:
        if sys.argv[2] == 'OFF':
          ARG = sys.argv[2]
          ARG2 = 'NONE'
        else:
          COMMAND = 'HELP'
          ARG = 'NONE'
          ARG2 = 'NONE'
      break
    else:
      COMMAND = 'HELP' 
      ARG = 'NONE'
      ARG2 = 'NONE'
else:
  COMMAND = 'HELP'
  ARG = 'NONE'
  ARG2 = 'NONE'

SEND( INIT_CMDS ) 
OPTIONS[COMMAND]( ARG,ARG2 )
  

Puerto.close()  


  



