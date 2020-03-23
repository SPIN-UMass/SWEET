import smtplib  
import sys

if len(sys.argv) !=2:
	sys.exit("provide the requested URL\n")

fromaddr = 'freeman77200@gmail.com'  
#toaddrs  = 'ahouman2@hatswitch.crhc.illinois.edu'  
toaddrs = 'houmansadr@gmail.com'

url = sys.argv[1]
msg = 'URL: '+ url  + '\n'

#print msg

# Credentials (if needed)  
username = 'freeman77200@gmail.com'  
password = 'alexilalas'  

# The actual mail send  
server = smtplib.SMTP('smtp.gmail.com:587')
#server = smtplib.SMTP('localhost')    
server.starttls()  
server.login(username,password)  
server.sendmail(fromaddr, toaddrs, msg)  
server.quit()  
