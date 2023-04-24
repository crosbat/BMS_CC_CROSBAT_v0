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
 @brief      Implementation of the joining process logic.
*/

/*=============================================================================
                                INCLUDES
=============================================================================*/
/* Stack specific includes */
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "net/nullnet/nullnet.h"
#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
#include "sf-tsch-schedule.h"
#endif
/* Application specific includes */
#include "sf_join.h"
#include "sf_joinFramer.h"
#include "sf_joinManager.h"
#include "sf_frameType.h"
#include "sf_types.h"
#include "sf_stateManager.h"
#include "sf_deviceMgmt.h"
#include "sf_configMgmt.h"
#include "project-conf.h"
/* Log configuration */
#include "sys/log.h"

/*=============================================================================
                                MACROS
=============================================================================*/
#define LOG_MODULE "Join"
#ifndef LOG_CONF_APP
  #define LOG_LEVEL     LOG_LEVEL_NONE
#else
  #define LOG_LEVEL     LOG_CONF_APP
#endif
/*============================================================================*/
/*!
 * @brief Converts an uint16_t to an uint8_t array.
 * @param x           uint8_t array to write the value in.
 * @param y           Value to convert.
 */
/*============================================================================*/
#define UINT16_TO_UINT8(x, y)  ((x)[0] = (uint8_t)((uint8_t)((y)>>8U) & 0xFFU)); \
                               ((x)[1] = (uint8_t)((y) & 0xFFU))

/* The join frame retransmit period until it is received by the end point */
#ifndef SF_JOIN_CONF_RETRY_TX_TIME
#define SF_JOIN_RETRY_TX_TIME         (3 * CLOCK_SECOND)
#else
#define SF_JOIN_RETRY_TX_TIME         (SF_JOIN_CONF_RETRY_TX_TIME * CLOCK_SECOND)
#endif
/* Successful Rx state timeout */
#ifndef SF_JOIN_CONF_SUCC_STATE_TIMEOUT
#define SF_JOIN_SUCC_STATE_TIMEOUT    (30 * CLOCK_SECOND)
#else
#define SF_JOIN_SUCC_STATE_TIMEOUT    (SF_JOIN_CONF_SUCC_STATE_TIMEOUT * CLOCK_SECOND)
#endif
/* Response Tx state timeout */
#ifndef SF_JOIN_CONF_RESP_STATE_TIMEOUT
#define SF_JOIN_RESP_STATE_TIMEOUT    (30 * CLOCK_SECOND)
#else
#define SF_JOIN_RESP_STATE_TIMEOUT    (SF_JOIN_CONF_RESP_STATE_TIMEOUT * CLOCK_SECOND)
#endif
/* Maximum number of joined devices. */
#ifndef SF_CONF_SENSOR_CNT_MAX
#error "Please define SF_CONF_SENSOR_CNT_MAX!"
#else
#define SF_JOIN_SENSOR_CNT_MAX        SF_CONF_SENSOR_CNT_MAX
#endif
/* Define the number of the states. */
#define SF_JOIN_STATES_COUNT          (3U)
/* Define maximum number of pending join requests. */
#define SF_JOIN_PENDING_REQ_MAX       SF_JOIN_SENSOR_CNT_MAX
/* This macro defines the manual window open time */
#ifndef SF_CONF_MANUAL_WINDOW_TIMEOUT
#define SF_JOIN_MANUAL_WINDOW_TIMEOUT (180 * CLOCK_SECOND)
#else
#define SF_JOIN_MANUAL_WINDOW_TIMEOUT (SF_CONF_MANUAL_WINDOW_TIMEOUT * CLOCK_SECOND)
#endif

/*=============================================================================
                                PROCESSES
=============================================================================*/
PROCESS(manual_window_process, "Manual window process");
PROCESS(response_tx_process, "Send response process");

/*=============================================================================
                                ENUMS
=============================================================================*/
/* Defines the join states of the coordinator. */
typedef enum
{
  /* Join request reception state. */
  E_JOIN_STATE_REQ_RX = 0,
  /* Join response transmission state. */
  E_JOIN_STATE_RESP_TX = 1,
  /* Join successful reception state. */
  E_JOIN_STATE_SUCC_RX = 2
} E_JOIN_MODE_STATE_t;

