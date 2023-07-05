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
 @brief      Implementation of minimal application for battery management
             system - communication controller.
*/

/*=============================================================================
                                INCLUDES
=============================================================================*/
/* Standard include */
#include "stdint.h"
#include <string.h>
#include <stdio.h>
/* Stack include */
#include "contiki.h"
#include "sys/node-id.h"
#include "dev/button-hal.h"
#include "sys/log.h"
/* Module specific include*/
#include "project-conf.h"
/* RF regions */
#include "sf_rfSettings.h"
#include "sf_rf.h"
#include "sf-tsch-schedule.h"
#include "sf_types.h"
#include "sf_configMgmt.h"
#include "sf_deviceMgmt.h"
#include "sf_callbackHandler.h"
#include "sf_joinManager.h"
#include "sf_app_api.h"
#include "sf_absoluteTime.h"
#include "sf_tsch.h"
#include "sf_led.h"
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

/*=============================================================================
                                MACROS
=============================================================================*/
/* Defines log source */
#define LOG_MODULE                      "App"
/* Defines log level*/
#define LOG_LEVEL                       LOG_LEVEL_INFO
/* The maximum number of smart sells that can connect*/
#ifndef SF_CONF_SENSOR_CNT_MAX
#error "Please define SF_CONF_SENSOR_CNT_MAX!"
#endif
/* Defines inactive smart cell check period */
#ifndef SF_CONF_LOST_SC_CHECK_INTERVAL
#define SF_APP_LOST_SC_CHECK_INTERVAL   (60 * CLOCK_SECOND)
#else
#define SF_APP_LOST_SC_CHECK_INTERVAL   (SF_CONF_LOST_SC_CHECK_INTERVAL * \
                                         CLOCK_SECOND)
#endif
/* The maximum length of payload */
#define SF_APP_PAYLOAD_LENGTH_MAX       (40U)

/*=============================================================================
                              STRUCTS
=============================================================================*/
/*! Defines the measurement data
    It shall be as same as the measurement
    definition at the SC @ref measurementHandler_api.h.
  */
typedef struct
{
  /* Timestamp */
  uint32_t timeStamp;
  /* Measurement value */
  float value;
} meas_t;

/*=============================================================================
                                GLOBAL VARIABLES
=============================================================================*/
/* Stores callback handler context parameters. */
static sf_callbackHandlerCtxt_t gAppCallbackHandlerCtxt = {sf_app_output_callback,
                                                           NULL};

int advertiseData = 16;

int sc_address_list[30]={0};

int choose_sc = 0;

int connected_slaves = 0; //This gives the total number of connected slaves.

float slave_data;

/*=============================================================================
                                PROCESSES
=============================================================================*/
PROCESS(bmscc_app_process, "The main BMS-CC process");
AUTOSTART_PROCESSES(&bmscc_app_process);
PROCESS(button_process, "Button process");

/*=============================================================================
                                LOCAL FUNCTIONS
=============================================================================*/
/*============================================================================*/
/**
 * \brief Check is a valid sensor list stored in the internal flash.
 *        If so, it adds registered sensors's data slot to the schedule.
 */
/*============================================================================*/
static void load_registered_sensors(void)
{
  if(E_SF_SUCCESS == sf_deviceMgmt_readSensorList() &&
     0 != sf_deviceMgmt_getRegisteredDeviceCount())
  {
    LOG_INFO("Valid sensor list is found in the flash \n");

    /* Add data slot of all registered SCs and set last
      seen timestamp to the current time for each */
    for(uint8_t i = 0; i < SF_CONF_SENSOR_CNT_MAX; i++)
    {
      sf_sensor_t *pSensor = sf_deviceMgmt_getDeviceByIndex(i);
      if(NULL != pSensor)
      {
        if(0 != pSensor->serialNr &&
            !linkaddr_cmp(&pSensor->shortAddress, &linkaddr_null))
        {
          /* Add device data slots for regular communication */
          sf_tsch_addDataSlots(&pSensor->shortAddress);
        }
      }
    }
  }
  else
  {
    LOG_INFO("No sensor list is found in the flash \n");
  }
}/* load_registered_sensors() */

