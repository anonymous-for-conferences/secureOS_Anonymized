./stop_db_server.sh
./clean.sh
for sub_dir in ./examples/*/ ; do (cd "$sub_dir" && ./clean.sh); done