/*=============================================================================
                                STRUCTS
=============================================================================*/
/* This struct defines the pending join request parameters. */
typedef struct
{
  /* Initial end point short address */
  linkaddr_t firstLinkaddr;
  /* The assigned short address */
  linkaddr_t newLinkaddr;
  /* The join request frame data */
  joinRequest_t joinRequest;
} sf_pendingRequest_t;

/*=============================================================================
                                GLOBAL VARIABLES
=============================================================================*/
/* Manual window timer's data. */
static struct etimer gManualWindowTimer;
/* Join response state's timer. */
static struct etimer gJoinResponseTimer;
/* Join successful state's timer. */
static struct ctimer gJoinSuccessfulTimer;
/* Stores the state manager context. */
static stateManager_ctx_t gJoinStateCtx = {0};
/* Stores the states parameters. */
static stateManager_state_t gpStates[SF_JOIN_STATES_COUNT];
/* The ongoing join request parameters. */
static sf_pendingRequest_t pendingRequest = {0};
/* Stores the pending join requests. */
static sf_pendingRequest_t gpPendingRequests[SF_JOIN_PENDING_REQ_MAX];
/* Stores the number of pending join requests. */
static uint8_t gPendingRequestsCount = 0;
/* false: to say no join response hanging in the mac.
   true: to say there is join response ongoing and we are waiting for a callback
         from the mac. */
static bool gRespActive = false;
/* Stores response Tx status. */
static bool gRespTxFailed = false;

/*=============================================================================
                          LOCAL FUNCTION IMPLEMENTATION
=============================================================================*/
/*------------------------------------------------------------------------------
  join_timeout_callback()
------------------------------------------------------------------------------*/
static void loc_join_timeout_callback(void * ptr)
{
  LOG_INFO("Join state timeout expired. Cancel current join process and\
            reset state machine.\n");

  /* Delete the join process slots. */
  sf_tsch_schedule_delete_jproc_slots(NULL);

  /* Clear current request handler. */
  memset(&pendingRequest, 0x00, sizeof(sf_pendingRequest_t));

  /* Proceed with the next join request. */
  sf_stateManager_setState(&gJoinStateCtx, (uint8_t)E_JOIN_STATE_REQ_RX);
  sf_stateManager_execState(&gJoinStateCtx);
} /* join_timeout_callback() */

/*------------------------------------------------------------------------------
  sf_joinManger_openManualWindow()
------------------------------------------------------------------------------*/
static void loc_closeWindow(void)
{
  /* Set set beacon flag to disable. */
  tsch_set_join_mode(E_SF_JOIN_BEACON_DISABLED);

  /* Turn green LED off as join window is closed. */
  leds_single_off(LEDS_LED2);

  LOG_INFO("Manual join window is closed; \n");
} /* loc_closeWindow() */

