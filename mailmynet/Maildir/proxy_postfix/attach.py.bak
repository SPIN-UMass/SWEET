import email, os, sys 
mail = email.message_from_string(sys.stdin.read()) 
for part in mail.walk():
    if part.get('Content-Disposition') is None:
	continue
    #filename = part.get_filename()
    #att_path = os.path.join(detach_dir, filename)
    fp = open(os.path.join('/home/mailmynet/Maildir/proxy/new/', part.get_filename()), 'wb')
    #fp = open(att_path, 'wb')
    fp.write(part.get_payload(decode=True))
    fp.close()

