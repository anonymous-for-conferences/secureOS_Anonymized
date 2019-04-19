#!/bin/bash

useradd A
useradd B

echo "I am file1." > f1
echo "I am file2." > f2

chown A:B f1
chmod 660 f1

chown A:A f2
chmod 660 f2

