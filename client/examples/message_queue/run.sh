gcc sender.c -o sender
gcc sender_invalid.c -o sender_invalid
gcc receiver.c -o receiver
touch tmp_file
chmod 666 tmp_file
touch secret_file
echo "Secret Content" > secret_file
chmod 600 secret_file
