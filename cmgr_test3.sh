#!/bin/bash

# Run client targets in separate terminals
#!/bin/bash

# Run client targets in separate terminals
#gnome-terminal -- bash -c "make run_server; read -p 'Press Enter to close this terminal';" &
gnome-terminal -- bash -c "make run_client1; sleep 2; exit" &
gnome-terminal -- bash -c "sleep 2; make run_client2; sleep 5; exit" &
gnome-terminal -- bash -c "sleep 4; make run_client3; sleep 5; exit"
exit
