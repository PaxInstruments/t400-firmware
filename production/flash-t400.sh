#!/bin/sh
#
set -e
clear
clear
echo "██████╗  █████╗ ██╗  ██╗    ██╗███╗   ██╗███████╗████████╗";
echo "██╔══██╗██╔══██╗╚██╗██╔╝    ██║████╗  ██║██╔════╝╚══██╔══╝";
echo "██████╔╝███████║ ╚███╔╝     ██║██╔██╗ ██║███████╗   ██║   ";
echo "██╔═══╝ ██╔══██║ ██╔██╗     ██║██║╚██╗██║╚════██║   ██║   ";
echo "██║     ██║  ██║██╔╝ ██╗    ██║██║ ╚████║███████║   ██║ ██╗";
echo "╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝    ╚═╝╚═╝  ╚═══╝╚══════╝   ╚═╝ ╚═╝";
echo "      ████████╗██╗  ██╗ ██████╗  ██████╗          ";
echo "      ╚══██╔══╝██║  ██║██╔═████╗██╔═████╗         ";
echo "         ██║   ███████║██║██╔██║██║██╔██║         ";
echo "         ██║   ╚════██║████╔╝██║████╔╝██║         ";
echo "         ██║        ██║╚██████╔╝╚██████╔╝ ██╗██╗██╗";
echo "         ╚═╝        ╚═╝ ╚═════╝  ╚═════╝  ╚═╝╚═╝╚═╝";
echo "                                                  ";
# Generated via http://patorjk.com/software/taag/#p=display&c=echo&f=ANSI%20Shadow&t=Pax%20Inst.
# Generated via http://patorjk.com/software/taag/#p=display&c=echo&f=ANSI%20Shadow&t=T400...
avrdude  -qq -patmega32u4 -cusbtiny -Pusb -e \
-Ulock:w:0x3F:m -Uefuse:w:0xcb:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m 
echo "***************************"
echo "*** Fuses via TinyISP OK ***"
echo "***************************"

avrdude -qq -patmega32u4 -cusbtiny -Pusb \
-Uflash:w:Caterina-T400-1.1.hex:i -Ulock:w:0x2F:m 
echo "*********************************"
echo "*** Bootloader via TinyISP OK ***"
echo "*********************************"

sleep 2
avrdude -patmega32u4 -cavr109 -P`ls /dev/cu.usbmodem*` -b57600 -D -Uflash:w:t400-1.0.hex:i
echo "***************************"
echo "*** Firmware via USB OK ***"
echo "***************************"

# sleep 2
# avrdude -patmega32u4 -cusbasp -Pusb -Uflash:w:t400.ino.hex:i 
# echo "******************************"
# echo "*** Firmware via TinyISP OK ***"
# echo "******************************"

echo ""
echo " ▄▄▄▄▄▄▄▄▄▄▄  ▄    ▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄         ▄ ";
echo "▐░░░░░░░░░░░▌▐░▌  ▐░▌▐░░░░░░░░░░░▌▐░▌       ▐░▌";
echo "▐░█▀▀▀▀▀▀▀█░▌▐░▌ ▐░▌ ▐░█▀▀▀▀▀▀▀█░▌▐░▌       ▐░▌";
echo "▐░▌       ▐░▌▐░▌▐░▌  ▐░▌       ▐░▌▐░▌       ▐░▌";
echo "▐░▌       ▐░▌▐░▌░▌   ▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌";
echo "▐░▌       ▐░▌▐░░▌    ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌";
echo "▐░▌       ▐░▌▐░▌░▌   ▐░█▀▀▀▀▀▀▀█░▌ ▀▀▀▀█░█▀▀▀▀ ";
echo "▐░▌       ▐░▌▐░▌▐░▌  ▐░▌       ▐░▌     ▐░▌     ";
echo "▐░█▄▄▄▄▄▄▄█░▌▐░▌ ▐░▌ ▐░▌       ▐░▌     ▐░▌     ";
echo "▐░░░░░░░░░░░▌▐░▌  ▐░▌▐░▌       ▐░▌     ▐░▌     ";
echo " ▀▀▀▀▀▀▀▀▀▀▀  ▀    ▀  ▀         ▀       ▀      ";
echo "                                               ";
# Generated via http://patorjk.com/software/taag/#p=display&c=echo&f=Electronic&t=OKAY

