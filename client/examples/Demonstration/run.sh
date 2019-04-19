#!/bin/bash

rm -f top_secret_file unprotected_file innocent_looking_malware remote_attacker

touch top_secret_file unprotected_file

echo "This is top secret data." > top_secret_file
echo "" > unprotected_file

chmod 600 top_secret_file
chmod 644 unprotected_file

gcc Disguised_Malware.c -o innocent_looking_malware
gcc Remote_Attacker.c -o remote_attacker