/*=============================================================================
                              PROCESSES IMPLEMENTATION
=============================================================================*/
/*------------------------------------------------------------------------------
  button_process()
------------------------------------------------------------------------------*/
PROCESS_THREAD(button_process, ev, data)
{
  /* Dummy downlink data */
  //static uint8_t pData[] = {34}; //orig 1
    uint8_t pData[] = {advertiseData}; //orig 1

  PROCESS_BEGIN();

  LOG_INFO("Button process started...\n");

  while(1)
  {

     //Can we see the list of SC connected here using a global variable?

//      for(uint8_t i = 0; i < SF_CONF_SENSOR_CNT_MAX; i++)
//      {
//        sf_sensor_t *pSensor = sf_deviceMgmt_getDeviceByIndex(i);
//        if(NULL != pSensor)
//        {
//          if(0 != pSensor->serialNr &&
//              !linkaddr_cmp(&pSensor->shortAddress, &linkaddr_null))
//          {
//              sc_address_list[i] = (int)pSensor->shortAddress.u16;
//          }
//        }
//      }


      /* Wait for button to be pressed. */
    PROCESS_YIELD_UNTIL( (ev == button_hal_release_event) ||
                         (ev == button_hal_periodic_event) );

    button_hal_button_t *button = (button_hal_button_t*) data;
    if(BUTTON_HAL_ID_USER_BUTTON == button->unique_id)
    {
      uint8_t pressDuration = button->press_duration_seconds;

      LOG_INFO("Button press duration [sec]: %u\n", pressDuration);

      if( ev == button_hal_release_event )
      {
        if(pressDuration > 10)
        {
          /* Only for testing */
          LOG_INFO("Remove all registered sensors from the sensor list \n");
          for(uint8_t i = 0; i < SF_CONF_SENSOR_CNT_MAX; i++)
          {
            sf_sensor_t *pSensor = sf_deviceMgmt_getDeviceByIndex(i);
            if(NULL != pSensor)
            {
              if(0 != pSensor->serialNr &&
                  !linkaddr_cmp(&pSensor->shortAddress, &linkaddr_null))
              {
                sf_tsch_deleteDataSlots(&pSensor->shortAddress);
                sf_deviceMgmt_removeDevice(pSensor->shortAddress);
              }
            }
          }
        }
        else if (pressDuration > 3U)
        {
          /* Only for testing */
          /* Send dummy downlink to all connected SCs */
//          for(uint8_t i = 0; i < SF_CONF_SENSOR_CNT_MAX; i++)
//          {
//            sf_sensor_t *pSensor = sf_deviceMgmt_getDeviceByIndex(i);
//            if(NULL != pSensor)
//            {
//              if(0 != pSensor->serialNr &&
//                  !linkaddr_cmp(&pSensor->shortAddress, &linkaddr_null))
//              {
//                sf_app_txData(pData, sizeof(pData), &pSensor->shortAddress);
//              }
//            }
//          }
          // Original code above - sends to all slaves

          sf_sensor_t *pSensor = sf_deviceMgmt_getDeviceByIndex(choose_sc);
          if(NULL != pSensor)
          {
            if(0 != pSensor->serialNr &&
                !linkaddr_cmp(&pSensor->shortAddress, &linkaddr_null))
            {
              sf_app_txData(pData, sizeof(pData), &pSensor->shortAddress);
            }
          }






        }


        else if(pressDuration <= 3U)
        {
          /* Open manual join window */
          sf_joinManger_openManualWindow();
          GPIO_writeDio(IOID_12,0);
          //advertiseData++; // old code just incrementing. if this number >20, the slave would bypass.

          //we will now change this advertiseData based on the value sent by slave itself.
          //so, if the voltage of battery goes less than 3.0V, we will bypass it.


          //choose_sc++; //just updating the next slave to bypass
          connected_slaves = (int)sf_deviceMgmt_getRegisteredDeviceCount;


          //Get the serial numbers of all the slaves that are connected to BMS.
          for(int i=0;i<SF_CONF_SENSOR_CNT_MAX;i++)
          {
              sc_address_list[i]=(sf_deviceMgmt_getSensorList()+i)->serialNr;
          }


        }
      }
    }
  }

  PROCESS_END();
} /* button_process() */

