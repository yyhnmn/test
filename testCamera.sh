#! /bin/bash
# Take photos for 100 times
# 
# 
# Usage:
#./testCamera.sh


# Parameters
Duplicates=100

clear

echo "Start..."
ATTEMPT=0
while [[ $ATTEMPT -ne $Duplicates ]]; do
	let ATTEMPT+=1
	echo "Attempt ${ATTEMPT} started."
	raspistill -t 100
	sleep 0.5
done
