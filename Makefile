TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

# when executing make, compile all exe's
all: sensor_gateway sensor_node file_creator

# When trying to compile one of the executables, first look for its .c files
# Then check if the libraries are in the lib folder
sensor_gateway : main.c connmgr.c datamgr.c sensor_db.c sbuffer.c lib/libdplist.so lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_gateway *****$(NO_COLOR)"
	gcc -c main.c      -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o main.o      -fdiagnostics-color=auto
	gcc -c connmgr.c   -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o connmgr.o   -fdiagnostics-color=auto
	gcc -c datamgr.c   -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o datamgr.o   -fdiagnostics-color=auto
	gcc -c sensor_db.c -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o sensor_db.o -fdiagnostics-color=auto
	gcc -c sbuffer.c   -Wall -std=c11 -Werror -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=5 -o sbuffer.o   -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_gateway *****$(NO_COLOR)"
	gcc main.o connmgr.o datamgr.o  sensor_db.o sbuffer.o -ldplist -ltcpsock -lpthread -o sensor_gateway -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto

#target for a quick build of your source code.
sensor_gateway_quick :
	gcc -w -o sensor_gateway main.c connmgr.c datamgr.c  sensor_db.c sbuffer.c lib/dplist.c lib/tcpsock.c -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=100 -lpthread
		
sensor_gateway_debug :
	gcc -g -w -o sensor_gateway main.c connmgr.c datamgr.c  sensor_db.c sbuffer.c lib/dplist.c lib/tcpsock.c -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20 -DTIMEOUT=100 -lpthread

#file_creator program to generate a room map	
file_creator : file_creator.c
	@echo "$(TITLE_COLOR)\n***** COMPILE & LINKING file_creator *****$(NO_COLOR)"
	gcc file_creator.c -o file_creator -Wall -fdiagnostics-color=auto

#test client
sensor_node : sensor_node.c lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_node *****$(NO_COLOR)"
	gcc -c sensor_node.c -DLOOPS=100 -Wall -std=c11 -Werror -o sensor_node.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_node *****$(NO_COLOR)"
	gcc sensor_node.o -ltcpsock -o sensor_node -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto

sensor_node2 : sensor_node.c lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_node *****$(NO_COLOR)"
	gcc -c sensor_node.c -Wall -std=c11 -Werror -o sensor_node2.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_node *****$(NO_COLOR)"
	gcc sensor_node2.o -ltcpsock -o sensor_node2 -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto

sensor_node3 : sensor_node.c lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_node *****$(NO_COLOR)"
	gcc -c sensor_node.c -Wall -std=c11 -Werror -o sensor_node3.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_node *****$(NO_COLOR)"
	gcc sensor_node3.o -ltcpsock -o sensor_node3 -Wall -L./lib -Wl,-rpath=./lib -fdiagnostics-color=auto

# If you only want to compile one of the libs, this target will match (e.g. make liblist)
libdplist : lib/libdplist.so
libtcpsock : lib/libtcpsock.so

lib/libdplist.so : lib/dplist.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB dplist *****$(NO_COLOR)"
	gcc -c lib/dplist.c -Wall -std=c11 -Werror -fPIC -o lib/dplist.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB dplist< *****$(NO_COLOR)"
	gcc lib/dplist.o -o lib/libdplist.so -Wall -shared -lm -fdiagnostics-color=auto

lib/libtcpsock.so : lib/tcpsock.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB tcpsock *****$(NO_COLOR)"
	gcc -c lib/tcpsock.c -Wall -std=c11 -Werror -fPIC -o lib/tcpsock.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB tcpsock *****$(NO_COLOR)"
	gcc lib/tcpsock.o -o lib/libtcpsock.so -Wall -shared -lm -fdiagnostics-color=auto

# do not look for files called clean, clean-all or this will be always a target
.PHONY : clean clean-all run zip

clean:
	rm -rf *.o sensor_gateway sensor_node file_creator *~

clean-all: clean
	rm -rf lib/*.so gateway.log data.csv cmgr_test main

run : sensor_gateway sensor_node
	@echo "Add your own implementation here..."

zip:
	zip lab_final.zip main.c connmgr.c connmgr.h datamgr.c datamgr.h sbuffer.c sbuffer.h sensor_db.c sensor_db.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h

#connection manager makes
run_connectionmgr: lib/libtcpsock.so
	gcc -g -Wall -std=c11 -Werror lib/tcpsock.c lib/dplist.c connmgr.c main.c sbuffer.c  datamgr.c sensor_db.c -o cmgr_test -DTIMEOUT=10 -DSET_MIN_TEMP=10 -DSET_MAX_TEMP=20.5 -lpthread

run_server: run_connectionmgr
	./cmgr_test 5678 1

run_client1: sensor_node
	./sensor_node 15 1 127.0.0.1 5678

run_client2: sensor_node
	./sensor_node 21 3 127.0.0.1 5678

run_client3: sensor_node
	./sensor_node 37 2 127.0.0.1 5678

run_client4: sensor_node
	./sensor_node 49 1 127.0.0.1 5678

run_client5: sensor_node
	./sensor_node 112 2 127.0.0.1 5678