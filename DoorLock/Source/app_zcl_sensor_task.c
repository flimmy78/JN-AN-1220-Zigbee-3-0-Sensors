/*****************************************************************************
 *
 * MODULE:             JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:          app_zcl_sensor_task.c
 *
 * DESCRIPTION:        ZLO LTO sensor Control Behavior (Implementation)
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
 * Copyright NXP B.V. 2016. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <AppApi.h>
#include "ZTimer.h"
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "PDM.h"
#include "dbg.h"
#include "pwrm.h"

#include "zps_apl_af.h"
#include "zps_apl_zdo.h"
#include "zps_apl_aib.h"
#include "zps_apl_zdp.h"
#include "rnd_pub.h"
#include "mac_pib.h"


#include "zcl_options.h"
#include "zcl.h"

#include "app_common.h"
#include "app_zlo_sensor_node.h"
#include "AHI_AES.h"
#include "app_events.h"
#include "LightingBoard.h"
#include "app_main.h"
#include "app_zcl_tick_handler.h"
#include "app_zcl_sensor_task.h"
#include "App_DoorLock.h"
#include "app_reporting.h"
#include "app_sensor_buttons.h"
#include "app_blink_led.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DEBUG_ZCL
#ifdef DEBUG_ZCL
    #define TRACE_ZCL   TRUE
#else
    #define TRACE_ZCL   FALSE
#endif

#ifdef DEBUG_SENSOR_TASK
    #define TRACE_SENSOR_TASK   TRUE
#else
    #define TRACE_SENSOR_TASK   FALSE
#endif



/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent);
PRIVATE void APP_ZCL_cbEndpointCallback(tsZCL_CallBackEvent *psEvent);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern PDM_tsRecordDescriptor sDevicePDDesc;
extern tsDeviceDesc sDeviceDesc;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_ZCL_vInitialise
 *
 * DESCRIPTION:
 * Initialises ZCL related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_ZCL_vInitialise(void)
{
    teZCL_Status eZCL_Status;

    /* Initialise ZCL */
    eZCL_Status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: Error eZCL_Initialise returned %d", eZCL_Status);
    }

    /* Register ZLO EndPoint */
    eZCL_Status = eApp_ZCL_RegisterEndpoint(&APP_ZCL_cbEndpointCallback);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_SENSOR_TASK, "\nAPP_ZCL: Error: eApp_ZCL_RegisterEndpoint:%d", eZCL_Status);
    }

    DBG_vPrintf(TRACE_SENSOR_TASK, "\nAPP_ZCL: Chan Mask %08x", ZPS_psAplAibGetAib()->apsChannelMask);
    DBG_vPrintf(TRACE_SENSOR_TASK, "\nAPP_ZCL: RxIdle TRUE");

    ZTIMER_eStart( u8TimerTick, ZCL_TICK_TIME);

    vAPP_ZCL_DeviceSpecific_Init();
}

