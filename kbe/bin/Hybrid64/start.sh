#!/bin/sh

./kbmachine&
sleep 1s
./dbmgr&
sleep 1s
./baseappmgr&
sleep 1s
./cellappmgr&
sleep 1s

