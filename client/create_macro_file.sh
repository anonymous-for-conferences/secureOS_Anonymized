macro_file_name="database_macros.h"
database_ops_file_name="database_operations.txt"
count=1

rm -f $macro_file_name
touch $macro_file_name

echo "#ifndef _DB_MACROS_H_" > $macro_file_name
echo "#define _DB_MACROS_H_" >> $macro_file_name
echo "" >> $macro_file_name

echo "#define RULE_ENGINE_SEMAPHORE \"/rule_engine_semaphore\"" >> $macro_file_name
echo "#define FORK_LOCK \"/fork_lock\"" >> $macro_file_name
echo "" >> $macro_file_name

echo "#define MQ_FILE_PATH \"/tmp/msg_queue_file\"" >> $macro_file_name
echo "#define REQUEST_MQ_PROJ_ID 1" >> $macro_file_name
echo "#define RESPONSE_MQ_PROJ_ID 2" >> $macro_file_name
echo "" >> $macro_file_name

echo "#define REQUEST_DELIM \" \"" >> $macro_file_name
echo "" >> $macro_file_name

echo "#define MAX_REQUEST_LENGTH 512" >> $macro_file_name
echo "" >> $macro_file_name

while IFS="" read -r line || [[ -n "$line" ]]; do
    if [[ $line =~ ^#.* ]] || [[ -z $line ]]
    then
        echo "" >> $macro_file_name
    else
        echo "#define $line $count" >> $macro_file_name
        ((count++))
    fi
done < "$database_ops_file_name"

echo "" >> $macro_file_name
echo "#endif" >> $macro_file_name
