#!/usr/bin/python
import smtplib
import sys
import socket
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email import Encoders

fromaddr = "netarch.emma@gmail.com"
toaddr = "mailmynet@hatswitch.crhc.illinois.edu"
subject = "www"

smtp = smtplib.SMTP('smtp.gmail.com:587')
#smtp = smtplib.SMTP('smtp.gmail.com', 587)
smtp.ehlo()
smtp.esmtp_features["auth"] = "LOGIN PLAIN" 
#smtp.debuglevel = 5
smtp.starttls()
smtp.ehlo()
smtp.login('netarch.emma', "emailcensor")

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('localhost', 8889))
while 1:
  fname,addr = s.recvfrom(1024)
  msg = MIMEMultipart()
  msg['From'] = fromaddr
  msg['To'] = toaddr
  msg['Subject'] = subject
  part = MIMEBase('application', "octet-stream")
  part.set_payload( open(fname,"rb").read() )
  Encoders.encode_base64(part)
  part.add_header('Content-Disposition', 'attachment; filename='+fname)
  msg.attach(part)
  smtp.sendmail(fromaddr, toaddr, msg.as_string() )
  #print 'after send'

smtp.quit()
