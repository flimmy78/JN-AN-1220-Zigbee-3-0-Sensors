/*****************************************************************************
 *
 * MODULE:          JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:       app_event_handler.c
 *
 * DESCRIPTION:     ZLO Demo: Handles all the different type of Application events
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5179].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2017. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "dbg.h"
#include "app_events.h"
#include "app_zlo_sensor_node.h"
#include "app_light_sensor_state_machine.h"
#include "ZTimer.h"
#include "app_main.h"
#include "App_FingerPrintLock.h"
#include "app_sleep_handler.h"
#include "app_event_handler.h"
#include "app_reporting.h"
#include "app_blink_led.h"
#include "app_zcl_tick_handler.h"
#include "AppHardwareApi.h"
#include "bdb_api.h"
#include "bdb_fb_api.h"
#include "app_nwk_event_handler.h"
#include "zps_gen.h"
#ifdef APP_NTAG_ICODE
#include "app_ntag_icode.h"
#include "nfc_nwk.h"
#endif
#ifdef APP_NTAG_AES
#include "app_ntag_aes.h"
#include "nfc_nwk.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DEBUG_EVENT_HANDLER
    #define TRACE_EVENT_HANDLER   TRUE
#else
    #define TRACE_EVENT_HANDLER   FALSE
#endif
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vDioEventHandler(te_TransitionCode eTransitionCode);
PRIVATE void vEventStartFindAndBind(void);
PRIVATE void vStartPersistantPolling(void);
PRIVATE void vStopPersistantPolling(void);
PRIVATE void vHandlePeriodicReportEvent(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern const uint8 u8MyEndpoint;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vDioEventHandler
 *
 * DESCRIPTION:
 * Processes the Dio events like binding and occupancy. Any other events that
 * come through we immediately attempt to go to sleep as we have no process for
 * them.
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDioEventHandler(te_TransitionCode eTransitionCode )
{

    DBG_vPrintf(TRACE_EVENT_HANDLER,"\nAPP Process Buttons: In vSensorStateMachine TransitionCode = %02x -> ",eTransitionCode);
    switch(eTransitionCode)
    {

    /* Fall through for the button presses as there will be a delayed action*/
    case COMM_BUTTON_PRESSED:
        vEventStartFindAndBind();
        break;

    case COMM_BUTTON_RELEASED:
        vEventStopFindAndBind();
        break;

#if (defined APP_NTAG_ICODE) || (defined APP_NTAG_AES)
    case FD_PRESSED:
    case FD_RELEASED:
        #if APP_NTAG_ICODE
            APP_vNtagStart(LIGHTSENSOR_SENSOR_ENDPOINT);
        #endif
        #if APP_NTAG_AES
            APP_vNtagStart(NFC_NWK_NSC_DEVICE_CLIMATE_SENSOR_DEVICE);
        #endif
        break;
#endif

    default:
        break;
    }

}

