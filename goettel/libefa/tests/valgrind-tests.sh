#! /bin/sh

wd=`pwd`
valgrind --quiet --log-file=efatests.vgd --leak-check=yes "$wd/tests/.libs/efatests" &> efatests.out
