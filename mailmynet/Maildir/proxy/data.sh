#!/bin/bash

awk '{print ($4-$2)*1000}' email_time > sending_time
awk '{print ($6-$2)*1000}' email_time > round_time

gnuplot <<!
set term post eps color
set output "email_delay"
set size 0.6,0.6
set border 3
#set xrange[0:60]
#set yrange[0:1]
set grid
set key bottom right
set title " "
#set logscale x
#set logscale y
set xlabel "time (ms)"
set ylabel "fraction of trials"

#set key off

plot "sending_time" u 1:(1./100.) smooth cumulative ti "sending delay" w l lw 4#,\
#"round_time" u 1:(1./100.) smooth cumulative ti "round trip delay" w l lw 4

!



