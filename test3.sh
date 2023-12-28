make all
port=5678
clients=3
echo -e "starting gateway "
#./sensor_gateway $port $clients &
sleep 3
echo -e 'starting 3 sensor nodes'
./sensor_node 14 3 127.0.0.1 $port &
sleep 2
./sensor_node 21 .6 127.0.0.1 $port &
sleep 2
./sensor_node 16 .5 127.0.0.1 $port &
sleep 50
killall sensor_node
sleep 30
killall sensor_gateway
