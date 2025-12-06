#!/usr/bin/env sh

steam-run assets/prog/stmCubeProg/bin/STM32_Programmer_CLI -c port=SWD -el assets/prog/stmCubeProg/bin/ExternalLoader/MT25TL01G_STM32H750B-DISCO.stldr -d "$1" 0x90000000 -v

