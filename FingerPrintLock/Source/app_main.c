/*****************************************************************************
 *
 * MODULE:             JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:          app_main.c
 *
 * DESCRIPTION:        ZLO Main handler (Implementation)
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

#include <stdint.h>
#include "jendefs.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "portmacro.h"
#include "zps_apl_af.h"
#include "mac_vs_sap.h"
#include "AppHardwareApi.h"
#include "dbg.h"
#include "app_main.h"
#include "app_light_sensor_buttons.h"
#include "app_events.h"
#include "app_zcl_sensor_task.h"
#include "app_light_sensor_state_machine.h"
#include "PDM.h"
#include "app_zlo_sensor_node.h"
#include "app_nwk_event_handler.h"
#include "app_power_on_counter.h"
#include "app_blink_led.h"
#include "App_FingerPrintLock.h"
#include "app_sleep_handler.h"
#ifdef APP_NTAG_ICODE
#include "app_ntag_icode.h"
#endif
#ifdef APP_NTAG_AES
#include "app_ntag_aes.h"
#endif
#include "ZQueue.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_APP
#define TRACE_APP   FALSE
#else
#define TRACE_APP   TRUE
#endif

#define TIMER_QUEUE_SIZE             8
#define MLME_QUEQUE_SIZE             8
#define MCPS_QUEUE_SIZE             24
#define ZPS_QUEUE_SIZE               1
#define ZCL_QUEUE_SIZE               1
#define APP_QUEUE_SIZE               8
#define BDB_QUEUE_SIZE               3
#if (defined APP_NTAG_ICODE) || (defined APP_NTAG_AES)
#define APP_ZTIMER_STORAGE           7 /* NTAG: Added timer */
#else
#define APP_ZTIMER_STORAGE           6
#endif

#if JENNIC_CHIP_FAMILY == JN517x
#define NVIC_INT_PRIO_LEVEL_SYSCTRL (1)
#define NVIC_INT_PRIO_LEVEL_BBC     (7)
#endif

#define TX_QUEUE_SIZE                                              150
#define RX_QUEUE_SIZE                                              150
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
PUBLIC uint8 u8TimerButtonScan;
PUBLIC uint8 u8TimerPoll;
PUBLIC uint8 u8TimerPowerOnCount;
PUBLIC uint8 u8TimerLightSensorSample;
PUBLIC uint8 u8TimerTick;
PUBLIC uint8 u8TimerBlink;
#if (defined APP_NTAG_ICODE) || (defined APP_NTAG_AES)
PUBLIC uint8 u8TimerNtag;
#endif
PUBLIC tszQueue APP_msgZpsEvents;
PUBLIC tszQueue APP_msgZclEvents;
PUBLIC tszQueue APP_msgAppEvents;
PUBLIC tszQueue APP_msgBdbEvents;

PUBLIC bool_t APP_bPersistantPolling = FALSE;

PUBLIC tszQueue           APP_msgSerialRx;
PUBLIC tszQueue           APP_msgSerialTx;
uint8                     au8AtRxBuffer [ RX_QUEUE_SIZE ];
uint8                     au8AtTxBuffer [ TX_QUEUE_SIZE ];
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + BDB_ZTIMER_STORAGE];

