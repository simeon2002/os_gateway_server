#!/bin/bash

# Run client targets in separate terminals
#!/bin/bash

# Run client targets in separate terminals
gnome-terminal -- bash -c "make runclient1; sleep 2; exit" &
gnome-terminal -- bash -c "make runclient2; sleep 2; exit" &
gnome-terminal -- bash -c "make runclient3; sleep 2; exit" &
gnome-terminal -- bash -c "make runclient4; sleep 2; exit" &
gnome-terminal -- bash -c "make runclient5; sleep 2; exit" &
exit
