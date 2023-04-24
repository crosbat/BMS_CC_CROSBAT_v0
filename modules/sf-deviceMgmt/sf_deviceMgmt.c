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
 * @brief      API implementation of the device management.
 *
 */

/*****************************************************************************/
/*                         INCLUDES                                          */
/*****************************************************************************/
/* Standard include */
#include <stdio.h>
#include <string.h>
/* Contiki include */
#include "sys/log.h"
/* Module specific include */
#include "sf_persistentDataStorage.h"
#include "sf-tsch-schedule.h"
#include "sf_deviceMgmt.h"
#include "sf_types.h"

/*****************************************************************************/
/*                         MACROS                                            */
/*****************************************************************************/
#define LOG_MODULE "App"
#ifndef LOG_CONF_APP
#define LOG_LEVEL                   LOG_LEVEL_NONE
#else
#define LOG_LEVEL                   LOG_CONF_APP
#endif

/*****************************************************************************/
/*                         GLOBAL VARIABLES                                  */
/*****************************************************************************/
/* Stores the connected devices data */
static sf_sensor_t gpSensorList[SF_DEVICEMGMT_SENSOR_CNT_MAX] = {0};
/* Stores the number of registered devices */
static uint16_t gRegisteredDevicesCnt = 0;

/*****************************************************************************/
/*                         LOCAL FUNCTIONS                                   */
/*****************************************************************************/
/*============================================================================*/
/**
 * @brief Stores the connected devices data and the registered devices count.
 *
 * @return @ref E_SF_RETURN_t
 * */
/*============================================================================*/
static E_SF_RETURN_t loc_writeSensorList(void);

/*****************************************************************************/
/*                         LOCAL FUNCTIONS IMPLEMENTATION                    */
/*****************************************************************************/
/*----------------------------------------------------------------------------*/
/*! loc_writeSensorList */
/*----------------------------------------------------------------------------*/
static E_SF_RETURN_t loc_writeSensorList(void)
{
  E_SF_RETURN_t returnValue = E_SF_ERROR;
  /* Buffer to store the sensor list data to be flashed. */
  sf_persistent_sensorList_t persistentSensorList;

  for(uint8_t i = 0; i < SF_DEVICEMGMT_SENSOR_CNT_MAX; i++)
  {
    memcpy(persistentSensorList.pSensorList[i].deviceAddr,
          (uint8_t*)gpSensorList[i].shortAddress.u8, LINKADDR_SIZE);
    persistentSensorList.pSensorList[i].deviceSerial = gpSensorList[i].serialNr;
  }

  persistentSensorList.registeredSensorCnt = gRegisteredDevicesCnt;

  returnValue = sf_persistentDataStorage_writeSensorList(&persistentSensorList);

  return returnValue;
}/* loc_writeSensorList */

