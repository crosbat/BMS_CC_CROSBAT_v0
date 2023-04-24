#ifdef __cplusplus
  extern "C" {
#endif

/**
 * @code
 *  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 * / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 * \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 * |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 * embedded.connectivity.solutions.==============
 * @endcode
 *
 * @copyright  STACKFORCE GmbH, Germany, www.stackforce.de
 * @author     STACKFORCE
 * @brief      API definition of the device management module.
 *
 */

#ifndef SF_DEVICEMGMT_H_
#define SF_DEVICEMGMT_H_

/**
 *  @addtogroup SF_DEVICE_MGMT
 *
 *  @details
 *
 *  - <b>DEVICE MGMT API</b>\n
 *    | API Function                              | Description                                     |
 *    |-------------------------------------------|-------------------------------------------------|
 *    | @ref sf_deviceMgmt_getSensorList()        | @copybrief sf_deviceMgmt_getSensorList()        |
 *    | @ref sf_deviceMgmt_getFreeAddress()       | @copybrief sf_deviceMgmt_getFreeAddress()       |
 *    | @ref sf_deviceMgmt_addDevice()            | @copybrief sf_deviceMgmt_addDevice()            |
 *    | @ref sf_deviceMgmt_removeDevice()         | @copybrief sf_deviceMgmt_removeDevice()         |
 *    | @ref sf_deviceMgmt_getDevice()            | @copybrief sf_deviceMgmt_getDevice()            |
 *    | @ref sf_deviceMgmt_getDeviceByIndex()     | @copybrief sf_deviceMgmt_getDeviceByIndex()     |
 *    | @ref sf_deviceMgmt_getDeviceBySerial()    | @copybrief sf_deviceMgmt_getDeviceBySerial()    |
 *    | @ref sf_deviceMgmt_getRegisteredDeviceCount()| @copybrief sf_deviceMgmt_getRegisteredDeviceCount()|
 *    | @ref sf_deviceMgmt_readSensorList()       | @copybrief sf_deviceMgmt_readSensorList()       |
 *  @{
 */

/*****************************************************************************/
/*                         INCLUDES                                          */
/*****************************************************************************/
/* Standard include */
#include <stddef.h>
/* Device specific includes */
#include <modules/common/sf_types.h>
#include "linkaddr.h"

/*****************************************************************************/
/*                         DEFINES                                           */
/*****************************************************************************/
/* The maximum number of sensors the bmscc can register */
#ifndef SF_CONF_SENSOR_CNT_MAX
#error "Please define SF_CONF_SENSOR_CNT_MAX!"
#else
#define SF_DEVICEMGMT_SENSOR_CNT_MAX           SF_CONF_SENSOR_CNT_MAX
#endif

/*****************************************************************************/
/*                         STRUCTS                                           */
/*****************************************************************************/
/*! The sensor entry parameters */
typedef struct
{
  /*! SC network address */
  linkaddr_t shortAddress;
  /*! SC serial number */
  uint32_t serialNr;
} sf_sensor_t;

/*****************************************************************************/
/*                         API FUNCTIONS                                     */
/*****************************************************************************/
/*============================================================================*/
/**
 * @brief Returns the registered sensors list
 *
 * @return Pointer to the sensors list
 */
/*============================================================================*/
sf_sensor_t* sf_deviceMgmt_getSensorList(void);

/*============================================================================*/
/**
 * @brief Returns a free short address or the assigned short address if
 *        the device is already registered.
 *
 * @param  pShortAddress    Pointer to SC short address
 * @param  serial           SC serial number
 *
 * @return @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_deviceMgmt_getFreeAddress(uint16_t *pShortAddress, uint32_t serial);

/*============================================================================*/
/**
 * @brief Adds new sensor entry to the sensor list.
 *
 * @param  shortAddress      SC short address
 * @param  serial            SC serial number
 *
 * @return @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_deviceMgmt_addDevice(linkaddr_t shortAddress, uint32_t serial);

/*============================================================================*/
/**
 * @brief Removes the sensor entry from the list if exist.
 *
 * @param  shortAddr  SC short address.
 *
 * @return @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_deviceMgmt_removeDevice(linkaddr_t shortAddr);

/*============================================================================*/
/**
 * @brief Searches a sensor list entry with matching short address.
 *
 * @param  shortAddress    SC Short address
 *
 * @return Pointer to sensor list entry.
 */
/*============================================================================*/
sf_sensor_t* sf_deviceMgmt_getDevice(linkaddr_t shortAddress);

/*============================================================================*/
/**
 * @brief Return list entry at the given index.
 *
 * @param  index         List index
 *
 * @return Pointer to sensor list entry.
 */
/*============================================================================*/
sf_sensor_t* sf_deviceMgmt_getDeviceByIndex(uint8_t index);

/*============================================================================*/
/**
 * @brief Searches sensor list entry with matching device serial number.
 *
 * @param  serial         SC Serial address
 *
 * @return Pointer to found sensor list entry.
 */
/*============================================================================*/
sf_sensor_t* sf_deviceMgmt_getDeviceBySerial(uint32_t serial);

/*============================================================================*/
/**
 * @brief Returns count of all devices which have been added to the sensor list.
 *
 * @return Registered devices count
 */
/*============================================================================*/
uint8_t sf_deviceMgmt_getRegisteredDeviceCount(void);

/*============================================================================*/
/**
 * \brief Read stored sensor list and the number of registered sensors out of
 *        the NVM memory.
 *
 * \return  @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_deviceMgmt_readSensorList(void);

/*! @} */

#endif /* SF_DEVICEMGMT_H_ */

#ifdef __cplusplus
}
#endif
