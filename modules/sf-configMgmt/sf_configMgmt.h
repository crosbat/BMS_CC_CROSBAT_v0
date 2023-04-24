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
 * @brief      API definition of the configuration management module.
 *
 */

#ifndef SF_CONFIGMGMT_H_
#define SF_CONFIGMGMT_H_

/**
 *  @addtogroup SF_CONFIG_MGMT
 *
 *  @details
 *
 *  - <b>CONFIG MGMT API</b>\n
 *    | API Function                              | Description                                     |
 *    |-------------------------------------------|-------------------------------------------------|
 *    | @ref sf_configMgmt_setParam()             | @copybrief sf_configMgmt_setParam()             |
 *    | @ref sf_configMgmt_getParam()             | @copybrief sf_configMgmt_getParam()             |
 *    | @ref sf_configMgmt_setDefaultConfig()     | @copybrief sf_configMgmt_setDefaultConfig()     |
 *    | @ref sf_configMgmt_readConfig()           | @copybrief sf_configMgmt_readConfig()           |
 *  @{
 */

/*****************************************************************************/
/*                         INCLUDES                                          */
/*****************************************************************************/
/* Standard include */
#include <stddef.h>
/* Stack include */
#include "linkaddr.h"
/* Device specific includes */
#include <modules/common/sf_types.h>

/*****************************************************************************/
/*                         STRUCTS                                           */
/*****************************************************************************/
/*! General device configuration. */
typedef struct
{
  /*! Serial number of the BMS-CC */
  uint32_t deviceSerial;
  /*! Link address of the BMS-CC. */
  linkaddr_t deviceAddr;
  /*! Network ID. */
  uint16_t panId;
} sf_deviceConfig_t;

/*****************************************************************************/
/*                         ENUMS                                             */
/*****************************************************************************/
/*! Device configurations identifiers */
typedef enum
{
  /*! BMS-CC serial number */
  E_CONFIGMGMT_PARAM_DEVICE_SERIAL,
  /*! BMS-CC short address */
  E_CONFIGMGMT_PARAM_DEVICE_ADDR,
  /*! Network ID */
  E_CONFIGMGMT_PARAM_PAN_ID,
} SF_CONFIGMGMT_PARAM_t;

/*****************************************************************************/
/*                         API FUNCTIONS                                     */
/*****************************************************************************/
/*============================================================================*/
/**
 * \brief Set the configuration parameter.
 *
 * \param pParamData       Pointer to the parameter value.
 * \param paramSize        Size of the parameter value.
 * \param paramType        The type of the parameter.
 *
 * \return  @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_configMgmt_setParam(const void* pParamData, size_t paramSize,
                                     SF_CONFIGMGMT_PARAM_t paramType);

/*============================================================================*/
/**
 * \brief Get the configuration parameter.
 *
 * \param pParamData       Pointer to the parameter storage.
 * \param paramSize        Size of the parameter value.
 * \param paramType        The type of the parameter.
 *
 * \return  @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_configMgmt_getParam(void* pParamData, size_t paramSize,
                                     SF_CONFIGMGMT_PARAM_t paramType);

/*============================================================================*/
/**
 * \brief Set the configurations to the default values. The assigned values will
 *        be stored persistently into the internal flash.
 *
 * \return  @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_configMgmt_setDefaultConfig(void);

/*============================================================================*/
/**
 * \brief Read stored device configuration out of the NVM.
 *
 * \return  @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_configMgmt_readConfig(void);

/*! @} */

#endif /* SF_CONFIGMGMT_H_ */

#ifdef __cplusplus
}
#endif
