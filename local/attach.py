import email, os, sys 
mail = email.message_from_string(sys.stdin.read()) 
for part in mail.walk():
    if part.get('Content-Disposition') is None:
	continue
    fp = open(os.path.join('/home/wzhou10/email_censor/local/', part.get_filename()), 'wb')
    fp.write(part.get_payload(decode=True))
    fp.close()

