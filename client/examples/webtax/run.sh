#!/bin/bash

useradd bob
useradd preparer

touch DB TD

echo "This is my salary slip." > TD
echo "Here are rule to calculate tax for this finiancial year" > DB

chown bob:preparer TD
chmod 640 TD

chown preparer:preparer DB
chmod 660 DB

echo "start secureshell as 'Preparer' and run Tax Computation program."
