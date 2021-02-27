#!/bin/sh

openssl dgst -sha256 -verify publickey.pem -signature simplestCOM.sign simplestCOM
