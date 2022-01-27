#!/bin/bash

# Use netstat if available, else try ss command
if ! command -v netstat &> /dev/null
then
	ss -netul | awk '{ print $5 }' | awk -F':' '{ print $NF }' | tail -n +3 > temp_netstat
else
	netstat -netul | awk '{ print $4 }' | awk -F':' '{ print $NF }' | tail -n +3 > temp_netstat
fi

TEMP=temp_netstat
port_server_assign=5006

for port in {5006..70000}
do
  flag=1
  while read line
  do
    p=`echo $line | awk '{ print $1 }'`
    if [ $port -eq $p ]
    then
      flag=0
      break
    fi
  done<$TEMP
  if [ $flag -eq 1 ]
  then
    port_server_assign=$port
    break
  fi
done
rm $TEMP
echo "$port_server_assign"