/*****************************************************************************/
/*                         API FUNCTIONS                                     */
/*****************************************************************************/
/*------------------------------------------------------------------------------
  sf_deviceMgmt_getSensorList()
------------------------------------------------------------------------------*/
sf_sensor_t* sf_deviceMgmt_getSensorList(void)
{
  return gpSensorList;
} /* sf_deviceMgmt_getSensorList() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_getFreeAddress()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_deviceMgmt_getFreeAddress(uint16_t *pShortAddress, uint32_t serial)
{
  E_SF_RETURN_t returnValue = E_SF_ERROR;
  uint8_t sensorListIndex = 0;
  uint8_t firstEmptyIndex = 0xFF;

  /* Search for empty field */
  for(sensorListIndex = 0; sensorListIndex < SF_DEVICEMGMT_SENSOR_CNT_MAX;
      sensorListIndex++)
  {
    /* Found empty sensor list entry */
    if((firstEmptyIndex == 0xFF) &&
       (linkaddr_cmp(&gpSensorList[sensorListIndex].shortAddress, &linkaddr_null)))
    {
      firstEmptyIndex = sensorListIndex;
      returnValue = E_SF_SUCCESS;
    }

    /* Check if serial is already in sensor list. If yes return the same entry. */
    if(gpSensorList[sensorListIndex].serialNr == serial)
    {
      LOG_INFO("Sensor (SerialNo: %ld) is already registered with addr: ", serial);
      LOG_INFO_LLADDR(&gpSensorList[sensorListIndex].shortAddress);
      LOG_INFO_("\n");

      firstEmptyIndex = sensorListIndex;
      returnValue = E_SF_SUCCESS;
      break;
    }
  }

  if(returnValue == E_SF_SUCCESS && firstEmptyIndex != 0xFF)
  {
    /* If the list entry is an empty entry, return the list index + 1
       as new link address. */
    *pShortAddress = firstEmptyIndex + 1;
  }

  return returnValue;
} /* sf_deviceMgmt_getFreeAddress() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_addDevice()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_deviceMgmt_addDevice(linkaddr_t shortAddress, uint32_t serial)
{
  E_SF_RETURN_t returnValue = E_SF_ERROR;

  bool alreadyInList = false;
  sf_sensor_t *sensorListEntry = NULL;

  /* Check if device is already in sensor list. */
  sensorListEntry = sf_deviceMgmt_getDevice(shortAddress);
  if(sensorListEntry)
  {
    alreadyInList = true;
  }
  else
  {
    sensorListEntry = sf_deviceMgmt_getDeviceByIndex(shortAddress.u16 - 1);
    if(sensorListEntry)
    {
      /* If device is not in the list check if sensor list entry is still empty. */
      if(linkaddr_cmp(&sensorListEntry->shortAddress, &linkaddr_null))
      {
        returnValue = E_SF_SUCCESS;
      }
    }
  }

  if(returnValue == E_SF_SUCCESS && sensorListEntry)
  {
    linkaddr_copy(&sensorListEntry->shortAddress, &shortAddress);
    sensorListEntry->serialNr = serial;

    if(!alreadyInList)
    {
      /* Increment devices count if sensor is not already in the list. */
      gRegisteredDevicesCnt++;
    }

    LOG_INFO("-- New Sensor List --\n");
    for(uint8_t listIndex=0; listIndex < SF_DEVICEMGMT_SENSOR_CNT_MAX; listIndex++)
    {
      uint32_t serial = (uint32_t)gpSensorList[listIndex].serialNr;

      LOG_INFO("Serial: %lu; ShortAddr:", serial);
      LOG_INFO_LLADDR(&gpSensorList[listIndex].shortAddress);
      LOG_INFO_("; \n");
    }

    /* Stores the new sensor list into the NVM */
    returnValue = loc_writeSensorList();
  }

  return returnValue;
} /* sf_deviceMgmt_addDevice() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_removeDevice()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_deviceMgmt_removeDevice(linkaddr_t shortAddr)
{
  E_SF_RETURN_t returnValue = E_SF_ERROR;
  uint8_t sensorListIndex = 0;

  if(gRegisteredDevicesCnt > 0)
  {
    for(sensorListIndex = 0; sensorListIndex < SF_DEVICEMGMT_SENSOR_CNT_MAX;
        sensorListIndex++)
    {
      if(linkaddr_cmp(&shortAddr, &gpSensorList[sensorListIndex].shortAddress))
      {
        /* Remove entry at sensor list index of listIdx */
        memset(&gpSensorList[sensorListIndex], 0U, sizeof(gpSensorList[sensorListIndex]));

        gRegisteredDevicesCnt--;

        returnValue = E_SF_SUCCESS;
        break;
      }
    }
  }

  if(E_SF_SUCCESS == returnValue)
  {
    /* Stores the new sensor list into the NVM */
    returnValue = loc_writeSensorList();
  }

  return returnValue;
} /* sf_deviceMgmt_removeDevice() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_getDevice()
------------------------------------------------------------------------------*/
sf_sensor_t* sf_deviceMgmt_getDevice(linkaddr_t shortAddress)
{
  sf_sensor_t* sensorListEntry = NULL;
  uint8_t sensorListIndex = 0;

  if(0 != gRegisteredDevicesCnt)
  {
    for(sensorListIndex = 0; sensorListIndex < SF_DEVICEMGMT_SENSOR_CNT_MAX;
        sensorListIndex++)
    {
      if(linkaddr_cmp(&shortAddress, &gpSensorList[sensorListIndex].shortAddress))
      {
        sensorListEntry = &gpSensorList[sensorListIndex];
        break;
      }
    }
  }

  return sensorListEntry;
} /* sf_deviceMgmt_getDevice() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_getDeviceByIndex()
------------------------------------------------------------------------------*/
sf_sensor_t* sf_deviceMgmt_getDeviceByIndex(uint8_t index)
{
  if(index < SF_DEVICEMGMT_SENSOR_CNT_MAX)
  {
    return &gpSensorList[index];
  }

  return NULL;
} /* sf_deviceMgmt_getDeviceByIndex() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_getDeviceBySerial()
------------------------------------------------------------------------------*/
sf_sensor_t* sf_deviceMgmt_getDeviceBySerial(uint32_t serial)
{
  sf_sensor_t* sensorListEntry = NULL;
  uint8_t sensorListIndex = 0;

  for(sensorListIndex = 0; sensorListIndex < SF_DEVICEMGMT_SENSOR_CNT_MAX;
      sensorListIndex++)
  {
    if(0 != gpSensorList[sensorListIndex].serialNr)
    {
      if(serial == gpSensorList[sensorListIndex].serialNr)
      {
        sensorListEntry = &gpSensorList[sensorListIndex];
        break;
      }
    }
  }

  return sensorListEntry;
} /* sf_deviceMgmt_getDeviceBySerial() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_getRegisteredDeviceCount()
------------------------------------------------------------------------------*/
uint8_t sf_deviceMgmt_getRegisteredDeviceCount(void)
{
  return gRegisteredDevicesCnt;
} /* sf_deviceMgmt_getRegisteredDeviceCount() */

/*------------------------------------------------------------------------------
  sf_deviceMgmt_readSensorList()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_deviceMgmt_readSensorList(void)
{
  /* Return value */
  E_SF_RETURN_t readValue;
  /* Buffer to store the sensor list data retrieved from the flash. */
  sf_persistent_sensorList_t persistentSensorList;

  readValue = sf_persistentDataStorage_readSensorList(&persistentSensorList);
  if(E_SF_SUCCESS == readValue)
  {
    for(uint8_t i = 0; i < SF_DEVICEMGMT_SENSOR_CNT_MAX; i++)
    {
      memcpy((uint8_t*)gpSensorList[i].shortAddress.u8,
             persistentSensorList.pSensorList[i].deviceAddr,
             LINKADDR_SIZE);
      gpSensorList[i].serialNr = persistentSensorList.pSensorList[i].deviceSerial;
    }

    gRegisteredDevicesCnt = persistentSensorList.registeredSensorCnt;
  }

  return readValue;
} /* sf_deviceMgmt_readSensorList() */
