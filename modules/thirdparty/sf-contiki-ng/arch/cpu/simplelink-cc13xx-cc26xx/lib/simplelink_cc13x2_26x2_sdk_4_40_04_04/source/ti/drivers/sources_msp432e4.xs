var commonHeaders = xdc.loadCapsule("ti/drivers/sources_common.xs").commonHeaders;
var commonSources = xdc.loadCapsule("ti/drivers/sources_common.xs").commonSources;

var msp432e4SrcList = commonSources.concat([
    "ADCBuf.c",
    "./adc/ADCMSP432E4.c",
    "./adcbuf/ADCBufMSP432E4.c",
    "AESCBC.c",
    "./aescbc/AESCBCMSP432E4.c",
    "AESCTR.c",
    "./aesctr/AESCTRMSP432E4.c",
    "AESCTRDRBG.c",
    "./aesctrdrbg/AESCTRDRBGXX.c",
    "AESCCM.c",
    "./aesccm/AESCCMMSP432E4.c",
    "AESECB.c",
    "./aesecb/AESECBMSP432E4.c",
    "AESGCM.c",
    "./aesgcm/AESGCMMSP432E4.c",
    "./cryptoutils/cryptokey/CryptoKeyPlaintextMSP432E4.c",
    "./cryptoutils/sharedresources/CryptoResourceMSP432E4.c",
    "./cryptoutils/utils/CryptoUtils.c",
    "CAN.c",
    "./can/CANMSP432E4.c",
    "Comparator.c",
    "./comparator/ComparatorMSP432E4.c",
    "CRC.c",
    "./crc/CRCMSP432E4.c",
    "./dma/UDMAMSP432E4.c",
    "./emac/EMACMSP432E4.c",
    "./gpio/GPIOMSP432E4.c",
    "./i2c/I2CMSP432E4.c",
    "./nvs/NVSMSP432E4.c",
    "./power/PowerMSP432E4.c",
    "./pwm/PWMMSP432E4.c",
    "./spi/SPIMSP432E4DMA.c",
    "ITM.c",
    "./itm/ITMMSP432E4.c",
    "SHA2.c",
    "./sha2/SHA2MSP432E4.c",
    "./timer/TimerMSP432E4.c",
    "./uart2/UART2MSP432E4.c",
    "./uart/UARTMSP432E4.c",
    "./watchdog/WatchdogMSP432E4.c"
]);

var msp432e4HdrList = commonHeaders.concat([
    "ADCBuf.h",
    "./adc/ADCMSP432E4.h",
    "./adcbuf/ADCBufMSP432E4.h",
    "AESCBC.h",
    "./aescbc/AESCBCMSP432E4.h",
    "AESCTR.h",
    "./aesctr/AESCTRMSP432E4.h",
    "AESCTRDRBG.h",
    "./aesctrdrbg/AESCTRDRBGXX.h",
    "AESCCM.h",
    "./aesccm/AESCCMMSP432E4.h",
    "AESECB.h",
    "./aesecb/AESECBMSP432E4.h",
    "AESGCM.h",
    "./aesgcm/AESGCMMSP432E4.h",
    "./cryptoutils/cryptokey/CryptoKey.h",
    "./cryptoutils/cryptokey/CryptoKeyPlaintext.h",
    "./cryptoutils/sharedresources/CryptoResourceMSP432E4.h",
    "./cryptoutils/utils/CryptoUtils.h",
    "CAN.h",
    "./can/CANMSP432E4.h",
    "Comparator.h",
    "./comparator/ComparatorMSP432E4.h",
    "CRC.h",
    "./crc/CRCMSP432E4.h",
    "./can/types.h",
    "./dma/UDMAMSP432E4.h",
    "./emac/EMACMSP432E4.h",
    "./emac/EMACMSP432E4.rov.js",
    "./gpio/GPIOMSP432E4.h",
    "./i2c/I2CMSP432E4.h",
    "./nvs/NVSMSP432E4.h",
    "./power/PowerMSP432E4.h",
    "./pwm/PWMMSP432E4.h",
    "ITM.h",
    "SHA2.h",
    "./sha2/SHA2MSP432E4.h",
    "./spi/SPIMSP432E4DMA.h",
    "./timer/TimerMSP432E4.h",
    "./uart2/UART2MSP432E4.h",
    "./uart/UARTMSP432E4.h",
    "./watchdog/WatchdogMSP432E4.h"
]);
