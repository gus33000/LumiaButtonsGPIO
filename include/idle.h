#pragma once

//
// Power Idle Workitem context
// 
typedef struct _IDLE_WORKITEM_CONTEXT
{    
    // Handle to a WDF device object
    WDFDEVICE FxDevice;

    // Handle to a WDF request object
    WDFREQUEST FxRequest;

} IDLE_WORKITEM_CONTEXT, *PIDLE_WORKITEM_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(IDLE_WORKITEM_CONTEXT, GetWorkItemContext)

NTSTATUS
BtnProcessIdleRequest(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request,
    OUT BOOLEAN *Pending
    );

VOID
BtnCompleteIdleIrp(
    IN PDEVICE_EXTENSION FxDeviceContext
    );

EVT_WDF_WORKITEM BtnIdleIrpWorkitem;