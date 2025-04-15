RC_ESP32 can connect to the rate app by using the ESP32 access point. Connect to the tablet to the ESP32 access point.(RateModule ...) In the rate app set the subnet to the access point network. 

To have windows auto re-connect to the ESP32 access point use method 1 on this page:
httpswww.kapilarya.comhow-to-use-wifi-and-ethernet-at-same-time-in-windows-11

To update the ESP32 over-the-air open a browser web page to (access point IP)/update. For example 192.168.100.1/update. Find the hex file on the tablet and upload.

How to Flash?
//list all the COM ports:
mode 

//flash:
esptool --chip esp32s3 --port "COM3" --baud 460800  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 ESP32_Section_activeON.ino.bootloader.bin 0x8000 ESP32_Section_activeON.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 ESP32_Section_activeON.ino.bin


esptool --chip esp32s3 --port "COM3" --baud 460800  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 ESP32_Section_activeOFF_Monosem.ino.bootloader.bin 0x8000 ESP32_Section_activeOFF_Monosem.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 ESP32_Section_activeOFF_Monosem.ino.bin
