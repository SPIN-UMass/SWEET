#! /usr/bin/python
from twisted.internet import reactor
from twisted.protocols import socks
#from twisted.protocols.socks import SOCKSv4Factory
#import socks 

if '__main__' == __name__:
    reactor.listenTCP(1080,socks.SOCKSv4Factory("./socks.log"))
    #reactor.listenTCP(1080,socks.SOCKSv4Factory("./socks.log"))
    reactor.run()
