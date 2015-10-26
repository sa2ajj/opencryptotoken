# Co jest potrzebne #
0. Jak zacząć swój udział w projekcie:
Na początek musimy się zaopatrzyć w niezbędne pakiety. Kompilator główne biblioteki, debugger i symulator.

a.  Jeśli używasz **Mandriva 2006** i nowszych możesz zajrzeć na stronę http://cdk4avr.sourceforge.net/ i tam pobrać pakiety i je zainstalować.

```
rpm -ihv cdk-avr-gcc-3.4.5-20060708.i586.rpm cdk-avr-libavr-20051024-20060609.noarch.rpm \
cdk-avr-libc-1.4.4-20060609.noarch.rpm cdk-avr-simulavr-0.1.2.2-20060709.i586.rpm \
cdk-avr-base-0.5-20060203.i586.rpm cdk-avr-binutils-2.16.1-20060708.i586.rpm \
cdk-avr-gdb-6.4-20060127.i586.rpm
```

b. Jeśli używasz **Ubuntu** powinieneś napisać

```
apt-get install gcc-avr binutils-avr avr-libc
```
(repozytorium universe)

jeśli chcesz mieć możliwość początkowego zaprogramowania procesora programator SPI, lub chcesz podłubać w kodzie bootloadera, zainstaluj także program uisp. [Tutaj](http://www.captain.at/electronics/atmel-programmer/) jest opisany sposób podłączenia programatora DAPA - Direct AVR Parallel Access, który składa się z dwóch rezystorów podłączonych do portu równoległego. Uwaga - numery nóżek tam podane odnoszą się  do AtMega16 w obudowie DIP-40.

1. Po tej operacji powinno działać już całe środowisko uruchomieniowe, czyli powinieneś móc skompilować firmware (katalog avrusbboot/firmware i opencryptotoken/firmware) za pomocą `make`













