#pragma once

//
// Function prototypes
//

NTSTATUS
BtnGetDeviceAttributes(
    IN WDFREQUEST Request
    );

NTSTATUS 
BtnGetFeatureReport(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    );

NTSTATUS
BtnGetHidDescriptor(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    );

NTSTATUS
BtnGetReportDescriptor(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    );

NTSTATUS 
BtnGetString(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    );

NTSTATUS
BtnProcessIdleRequest(
    IN  WDFDEVICE Device,
    IN  WDFREQUEST Request,
    OUT BOOLEAN *Pending
    );

NTSTATUS 
BtnSetFeatureReport(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request
    );
    
NTSTATUS 
BtnReadReport(
    IN  WDFDEVICE Device,
    IN  WDFREQUEST Request,
    OUT BOOLEAN *Pending
    );

//
// HID collections
// 
#include "HidCommon.h"

#define LUMIA_GPIO_BUTTONS_DESC \
	        USAGE_PAGE, 0x01,                           /*Generic Desktop*/ \
            USAGE, 0x06,                                /*Keyboard*/        \
            BEGIN_COLLECTION, 0x01,                     /*Application*/     \
                REPORT_ID, REPORTID_CAPKEY_KEYBOARD, \
                USAGE_PAGE, 0x07,                       /*Keyboard*/        \
                USAGE, 0xe3,                            /*Keyboard Left GUI - Start/Home */ \
                LOGICAL_MINIMUM, 0x00, \
                LOGICAL_MAXIMUM, 0x01, \
                REPORT_SIZE, 0x01, \
                REPORT_COUNT, 0x01, \
                INPUT, 0x02,                            /*(Data,Var,Abs)*/ \
                REPORT_COUNT, 0x01, \
                REPORT_SIZE, 0x07,                      /*(Cnst,Var,Abs)*/ \
                INPUT, 0x03, \
            END_COLLECTION, \
            USAGE_PAGE, 0x0C,                           /*Consumer*/         \
            USAGE, 0x01,                                /*Consumer Control*/ \
            BEGIN_COLLECTION, 0x01,                     /*Application*/      \
                REPORT_ID, REPORTID_CAPKEY_CONSUMER, \
                USAGE, 0xE9, /*Volume Up*/ \
                USAGE, 0xEA, /*Volume Down*/ \
                LOGICAL_MINIMUM, 0x00, \
                LOGICAL_MAXIMUM, 0x01, \
                REPORT_SIZE, 0x01, \
                REPORT_COUNT, 0x02, \
                INPUT, 0x02, /*(Data,Var,Abs)*/ \
                REPORT_COUNT, 0x01, \
                REPORT_SIZE, 0x06, \
                INPUT, 0x03, /*(Cnst,Var,Abs)*/ \
            END_COLLECTION, \
            USAGE_PAGE, 0x01,                           /*Generic Desktop*/ \
            USAGE, 0x80,                                /*System Control*/  \
            BEGIN_COLLECTION, 0x01,                     /*Application*/     \
                REPORT_ID, REPORTID_CAPKEY_CONTROL, \
                USAGE, 0x81, /*System power down*/ \
                USAGE, 0xCA,  /*System Display Rotation Lock Slider Switch*/ \
                LOGICAL_MINIMUM, 0x00, \
                LOGICAL_MAXIMUM, 0x01, \
                REPORT_SIZE, 0x01, \
                REPORT_COUNT, 0x02, \
                INPUT, 0x02, /*(Data,Var,Abs)*/ \
                REPORT_COUNT, 0x01, \
                REPORT_SIZE, 0x06, \
                INPUT, 0x03, /*(Cnst,Var,Abs)*/ \
            END_COLLECTION