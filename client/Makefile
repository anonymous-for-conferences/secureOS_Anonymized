#This make file compiles necessary c files which needs to be run at the beginning. The files are the database_server.c for the database server \
add_users_and_groups.c for adding the existing users and groups, enable_rwfm.c for enabling the RWFM monitor. \
This make file also compiles the preload.c and sets it as the alternative for the shared libc calls. 

DL=`ldd /bin/ls | grep "libdl.so" | awk '{ print $$3 }'`
LIBC=`ldd /bin/ls | grep "libc.so" | awk '{ print $$3 }'`
DEBUG=1
HOSTNAME=`hostname`

all: database_server.c add_users_and_groups.c preload.c
	echo "#define LIBC \"${LIBC}\"" > preload.h
	echo "#define LIBPTHREAD \"${LIBPTHREAD}\"" >> preload.h
	echo "#define HOSTNAME \"${HOSTNAME}\"" >> preload.h
	gcc database_server.c -o database_server
	gcc add_users_and_groups.c -o add_users_and_groups
	gcc enable_rwfm.c -o enable_rwfm
	gcc -rdynamic -shared -fPIC -pthread -o preload.so preload.c ${DL}

debug: database_server.c add_users_and_groups.c preload.c
	echo "#define LIBC \"${LIBC}\"" > preload.h
	echo "#define DEBUG" >> preload.h
	gcc database_server.c -o database_server
	gcc add_users_and_groups.c -o add_users_and_groups
	gcc enable_rwfm.c -o enable_rwfm
	gcc -rdynamic -shared -fPIC -pthread -o preload.so preload.c ${DL}
