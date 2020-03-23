# SWEET
SWEET circumvention system

# Relevant papers
[1] https://ieeexplore.ieee.org/document/7819465
[2] https://arxiv.org/pdf/1211.3191v2.pdf

@ARTICLE{SWEET-ToN,
  author={A. Houmansadr and W. Zhou and M. Caesar and N. Borisov},
  journal={IEEE/ACM Transactions on Networking},
  title={{SWEET: Serving the Web by Exploiting Email Tunnels}},
  year={2017},
  month={Jan},
  volume={25},
  number={3},
  pages={}
}

# Instructions

Codes under local/ are on client.
Codes under mailmynet/Maildir/proxy are on SWEET server.

To run them:

1. On both sides:
	a. Make
	b. run postfix
	c. forward incoming emails to attach.py to get the attachment and place it in some directory.
	(There's one example for .forward file (forward) in each directory, and where the current attach.py puts attachments is hardcoded there.)

2. On the server side:
	a. run a real http proxy 
	b. run ./mail_server, which communicates with the real through port 9034 (kPort in the code server.cc), and monitors the directorywhere email attachments are stored (kMailDir in server.cc) using inotify.


3. On the client side:
	a. run mail_daemon.py as a background process, which listens on port 8889 and is responsible for sending emails.
	b. run ./mail_client, which listens on port 9034 (kPort in the code client.cc), and monitors the directorywhere email attachments are stored (kMailDir in client.cc) using inotify.
	c. set browser's http proxy as localhost:9034
