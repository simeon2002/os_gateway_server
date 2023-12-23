#!/bin/bash

# Run client targets in separate terminals
#!/bin/bash

# Run client targets in separate terminals
#gnome-terminal -- bash -c "make run_server; read -p 'Press Enter to close this terminal';" &
gnome-terminal -- bash -c "make run_client1; sleep 2; exit" &
exit