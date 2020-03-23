import smtplib  
import sys
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.Utils import COMMASPACE, formatdate
from email import Encoders



if len(sys.argv) !=3:
	sys.exit("provide emails")
fromaddr = sys.argv[1]  
toaddrs  = sys.argv[2]  
text = 'http://sdfsdf.csdf sdfsdfsfsfsdf sddfsdfsdfsdfsdfsdfsdfsd'  

msg = MIMEMultipart()
msg.attach( MIMEText(text) )

server = smtplib.SMTP('localhost')
server.set_debuglevel(1)
server.sendmail(fromaddr, toaddrs, msg)
server.quit()