/****************************************************************************
 *
 * NAME: ZCL_Task
 *
 * DESCRIPTION:
 * ZCL Task for the sensor
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_cbTimerZclTick( void *pvParam)
{
    vAPP_ZCL_Tick();
    ZTIMER_eStart( u8TimerTick, ZCL_TICK_TIME);

}

/****************************************************************************
 *
 * NAME: APP_ZCL_vEventHandler
 *
 * DESCRIPTION:
 * Sends the stack event to the ZCL event handler.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_ZCL_vEventHandler(ZPS_tsAfEvent* psStackEvent)
{
    tsZCL_CallBackEvent sCallBackEvent;

    sCallBackEvent.pZPSevent = psStackEvent;
    sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
    vZCL_EventHandler(&sCallBackEvent);
}

PUBLIC void vAPP_ZCL_Tick(void)
{
    ZPS_tsAfEvent sStackEvent;
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = &sStackEvent;

    vDecrementTickCount();
    sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
    vZCL_EventHandler(&sCallBackEvent);
}
/****************************************************************************
 *
 * NAME: APP_ZCL_cbGeneralCallback
 *
 * DESCRIPTION:
 * General callback for ZCL events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent)
{

    switch (psEvent->eEventType)
    {

    case E_ZCL_CBET_UNHANDLED_EVENT:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EVT Unhandled Event");
        break;

    case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EVT Read attributes response");
        break;

    case E_ZCL_CBET_READ_REQUEST:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EVT Read request");
        break;

    case E_ZCL_CBET_DEFAULT_RESPONSE:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EVT Default response");
        break;

    case E_ZCL_CBET_ERROR:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EVT Error");
        break;

    case E_ZCL_CBET_TIMER:
        break;

    case E_ZCL_CBET_ZIGBEE_EVENT:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EVT ZigBee");
        break;

    case E_ZCL_CBET_CLUSTER_CUSTOM:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EP EVT Custom");
        break;

    default:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: Invalid event type");
        break;

    }
}

/****************************************************************************
 *
 * NAME: APP_ZCL_cbEndpointCallback
 *
 * DESCRIPTION:
 * Endpoint specific callback for ZCL events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_ZCL_cbEndpointCallback(tsZCL_CallBackEvent *psEvent)
{

    switch (psEvent->eEventType)
    {

    case E_ZCL_CBET_REPORT_INDIVIDUAL_ATTRIBUTES_CONFIGURE:
        {
            tsZCL_AttributeReportingConfigurationRecord    *psAttributeReportingRecord= &psEvent->uMessage.sAttributeReportingConfigurationRecord;
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: Individual Configure attribute for Cluster = %d",psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: eAttributeDataType = %d",psAttributeReportingRecord->eAttributeDataType);
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: u16AttributeEnum = %d",psAttributeReportingRecord->u16AttributeEnum );
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: u16MaximumReportingInterval = %d",psAttributeReportingRecord->u16MaximumReportingInterval );
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: u16MinimumReportingInterval = %d",psAttributeReportingRecord->u16MinimumReportingInterval );
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: u16TimeoutPeriodField = %d",psAttributeReportingRecord->u16TimeoutPeriodField );
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: u8DirectionIsReceived = %d",psAttributeReportingRecord->u8DirectionIsReceived );
            DBG_vPrintf(TRACE_ZCL,"\nAPP_ZCL: uAttributeReportableChange = %d",psAttributeReportingRecord->uAttributeReportableChange );
            if (E_ZCL_SUCCESS == psEvent->eZCL_Status)
            {
                if(CLOSURE_CLUSTER_ID_DOOR_LOCK == psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum)
                {
                    vSaveReportableRecord(CLOSURE_CLUSTER_ID_DOOR_LOCK,psAttributeReportingRecord);
                }
            }
        }
        break;

    case E_ZCL_CBET_UNHANDLED_EVENT:
    case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
    case E_ZCL_CBET_READ_REQUEST:
    case E_ZCL_CBET_DEFAULT_RESPONSE:
    case E_ZCL_CBET_ERROR:
    case E_ZCL_CBET_TIMER:
    case E_ZCL_CBET_ZIGBEE_EVENT:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EP EVT No action");
        break;

    case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
        DBG_vPrintf(TRACE_SENSOR_TASK, "\nAPP_ZCL: Read Attrib Rsp %d %02x", psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
            *((uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
        break;

    case E_ZCL_CBET_CLUSTER_CUSTOM:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EP EVT: Custom %04x", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);

        switch (psEvent->uMessage.sClusterCustomMessage.u16ClusterId)
        {
        case CLOSURE_CLUSTER_ID_DOOR_LOCK:
        {
        	tsCLD_DoorLockCallBackMessage *psCallBackMessage = (tsCLD_DoorLockCallBackMessage*)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
        	tsMS_ManuSpecCommand *psManuSpecCommandPayload = psCallBackMessage->uMessage.psManuSpecCommandPayload;

        	DBG_vPrintf(TRACE_ZCL, "Door lock:cmd id %x, sub cmd id %x\n",
        			psCallBackMessage->u8CommandId,
        			psManuSpecCommandPayload->u8ManuSpecCommandId);

        	DBG_vPrintf(TRACE_ZCL, "Door lock:%x, %x, %x, %x\n",
        			psManuSpecCommandPayload->u8Payload[0],
        			psManuSpecCommandPayload->u8Payload[1],
        			psManuSpecCommandPayload->u8Payload[2],
        			psManuSpecCommandPayload->u8Payload[3]);

        	//uart1 send
        }
        break;

        case GENERAL_CLUSTER_ID_BASIC:
        {
            tsCLD_BasicCallBackMessage *psCallBackMessage = (tsCLD_BasicCallBackMessage*)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
            if (psCallBackMessage->u8CommandId == E_CLD_BASIC_CMD_RESET_TO_FACTORY_DEFAULTS )
            {
                DBG_vPrintf(TRACE_ZCL, "Basic Factory Reset Received\n");
                /* resetting the sensor structure back to zero*/
                memset(&sDoorLock,0,sizeof(tsHA_DoorLockDevice));
                vAPP_ZCL_DeviceSpecific_Init();
                eApp_ZCL_RegisterEndpoint(&APP_ZCL_cbEndpointCallback);
            }
        }
        break;

        case GENERAL_CLUSTER_ID_IDENTIFY:
            {
                DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: for identify cluster");
                tsCLD_IdentifyCallBackMessage *psCallBackMessage = (tsCLD_IdentifyCallBackMessage*)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
                if (psCallBackMessage->u8CommandId == E_CLD_IDENTIFY_CMD_IDENTIFY)
                {
                    DBG_vPrintf(TRACE_ZCL, "\nEP E_CLD_IDENTIFY_CMD_IDENTIFY");
                    /* provide callback to BDB handler for identify query response on initiator*/
                    if(psEvent->psClusterInstance->bIsServer == FALSE)
                    {
                        tsBDB_ZCLEvent  sBDBZCLEvent;
                        DBG_vPrintf(TRACE_ZCL, "\nCallBackBDB");
                        sBDBZCLEvent.eType = BDB_E_ZCL_EVENT_IDENTIFY_QUERY;
                        sBDBZCLEvent.psCallBackEvent = psEvent;
                        BDB_vZclEventHandler(&sBDBZCLEvent);
                    }
                    else
                    {
                        if(sDoorLock.sIdentifyServerCluster.u16IdentifyTime == 0)
                        {
                            vStopBlinkTimer();
                        }
                    }
                }
                else if((psCallBackMessage->u8CommandId == E_CLD_IDENTIFY_CMD_TRIGGER_EFFECT) &&
                        psEvent->psClusterInstance->bIsServer)
                {
                    DBG_vPrintf(TRACE_ZCL, "Trigger Effect ID %d Vr %d\r\n",
                            psCallBackMessage->uMessage.psTriggerEffectRequestPayload->eEffectId,
                            psCallBackMessage->uMessage.psTriggerEffectRequestPayload->u8EffectVarient);
                }
            }
            break;

        case GENERAL_CLUSTER_ID_GROUPS:
            DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: for groups cluster");
            break;

        case 0x1000:
            DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: - for 0x1000");
            break;

        default:
            DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: for unknown cluster %d", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);
            break;
        }
        break;

    case E_ZCL_CBET_CLUSTER_UPDATE:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: Update Id %04x", psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
        break;

    default:
        DBG_vPrintf(TRACE_ZCL, "\nAPP_ZCL: EP EVT Invalid event type");
        break;
    }

}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