/*------------------------------------------------------------------------------
  bmscc_app_process()
------------------------------------------------------------------------------*/
PROCESS_THREAD(bmscc_app_process, ev, data)
{
  /* BMSCC short address */
  linkaddr_t bmsccAddr = linkaddr_null;
  /* Network ID */
  uint16_t panId = {0};

  PROCESS_BEGIN();

  LOG_INFO("Starting as bms-cc\n");
  LOG_INFO("Version: 1.0.0\n");
  LOG_INFO("Bitrate: %u kbps\n", SF_RADIO_BIT_RATE);
  LOG_INFO("- 802.15.4 PANID: 0x%04x\n", IEEE802154_PANID);

  /* Get reboot source of last restart. */
    uint32_t resetSource = SysCtrlResetSourceGet();
    if(RSTSRC_WAKEUP_FROM_SHUTDOWN == resetSource)
    {
      LOG_ERR("Reboot shut, button\n");
    }
    else if(RSTSRC_WARMRESET == resetSource)
    {
      LOG_ERR("Reboot watchdog\n");
    }
    else if(RSTSRC_PIN_RESET == resetSource)
    {
      /* Hardware reset by the reset pin. */
      LOG_INFO("Reboot pin reset\n");
    }
    else if(RSTSRC_SYSRESET == resetSource)
    {
      /* Reboot source is a software reset. */
      LOG_INFO("Reboot SW reset\n");
    }
    else if(RSTSRC_WAKEUP_FROM_SHUTDOWN == resetSource)
    {
      /* Reboot from shutdown mode. */
      LOG_INFO("Reboot shutdown\n");
    }
    else
    {
      /* Reboot source is unknown. */
      LOG_ERR("Reboot unk\n");
    }

  /* Select 2.4GHz radio frequency region */
  if(false == sf_rf_selectRegion(E_SF_RF_REGION_GLOB2G4))
  {
    LOG_ERR("!Failed to select GLOB2G4 RF region\n");
  }

  /* Set Tx power to to Max */
  sf_tsch_setTxPowerMax();

  /* Initialize TSCH */
  sf_tsch_init();

  /* Start TSCH as gateway */
  sf_tsch_start();

  /* Start button handle process. */
  process_start(&button_process, NULL);

  /* Read device configuration. */
  if(E_SF_SUCCESS == sf_configMgmt_readConfig())
  {
    LOG_INFO("Valid configurations are found in the Flash\n");
  }
  else
  {
    LOG_INFO("No valid configurations are found in the Flash\n");
    /* If we reach here that means that this is a new born BMSCC.
       Please ensure that you set a unique link address, network ID
       and device serial number.
      As for testing, we will use the default configurations. */
    if(E_SF_SUCCESS != sf_configMgmt_setDefaultConfig())
    {
      LOG_ERR("!Failed to set the default configs\n");
    }
    else
    {
      LOG_INFO("The default configurations are set \n");
    }
  }

  /* Set the configured link address, if valid, into the TSCH stack */
  if(E_SF_SUCCESS != sf_configMgmt_getParam(bmsccAddr.u8, LINKADDR_SIZE,
                                            E_CONFIGMGMT_PARAM_DEVICE_ADDR))
  {
    LOG_ERR("!Failed to get the bmscc short address \n");
  }
  else
  {
    LOG_INFO("Stored short address ");
    LOG_INFO_LLADDR(&bmsccAddr);
    LOG_INFO_("\n");

    /* Set BMS-CC link address into the stack */
    sf_tsch_setDeviceAddress((linkaddr_t*)&bmsccAddr);
  }

  /* Set the configured pan ID, if valid, into the TSCH stack */
  if(E_SF_SUCCESS != sf_configMgmt_getParam(&panId, sizeof(panId),
                                          E_CONFIGMGMT_PARAM_PAN_ID))
  {
    LOG_ERR("!Failed to get the network ID \n");
  }
  else
  {
    LOG_INFO("Stored network ID; 0x%04X \n", panId);

    /* Set network ID into the stack */
    sf_tsch_setPanId(panId);
  }

  /* Set absolute time time-base.
     This is a dummy time base for testing only */
  sf_absoluteTime_setTimeBase(546431);

  if(0 == sf_absoluteTime_getTime())
  {
    LOG_INFO("Waiting until valid absolute time is set...\n");

    PROCESS_YIELD_UNTIL(0 != sf_absoluteTime_getTime());
  }

  if(0 == sf_tsch_getPanId())
  {
    LOG_INFO("Waiting until valid network ID is set...\n");

    PROCESS_YIELD_UNTIL(0 != sf_tsch_getPanId());
  }

  /* Check if a valid sensor list is stored in the flash.
     If so, configure TSCH to open the communication to the
     registered smart cells */
  load_registered_sensors();

  /* Start absolute time incrementing */
  sf_absoluteTime_startTimer();

  /* Start join process */
  sf_joinManger_start();

#ifdef TSCH_TIMING_PIN_0
  gpio_hal_arch_pin_set_output( NULL, TSCH_TIMING_PIN_0 );
#endif

#ifdef TSCH_TIMING_PIN_1
  gpio_hal_arch_pin_set_output( NULL, TSCH_TIMING_PIN_1 );
#endif

#ifdef TSCH_TIMING_PIN_2
  gpio_hal_arch_pin_set_output( NULL, TSCH_TIMING_PIN_2 );
#endif

#ifdef TSCH_TIMING_PIN_3
  gpio_hal_arch_pin_set_output( NULL, TSCH_TIMING_PIN_3 );
#endif

#ifdef TSCH_TIMING_PIN_4
  gpio_hal_arch_pin_set_output( NULL, TSCH_TIMING_PIN_4 );
#endif

  PROCESS_END();
}/* bmscc_app_process() */

