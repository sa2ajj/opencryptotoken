#include <avr/pgmspace.h>
#include "cert.h"
#include "file.h"

file_entry_t flash_files[] PROGMEM= {{P_certificate,sizeof(P_certificate)}};
