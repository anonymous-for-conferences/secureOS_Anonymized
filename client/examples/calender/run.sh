#!/bin/bash

useradd alice
useradd bob
useradd scheduler

touch AC BC

echo "This is my Alice's calender." > AC
echo "This is my Bob's calender." > BC

chown scheduler:scheduler .

chown alice:scheduler AC
chmod 640 AC

chown bob:scheduler BC
chmod 640 BC

echo "start secureshell as 'scheduler' and run schedule program."