/*------------------------------------------------------------------------------
  loc_rxRequest()
------------------------------------------------------------------------------*/
static void loc_rxRequest(void)
{
  /* True if a join request from already registered sensor is received,
     false if it is a new sensor. */
  bool oldSensor = false;
  /* True if valid join request is pulled from the list, false otherwise */
  bool requestPulled = false;

  if(0x00 < gPendingRequestsCount)
  {
    do
    {
      /* Get next pending join request. */
      memcpy(&pendingRequest, &gpPendingRequests[gPendingRequestsCount - 1],
             sizeof(sf_pendingRequest_t));
      gPendingRequestsCount -= 1;

      if((!linkaddr_cmp(&linkaddr_null, &pendingRequest.firstLinkaddr)) &&
         (pendingRequest.joinRequest.serialNumber > 0))
      {
        requestPulled = true;
      }
    } while((false == requestPulled) && (0 < gPendingRequestsCount));

    if(true == requestPulled)
    {
      /* Check if it is an old or new sensor. */
      if(NULL != sf_deviceMgmt_getDeviceBySerial(pendingRequest.joinRequest.serialNumber))
      {
        oldSensor = true;
      }

      if((SF_JOIN_SENSOR_CNT_MAX > sf_deviceMgmt_getRegisteredDeviceCount()) ||
        (true == oldSensor))
      {
        LOG_INFO("A join request is pulled from the queue; pending join request; %d src; ",
                  gPendingRequestsCount);
        LOG_INFO_LLADDR(&pendingRequest.firstLinkaddr);
        LOG_INFO_(" SerialNo: %ld\n", pendingRequest.joinRequest.serialNumber);

        /* Assign new address to be used for regular communication  */
        sf_deviceMgmt_getFreeAddress(&pendingRequest.newLinkaddr.u16,
                                     pendingRequest.joinRequest.serialNumber);
#if CONTIKI_TARGET_COOJA
        /* In case of COOJA, address 0x0001 is reserved to the gateway.
           Therefore offset of 1 is added to the assigned address */
        pendingRequest.newLinkaddr.u16 += 1;
#endif

        /* Set next state to E_JOIN_STATE_RESP_TX */
        sf_stateManager_setState(&gJoinStateCtx, (uint8_t)E_JOIN_STATE_RESP_TX);
        sf_stateManager_execState(&gJoinStateCtx);
      }
      else
      {
        LOG_INFO("!Can not join new device; Registered devices reached limit;\
                  Device count %d; Requests left %d\n;",
                  sf_deviceMgmt_getRegisteredDeviceCount(), gPendingRequestsCount);

        /* Remove all pending requests. */
        gPendingRequestsCount = 0x00;

        /* Clear current request. */
        memset(&pendingRequest, 0x00, sizeof(sf_pendingRequest_t));
      }
    }
    else
    {
      LOG_ERR("!No valid join request pulled from queue!");
    }
  }
  else
  {
    LOG_INFO("No pending join request \n");
  }
} /* loc_rxRequest() */

/*------------------------------------------------------------------------------
  loc_txResponse()
------------------------------------------------------------------------------*/
static void loc_txResponse(void)
{
  /* Start state timeout. */
  etimer_set(&gJoinResponseTimer, SF_JOIN_RESP_STATE_TIMEOUT);

  /* Schedule join process slots. */
  sf_tsch_schedule_add_jproc_slots((const linkaddr_t*)&pendingRequest.firstLinkaddr);

  /* Start response transmission process. */
  if(process_is_running(&response_tx_process))
  {
    process_exit(&response_tx_process);
  }

  process_start(&response_tx_process, NULL);
} /* loc_txResponse() */

/*------------------------------------------------------------------------------
  loc_rxSuccessful()
------------------------------------------------------------------------------*/
static void loc_rxSuccessful(void)
{
  /* Start state timeout. */
  ctimer_set(&gJoinSuccessfulTimer, SF_JOIN_SUCC_STATE_TIMEOUT,
             loc_join_timeout_callback, NULL);

  LOG_INFO("Waiting for successful Rx from; ");
  LOG_INFO_LLADDR(&pendingRequest.newLinkaddr);
  LOG_INFO_("\n");
} /* loc_rxSuccessful() */

/*=============================================================================
                          PROCESSES IMPLEMENTATION
=============================================================================*/
/*------------------------------------------------------------------------------
  manual_window_process()
------------------------------------------------------------------------------*/
PROCESS_THREAD(manual_window_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Manual join window is open. It will close again after %umin \n",
            (unsigned int)(SF_JOIN_MANUAL_WINDOW_TIMEOUT/(CLOCK_SECOND * 60)));

  /* Set window timeout. */
  etimer_set(&gManualWindowTimer, SF_JOIN_MANUAL_WINDOW_TIMEOUT);

  /* Wait for window timeout expiration to close the join window. */
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&gManualWindowTimer));

  /* Close the window. */
  loc_closeWindow();

  PROCESS_END();
} /* manual_window_process() */

