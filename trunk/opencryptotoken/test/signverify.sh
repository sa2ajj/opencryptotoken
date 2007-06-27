#!/bin/bash
set -v
echo test | openssl smime -nointern -noverify -sign -signer mrk.pem -CAfile cacert.pem -engine oct -keyform E >signed.txt
cat signed.txt | openssl smime -verify -signer mrk.pem -CAfile cacert.pem 
