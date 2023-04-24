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
 * @brief      API implementation of the config management.
 *
 */

/*****************************************************************************/
/*                         INCLUDES                                          */
/*****************************************************************************/
/* Standard include */
#include <stdio.h>
#include <string.h>
/* Stack includes */
#if !CONTIKI_TARGET_COOJA
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_fcfg1.h)
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include "ieee-addr.h"
#else
#include <dev/moteid.h>
#endif
/* Module specific include */
#include "sf_persistentDataStorage.h"
#include "sf_configMgmt.h"
#include "sf_types.h"

/*****************************************************************************/
/*                         MACROS                                            */
/*****************************************************************************/
/* IEEE 802.15.4 MAC address. This is a unique entity provided by the vendor.
   We will use it to configure the default device serial number. */
#define SF_CONFIGMGMT_MAC_PRIMARY_ADDRESS          (FCFG1_BASE + FCFG1_O_MAC_15_4_0)

/*****************************************************************************/
/*                         GLOBAL VARIABLES                                  */
/*****************************************************************************/
/* SC configuration parameters */
sf_deviceConfig_t gDeviceConfig = {0};

/*****************************************************************************/
/*                         API FUNCTIONS                                     */
/*****************************************************************************/
/*------------------------------------------------------------------------------
  sf_configMgmt_setParam()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_configMgmt_setParam(const void* pParamData, size_t paramSize,
                                     SF_CONFIGMGMT_PARAM_t paramType)
{
  E_SF_RETURN_t returnValue = E_SF_ERROR_INVALID_PARAM;
  /* Buffer to store the configuration to be written into the flash. */
  sf_persistent_deviceConfig_t persistentDeviceConfig;

  if(NULL == pParamData)
  {
    return returnValue;
  }

  switch (paramType)
  {
    case E_CONFIGMGMT_PARAM_DEVICE_SERIAL:
      if(paramSize == sizeof(gDeviceConfig.deviceSerial))
      {
        memcpy(&gDeviceConfig.deviceSerial, pParamData, paramSize);
        returnValue = E_SF_SUCCESS;
      }
      break;

    case E_CONFIGMGMT_PARAM_DEVICE_ADDR:
      if(paramSize == sizeof(gDeviceConfig.deviceAddr))
      {
        memcpy(gDeviceConfig.deviceAddr.u8, pParamData, paramSize);
        returnValue = E_SF_SUCCESS;
      }
      break;

    case E_CONFIGMGMT_PARAM_PAN_ID:
      if(paramSize == sizeof(gDeviceConfig.panId))
      {
        memcpy(&gDeviceConfig.panId, pParamData, paramSize);
        returnValue = E_SF_SUCCESS;
      }
      break;

    default:
      break;
  }

  if(E_SF_SUCCESS == returnValue)
  {
    persistentDeviceConfig.deviceSerial = gDeviceConfig.deviceSerial;
    persistentDeviceConfig.panId = gDeviceConfig.panId;
    memcpy(persistentDeviceConfig.deviceAddr, gDeviceConfig.deviceAddr.u8,
          sizeof(persistentDeviceConfig.deviceAddr));
    returnValue = sf_persistentDataStorage_writeConfig(&persistentDeviceConfig);
  }

  return returnValue;
}/* sf_configMgmt_setParam() */


