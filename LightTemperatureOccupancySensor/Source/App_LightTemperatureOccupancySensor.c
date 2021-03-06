/*****************************************************************************
 *
 * MODULE:             JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:          App_LightSensor.c
 *
 * DESCRIPTION:        ZLO Demo LTO Sensor -Implementation
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
#include "zps_gen.h"
#include "dbg.h"
#include <string.h>
#include "app_led_interface.h"
//#include "TSL2550.h"
#include "app_common.h"
#include "App_LightTemperatureOccupancySensor.h"
#include "app_main.h"
#include "sht.h"
#include "pdum_gen.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
tsZLO_LightSensorDevice sSensor;

const uint8 u8MyEndpoint = LIGHTTEMPERATUREOCCUPANCYSENSOR_SENSOR_ENDPOINT;

#define DEBUG_LIGHT_TEMP_OCC_SENSOR
#ifdef DEBUG_LIGHT_TEMP_OCC_SENSOR
    #define TRACE_LIGHT_TEMP_OCC_SENSOR TRUE
#else
    #define TRACE_LIGHT_TEMP_OCC_SENSOR FALSE
#endif

/****************************************************************************/
/* define the default reports */
tsReports asDefaultReports[ZCL_NUMBER_OF_REPORTS] = \
{\
    {MEASUREMENT_AND_SENSING_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,{0, E_ZCL_UINT16,E_CLD_ILLMEAS_ATTR_ID_MEASURED_VALUE,ZLO_MIN_REPORT_INTERVAL,ZLO_MAX_REPORT_INTERVAL,0,{LIGHT_SENSOR_MINIMUM_REPORTABLE_CHANGE}}},\
    {MEASUREMENT_AND_SENSING_CLUSTER_ID_OCCUPANCY_SENSING,{0, E_ZCL_BMAP8, E_CLD_OS_ATTR_ID_OCCUPANCY, ZLO_MIN_REPORT_INTERVAL, ZLO_MAX_REPORT_INTERVAL, 0, {0}}},\
    {MEASUREMENT_AND_SENSING_CLUSTER_ID_TEMPERATURE_MEASUREMENT,{0, E_ZCL_INT16,E_CLD_TEMPMEAS_ATTR_ID_MEASURED_VALUE,ZLO_MIN_REPORT_INTERVAL,ZLO_MAX_REPORT_INTERVAL,0,{TEMPERATURE_SENSOR_MINIMUM_REPORTABLE_CHANGE}}},\
};

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: eApp_ZCL_RegisterEndpoint
 *
 * DESCRIPTION:
 * Register ZLO endpoints
 *
 * PARAMETER
 * Type                        Name                  Descirption
 * tfpZCL_ZCLCallBackFunction  fptr                  Pointer to ZCL Callback function
 *
 * RETURNS:
 * teZCL_Status
 *
 ****************************************************************************/
teZCL_Status eApp_ZCL_RegisterEndpoint(tfpZCL_ZCLCallBackFunction fptr)
{
    teZCL_Status eZCL_Status;

    // Register the Light Sensor on its own endpoint
    eZCL_Status = eZLO_RegisterLightSensorEndPoint(LIGHTTEMPERATUREOCCUPANCYSENSOR_SENSOR_ENDPOINT,
                                                  fptr,
                                                  &sSensor);

    // Check if the light sensor was successfully registered
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_LIGHT_TEMP_OCC_SENSOR, "Unable to register light sensor : %x", eZCL_Status);
    }

    return eZCL_Status;
}

