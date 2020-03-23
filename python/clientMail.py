
import smtplib
import sys
import shutil
import os
import time
import datetime
import math
import urllib
from array import array

from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders
import os


def sendMail(toaddr, fromaddr, subject, text, files=[],server="localhost"):
    #assert type(to)==list
    #assert type(files)==list
    
    msg = MIMEMultipart()
    msg['From'] = fromaddr
    msg['To'] = toaddr
    msg['Date'] = formatdate(localtime=True)
    msg['Subject'] = subject

    msg.attach( MIMEText(text) )

    for file in files:
    	part = MIMEBase('application', "octet-stream")
    	#part.set_payload( open(file,"rb").read() )
    	part.set_payload(files.read())
    	Encoders.encode_base64(part)
    	part.add_header('Content-Disposition', 'attachment; filename=website.html')
#                       % os.path.basename(file))
    	msg.attach(part)

    smtp = smtplib.SMTP(server)
    smtp.sendmail(fromaddr, toaddr, msg.as_string() )
    smtp.close()

if len(sys.argv) !=2:
	sys.exit("provide the requested URL\n")


fromaddr = "freeman77200@gmail.com"
toaddr = "ahouman2@hatswitch.crhc.illinois.edu"  

url = sys.argv[1]
subject = url
text = ""

sendMail(toaddr,fromaddr,subject,text)