/*------------------------------------------------------------------------------
  response_tx_process()
------------------------------------------------------------------------------*/
PROCESS_THREAD(response_tx_process, ev, data)
{
  /* Tx timeout timer handler. */
  static struct etimer txTimer;
  E_SF_RETURN_t ret = E_SF_ERROR;

  PROCESS_BEGIN();

  while(1)
  {
    LOG_INFO("Wait until Tx timeout (%lu second) expires.\n",
             (long unsigned)(SF_JOIN_RETRY_TX_TIME / CLOCK_SECOND));
    /* Set Tx timeout. */
    etimer_set(&txTimer, (clock_time_t)SF_JOIN_RETRY_TX_TIME);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&txTimer));

    if(E_JOIN_STATE_RESP_TX == (E_JOIN_MODE_STATE_t)sf_stateManager_getState(&gJoinStateCtx))
    {
      /* Check if the state timeout is already expired */
      if(etimer_expired(&gJoinResponseTimer) && false == gRespActive)
      {
        loc_join_timeout_callback(NULL);

        break;
      }

      if(false == gRespActive)
      {
        gRespActive = true;
        gRespTxFailed = false;
        ret = sf_join_response_send(&pendingRequest.firstLinkaddr,
                                    &pendingRequest.newLinkaddr);
        if(E_SF_SUCCESS != ret)
        {
          gRespActive = false;

          LOG_ERR("!Fail to send join response. Waiting for state timeout \
                   to expire to pursue\n");

          PROCESS_EXIT();
        }
      }
      else
      {
        LOG_INFO("!Can not send new join response. There is an active \
                  join response in the lower layer. \n");
      }
    }
    else
    {
      PROCESS_EXIT();
    }
  }

  PROCESS_END();
} /* response_tx_process() */

/*=============================================================================
                          API IMPLEMENTATION
=============================================================================*/
/*------------------------------------------------------------------------------
  sf_joinManger_openManualWindow()
------------------------------------------------------------------------------*/
bool sf_joinManger_openManualWindow(void)
{
  bool windowOpen = false;

  /* Check if the registered devices reached the limit. */
  if(SF_JOIN_SENSOR_CNT_MAX > sf_deviceMgmt_getRegisteredDeviceCount())
  {
    /* Turn green LED on as manual window is opened. */
    leds_single_on(LEDS_LED2);

    /* Set join flag at beacon telegram. */
    if(E_SF_JOIN_BEACON_ENABLED != (E_SF_JOIN_BEACON_t)tsch_get_join_mode())
    {
      tsch_set_join_mode(E_SF_JOIN_BEACON_ENABLED);
    }

    if(process_is_running(&manual_window_process))
    {
      process_exit(&manual_window_process);
    }

    process_start(&manual_window_process, NULL);

    windowOpen = true;
  }
  else
  {
    LOG_INFO("!The joined device limit is reached;\
              The join window is closed again. \n");
  }

  return windowOpen;
} /* sf_joinManger_openManualWindow() */

/*------------------------------------------------------------------------------
  sf_joinManger_start()
------------------------------------------------------------------------------*/
void sf_joinManger_start()
{
  /* Initialize state manager. */
  gpStates[0].state = (uint8_t)E_JOIN_STATE_REQ_RX;
  gpStates[0].stateFct =&loc_rxRequest;
  gpStates[1].state = (uint8_t)E_JOIN_STATE_RESP_TX;
  gpStates[1].stateFct =&loc_txResponse;
  gpStates[2].state = (uint8_t)E_JOIN_STATE_SUCC_RX;
  gpStates[2].stateFct = &loc_rxSuccessful;
  sf_stateManager_init(&gJoinStateCtx, gpStates, SF_JOIN_STATES_COUNT);

  /* Initialize join request queue. */
  memset(gpPendingRequests, 0U, sizeof(gpPendingRequests));
  gPendingRequestsCount = 0x00;

  /* Add join request slots */
  sf_tsch_schedule_add_jreq_slots();

  /* Initially close the manual window and open roaming window. */
  loc_closeWindow();
} /* sf_joinManger_start() */