/****************************************************************************
 *
 * NAME: vAppHandleAppEvent
 *
 * DESCRIPTION:
 * interprets the button press and calls the state machine.
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppHandleAppEvent(APP_tsEvent sButton)
{
    ZPS_eAplZdoPoll();
    te_TransitionCode eTransitionCode=NUMBER_OF_TRANSITION_CODE;

    switch(sButton.eType)
    {

    case APP_E_EVENT_BUTTON_DOWN:
        DBG_vPrintf(TRACE_EVENT_HANDLER, "\nAPP Process Buttons: Button Number= %d",sButton.uEvent.sButton.u8Button);
        DBG_vPrintf(TRACE_EVENT_HANDLER, "\nAPP Process Buttons: DIO State    = %08x",sButton.uEvent.sButton.u32DIOState);

        eTransitionCode=sButton.uEvent.sButton.u8Button;

        DBG_vPrintf(TRACE_EVENT_HANDLER, "\nAPP Process Buttons: Transition Code = %d",eTransitionCode);
        vDioEventHandler(eTransitionCode);
        break;

    case APP_E_EVENT_BUTTON_UP:
        DBG_vPrintf(TRACE_EVENT_HANDLER, "\nAPP Process Buttons: Button Number= %d",sButton.uEvent.sButton.u8Button);
        DBG_vPrintf(TRACE_EVENT_HANDLER, "\nAPP Process Buttons: DIO State    = %08x",sButton.uEvent.sButton.u32DIOState);

        eTransitionCode = BUTTON_RELEASED_OFFSET | sButton.uEvent.sButton.u8Button;

        DBG_vPrintf(TRACE_EVENT_HANDLER, "\nAPP Process Buttons: Transition Code = %d",eTransitionCode);
        vDioEventHandler(eTransitionCode);
        break;

    case APP_E_EVENT_PERIODIC_REPORT:
        vHandlePeriodicReportEvent();
        break;

    case APP_E_EVENT_KEEPALIVE_START:
        vStartPersistantPolling();
        break;

    case APP_E_EVENT_KEEPALIVE_STOP:
        vStopPersistantPolling();
        break;

    case APP_E_EVENT_SEND_REPORT:
    case APP_E_EVENT_WAKE_TIMER:
    case APP_E_EVENT_NONE:
        break;

    default :
        break;

    }

}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vEventStartFindAndBind
 *
 * DESCRIPTION:
 * Initiates the find and bind procedure, Starts a poll timer and the blink
 * timer.
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vEventStartFindAndBind(void)
{
    DBG_vPrintf(TRACE_EVENT_HANDLER,"\nAPP Process Buttons: eEZ_FindAndBind");
    sBDB.sAttrib.u16bdbCommissioningGroupID = 0xFFFF;
    vAPP_ZCL_DeviceSpecific_SetIdentifyTime(0xFF);
    BDB_eFbTriggerAsInitiator(u8MyEndpoint);
    vStartPollTimer(POLL_TIME);
    vStartBlinkTimer(APP_FIND_AND_BIND_BLINK_TIME);
}

/****************************************************************************
 *
 * NAME: vEventStopFindAndBind
 *
 * DESCRIPTION:
 * Stops the find and bind procedure and attempts to sleep.
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vEventStopFindAndBind(void)
{
    DBG_vPrintf(TRACE_EVENT_HANDLER,"\nAPP Process Buttons: Exit Easy Mode");
    vAPP_ZCL_DeviceSpecific_IdentifyOff();
    BDB_vFbExitAsInitiator();
    vStopBlinkTimer();
    ZTIMER_eStop(u8TimerPoll);
}

/****************************************************************************
 *
 * NAME: vStartPersistantPolling
 *
 * DESCRIPTION:
 * Starts the Poll timer which will in turn keep the device awake so it can
 * receive data from it's parent.
 *
 ****************************************************************************/
PRIVATE void vStartPersistantPolling(void)
{
    DBG_vPrintf(TRACE_EVENT_HANDLER,"\nAPP Process Buttons: Start PersistantPolling");
    APP_bPersistantPolling |= TRUE;
    ZTIMER_eStart(u8TimerLightSensorSample, POLL_TIME);
    vStartPollTimer(POLL_TIME);
    vStartBlinkTimer(APP_KEEP_AWAKE_TIME);
}

/****************************************************************************
 *
 * NAME: vStopPersistantPolling
 *
 * DESCRIPTION:
 * Stops the poll timer which will allow the device to go back to sleep.
 *
 ****************************************************************************/
PRIVATE void vStopPersistantPolling(void)
{
    DBG_vPrintf(TRACE_EVENT_HANDLER,"\nAPP Process Buttons: Stop PersistantPolling");
    APP_bPersistantPolling &= FALSE;
    ZTIMER_eStop(u8TimerLightSensorSample);
    ZTIMER_eStop(u8TimerPoll);
    vStopBlinkTimer();
}


PRIVATE void vHandlePeriodicReportEvent(void)
{
    vUpdateZCLTickSinceSleep();
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
