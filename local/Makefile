.PHONY:all clean
CC = gcc

CFLAGS = -O0 -g -Wall
LDFLAGS = -O0 -g -Wall
DEFS = 
#DEFS =  -D_DEBUG
INCS =

CLIENT_LIBS = -pthread -lstdc++ -lrt
SERVER_LIBS = -pthread -lstdc++ -lrt

#
#	FIXME: the proxy library could actually be written in
#	plugin format.
#
ALL_OBJ = $(patsubst %.cc, %.o, $(wildcard *.cc))
TEST_OBJ = $(patsubst %.cc, %.o, $(wildcard tst-*.cc) $(wildcard test-*.cc))
TESTS = test1 test2
COMMON_OBJ = $(filter-out client.o server.o sender.o $(TEST_OBJ), $(ALL_OBJ))
#concentrator.o mail.o request.o response.o
CLIENT_OBJ = client.o sender-client.o $(COMMON_OBJ) $(PROXY_OBJ)
SERVER_OBJ = server.o sender-server.o $(COMMON_OBJ) $(PROXY_OBJ)

all: mail_client mail_server

mail_client: $(CLIENT_OBJ)
	@echo 'linking client' 
	$(CC) $(LDFLAGS) $^ -o $@ $(CLIENT_LIBS)
	@cp $@ client

mail_server: $(SERVER_OBJ)
	@echo 'linking server'
	$(CC) $(LDFLAGS) $^ -o $@ $(SERVER_LIBS)
	@cp $@ server

tests: $(TESTS)

test1: test-rw.cc tunnel_data.cc
	$(CC) $(CFLAGS) $(DEFS) $(INCS) $^ -o test1 $(SERVER_LIBS)

test2: test-client-connect.cc mail_client
	$(CC) $(CFLAGS) $(DEFS) $(INCS) $< -o test2 $(CLIENT_LIBS)

$(PROXY_OBJ):%.o:%.cc
	$(CC) $(PROXY_FLAGS) $(PROXY_DEFS) $(PROXY_INCS) -c $< -o $@

sender-client.o: sender.cc
	$(CC) -DUSE_MAIL_DAEMON $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

sender-server.o: sender.cc
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

%.o:%.cc
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

clean:
	@echo 'cleaning up'
	@rm -f mail_client
	@rm -f mail_server
	@rm -f $(TESTS)
	@rm -f *.o

