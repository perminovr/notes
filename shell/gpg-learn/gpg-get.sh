#!/bin/bash

# $1 = filename

if [ "$1" != "" ]; then

	gpg --batch --yes --passphrase 1234 --output $1 -d $1.enc

else
	echo " 1 - filename"
fi
