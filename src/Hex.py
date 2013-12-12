#!/usr/bin/python

"""
Copyright (C) 2012  kirill Kulakov & Jose Carlos Granja

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import os
import sys

class Hex:
  
  def __init__ (self, fileName):
    self.memory = [0]*0x8000
    
    self.offset = 0
    self.fuseValue = [0]*0xF
    self.fuseStatus = [0]*0xF
    
    fileSize = int(os.path.getsize(fileName))
    hexFile = open(fileName, 'r')
    
    for i in range(0x8000):
      self.memory[i] = 0xFF
    for i in range(0x0F):
      self.fuseStatus[i] = 0
      
    while True:
      buf = hexFile.readline(128).translate(None, ':\n')
      if buf == "":
	break
      if self.reformat(buf) == 1:
	print "Hex file not valid."
	sys.exit(2)
    hexFile.close()
       
       
  def getData (self, address):
    return self.memory[address]
  def fuseChanged(self, fuseID):
    return self.fuseStatus[fuseID]
  def getFuse(self, fuseID):
    return self.fuseValue[fuseID]
  
  
  # HEX parser
  def reformat (self, hexData):
    buf = hexData
    iSize, buf = int(buf[:2], 16), buf[2:]
    if  iSize == -1:
      return 1
    iAddress, buf = int(buf[:4], 16), buf[4:]
    if iAddress == -1:
      return 1
    #record type
    iRecord = int(buf[:2], 16)
       
    if iRecord == -1:
      return 1
    elif iRecord == 4:
      
      buf = buf[2:]
      temp = int (buf[:2], 16)
      if temp == -1:
	return 1
      self.offset += temp<<24
      buf = buf[2:]
      temp = int (buf[:2], 16)
      if temp == -1:
	return 1
      self.offset = temp<<16
      
    elif iRecord == 2:
      
      buf = buf[2:]
      temp = int (buf[:2], 16)
      if temp == -1:
	return 1
      self.offset += temp<<12
      buf = buf[2:]
      temp = int (buf[:2], 16)
      if temp == -1:
	return 1
      self.offset = temp<<4
    
    elif iRecord == 0:
      
      if (iAddress + self.offset) < 0x8000 or (iAddress + self.offset) >= 0xF00000 :
	for i in range(iSize/2):
	  buf = buf[2:]
	  self.memory[iAddress + i*2 + self.offset] = int(buf[:2], 16)
	  if self.memory[iAddress + i*2 + self.offset] == -1:
	    return 1
	  
	  buf = buf[2:]
	  self.memory[iAddress + i*2 + self.offset + 1] = int(buf[:2], 16)
	  if self.memory[iAddress + i*2 + self.offset + 1] == -1:
	    return 1
      
      elif iAddress + self.offset >= 0x300000 and iAddress + self.offset <= 0x30000F:
	for i in range(iSize):
	  buf = buf[2:]
	  self.fuseValue[iAddress + self.offset + i - 0x300000] = int(buf[:2], 16)
	  if int(buf[:2], 16) == -1:
	    return 1;
	  
	  self.fuseStatus[iAddress + self.offset + i - 0x300000] = 1 
	  
      else:
	return 1
	
    return 0
  
  