/*****************************************************************************
 *
 * MODULE:             JN-AN-1220 ZLO Sensor Demo
 *
 * COMPONENT:          zcl_options.h
 *
 * DESCRIPTION:        ZCL Options Header for ZLO LTO Sensor
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

#ifndef ZCL_OPTIONS_H
#define ZCL_OPTIONS_H

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/*                      ZCL Specific initialization                         */
/****************************************************************************/
/* Disable APS acks */
#define ZCL_DISABLE_APS_ACK                                 1

/* This is the NXP manufacturer code. If creating new a manufacturer        */
/* specific command apply to the Zigbee alliance for an Id for your company */
/* Also update the manufacturer code in .zpscfg: Node Descriptor->misc      */
#define ZCL_MANUFACTURER_CODE                                0x1037

/*  Enabling various server and clients for command which act across entire  */
/*  profile                                                                  */
#define ZCL_ATTRIBUTE_READ_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_READ_CLIENT_SUPPORTED
#define ZCL_ATTRIBUTE_WRITE_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_REPORTING_SERVER_SUPPORTED
#define ZCL_CONFIGURE_ATTRIBUTE_REPORTING_SERVER_SUPPORTED
#define ZCL_READ_ATTRIBUTE_REPORTING_CONFIGURATION_SERVER_SUPPORTED

/* Binding server related options*/
#define CLD_BIND_SERVER
#define MAX_NUM_BIND_QUEUE_BUFFERS                          5
#define MAX_PDU_BIND_QUEUE_PAYLOAD_SIZE                     24

/* Three reportable attribute :                                               */
/*  MeasuredValue (Illuminance Measurement)                                   */
/*  MeasuredValue (Temperature Measurement)                                   */
/*  Occupancy (Occupancy Sensing )                                            */
#define ZCL_NUMBER_OF_REPORTS     3
#define ZLO_MIN_REPORT_INTERVAL   1
#define ZLO_MAX_REPORT_INTERVAL   0x3d

/* Enable wild card profile */
#define ZCL_ALLOW_WILD_CARD_PROFILE
/****************************************************************************/
/*                             Enable Cluster                               */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to enable         */
/* cluster and their client or server instances                             */
/****************************************************************************/
#define CLD_BASIC
#define BASIC_SERVER

#define CLD_SCENES
#define SCENES_CLIENT

#define CLD_IDENTIFY
#define IDENTIFY_CLIENT
#define IDENTIFY_SERVER

#define CLD_GROUPS
#define GROUPS_CLIENT

#define CLD_ILLUMINANCE_MEASUREMENT
#define ILLUMINANCE_MEASUREMENT_SERVER

#define CLD_TEMPERATURE_MEASUREMENT
#define TEMPERATURE_MEASUREMENT_SERVER

#define CLD_OCCUPANCY_SENSING
#define OCCUPANCY_SENSING_SERVER

#define CLD_RELATIVE_HUMIDITY_MEASUREMENT
#define RELATIVE_HUMIDITY_MEASUREMENT_SERVER
/****************************************************************************/
/*             Basic Cluster - Optional Attributes                          */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to add optional   */
/* attributes to the basic cluster.                                         */
/****************************************************************************/
#define   CLD_BAS_ATTR_APPLICATION_VERSION
#define   CLD_BAS_ATTR_STACK_VERSION
#define   CLD_BAS_ATTR_HARDWARE_VERSION
#define   CLD_BAS_ATTR_MANUFACTURER_NAME
#define   CLD_BAS_ATTR_MODEL_IDENTIFIER
#define   CLD_BAS_ATTR_DATE_CODE
#define   CLD_BAS_ATTR_SW_BUILD_ID
#define   CLD_BAS_ATTR_GENERIC_DEVICE_CLASS
#define   CLD_BAS_ATTR_GENERIC_DEVICE_TYPE
#define   CLD_BAS_ATTR_PRODUCT_CODE
#define   CLD_BAS_ATTR_PRODUCT_URL

#define CLD_BAS_APP_VERSION         (1)
#define CLD_BAS_STACK_VERSION       (2)
#define CLD_BAS_HARDWARE_VERSION    (1)
#define CLD_BAS_MANUF_NAME_SIZE     (3)
#define CLD_BAS_MODEL_ID_SIZE       (13)
#define CLD_BAS_DATE_SIZE           (8)
#define CLD_BAS_POWER_SOURCE        E_CLD_BAS_PS_BATTERY
#define CLD_BAS_SW_BUILD_SIZE       (9)
#define CLD_BAS_URL_SIZE            (12)
#define CLD_BAS_PCODE_SIZE          (4)

/****************************************************************************/
/*             Illuminance Measurement Cluster - Optional Attributes        */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to add optional   */
/* attributes to the Illuminance measurement                                */
/****************************************************************************/
#define CLD_ILLMEAS_ATTR_TOLERANCE
#define CLD_ILLMEAS_ATTR_LIGHT_SENSOR_TYPE

/****************************************************************************/
/*             Occupancy Sensing Cluster - Optional Attributes              */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to add optional   */
/* attributes to the Occupancy Sensing                                      */
/****************************************************************************/
#define CLD_OS_ATTR_PIR_OCCUPIED_TO_UNOCCUPIED_DELAY
#ifdef PIR_TYPE_PWM
    #define CLD_OS_ATTR_PIR_UNOCCUPIED_TO_OCCUPIED_DELAY
    #define CLD_OS_ATTR_PIR_UNOCCUPIED_TO_OCCUPIED_THRESHOLD
#endif

/****************************************************************************/
/*             Basic Cluster - Optional Commands                            */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to add optional   */
/* commands to the basic cluster.                                           */
/****************************************************************************/
#define CLD_BAS_CMD_RESET_TO_FACTORY_DEFAULTS

/****************************************************************************/
/*             Identify Cluster - Optional Commands                         */
/*                                                                          */
/* Add the following #define's to your zcl_options.h file to add optional   */
/* commands to the identify cluster.                                        */
/****************************************************************************/
#define   CLD_IDENTIFY_CMD_TRIGGER_EFFECT
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

#endif /* ZCL_OPTIONS_H */
