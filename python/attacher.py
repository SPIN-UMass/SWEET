


#import formatter
#import htmllib

url="http://www.cnn.com"
filehandle = urllib.urlopen(url)



#w = formatter.DumbWriter() # plain text
#f = formatter.AbstractFormatter(w)

#p = htmllib.HTMLParser(f)
#p.feed(filehandle.read())

#p.close()
#filehandle.close()

fromaddr = "ahouman2@hatswitch.crhc.illinois.edu"

msg = MIMEMultipart()
msg['From'] = "amir"
msg['To'] = "asdsadad"
msg['Date'] = formatdate(localtime=True)
msg['Subject'] = "salam"

text = "saaalaaam"
msg.attach( MIMEText(text) )


part = MIMEBase('application', "octet-stream")
part.set_payload( filehandle.read() )
Encoders.encode_base64(part)
part.add_header('Content-Disposition', 'attachment; filename="file.html"')
msg.attach(part)

smtp = smtplib.SMTP("localhost")
smtp.sendmail("amir@localhost", "freeman77200@gmail.com", msg.as_string() )
smtp.close()

