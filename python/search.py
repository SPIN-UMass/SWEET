import smtplib  
import sys
import shutil
import os
import time
import datetime
import math
import urllib
from array import array


#data = sys.stdin.readlines()
#print "Counted", len(data), "lines."

for line in sys.stdin.readlines():
	if line[0:4] == 'URL:':
        	url = line[5:-1]
	elif line[0:5] == 'From:':
		sender = line[6:-1]
	

#print "the URL: ", url, " requested by ", sender

#writing to a file
#FILE = open("/home/amir/Desktop/filename","w")
# Write all the lines at once:
#FILE.writelines(sender)    
# Alternatively write them one by one:
#for name in namelist:
#    FILE.write(name)
#FILE.close()

##fetching the url

filehandle = urllib.urlopen(url)

#for lines in filehandle.readlines():
#	print lines

msg = filehandle.readlines()
filehandle.close()

## sending the email
fromaddr = 'amir@houman.crhc.illinois.edu'  
toaddrs  = sender  
#msg = "you asked for " + url 

server = smtplib.SMTP('localhost')
server.set_debuglevel(1)
server.sendmail(fromaddr, toaddrs, msg)
server.quit()
