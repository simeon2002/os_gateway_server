#!/bin/bash

# Run client targets in separate terminals
#!/bin/bash

# Run client targets in separate terminals
gnome-terminal -- bash -c "make run_client1; sleep 2; exit" &
gnome-terminal -- bash -c "make run_client2; sleep 2; exit" &
gnome-terminal -- bash -c "make run_client3; sleep 2; exit"
gnome-terminal -- bash -c "make run_client4; sleep 2; exit"
gnome-terminal -- bash -c "make run_client5; sleep 2; exit"
exit
