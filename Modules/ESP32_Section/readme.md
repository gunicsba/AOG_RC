activeON changes how it behaves when we lose network connectivity. (I.e. put sections high in case of no network)


|           | Invert Relay<br>ON                                | Invert Relay<br>Off                                |
|-----------|---------------------------------------------------|----------------------------------------------------|
| ActiveON  | No network: Output ON<br>Implement ON: Output OFF | No network: Output OFF<br>Implement ON: Output ON  |
| ActiveOFF | No network: Output ON<br>Implement ON: Output ON  | No network: Output OFF<br>Implement ON: Output OFF |


//flash:
.\esptool.exe --chip esp32s3 --port "COM3" --baud 460800  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 ESP32_Section_activeON.ino.bootloader.bin 0x8000 ESP32_Section_activeON.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 ESP32_Section_activeON.ino.bin


.\esptool.exe --chip esp32s3 --port "COM3" --baud 460800  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 ESP32_Section_activeOFF.ino.bootloader.bin 0x8000 ESP32_Section_activeOFF.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 ESP32_Section_activeOFF.ino.bin