/****************************************************************************
 *
 * NAME: vAPP_ZCL_DeviceSpecific_Init
 *
 * DESCRIPTION:
 * ZCL Device Specific initialization
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
void vAPP_ZCL_DeviceSpecific_Init(void)
{
    /* -- Initialise Illuminance Endpoint cluster attributes -- */
    // Initialise the strings in Basic
    memcpy(sSensor.sBasicServerCluster.au8ManufacturerName, "NXP", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sSensor.sBasicServerCluster.au8ModelIdentifier, "ZLO-LTOSensor", CLD_BAS_MODEL_ID_SIZE);
    memcpy(sSensor.sBasicServerCluster.au8DateCode, "20160210", CLD_BAS_DATE_SIZE);
    memcpy(sSensor.sBasicServerCluster.au8SWBuildID, "4000-0001", CLD_BAS_SW_BUILD_SIZE);
    memcpy(sSensor.sBasicServerCluster.au8ProductURL, "www.nxp.com", CLD_BAS_URL_SIZE);
    memcpy(sSensor.sBasicServerCluster.au8ProductCode, "1234", CLD_BAS_PCODE_SIZE);
    sSensor.sBasicServerCluster.eGenericDeviceType = E_CLD_BAS_GENERIC_DEVICE_TYPE_MOTION_OR_LIGHT_SENSOR;

    /* Initialise the attribute in illuminance Measurement */
    sSensor.sIlluminanceMeasurementServerCluster.u16MeasuredValue = 0;
    sSensor.sIlluminanceMeasurementServerCluster.eLightSensorType = E_CLD_ILLMEAS_LST_CMOS;
    sSensor.sIlluminanceMeasurementServerCluster.u16MinMeasuredValue = LIGHT_SENSOR_MINIMUM_MEASURED_VALUE;
    sSensor.sIlluminanceMeasurementServerCluster.u16MaxMeasuredValue = LIGHT_SENSOR_MAXIMUM_MEASURED_VALUE;

    /* Initialise the attribute in Occupancy Sensing */
    // Initialise the strings in Basic
    sSensor.sOccupancySensingServerCluster.eOccupancySensorType = E_CLD_OS_SENSORT_TYPE_PIR;
    sSensor.sOccupancySensingServerCluster.u8Occupancy = 0;

    #ifdef PIR_TYPE_PWM
        sSensor.sOccupancySensingServerCluster.u8PIRUnoccupiedToOccupiedThreshold = APP_OCCUPANCY_SENSOR_TRIGGER_THRESHOLD;
        sSensor.sOccupancySensingServerCluster.u8PIRUnoccupiedToOccupiedDelay = APP_OCCUPANCY_SENSOR_UNOCCUPIED_TO_OCCUPIED_DELAY;
    #endif
    sSensor.sOccupancySensingServerCluster.u16PIROccupiedToUnoccupiedDelay = APP_OCCUPANCY_SENSOR_OCCUPIED_TO_UNOCCUPIED_DELAY;

    /* Initialise the attribute in Temperature Measurement */
    sSensor.sTemperatureMeasurementServerCluster.i16MeasuredValue = 0;
    sSensor.sTemperatureMeasurementServerCluster.i16MinMeasuredValue = TEMPERATURE_SENSOR_MINIMUM_MEASURED_VALUE;
    sSensor.sTemperatureMeasurementServerCluster.i16MaxMeasuredValue = TEMPERATURE_SENSOR_MAXIMUM_MEASURED_VALUE;
}

/****************************************************************************
 *
 * NAME: vAPP_ZCL_DeviceSpecific_SetIdentifyTime
 *
 * DESCRIPTION:
 * ZCL Device Specific setting of identify time
 *
 * PARAMETER:
 * uint16 u16Time Identify time duration
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vAPP_ZCL_DeviceSpecific_SetIdentifyTime(uint16 u16Time)
{
    sSensor.sIdentifyServerCluster.u16IdentifyTime=u16Time;
}
/****************************************************************************
 *
 * NAME: vAPP_ZCL_DeviceSpecific_UpdateIdentify
 *
 * DESCRIPTION:
 * ZCL Device Specific identify updates
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vAPP_ZCL_DeviceSpecific_UpdateIdentify(void)
{
    if(sSensor.sIdentifyServerCluster.u16IdentifyTime%2)
    {
    	APP_vSetLed(TRUE);
//        bRGB_LED_SetGroupLevel(LED_LEVEL);
//        bRGB_LED_On();
    }
    else
    {
    	APP_vSetLed(FALSE);
//        bRGB_LED_Off();
    }
}
/****************************************************************************
 *
 * NAME: vAPP_ZCL_DeviceSpecific_IdentifyOff
 *
 * DESCRIPTION:
 * ZCL Device Specific stop identify
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vAPP_ZCL_DeviceSpecific_IdentifyOff(void)
{
    vAPP_ZCL_DeviceSpecific_SetIdentifyTime(0);
    APP_vSetLed(FALSE);
//    bRGB_LED_Off();
}

/****************************************************************************
 *
 * NAME: APP_cbTimerLightTempSensorSample
 *
 * DESCRIPTION:
 * Callback for timer of Light and Temp sensor sampling
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void APP_cbTimerLightTempSensorSample(void *pvParam)
{
    if(APP_bPersistantPolling == TRUE)
    {
        vAPP_TemperatureSensorSample();
        /* Start sample timer so that you keep on sampling if KEEPALIVE_TIME is too high*/
        ZTIMER_eStart(u8TimerLightTempSensorSample, ZTIMER_TIME_MSEC(1000 * LIGHT_SENSOR_SAMPLING_TIME_IN_SECONDS));
    }
}

