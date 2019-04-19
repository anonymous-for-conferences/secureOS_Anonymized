gcc client.c -o client
gcc server.c -o server
gcc server_invalid.c -o server_invalid
gcc multi_fork_server.c -o multi_fork_server
gcc multi_non_fork_server.c -o multi_non_fork_server
touch temp_file
chmod 600 temp_file
