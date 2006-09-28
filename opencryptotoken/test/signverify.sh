#!/bin/bash
set -v
echo test | openssl smime -sign -signer cert.pem -engine oct -keyform E >signed.txt
cat signed.txt | openssl smime -verify -signer cert.pem -CAfile cert.pem 
