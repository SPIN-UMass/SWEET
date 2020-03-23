#!/bin/bash

echo $1 $2
if [ $2 -eq 0 ]
then
  mv $1 /tmp/in_mails/client/
else
  mv $1 /tmp/in_mails/server/
fi
