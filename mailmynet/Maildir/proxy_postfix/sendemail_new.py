import smtplib
import sys

from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email import Encoders

#toaddr = "emma.netarch@gmail.com"
toaddr = "netarch.emma@gmail.com"
#toaddr = "wenxuanemma@gmail.com"
fromaddr = "mailmynet@hatswitch.crhc.illinois.edu"
attach = sys.argv[1]
subject = "www"

msg = MIMEMultipart()
msg['From'] = fromaddr
msg['To'] = toaddr
msg['Subject'] = subject

part = MIMEBase('application', "octet-stream")
part.set_payload( open(attach,"rb").read() )
Encoders.encode_base64(part)
part.add_header('Content-Disposition', 'attachment; filename='+attach)
msg.attach(part)

smtp = smtplib.SMTP("localhost")
smtp.sendmail(fromaddr, toaddr, msg.as_string() )
smtp.close()
