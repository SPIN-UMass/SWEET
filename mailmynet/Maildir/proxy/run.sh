#!/bin/bash
rm email_time
for (( i=0; i<100; i++ ));do
    ./test_inotify >> email_time 
done

