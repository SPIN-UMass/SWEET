import imaplib2, time, email, os, webbrowser
from threading import *

detach_dir = '.'
# This is the threading object that does all the waiting on 
# the event
class Idler(object):
    def __init__(self, conn):
        self.thread = Thread(target=self.idle)
        self.M = conn
        self.event = Event()
 
    def start(self):
        self.thread.start()
 
    def stop(self):
        # This is a neat trick to make thread end. Took me a 
        # while to figure that one out!
        self.event.set()
 
    def join(self):
        self.thread.join()
 
    def idle(self):
        # Starting an unending loop here
        while True:
            # This is part of the trick to make the loop stop 
            # when the stop() command is given
            if self.event.isSet():
                return
            self.needsync = False
            # A callback method that gets called when a new 
            # email arrives. Very basic, but that's good.
            def callback(args):
                if not self.event.isSet():
                    self.needsync = True
                    self.event.set()
            # Do the actual idle call. This returns immediately, 
            # since it's asynchronous.
            self.M.idle(callback=callback)
            # This waits until the event is set. The event is 
            # set by the callback, when the server 'answers' 
            # the idle call and the callback function gets 
            # called.
            self.event.wait()
            # Because the function sets the needsync variable,
            # this helps escape the loop without doing 
            # anything if the stop() is called. Kinda neat 
            # solution.
            if self.needsync:
                self.event.clear()
                self.dosync()
 
    # The method that gets called when a new email arrives. 
    # Replace it with something better.
    def dosync(self):
        print "Got an event!"
        typ, data = self.M.SEARCH(None, 'UNSEEN')
        
        for emailid in data[0].split():
            resp, data = self.M.fetch(emailid, "(RFC822)")
            email_body = data[0][1] # getting the mail content
            mail = email.message_from_string(email_body) # parsing the mail content to get a mail object
            #if mail['From'] == '<hope_ren_0_0@yahoo.com.cn>':
            #   print "from zixuan"
            #Check if any attachments at all
            if mail.get_content_maintype() != 'multipart':
                continue
            
            print "["+mail["From"]+"] :" + mail["Subject"]

    # we use walk to create a generator so we can iterate on the parts and forget about the recursive headach
            for part in mail.walk():
            # multipart are just containers, so we skip them
                if part.get_content_maintype() == 'multipart':
                    continue

            # is this part an attachment ?
		if part.get('Content-Disposition') is None:
		    continue

            	filename = part.get_filename()
            #print filename
            	counter = 1
            
            # if there is no filename, we create one with a counter to avoid duplicates
            	if not filename:
               	    filename = 'part-%03d%s' % (counter, 'bin')
                    counter += 1
#		filename = 'website'
                att_path = os.path.join(detach_dir, filename)

            #Check if its already there
            	if not os.path.isfile(att_path) :
                	# finally write the stuff
                    fp = open(att_path, 'wb')
                    fp.write(part.get_payload(decode=True))
                    fp.close()

        
        #for id in data[0].split():
         #   if not id in self.knownAboutMail:
            #get From and Subject fields from header
          #  headerFields = self.getMessageHeaderFieldsById(id, ('From', 'Subject'))


# Had to do this stuff in a try-finally, since some testing 
# went a little wrong.....
try:
    # Set the following two lines to your creds and server
    M = imaplib2.IMAP4_SSL()
    #M = imaplib2.IMAP4_SSL("imap.gmail.com")
    #M.login("netarch.emma","I'veknown")
    # We need to get out of the AUTH state, so we just select 
    # the INBOX.
    M.select("INBOX")
    # Start the Idler thread
    idler = Idler(M)
    idler.start()
    # Because this is just an example, exit after 1 minute.
    time.sleep(29*60)
finally:
    # Clean up.
    idler.stop()
    idler.join()
    M.close()
    # This is important!
    M.logout()