/****************************************************************************
 *
 * NAME: vAPP_TemperatureSensorSample
 *
 * DESCRIPTION:
 * Temperature sensor sampling
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vAPP_TemperatureSensorSample(void)
{
	int32_t temperature, humidity;
	/* Measure temperature and relative humidity and store into variables
	 * temperature, humidity (each output multiplied by 1000).
	 */
	int8_t ret = sht_measure_blocking_read(&temperature, &humidity);
	if (ret == STATUS_OK) {
    	DBG_vPrintf(TRACE_LIGHT_TEMP_OCC_SENSOR, "measured temperature: %d degreeCelsius, "
				  "measured humidity: %d percentRH\n",
				  temperature,
				  humidity);
	} else {
		DBG_vPrintf(TRACE_LIGHT_TEMP_OCC_SENSOR, "error reading measurement\n");
	}

    if(temperature >= (TEMPERATURE_SENSOR_MINIMUM_MEASURED_VALUE))
        sSensor.sTemperatureMeasurementServerCluster.i16MeasuredValue = temperature/10;
    else
        sSensor.sTemperatureMeasurementServerCluster.i16MeasuredValue = TEMPERATURE_SENSOR_MINIMUM_MEASURED_VALUE;

    if(humidity > (TEMPERATURE_SENSOR_MINIMUM_MEASURED_VALUE))
        sSensor.sIlluminanceMeasurementServerCluster.u16MeasuredValue = humidity/10;//
    else
        sSensor.sIlluminanceMeasurementServerCluster.u16MeasuredValue =TEMPERATURE_SENSOR_MINIMUM_MEASURED_VALUE;
}

/****************************************************************************
 *
 * NAME: app_u8GetDeviceEndpoint
 *
 * DESCRIPTION:
 * Returns the application endpoint
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC uint8 app_u8GetDeviceEndpoint( void)
{
    return LIGHTTEMPERATUREOCCUPANCYSENSOR_SENSOR_ENDPOINT;
}

/****************************************************************************
 *
 * NAME: APP_cbTimerReporting
 *
 * DESCRIPTION:
 * Timer callback to attribute reporting
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void APP_TemperatureReporting(void)
{
    teZCL_Status zcl_status;
    tsZCL_Address sAddress;
    PDUM_thAPduInstance    hAPduInst;

	//reporting
	hAPduInst =  PDUM_hAPduAllocateAPduInstance ( apduZCL );
	if ( PDUM_INVALID_HANDLE != hAPduInst )
	{
		sAddress.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;
		sAddress.uAddress.u16DestinationAddress = 0;//coordinator
		zcl_status = eZCL_ReportAttribute(&sAddress, 0x0402, 0, LIGHTTEMPERATUREOCCUPANCYSENSOR_SENSOR_ENDPOINT, 1, hAPduInst);
		DBG_vPrintf(TRACE_LIGHT_TEMP_OCC_SENSOR, "REPORT: temp %d\n", zcl_status);
//		zcl_status = eZCL_ReportAttribute(&sAddress, 0x0400, 0, LIGHTTEMPERATUREOCCUPANCYSENSOR_SENSOR_ENDPOINT, 1, hAPduInst);
//		DBG_vPrintf(TRACE_LIGHT_TEMP_OCC_SENSOR, "REPORT: humidity %d\n", zcl_status);
	}

}
/****************************************************************************
 *
 * NAME: APP_cbTimerReporting
 *
 * DESCRIPTION:
 * Timer callback to attribute reporting
 *
 * PARAMETER:
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void APP_HumidityReporting(void)
{
    teZCL_Status zcl_status;
    tsZCL_Address sAddress;
    PDUM_thAPduInstance    hAPduInst;

	//reporting
	hAPduInst =  PDUM_hAPduAllocateAPduInstance ( apduZCL );
	if ( PDUM_INVALID_HANDLE != hAPduInst )
	{
		sAddress.eAddressMode = E_ZCL_AM_SHORT_NO_ACK;
		sAddress.uAddress.u16DestinationAddress = 0;//coordinator
		zcl_status = eZCL_ReportAttribute(&sAddress, 0x0400, 0, LIGHTTEMPERATUREOCCUPANCYSENSOR_SENSOR_ENDPOINT, 1, hAPduInst);
		DBG_vPrintf(TRACE_LIGHT_TEMP_OCC_SENSOR, "REPORT: humidity %d\n", zcl_status);
	}

}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
