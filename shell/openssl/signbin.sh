#!/bin/bash

echo "Enter pass phrase for privatekey.pem: "
read -s passw

openssl dgst -passin pass:$passw -sha256 -sign privatekey.pem -out simplestCOM.sign simplestCOM
openssl enc -base64 -in simplestCOM.sign -out simplestCOM-b64.sign
