#ifdef __cplusplus
extern "C" {
#endif

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
 @brief      This header contains the application interfaces.
*/

#ifndef __APP_API_H__
#define __APP_API_H__

/**
 *  @addtogroup SF_APP_API
 *
 *  @details
 *
 *  - <b>BMSCC APP API</b>\n
 *    | API Function                              | Description                                     |
 *    |-------------------------------------------|-------------------------------------------------|
 *    | @ref sf_app_txData()                      | @copybrief sf_app_txData()                      |
 *    | @ref sf_app_output_callback()             | @copybrief sf_app_output_callback()             |
 *    | @ref sf_app_handleMeasurement()           | @copybrief sf_app_handleMeasurement()           |
 *  @{
 */

/*=============================================================================
                                INCLUDES
=============================================================================*/
/* Standard include */
#include <stdint.h>
#include <stdlib.h>
/* Stack specific includes */
#include "nullnet.h"
/* Application includes */
#include "sf_types.h"

/*=============================================================================
                                API FUNCTIONS
=============================================================================*/
/*============================================================================*/
/**
 * \brief This function sends data to the given end point address. This can be
 *        used by the customer to send remote configurations or commands to the
 *        connected SCs.
 *
 * \param pData       Pointer to the data storage.
 * \param dataLen     The length of the data.
 * \param pDest       Pointer to the smart cell link address.
 *
 * \return @ref E_SF_RETURN_t
 */
/*============================================================================*/
E_SF_RETURN_t sf_app_txData(uint8_t* pData, uint8_t dataLen, linkaddr_t* pDest);

/*============================================================================*/
/**
 * \brief This is a Tx callback function. TSCH calls this function to inform the
 *        application about the message transmission status.
 *
 * \param ptr         Pointer to application data.
 * \param status      Status of Tx.
 */
/*============================================================================*/
void sf_app_output_callback(void *ptr, nullnet_tx_status_t status);

/*============================================================================*/
/**
 * \brief Handle received measurement. The customer can add his specific
 *        implementation.
 *
 * \param pInBuf        Pointer to the frame.
 * \param length        Frame length.
 * \param pSrc          End point short address.
 */
/*============================================================================*/
void sf_app_handleMeasurement(uint8_t* pInBuf, uint8_t length, linkaddr_t* pSrc);

/*! @} */

#endif /* __APP_API_H__ */

#ifdef __cplusplus
}
#endif
