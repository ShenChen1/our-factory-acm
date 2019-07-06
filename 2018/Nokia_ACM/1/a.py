# -*- coding: utf-8 -*-
import sys
import os
import re
import itertools

PATH1 = "/opt/OLCC/Competition/Hangzhou_Coding_Contest_2018/problem1-20181112000000-20181125235959/result/.././cases/"
PATH5 = "/opt/OLCC/Competition/Hangzhou_Coding_Contest_2018/problem5-20181112000000-20181125235959/result/.././cases/"

def main():
  print(sys.argv)
  lsdir = os.listdir(PATH1)
  #print(lsdir)
  
  for item in lsdir:
      with open(PATH1+item,"r") as f:
          str = f.read() 
          print("*******")
          print(item,">>> \n", str)
          print("*******")
    
  exit(0)
  
if __name__ == '__main__':
  main()

