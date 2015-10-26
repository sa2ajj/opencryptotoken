[Wersja polska](OpenCryptoToken.md)

USB crypto token based on cheap Atmel's AVR AtMega8 using strong ECC public/private key cryptography... It is OpenHardware project, hardware specs, firmware and pc drivers available on open license.

![http://opencryptotoken.googlecode.com/svn/trunk/opencryptotoken/circuit/oct3d.png](http://opencryptotoken.googlecode.com/svn/trunk/opencryptotoken/circuit/oct3d.png)
[circuit](http://opencryptotoken.googlecode.com/svn/trunk/opencryptotoken/circuit/circuit.png)

Project is based on [Firmware-Only USB](http://www.obdev.at/products/avrusb/index.html)

[mailing list](http://groups.google.com/group/opencryptotoken)

### Features ###
  * On-token private key operations (ECDSA Sign) with acceptable speed (~2seconds/sign for prime192v1 NIST curve)
  * chip hardware (few $) - all crypto operations and USB support even done in software
  * easy uC programming (initial uC programming via Atmel's SPI interface, further firmware updates via USB)
  * working openct driver, opensc ECC patch, firefox pkcs11 support (via opensc pkcs11 plugin) - client certificate login tested

### TODO ###

  * better hardware RNG (needed for ECDSA Sign and onboard key generation) - I have primitive one (single capacitor :), but i think it shoud be better.
  * onboard private key generation
  * PIN support
  * better pkcs11/15 compilance
  * multiple key/certificate support
  * a lot of testing and bug hunting
  * better ECC application support (mail signing in thunderbird, kerberos PKINIT support etc)