/*=============================================================================
                              API IMPLEMENTATION
=============================================================================*/
E_SF_RETURN_t sf_app_txData(uint8_t* pData, uint8_t dataLen, linkaddr_t* pDest)
{
  /* Storage for the frame. */
  uint8_t pFrameBuf[SF_APP_PAYLOAD_LENGTH_MAX] = {0x00};
  /* Frame length */
  uint8_t frameLen = 0;
  /* Return value */
  E_SF_RETURN_t retVal = E_SF_ERROR;

  if(NULL == pData || NULL == pDest)
  {
    return E_SF_ERROR_NPE;
  }

  /* Build the frame */
  sf_frameType_set(pFrameBuf, E_FRAME_TYPE_REMOTE);
  frameLen += SF_FRAME_TYPE_LEN;

  memcpy(pFrameBuf + frameLen, pData, dataLen);
  frameLen += dataLen;

  LOG_INFO("A downlink is transmitted to ; ");
  LOG_INFO_LLADDR(pDest);
  LOG_INFO_("; ");
  LOG_INFO_BYTES(pFrameBuf, frameLen);
  LOG_INFO_("\n");

  /* Schedule frame Tx */
  retVal = sf_tsch_send(pFrameBuf, frameLen, pDest, &gAppCallbackHandlerCtxt);

  return retVal;
}

/*------------------------------------------------------------------------------
  sf_output_callback_handler()
------------------------------------------------------------------------------*/
__attribute__((weak)) void sf_app_output_callback(void *ptr, nullnet_tx_status_t status)
{
  if(NULLNET_TX_OK == status)
  {
    LOG_INFO("Tx successful\n");
  }
  else
  {
    LOG_INFO("Tx failed\n");
  }
} /* sf_output_callback_handler() */

/*------------------------------------------------------------------------------
  sf_app_handleMeasurement()

  //AK Update 05.07.2023

  the variable meas holds the data sent by the slave.
  right now this is only the battery voltage.

  if the meas meets some threshold, we make the variable advertiseData to go high enough
  this will be sent back to the slave using af_app_txdata function.

  As of today, only one slave it is sent to only one slave.

  Based on what we send to the slave, the slave either toggle PWM or stay in previous state.

  Now we do not need button press intervention.

  TBD: change advertiseData to boolean.



------------------------------------------------------------------------------*/
__attribute__((weak)) void sf_app_handleMeasurement(uint8_t* pInBuf, uint8_t length, linkaddr_t* pSrc)
{
  int8_t rssi = 0;
  /* Pointer to sensor element. */
  sf_sensor_t *sensor = NULL;
  /* Storage for the received measurement. */
  meas_t meas = {0};

  /* Get RSSI of received packet */
  rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

  LOG_INFO("Measurement received; device address; ");
  LOG_INFO_LLADDR(pSrc);
  LOG_INFO_(" RSSI; %d;", rssi);
  LOG_INFO_BYTES(pInBuf, length);
  LOG_INFO_("\n");

  /* Measurement frame structure
     frame type  |  measurement data
     ------------|-------------------
        uint8_t  | meas_t              */
  memcpy(&meas, pInBuf + SF_FRAME_TYPE_LEN, sizeof(meas_t));

  slave_data = meas.value;

  // to initiate bypass if the voltage goes low
  if(slave_data < 1100000) //corresponds to 1.1V from adc output, here we need to bypass
      advertiseData = 25;
  else
      advertiseData = 12;


  //send data after processing the measurement?
  uint8_t pData[] = {advertiseData}; //orig 1
  sf_sensor_t *pSensor = sf_deviceMgmt_getDeviceByIndex(choose_sc);
  if(NULL != pSensor)
  {
    if(0 != pSensor->serialNr &&
        !linkaddr_cmp(&pSensor->shortAddress, &linkaddr_null))
    {
      sf_app_txData(pData, sizeof(pData), &pSensor->shortAddress);
    }
  }


  // else we do not do anything, so the slave will be inserted.

  LOG_INFO("Measurement timestamp: %uh : %umin : %usec \n",
            (unsigned int)sf_absoluteTime_getHours(meas.timeStamp),
            (unsigned int)sf_absoluteTime_getMinutes(meas.timeStamp),
            (unsigned int)sf_absoluteTime_getSeconds(meas.timeStamp));
#if !CONTIKI_TARGET_COOJA
  LOG_INFO("Measurement value: %f \n", meas.value);
#else
  LOG_INFO("Measurement value: %d.%u \n", (int)meas.value,
          (uint32_t)(1000*(meas.value-(int)meas.value)));
#endif

  /* Get sensor element corresponding to the source address. */
  sensor = sf_deviceMgmt_getDevice(*pSrc);
  /* Update last seen timestamp for message source device */
  if(NULL == sensor)
  {
    LOG_ERR("!Device is not in sensor list. Reject measurement message.\n");
  }
} /* sf_app_handleMeasurement() */
