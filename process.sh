#!/bin/bash

# Open terminal windows and run each program
gnome-terminal -- bash -c "echo 'Vehicle health'; ./alert; exec bash"
sleep 1

gnome-terminal -- bash -c "echo 'Collision'; ./collision; exec bash"
sleep 1

gnome-terminal -- bash -c "echo 'Driver Monitor'; ./drivermonitor; exec bash"
sleep 1

gnome-terminal -- bash -c "echo 'Env sensor'; ./environmental; exec bash"
sleep 1

gnome-terminal -- bash -c "echo 'gps'; ./gpsloc; exec bash"
sleep 1

gnome-terminal -- bash -c "echo 'Dashboard'; ./dashboard; exec bash"
sleep 1
#gnome-terminal -- bash -c "echo 'Starting Server with sudo'; sudo ./server; exec bash"

