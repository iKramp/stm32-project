#!/usr/bin/env sh
# set -e
#
# INPUT="$1"
#
# if [ -z "$INPUT" ] || [ ! -f "$INPUT" ]; then
#     echo "Usage: $0 <file.bin>"
#     exit 1
# fi
#
# # Create temp file in current directory
# TMP="./.$(basename "$INPUT").swapped.$$.bin"
#
# # Ensure cleanup even if script exits early
# trap 'rm -f "$TMP"' EXIT
#
# # 1) prepend one dummy byte (0x00)
# printf '\x00' > "$TMP"
#
# # 2) swap every pair of bytes and append
# dd if="$INPUT" bs=2 conv=swab status=none >> "$TMP"
#
# # Flash
# steam-run assets/prog/stmCubeProg/bin/STM32_Programmer_CLI \
#     -c port=SWD \
#     -el assets/prog/stmCubeProg/bin/ExternalLoader/MT25TL01G_STM32H750B-DISCO.stldr \
#     -d "$TMP" 0x90000000 \
#     -v

steam-run assets/prog/stmCubeProg/bin/STM32_Programmer_CLI -c port=SWD -el assets/prog/stmCubeProg/bin/ExternalLoader/MT25TL01G_STM32H750B-DISCO.stldr -d "$1" 0x90000000 -v