PRIVATE zps_tsTimeEvent asTimeEvent[TIMER_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMlmeVsDcfmInd asMacMlmeVsDcfmInd[MLME_QUEQUE_SIZE];
PRIVATE ZPS_tsAfEvent asZpsStackEvent[ZPS_QUEUE_SIZE];
PRIVATE ZPS_tsAfEvent asZclStackEvent[ZCL_QUEUE_SIZE];

PRIVATE APP_tsEvent asAppEvent[APP_QUEUE_SIZE];
PRIVATE BDB_tsZpsAfEvent asBdbEvent[BDB_QUEUE_SIZE];
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


extern void zps_taskZPS(void);
extern void PWRM_vManagePower(void);

/*start of file*/
/****************************************************************************
 *
 * NAME: APP_vMainLoop
 *
 * DESCRIPTION:
 * Main  execution loop
 *
 * RETURNS:
 * Never
 *
 ****************************************************************************/
PUBLIC void APP_vMainLoop(void)
{

    /* idle task commences on exit from OS start call */
    while (TRUE)
    {
        DBG_vPrintf(TRACE_APP, "ZPS\n");
        zps_taskZPS();

        DBG_vPrintf(TRACE_APP, "APP: Entering bdb_taskBDB\n");
        bdb_taskBDB();

        DBG_vPrintf(TRACE_APP, "TMR\n");
        ZTIMER_vTask();

        DBG_vPrintf(TRACE_APP, "ZLO\n");
        APP_taskSensor();

        /* Re-load the watch-dog timer. Execution must return through the idle
         * task before the CPU is suspended by the power manager. This ensures
         * that at least one task / ISR has executed with in the watchdog period
         * otherwise the system will be reset.
         */
        vAHI_WatchdogRestart();

        /* See if we are able to sleep or not */
        vAttemptToSleep();

        /*
         * suspends CPU operation when the system is idle or puts the device to
         * sleep if there are no activities in progress
         */
        PWRM_vManagePower();
    }
}

/****************************************************************************
 *
 * NAME: APP_vSetUpHardware
 *
 * DESCRIPTION:
 * Set up interrupts
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vSetUpHardware(void)
{
#if (JENNIC_CHIP_FAMILY == JN517x)
vAHI_SysCtrlRegisterCallback ( vISR_SystemController );
    u32AHI_Init();
    vAHI_InterruptSetPriority ( MICRO_ISR_MASK_BBC,        NVIC_INT_PRIO_LEVEL_BBC );
    vAHI_InterruptSetPriority ( MICRO_ISR_MASK_SYSCTRL, NVIC_INT_PRIO_LEVEL_SYSCTRL );
#endif

#if (JENNIC_CHIP_FAMILY == JN516x)
    TARGET_INITIALISE();
    /* clear interrupt priority level  */
    SET_IPL(0);
    portENABLE_INTERRUPTS();
#endif

}

/****************************************************************************
 *
 * NAME: APP_vInitResources
 *
 * DESCRIPTION:
 * Initialise resources (timers, queue's etc)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitResources(void)
{

    /* Initialise the Z timer module */
    ZTIMER_eInit(asTimers, sizeof(asTimers) / sizeof(ZTIMER_tsTimer));

    /* Create Z timers */
    ZTIMER_eOpen(&u8TimerPoll,              APP_cbTimerPoll,                NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerButtonScan,        APP_cbTimerButtonScan,          NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerLightSensorSample, APP_cbTimerLightSensorSample,   NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerTick,              APP_cbTimerZclTick,             NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerPowerOnCount,      vAPP_cbTimerPowerOnCount,       NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerBlink,             vAPP_cbBlinkLED,                NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
#if (defined APP_NTAG_ICODE) || (defined APP_NTAG_AES)
    ZTIMER_eOpen(&u8TimerNtag,              APP_cbNtagTimer,                NULL,   ZTIMER_FLAG_PREVENT_SLEEP);
#endif

    /* create all the queues*/
    ZQ_vQueueCreate(&APP_msgBdbEvents,      BDB_QUEUE_SIZE,     sizeof(BDB_tsZpsAfEvent),   (uint8*)asBdbEvent);
    ZQ_vQueueCreate(&APP_msgZpsEvents,      ZPS_QUEUE_SIZE,     sizeof(ZPS_tsAfEvent),      (uint8*)asZpsStackEvent);
    ZQ_vQueueCreate(&APP_msgZclEvents,      ZCL_QUEUE_SIZE,     sizeof(ZPS_tsAfEvent),      (uint8*)asZclStackEvent);
    ZQ_vQueueCreate(&APP_msgAppEvents,      APP_QUEUE_SIZE,     sizeof(APP_tsEvent),        (uint8*)asAppEvent);
    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd,    MLME_QUEQUE_SIZE,   sizeof(MAC_tsMlmeVsDcfmInd),(uint8*)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd,    MCPS_QUEUE_SIZE,    sizeof(MAC_tsMcpsVsDcfmInd),(uint8*)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_TimeEvents,        TIMER_QUEUE_SIZE,   sizeof(zps_tsTimeEvent),    (uint8*)asTimeEvent);
//    ZQ_vQueueCreate ( &APP_msgSerialTx,       TX_QUEUE_SIZE,          sizeof ( uint8 ),                  (uint8*)au8AtTxBuffer );
//    ZQ_vQueueCreate ( &APP_msgSerialRx,       RX_QUEUE_SIZE,          sizeof ( uint8 ),                  (uint8*)au8AtRxBuffer );

}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
