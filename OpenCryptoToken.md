OpenCryptoToken - Token USB oparty na tanim i popularnym mikrokontrolerze jednoukładowym Atmel'a z rodziny AVR - AtMega8, implementujący kryptografię Krzywych Eliptycznych ECC.
Open w nazwie oznacza, że zarówno firmware, jak i specyfikacja hardware jest otwarta, dostępna na wolnej licencji. Projekt rozwijany jest na razie tylko na linuksie, ale porty pod inne systemy są jak najbardziej możliwe.

![http://opencryptotoken.googlecode.com/svn/trunk/opencryptotoken/circuit/oct3d.png](http://opencryptotoken.googlecode.com/svn/trunk/opencryptotoken/circuit/oct3d.png)
[schemat](http://opencryptotoken.googlecode.com/svn/trunk/opencryptotoken/circuit/circuit.png)

Projekt bazuje na [Firmware-Only USB](http://www.obdev.at/products/avrusb/index.html)

[Grupa (lista) dyskusyjna](http://groups.google.com/group/opencryptotoken)

Dlaczego ECC, a nie popularne RSA? ECC zostało wybrane ze względu na dużo mniejsze wymagania obliczeniowe/pamięciowe - mikrokontroler taki jak AtMega8 (8kB pamięci programu, 1kB ramu (sic!), taktowanie 12Mhz) po prostu nie dał by sobie rady z RSA, natomiast z ECC idzie mu całkiem sprawnie...

### Cechy ###
  * Wszelkie operacje na kluczu prywatnym wykonywane przez token, z akceptowalną prędkością (ok. 2 sekundy/podpis ECDSA dla 192 bitowej krzywej prime192v1 NIST)
  * bardzo prosty układ, tanie podzespoły - koszt elementów to kilkanaście PLN (mikrokontroler, kwarc, kilka kondensatorów i rezystorów)
  * łatwe programowanie kontrolera - pierwsze programowanie można wykonać przez szeregowy programator SPI (port równoległy + kilka rezystorów), późniejsze zmiany można wprowadzać przez USB. Zainteresowanym mogę wysłać oczywiście wstępnie zaprogramowany układ, więc wystarczy jedynie port USB
  * opracowany driver do openct, patch ECC do opensc, działający plugin pkcs11 do firefox'a - generalnie można już logować się za pomocą certyfikatu klienta

### Jak zacząć ###
  * [Jakie pakiety](JakZaczacSwojUdzialWProjekcie.md) i skąd można je pobrać
  * W jaki sposób skompilować program
  * Jak wygenerować klucze
  * Jak testować
### TODO ###
  * projekt płytki drukowanej (PCB) token'a - poszukiwane osoby z doświadczeniem w tym temacie
  * wykonanie na zaprojektowanej płytce prototypowych tokenów (kluczyków) USB
  * lepszy sprzętowy generator liczb losowych (potrzebny do generowania sygnatur ECDSA i generowania nowego klucza). Mam prymitywny generator (na jednym kondensatorze :), ale pewnie da się to zrobić lepiej
  * generowanie klucza na tokenie (na razie klucz prywatny jest zapisywany podczas programowania token'a)
  * obsługa PIN
  * Wsparcie dla wielu kluczy/certyfikatów ? (za bardzo nie można poszaleć - na AtMega8 w 8kB trzeba się zmieścić z kodem i kluczami/certyfikatami, ale oczywiście można wykorzystać układ z większą pamięcią)
  * wsparcie ze strony aplikacji (podpisywanie/szyfrowanie poczty w thunderbirdzie, wsparcie dla PKINIT w kerberosie (heimdal), itd)
  * więcej testów....



