/*=============================================================================
                          CALLBACK IMPLEMENTATION
=============================================================================*/
/*------------------------------------------------------------------------------
  sf_join_output_callback()
------------------------------------------------------------------------------*/
void sf_join_output_callback(void *ptr, nullnet_tx_status_t status)
{
  if(E_JOIN_STATE_RESP_TX == (E_JOIN_MODE_STATE_t)sf_stateManager_getState(&gJoinStateCtx))
  {

    if((NULLNET_TX_OK != status) && (NULLNET_TX_BUSY != status))
    {
      /* A response fragment not successfully transmitted. */
      LOG_INFO("Response frag not successfully sent \n");
      gRespTxFailed = true;
    }

    if(false == gRespTxFailed)
    {
      LOG_INFO("Join response is sent successfully to: %ld, firstAddr:",
               pendingRequest.joinRequest.serialNumber);
      LOG_INFO_LLADDR(&pendingRequest.firstLinkaddr);
      LOG_INFO_("; New device address: ");
      LOG_INFO_LLADDR(&pendingRequest.newLinkaddr);
      LOG_INFO_("\n");

      etimer_stop(&gJoinResponseTimer);

      /* Set next state to E_JOIN_STATE_SUCC_RX */
      sf_stateManager_setState(&gJoinStateCtx, (uint8_t)E_JOIN_STATE_SUCC_RX);
      sf_stateManager_execState(&gJoinStateCtx);
    }
    else
    {
      if(etimer_expired(&gJoinResponseTimer))
      {
        process_exit(&response_tx_process);

        loc_join_timeout_callback(NULL);
      }
    }

    /* If reach Here, then we are sure that no active join response
        exists in the mac. */
    gRespActive = false;
  }
} /* sf_join_output_callback() */

/*=============================================================================
                          INDICATIONS
=============================================================================*/
/*------------------------------------------------------------------------------
  sf_join_request_receive()
------------------------------------------------------------------------------*/
void sf_join_request_receive(linkaddr_t *src)
{
  /* Join request frame params. */
  joinRequest_t joinReqParams = {0};
  /* Pending request index. */
  uint8_t reqIdx = 0x00;
  /* True if a join request from already registEred sensor is received,
     false otherwise. */
  bool oldSensor = false;

  /* Join request frame params. */
  joinReqParams = sf_joinFramer_getRequestParams();

  LOG_INFO("Received join request from ; ");
  LOG_INFO_LLADDR(src);
  LOG_INFO_(", serialNo: %ld\n", joinReqParams.serialNumber);

  if(joinReqParams.serialNumber == 0U)
  {
    LOG_INFO("Reject join request. Device has no valid serial number!\n");
    return;
  }

  /* Check if it is an old or new device. */
  if(NULL != sf_deviceMgmt_getDeviceBySerial(joinReqParams.serialNumber))
  {
    oldSensor = true;
  }

  if((SF_JOIN_SENSOR_CNT_MAX > sf_deviceMgmt_getRegisteredDeviceCount()) ||
     (true == oldSensor))
  {
    while((reqIdx < gPendingRequestsCount) &&
          !linkaddr_cmp(&gpPendingRequests[reqIdx].firstLinkaddr, src))
    {
      reqIdx += 0x01;
    }

    if(reqIdx == gPendingRequestsCount)
    {
      if(SF_JOIN_PENDING_REQ_MAX > gPendingRequestsCount)
      {
        gpPendingRequests[gPendingRequestsCount].firstLinkaddr = *src;
        memcpy(&gpPendingRequests[gPendingRequestsCount].joinRequest, &joinReqParams,
               sizeof(joinRequest_t));

        gPendingRequestsCount += 1;

        LOG_INFO("Added received join request to the queue (index: %d; src; ",
                 gPendingRequestsCount);
        LOG_INFO_LLADDR(src);
        LOG_INFO_("\n");
      }
      else
      {
        LOG_INFO("Join request queue is full; src; ");
        LOG_INFO_LLADDR(src);
        LOG_INFO_("\n");
      }
    }
    else
    {
      gpPendingRequests[reqIdx].firstLinkaddr = *src;
      memcpy(&gpPendingRequests[reqIdx].joinRequest, &joinReqParams,
             sizeof(joinRequest_t));

      LOG_INFO("!Join request exists already in the queue; The old request \
                is overwritten; Device address; ");
      LOG_INFO_LLADDR(src);
      LOG_INFO_("\n");
    }

    /* If no device is in join process handle, start join process. */
    if(linkaddr_cmp(&linkaddr_null, &pendingRequest.firstLinkaddr))
    {
      /* Process the requests. */
      sf_stateManager_setState(&gJoinStateCtx, (uint8_t)E_JOIN_STATE_REQ_RX);
      sf_stateManager_execState(&gJoinStateCtx);
    }
  }
  else
  {
    LOG_INFO("!Cannot join new device; Registered devices reached limit; \
              Device count %d\n;",
              sf_deviceMgmt_getRegisteredDeviceCount());
  }
} /* sf_join_request_receive() */