/*------------------------------------------------------------------------------
  sf_configMgmt_getDeviceSerial()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_configMgmt_getParam(void* pParamData, size_t paramSize,
                                     SF_CONFIGMGMT_PARAM_t paramType)
{
  E_SF_RETURN_t returnValue = E_SF_ERROR_INVALID_PARAM;

  if(NULL == pParamData)
  {
    return returnValue;
  }

  switch (paramType)
  {
    case E_CONFIGMGMT_PARAM_DEVICE_SERIAL:
      if(paramSize == sizeof(gDeviceConfig.deviceSerial))
      {
        memcpy(pParamData, &gDeviceConfig.deviceSerial, paramSize);
        returnValue = E_SF_SUCCESS;
      }
      break;

    case E_CONFIGMGMT_PARAM_DEVICE_ADDR:
      if(paramSize == sizeof(gDeviceConfig.deviceAddr))
      {
        memcpy(pParamData, gDeviceConfig.deviceAddr.u8, paramSize);
        returnValue = E_SF_SUCCESS;
      }
      break;

    case E_CONFIGMGMT_PARAM_PAN_ID:
      if(paramSize == sizeof(gDeviceConfig.panId))
      {
        memcpy(pParamData, &gDeviceConfig.panId, paramSize);
        returnValue = E_SF_SUCCESS;
      }
      break;

    default:
      break;
  }

  return returnValue;
}

/*------------------------------------------------------------------------------
  sf_configMgmt_setDefaultConfig()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_configMgmt_setDefaultConfig(void)
{
  E_SF_RETURN_t returnVal = E_SF_ERROR;
  /* BMSCC short address */
  linkaddr_t bmsccAddr = linkaddr_null;
  /* Network ID */
  uint16_t panId = 0;
  /* Serial number */
  uint32_t serialNr = 0;
#if !CONTIKI_TARGET_COOJA
  /* Pointer to IEEE MAC address */
  uint8_t *pMacAddr = NULL;
#endif

#if !CONTIKI_TARGET_COOJA
  /* Set the IEEE MAC address as device link address as it should be unique. */
  ieee_addr_cpy_to(bmsccAddr.u8, LINKADDR_SIZE);
#else
  bmsccAddr.u16 = (uint16_t)simMoteID;
#endif
  returnVal = sf_configMgmt_setParam(bmsccAddr.u8,
                                     LINKADDR_SIZE,
                                     E_CONFIGMGMT_PARAM_DEVICE_ADDR);

  if(E_SF_SUCCESS == returnVal)
  {
    /* Use the IEEE MAC address to set the network ID as it should be unique.*/
#if !CONTIKI_TARGET_COOJA
    ieee_addr_cpy_to((uint8_t*)&panId, sizeof(panId));
#else
  panId = (uint16_t)simMoteID;
#endif
    returnVal = sf_configMgmt_setParam(&panId,
                                       sizeof(panId),
                                       E_CONFIGMGMT_PARAM_PAN_ID);
  }

  if(E_SF_SUCCESS == returnVal)
  {
#if !CONTIKI_TARGET_COOJA
    pMacAddr = (uint8_t*)SF_CONFIGMGMT_MAC_PRIMARY_ADDRESS;
    memcpy(&serialNr, pMacAddr, sizeof(serialNr));
#else
    serialNr = (uint32_t)simMoteID;
#endif
    returnVal = sf_configMgmt_setParam(&serialNr, sizeof(serialNr),
                                       E_CONFIGMGMT_PARAM_DEVICE_SERIAL);
  }

  return returnVal;
} /* sf_configMgmt_setDefaultConfig() */

/*------------------------------------------------------------------------------
  sf_configMgmt_readConfig()
------------------------------------------------------------------------------*/
E_SF_RETURN_t sf_configMgmt_readConfig(void)
{
  E_SF_RETURN_t readStatus = E_SF_ERROR;
  /* Buffer to store the data device lifetime configuration read from
     the flash. */
  sf_persistent_deviceConfig_t persistentDeviceConfig;

  readStatus = sf_persistentDataStorage_readConfig(&persistentDeviceConfig);
  if(E_SF_SUCCESS == readStatus)
  {
    gDeviceConfig.deviceSerial = persistentDeviceConfig.deviceSerial;
    gDeviceConfig.panId = persistentDeviceConfig.panId;
    memcpy(gDeviceConfig.deviceAddr.u8, persistentDeviceConfig.deviceAddr,
            sizeof(persistentDeviceConfig.deviceAddr));
  }

  return readStatus;
} /* sf_configMgmt_readConfig() */
