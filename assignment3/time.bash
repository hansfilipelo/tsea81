#!/bin/bash

START=$(date '+%s')

./lift_pthreads

STOP=$(date '+%s')-START

echo $STOP

