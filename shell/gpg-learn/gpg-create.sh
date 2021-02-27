#!/bin/bash

# $1 = filename

if [ "$1" != "" ]; then

	#echo "Enter passphrase:"
	#read -s passw
	#gpg --batch --yes --passphrase $passw --output $1.enc --cipher-algo AES256 -c $1

	gpg --output $1.enc --cipher-algo AES256 -c $1
	rm -rf $1

else
	echo " 1 - filename"
fi
