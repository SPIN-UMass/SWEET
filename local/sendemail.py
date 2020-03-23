import smtplib
import sys

from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email import Encoders

fromaddr = "netarch.emma@gmail.com"
toaddr = "mailmynet@hatswitch.crhc.illinois.edu"
#attach = sys.argv[1]
subject = "www"

msg = MIMEMultipart()
msg['From'] = fromaddr
msg['To'] = toaddr
msg['Subject'] = subject

#part = MIMEBase('application', "octet-stream")
#part.set_payload( open(attach,"rb").read() )
#Encoders.encode_base64(part)
#part.add_header('Content-Disposition', 'attachment; filename='+attach)
#msg.attach(part)

smtp = smtplib.SMTP('smtp.gmail.com:587')
#smtp = smtplib.SMTP('smtp.gmail.com', 587)
smtp.ehlo()
smtp.esmtp_features["auth"] = "LOGIN PLAIN" 
#smtp.debuglevel = 5
smtp.starttls()
smtp.ehlo()
smtp.login('netarch.emma', "emailcensor")
smtp.sendmail(fromaddr, toaddr, msg.as_string() )
smtp.quit()
#smtp.close()