/*------------------------------------------------------------------------------
  sf_join_successful_receive()
------------------------------------------------------------------------------*/
void sf_join_successful_receive(linkaddr_t *src)
{
  E_SF_RETURN_t ret = E_SF_ERROR;
  /* Stores current state. */
  E_JOIN_MODE_STATE_t currentState = 0;

  currentState = (E_JOIN_MODE_STATE_t)sf_stateManager_getState(&gJoinStateCtx);

  if((E_JOIN_STATE_SUCC_RX == currentState) &&
     (linkaddr_cmp(&pendingRequest.newLinkaddr, src)))
  {
    LOG_INFO("Received join successful from: ");
    LOG_INFO_LLADDR(src);
    LOG_INFO_(" SerialNo: %ld old addr was: ",
              pendingRequest.joinRequest.serialNumber);
    LOG_INFO_LLADDR(&pendingRequest.firstLinkaddr);
    LOG_INFO_("\n");

    /* Stop join timeout timer. */
    ctimer_stop(&gJoinSuccessfulTimer);

    /* Delete the join process slots from the schedule. */
    sf_tsch_schedule_delete_jproc_slots(NULL);

#if CONTIKI_TARGET_COOJA
    /* Removed added offset to add the new sensor to the correct list entry. */
    linkaddr_t tmpLinkaddr;
    linkaddr_copy(&tmpLinkaddr, &pendingRequest.newLinkaddr);
    tmpLinkaddr.u16 -= 1;
    /* Add joined device to the sensor list. */
    ret = sf_deviceMgmt_addDevice(tmpLinkaddr,
                                  pendingRequest.joinRequest.serialNumber);
#else
    /* Add joined device to the sensor list. */
    ret = sf_deviceMgmt_addDevice(pendingRequest.newLinkaddr,
                                  pendingRequest.joinRequest.serialNumber);
#endif

    if(E_SF_SUCCESS != ret)
    {
      LOG_INFO("!Can not add sensor with shortAddr ");
      LOG_INFO_LLADDR(&pendingRequest.newLinkaddr);
      LOG_INFO_(". Join request rejected!\n");
    }
    else
    {
      if(SF_JOIN_SENSOR_CNT_MAX <= sf_deviceMgmt_getRegisteredDeviceCount())
      {
        LOG_INFO("Max. device count registered. Join disabled; \n");
        tsch_set_join_mode(E_SF_JOIN_BEACON_DISABLED);
      }

      /* Schedule data slots for the joined device using the assigned device address. */
      sf_tsch_schedule_add_data_slots((const linkaddr_t*)&pendingRequest.newLinkaddr,
                                      E_SF_TSCH_SCHEDULE_DATA_SLOTS_ALL);
    }

    /* Reset pending request handler. */
    memset(&pendingRequest, 0x00, sizeof(sf_pendingRequest_t));

    /* Check if manual window timeout has not expired. */
    if(!etimer_expired(&gManualWindowTimer))
    {
      /* Restart the manual window timeout. */
      LOG_INFO("Restart manual join window timeout.\n");
      sf_joinManger_openManualWindow();
    }

    /* Set next state to E_JOIN_STATE_REQ_RX.
      New device join can be processed. */
    sf_stateManager_setState(&gJoinStateCtx, (uint8_t)E_JOIN_STATE_REQ_RX);
    sf_stateManager_execState(&gJoinStateCtx);
  }
} /* sf_join_successful_receive() */

#ifdef __cplusplus
}
#endif
