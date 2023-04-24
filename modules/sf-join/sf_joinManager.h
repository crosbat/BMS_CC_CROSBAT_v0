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
 @brief      This header contains the join mode interfaces.
*/

#ifndef _SF_JOINMANAGER_H_
#define _SF_JOINMANAGER_H_

/**
 *  @addtogroup SF_JOIN_MANAGER
 *
 *  @details
 *
 *  - <b>SF JOIN MANAGER API</b>\n
 *    | API Function                              | Description                                     |
 *    |-------------------------------------------|-------------------------------------------------|
 *    | @ref sf_joinManger_start()                | @copybrief sf_joinManger_start()                |
 *    | @ref sf_joinManger_openManualWindow()     | @copybrief sf_joinManger_openManualWindow()     |
 *  @{
 */

/*=============================================================================
                                INCLUDES
=============================================================================*/
/* Standard include */
#include <stdint.h>
/* Stack include */
#include "net/linkaddr.h"
#include "net/nullnet/nullnet.h"
/* Module specific include */
#include "sf_frameType.h"

/*=============================================================================
                                API FUNCTIONS
=============================================================================*/
/*============================================================================*/
/**
 * \brief Start the join service. Once started, it keeps running
 *        all life time.
 */
/*============================================================================*/
void sf_joinManger_start(void);

/*============================================================================*/
/**
 * \brief Opens manual join window.
 *
 * \return true if the window is successfully opened
 *         false otherwise
 */
/*============================================================================*/
bool sf_joinManger_openManualWindow(void);

/*! @} */

#endif /* _SF_JOINMANAGER_H_ */

#ifdef __cplusplus
}
#endif
