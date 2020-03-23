/*
 ** selectserver.c -- a cheezy multiperson chat server
 */
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define PORT "1080"		// port we're listening on
#define MONITOR_DIR "/home/mailmynet/Maildir/proxy/new/"
#define LOCAL_DIR "/home/mailmynet/Maildir/proxy/"
//#define MONITOR_DIR "/home/wzhou10/email_censor/local/new/"

static double gettime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + 1e-6 * t.tv_usec;
}

void geturl(char *buf, char *url)
{
    int i;			//, j = 0;
    //char url[128]={0};
    //char* url;// = malloc(128);
    for (i = 5; buf[i] != ' '; i++);
    //printf("len: %d\n", i);
    strncpy(url, buf + 5, i - 5);
    *(url + i - 5) = '\0';
    //printf("============================\n");
    //printf("url1: %s\n", url);
    //return url;
}

int get_sfd(char *filename)
{
    return (filename[0] - '0');
}

void *send_email(void *arg)
{
    char *cmd = arg;
    system(cmd);
    printf("email sent.\n");
    free(cmd);
}

void nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    fd_set master;		// master file descriptor list
    fd_set read_fds;		// temp file descriptor list for select()
    int fdmax;			// maximum file descriptor number

    int sockfd;		
    struct sockaddr_storage remoteaddr;	// proxy address
    socklen_t addrlen;

    char buf[1500];		// buffer for client data
    int nbytes;
    char cmd[128];
    char* request[1500];

    char remoteIP[INET6_ADDRSTRLEN];

    int i, j, rv;
    int ind = 0;

    struct addrinfo hints, *ai, *p;
    
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    FD_ZERO(&master);		// clear the master and temp sets
    FD_ZERO(&read_fds);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &ai)) != 0) {
	fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
	exit(1);
    }

    struct timeval timeout = {300, 0};

    for (p = ai; p != NULL; p = p->ai_next) {
	sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if (sockfd == -1) {
	    perror("socket");
	    continue;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));

	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    close(sockfd);
	    perror("connect");
	    continue;
	}

	break;
    }

    if (p == NULL) {
	fprintf(stderr, "client: failed to connect\n");
	exit(2);
    }

    freeaddrinfo(ai);		// all done with this
    printf("sockfd: %d\n", sockfd);
    struct inotify_event *e;
    char file_buf[1500];
    e = (struct inotify_event *) file_buf;
    int ifd = inotify_init();
    printf("ifd: %d\n", ifd);
    if (ifd < 0)
	perror("inotify_init");
    int wd =
	//inotify_add_watch(ifd, MONITOR_DIR, IN_CREATE | IN_ONLYDIR);
	inotify_add_watch(ifd, MONITOR_DIR, IN_CLOSE_WRITE | IN_ONLYDIR);
	//inotify_add_watch(ifd, MONITOR_DIR, IN_MOVED_TO | IN_ONLYDIR);
    printf("wd: %d\n", wd);
    if (wd < 0)
	perror("inotify_add_watch");
    nonblock(ifd);
    FD_SET(ifd, &master);
    // add the sockfd to the master set
    nonblock(sockfd);
    FD_SET(sockfd, &master);

    // keep track of the biggest file descriptor
    fdmax = (sockfd > ifd) ? sockfd : ifd;

    int counter = 0;
    // main loop
    for (;;) {
	read_fds = master;	// copy it
	if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
	    perror("select");
	    exit(4);
	}
	// run through the existing connections looking for data to read
	for (i = 0; i <= fdmax; i++) {
	    if (FD_ISSET(i, &read_fds)) {	// we got one!!
/*		if (i == sockfd) {
		    // handle new connections
		    addrlen = sizeof remoteaddr;
		    newfd = accept(sockfd,
				   (struct sockaddr *) &remoteaddr,
				   &addrlen);

		    if (newfd < 0) {
			if (errno == EAGAIN) {
			    continue;
			} else {
			    perror("accept");
			}
			//if (newfd == -1) {
			//    perror("accept");
		    } else {
			nonblock(newfd);
			FD_SET(newfd, &master);	// add to master set
			if (newfd > fdmax) {	// keep track of the max
			    fdmax = newfd;
			}
			printf("selectserver: new connection from %s on "
			       "socket %d\n",
			       inet_ntop(remoteaddr.ss_family,
					 get_in_addr((struct sockaddr *)
						     &remoteaddr),
					 remoteIP, INET6_ADDRSTRLEN),
			       newfd);
		    }
		} else 
*/		
		if (i == ifd) {
		    printf("ifd!\n");
		    int ret = read(ifd, e, sizeof(file_buf));
		    if (ret < 0) {
			if (errno == EAGAIN) {
			    continue;
			} else {
			    perror("read");
			    exit(0);
			}
		    } else if (ret == 0)
			printf("ret = 0\n");
		    else
			printf("ret > 0\n");
		    printf("[%s]\n", e->name);

		    char file_name[256];
		    strcpy(file_name, MONITOR_DIR);
		    strcat(file_name, e->name);
		    printf("file_name: %s\n", file_name);
		   // sprintf(cmd, "cat %s | python attach.py", file_name);
		   // system(cmd);
	            sprintf(cmd, "sudo chmod 777 %s", file_name);
                    system(cmd);

		    int file_fd = open(file_name, O_RDONLY);
		    //int file_fd = open(file_name, O_RDONLY);
		    if (file_fd < 0)
			perror("infile open");

		    //fstat(fd, &stat_buf);
		    struct stat stat_buf;
		    off_t offset = 0;
		    stat(file_name, &stat_buf);
		    printf("%s, %d, %lu\n", e->name, file_fd, stat_buf.st_size);
		    //printf("fd: %d, fdsize: %lu\n", file_fd, stat_buf.st_size);
		    //get new_fd 
		    //int new_fd = get_sfd(e->name);
		    //printf("new_fd: %d\n", new_fd);
		    
		    char data[10000];
		    int j, bytes = read(file_fd, data, 1500);
		    printf("data: %s\n", data);
		    for(j = 0; j < bytes; j++)
			printf("%x ", data[j]);
		    printf("\n");
		 /*   if(!ind){
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		            close(sockfd);
            		    perror("connect");
            		    exit(0);
        		} else {
			    printf("connect\n");
			    ind=1;	
			}
		    }
*/
		    send(sockfd, data, bytes, 0);
/*		    while (1) {
			int snd_cnt =
			    sendfile(sockfd, file_fd, &offset,
				     stat_buf.st_size);
			if (snd_cnt < 0) {
			    perror("send");
			} else if (snd_cnt == 0) {
			    printf("snd_cnt == 0\n");
			//    FD_CLR(new_fd, &master);
			//    close(new_fd);
			    break;
			}
		    }*/
		    close(file_fd);
		    sprintf(cmd, "rm %s", file_name);
                    printf("cmd: %s\n", cmd);
                    system(cmd);
		   // sprintf(cmd, "rm %s%s",MONITOR_DIR,
		//	    e->name);
		  //  printf("cmd: %s\n", cmd);
		   // system(cmd);

		} else {
		    // i=sockfd, handle data from the proxy
		    printf("sockfd!\n");
		    if ((nbytes = recv(sockfd, buf, sizeof buf, 0)) <= 0) {
			// got error or connection closed by proxy
			if (nbytes < 0) {
			    if (errno == EAGAIN) {
				continue;
			    } else {
				perror("recv");
				exit(0);
			    }
			} else {
			    // connection closed
			    printf("socket %d hung up\n", i);
			}
//			close(i);	// bye!
			FD_CLR(i, &master);	// remove from master set
		    } else {
			// we got some data from the socks proxy
			counter++;		
			double aft_recv = gettime();
                        printf("nbytes: %d\nbuf: %s\n", nbytes, buf);
			for(i = 0; i < nbytes; i++)
				printf("%x ", buf[i]);
			printf("\n");
		
			char filename[128];
			char reply[16];
			sprintf(filename, "reply_%d", counter);
			//sprintf(reply, "reply_%d", counter);
			//strcpy(filename, LOCAL_DIR);
                    	//strcat(filename, reply);
                    	printf("filename: %s\n", filename);

			FILE *fp_out = fopen(filename, "w");
			if (fp_out == NULL)
			    perror("fopen");
			fwrite(buf, nbytes, 1, fp_out);
			fclose(fp_out);

/*			int fd_out = open(filename, O_WRONLY);
                        if (fd_out < 0)
                            perror("outfile open");

			write(fd_out, buf, nbytes);
			close(fd_out);
*/
			sprintf(cmd, "sendemail -f mailmynet@hatswitch.crhc.illinois.edu -t netarch.emma@gmail.com -m 'oo' -u 'RE: www.google.com' -a %s", filename);
			system(cmd);
			/*sprintf(cmd, "python sendemail_new.py %s", filename);

			pthread_t pid;
			char *cmd_copy = strdup(cmd);
			pthread_create(&pid, NULL, send_email, cmd_copy);
			pthread_detach(pid);*/
			//if(counter == 2)
			//    exit(0);

			//double bef_python = gettime();
			//system(cmd);
			//double aft_python = gettime();
			/*for(j = 0; j <= fdmax; j++) {
			   // send to everyone!
			   if (FD_ISSET(j, &master)) {
			   // except the sockfd and ourselves
			   if (j != sockfd && j != i) {
			   if (send(j, buf, nbytes, 0) == -1) {
			   perror("send");
			   }
			   }
			   }
			   } */
		    }
		}		// END handle data from proxy
	    }			// END got new incoming connection
	}			// END looping through file descriptors
    }				// END for(;;)--and you thought it would never end!

    return 0;
}
