#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import signal
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

linea = []
cont = 0

POS = ['HIST','LINE']


ARG1 = ''

if len(sys.argv)  == 3:
  if os.path.exists(sys.argv[1]):
    ARG1 = sys.argv[1]
  else:
    ARG1 = 'NONE'
    print '\n \n	ERROR: ' + sys.argv[1] + ' File not found 		\n \n'
  if ARG1 != 'NONE':
    for i in POS:
      if sys.argv[2] == i:
        ARG2 = sys.argv[2]
        break
      else:
        ARG2 = 'HELP'
      
else:
  ARG2 = 'HELP'
  ARG1 = 'NONE'
      

if ARG1 != 'NONE' and ARG2 != 'HELP':
  file = open( sys.argv[1] ,'r')
  
  for line in file:
    linea.append(int(line))
    cont =cont +1
  
  linea[0] = linea[1]
  linea[cont-1] = linea[cont -2]
  file.close()

else:
  print '\n'
  print 'Usage:	'
  print '		python analysis.py /[path]/file_name HIST/LINE'
  print '\n'
  print 'LINE: Function plot'
  print	'HIST: Histogram plot'
  print '\n'

def HIST():
  print 'Mean: ' + repr(np.mean(linea))
  print 'Standard Deviation: ' +  repr(np.std(linea))
  n, bins, patches = plt.hist(linea, bins= 80)
  plt.show()

def LINE():
  plt.plot(linea)
  plt.show()
  
OPTIONS = {
  'HIST': HIST,
  'LINE': LINE,
}

OPTIONS[ARG2]()
 
  
        
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    