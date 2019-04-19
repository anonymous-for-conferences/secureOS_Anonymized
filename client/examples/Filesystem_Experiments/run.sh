rm create read_f read_write write_f temp_file
gcc create_new_file.c -o create
gcc chunk_wise_write.c -o write_f
gcc chunk_wise_read_then_write.c -o read_write
gcc chunk_wise_read.c -o read_f

