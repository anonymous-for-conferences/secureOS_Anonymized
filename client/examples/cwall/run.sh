#!/bin/bash

useradd u1
useradd u2
useradd u3
useradd u4
useradd u5

useradd d11
useradd d12
useradd d2

groupadd G1
groupadd G2
groupadd G3

adduser d11 G1
adduser u1 G1
adduser u3 G1

adduser d12 G2
adduser u2 G2
adduser u4 G2

adduser d2 G3
adduser u1 G3
adduser u2 G3
adduser u5 G3

touch D11 D12 D2

chown d11:G1 D11
chmod 640 D11

chown d12:G2 D12
chmod 640 D12

chown d2:G3 D2
chmod 640 D2


