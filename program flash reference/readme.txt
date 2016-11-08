Compile Option: FLASH_MAP = 2   512KB + 512KB( 1MB Flash)

master_device_key.bin                 FlashAddr: 0x7E000
esp_init_data_default.bin             FlashAddr: 0xFC000
blank.bin                             FlashAddr: 0xFE000
boot.bin                              FlashAddr: 0x00000
user1.bin version:  v1.0.5t45772(a)   FlashAddr: 0x01000


upgrade version:

user1.bin  v1.0.6t45772(a) 
user2.bin  v1.0.6t45772(a) 



Compile Option: FLASH_MAP = 4   512KB + 512KB + 3MB data( 4MB Flash)

master_device_key.bin                 FlashAddr: 0x7E000
esp_init_data_default.bin             FlashAddr: 0x3FC000
blank.bin                             FlashAddr: 0x3FE000
boot.bin                              FlashAddr: 0x00000
user1.bin version:  v1.0.5t45772(a)   FlashAddr: 0x01000


