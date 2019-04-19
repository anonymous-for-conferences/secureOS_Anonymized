gcc pipe_basic.c -o pipe_basic
gcc fifo_reader.c -o fifo_reader
gcc fifo_writer.c -o fifo_writer
gcc fifo_writer_invalid.c -o fifo_writer_invalid
gcc pipe_invalid.c -o pipe_invalid
mkfifo "tmp_fifo"
chmod 666 tmp_fifo
touch "tmp_file"
echo "This is secret content" > tmp_file
chmod 600 tmp_file
