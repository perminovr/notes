#!/bin/bash

echo "Enter pass phrase for privatekey.pem: "
read -s passw

#generate private key 
openssl genrsa -passout pass:$passw -des3 -out privatekey.pem 2048
#generate public key 
openssl rsa -passin pass:$passw -in privatekey.pem -outform PEM -pubout -out publickey.pem
