// Copyright (c) Microsoft Corporation. All Rights Reserved. 
// Copyright (c) Bingxing Wang. All Rights Reserved. 

#pragma once

#include <wdm.h>
#include <wdf.h>
#include <hidport.h>
#define RESHUB_USE_HELPER_ROUTINES
#include <reshub.h>
#include "trace.h"

//
// HID descriptor & reporting
//

#define REPORTID_CAPKEY_KEYBOARD        4
#define REPORTID_CAPKEY_CONSUMER        5
#define REPORTID_CAPKEY_CONTROL         6

typedef enum _BUTTON_STATE
{
    ButtonStateUnpressed = 0,
    ButtonStatePressed = 1
} BUTTON_STATE;

typedef enum _BUTTON_TYPE
{
    Power = 0,
    VolumeUp,
    VolumeDown,
    CameraFocus,
    Camera,
    Slider
} BUTTON_TYPE;

typedef struct _BTN_REPORT {
    UCHAR       ReportID;
    union
    {
        struct
        {
            BYTE Del : 1;
            BYTE F14 : 1;
            BYTE F15 : 1;
            BYTE LeftCtrl : 1;
            BYTE LeftAlt : 1;
            BYTE LeftWin : 1;
            BYTE Reserved : 1;
        } Keyboard;
        struct
        {
            BYTE VolumeUp : 1;
            BYTE VolumeDown : 1;
            BYTE Reserved : 6;
        } Consumer;
        struct
        {
            BYTE SystemPower : 1;
            BYTE Reserved : 7;
        } Control;
        BYTE Raw;
    } KeysData;
} BTN_REPORT, * PBTN_REPORT;

//
// Device context
//

typedef struct _DEVICE_EXTENSION
{
    //
    // Device related
    //
    WDFDEVICE FxDevice;
    WDFQUEUE DefaultQueue;
    WDFQUEUE PingPongQueue;

    //
    // Interrupt servicing
    //
    WDFINTERRUPT InterruptPower;
    WDFINTERRUPT InterruptVolumeUp;
    WDFINTERRUPT InterruptVolumeDown;
    WDFINTERRUPT InterruptCameraFocus;
    WDFINTERRUPT InterruptCamera;
    WDFINTERRUPT InterruptSlider;
    BOOLEAN ServiceInterruptsAfterD0Entry;
    BOOLEAN ProcessInterrupts;
    
    // 
    // Power related
    //
    WDFQUEUE IdleQueue;

    //
    // Button states
    //
    BUTTON_STATE StatePower;
    BUTTON_STATE StateVolumeUp;
    BUTTON_STATE StateVolumeDown;
    BUTTON_STATE StateCameraFocus;
    BUTTON_STATE StateCamera;
    BUTTON_STATE StateSlider;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, GetDeviceContext)

//
// Memory tags
//
#define BTN_POOL_TAG                  (ULONG)'ntuB'