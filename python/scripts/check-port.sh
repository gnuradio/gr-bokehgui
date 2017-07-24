#!/bin/bash

netstat -netul | awk '{ print $4 }' | awk -F':' '{ print $NF }' | tail -n +3 > temp_netstat

TEMP=temp_netstat
port_server_assign=0

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

