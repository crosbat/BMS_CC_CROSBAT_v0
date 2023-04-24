#ifdef __cplusplus
extern "C" {
#endif

#ifndef __SF_PERSISTENT_DATA_STORAGE_H__
#define __SF_PERSISTENT_DATA_STORAGE_H__

/**
 @code
  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 embedded.connectivity.solutions.==============
 @endcode

 @file
 @copyright  STACKFORCE GmbH, Germany, www.stackforce.de
 @author     STACKFORCE
 @brief      This header contains information for using the persistent
             data storage module.
*/

/*==============================================================================
                            INCLUDES
==============================================================================*/
/* Standart libraries */
#include <stdint.h>
/* Satckforce types. */
#include "sf_types.h"
#include "project-conf.h"

/*==============================================================================
                             MACROS
==============================================================================*/
#if defined(__IAR_SYSTEMS_ICC__)
  #define SF_INLINE              inline
  #define PACKED_STRUCT(__declaration__,__name__) \
    _Pragma("pack(push,1)") __declaration__  __name__ _Pragma("pack(pop)")
  #define BIG_ENDIAN              false
  #define LITTLE_ENDIAN           true

#elif defined(_MSC_VER)
  #define SF_INLINE              __inline
  #define PACKED_STRUCT(__declaration__, __name__) \
    __pragma( pack(push, 1) ) __declaration__ __name__ __pragma( pack(pop) )
  #define BIG_ENDIAN              false
  #define LITTLE_ENDIAN           true

#elif defined(__CCE__)
  #define SF_INLINE
  #define PACKED_STRUCT(__declaration__,__name__) \
    __declaration__  __name__
  #define BIG_ENDIAN              false
  #define LITTLE_ENDIAN           true

#elif defined(__C51__)
  #define PACKED_STRUCT(__declaration__, __name__) \
    __declaration__ __name__
  #define BIG_ENDIAN              true
  #define LITTLE_ENDIAN           false
  #define SF_INLINE

#elif defined (__GNUC__)
  #define PACKED_STRUCT(__declaration__, __name__) \
    __declaration__ __attribute__ ((__packed__)) __name__
  #ifndef BIG_ENDIAN
  #define BIG_ENDIAN              false
  #endif
  #ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN           true
  #endif
  #define SF_INLINE
  #define __no_init

#elif defined (__TI_COMPILER_VERSION__)
  #define PACKED_STRUCT(__declaration__, __name__) \
    __declaration__ __attribute__ ((__packed__)) __name__
  #ifndef BIG_ENDIAN
  #define BIG_ENDIAN              false
  #endif
  #ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN           true
  #endif
  #define SF_INLINE
  #define __no_init

#elif defined(__RENESAS__)
  #define PACKED_STRUCT(__declaration__, __name__) \
    __declaration__ __name__
  #define BIG_ENDIAN              false
  #define LITTLE_ENDIAN           true
  #define SF_INLINE
  #define __no_init

#elif defined(__arm__)
  #ifdef __ARMCC_VERSION
    #define PACKED_STRUCT(__declaration__, __name__) \
    __packed __declaration__ __name__
    #define BIG_ENDIAN            false
    #define LITTLE_ENDIAN         true
    #define SF_INLINE             __inline
  #endif /* __ARMCC_VERSION */

#else
  #error The compiler is not supported.
#endif /* Compiler */

/* The maximum number of sensors the bmscc can register */
#ifndef SF_CONF_SENSOR_CNT_MAX
#error "Please define SF_CONF_SENSOR_CNT_MAX!"
#else
#define SF_PERSISTENT_SENSOR_CNT_MAX           SF_CONF_SENSOR_CNT_MAX
#endif

/*==============================================================================
                             STRUCTS
==============================================================================*/
/*! This structure defines the device life time configuration. It will be stored in RAM or
    ROM memory. */
PACKED_STRUCT(typedef struct
{
  uint8_t flag;
  /* Serial number of the device. */
  uint32_t deviceSerial;
  /* Network ID. */
  uint16_t panId;
  /* device short address. */
  uint8_t deviceAddr[2];
  /* The calculated CRC. */
  uint16_t crc;
}, sf_persistent_deviceConfig_t);

/* The sensor data */
typedef struct
{
  /* Serial number of the sensor. */
  uint32_t deviceSerial;
  /* Sensor short address. */
  uint8_t deviceAddr[2];
} sf_persistent_sensor_t;

/*! This structure defines the registered sensor parameters. It will be stored in RAM or
    ROM memory. */
PACKED_STRUCT(typedef struct
{
  uint8_t flag;
  /* Registered sensors */
  sf_persistent_sensor_t pSensorList[SF_PERSISTENT_SENSOR_CNT_MAX];
  /* The number of registered sensors */
  uint8_t registeredSensorCnt;
  /* The calculated CRC. */
  uint16_t crc;
}, sf_persistent_sensorList_t);

/*==============================================================================
                            FUNCTION PROTOTYPES
==============================================================================*/
/*============================================================================*/
/**
 * \brief Store device life time configuration into NVM memory.
 *
 * \param pPersistentDeviceConfig      The device configuration to be stored.
 *
 * \return @ref E_SF_RETURN_t.
 */
/*============================================================================*/
E_SF_RETURN_t sf_persistentDataStorage_writeConfig(sf_persistent_deviceConfig_t*
                                                   pPersistentDeviceConfig);

/*============================================================================*/
/**
 * \brief Read the device life time configuration from the NVM memory.
 *
 * \param pPersistentDeviceConfig      Pointer to the device configuration
 *                                     read from the NVM memory.
 *
 * \return @ref E_SF_RETURN_t.
 */
/*============================================================================*/
E_SF_RETURN_t sf_persistentDataStorage_readConfig(sf_persistent_deviceConfig_t*
                                                  pPersistentDeviceConfig);

/*============================================================================*/
/**
 * \brief Remove the device lifetime configurations from the NVM memory.
 *
 * \return @ref E_SF_RETURN_t.
 */
/*============================================================================*/
E_SF_RETURN_t sf_persistentDataStorage_removeConfig(void);

/*============================================================================*/
/**
 * \brief Store the sensor list and the registered sensors count in the
 *        NVM memory.
 *
 * \param pPersistentSensorList    Pointer to the data to be stored.
 *
 * \return @ref E_SF_RETURN_t.
 */
/*============================================================================*/
E_SF_RETURN_t sf_persistentDataStorage_writeSensorList(sf_persistent_sensorList_t*
                                                       pPersistentSensorList);

/*============================================================================*/
/**
 * \brief Read the sensor list and the registered sensors count from the
 *        NVM memory.
 *
 * \param pPersistentSensorList    Pointer to data read from the NVM
 *                                 memory.
 *
 * \return @ref E_SF_RETURN_t.
 */
/*============================================================================*/
E_SF_RETURN_t sf_persistentDataStorage_readSensorList(sf_persistent_sensorList_t*
                                                       pPersistentSensorList);

/*============================================================================*/
/**
 * \brief Remove the sensor list from the NVM memory.
 *
 * \return @ref E_SF_RETURN_t.
 */
/*============================================================================*/
E_SF_RETURN_t sf_persistentDataStorage_removeSensorList(void);


#endif /* __SF_PERSISTENT_DATA_STORAGE_H__ */

#ifdef __cplusplus
}
#endif
