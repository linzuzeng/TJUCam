#!/bin/env python
import sys
dict={}
for line in sys.stdin:
	sp=line.split(";")
	if len(sp)>=7 and sp[0]=="=" and sp[3].find("cam")>=0:
		dict[sp[6]]=sp[7]
for each in dict:
	if len(sys.argv)>1:
		print (each+":"+dict[each])
	else:
		print (dict[each])
