#!/bin/bash

touch file1 file2

echo "I am file1." > file1
echo "" > file2

chmod 600 file1
chmod 644 file2

gcc P1.c -o p1
gcc P2.c -o p2
