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
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MONITOR_DIR "/home/mailmynet/Maildir/new/"

static double gettime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + 1e-6 * t.tv_usec;
}

int main(int argc, char *argv[])
{

    struct inotify_event *e;
    char file_buf[1500];
    
    e = (struct inotify_event *) file_buf;
    int ifd = inotify_init();
    //printf("ifd: %d\n", ifd);
    if (ifd < 0)
	perror("inotify_init");
    int wd =
	inotify_add_watch(ifd, MONITOR_DIR, IN_CREATE | IN_ONLYDIR);
	//inotify_add_watch(ifd, MONITOR_DIR, IN_CLOSE_WRITE | IN_MOVED_TO);
	//inotify_add_watch(ifd, MONITOR_DIR, IN_CLOSE_WRITE | IN_ONLYDIR);
	//inotify_add_watch(ifd, MONITOR_DIR, IN_MOVED_TO | IN_ONLYDIR);
    //printf("wd: %d\n", wd);
    if (wd < 0)
	perror("inotify_add_watch");

	char cmd[128];                      
	sprintf(cmd, "python sendemail.py");
	printf("bf_send: %lf\t", gettime());
        system(cmd);

	printf("aft_send: %lf\t", gettime());

		    int ret = read(ifd, e, sizeof(file_buf));
		    if (ret < 0) {
	//		if (errno == EAGAIN) {
	//		    continue;
	//		} else {
			    perror("read");
			    exit(0);
	//		}
		    } else if (ret == 0)
			printf("ret = 0\n");
		    else
			printf("rcvd: %lf\n", gettime());
	//		printf("ret > 0\n");
	//	    printf("[%s]\n", e->name);

//			char cmd[128];
//			sprintf(cmd, "sendemail -f mailmynet@hatswitch.crhc.illinois.edu -t netarch.emma@gmail.com -m 'oo' -u 'RE: www.google.com' -a data");
//			system(cmd);

    return 0;
}
