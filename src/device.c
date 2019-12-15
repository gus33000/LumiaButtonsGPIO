// Copyright (c) Microsoft Corporation. All Rights Reserved. 
// Copyright (c) Bingxing Wang. All Rights Reserved. 

#include <internal.h>
#include <device.h>
#include <spb.h>
#include <idle.h>
#include <trace.h>

#ifdef ALLOC_PRAGMA
  #pragma alloc_text(PAGE, OnD0Exit)
#endif

VOID SubmitKeyPress(
    _In_ PDEVICE_EXTENSION deviceContext,
    _In_ BUTTON_TYPE  ButtonType)
{
    BTN_REPORT hidReportFromDriver = { 0 };

    switch (ButtonType)
    {
    case VolumeUp:
    {
        deviceContext->StateVolumeUp = !deviceContext->StateVolumeUp;

        hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONSUMER;
        hidReportFromDriver.KeysData.Consumer.VolumeUp = deviceContext->StateVolumeUp;
        break;
    }
    case VolumeDown:
    {
        deviceContext->StateVolumeDown = !deviceContext->StateVolumeDown;

        hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONSUMER;
        hidReportFromDriver.KeysData.Consumer.VolumeDown = deviceContext->StateVolumeDown;
        break;
    }
    case Power:
    {
        deviceContext->StatePower = !deviceContext->StatePower;

        hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONTROL;
        hidReportFromDriver.KeysData.Control.Power = deviceContext->StatePower;
        break;
    }
    case Camera:
    {
        deviceContext->StateCamera = !deviceContext->StateCamera;

        hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
        hidReportFromDriver.KeysData.Keyboard.Start = deviceContext->StateCamera;
        break;
    }
    case CameraFocus:
    {
        deviceContext->StateCameraFocus = !deviceContext->StateCameraFocus;

        hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
        hidReportFromDriver.KeysData.Keyboard.Start = deviceContext->StateCameraFocus;
        break;
    }
    }

    NTSTATUS status;
    WDFREQUEST request = NULL;
    PBTN_REPORT hidReportRequestBuffer = { 0 };
    size_t hidReportRequestBufferLength;

    status = WdfIoQueueRetrieveNextRequest(
        deviceContext->PingPongQueue,
        &request);

    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
            "No request pending from HIDClass, ignoring report - STATUS:%X",
            status);
    }

    status = WdfRequestRetrieveOutputBuffer(
        request,
        sizeof(BTN_REPORT),
        &hidReportRequestBuffer,
        &hidReportRequestBufferLength);

    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
            "Error retrieving HID read request output buffer - STATUS:%X",
            status);
    }
    else
    {
        //
        // Validate the size of the output buffer
        //
        if (hidReportRequestBufferLength < sizeof(BTN_REPORT))
        {
            status = STATUS_BUFFER_TOO_SMALL;

            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Error HID read request buffer is too small (%d bytes) - STATUS:%X\n",
                hidReportRequestBufferLength,
                status);
        }
        else
        {
            RtlCopyMemory(
                hidReportRequestBuffer,
                &hidReportFromDriver,
                sizeof(BTN_REPORT));

            WdfRequestSetInformation(request, sizeof(BTN_REPORT));
        }
    }

    WdfRequestComplete(request, status);
}

void InterruptPowerWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from Power!\n");

    SubmitKeyPress(devCtx, Power);
}

void InterruptVolumeUpWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from VolumeUp!\n");

    SubmitKeyPress(devCtx, VolumeUp);
}

void InterruptVolumeDownWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from VolumeDown!\n");

    SubmitKeyPress(devCtx, VolumeDown);
}

void InterruptCameraFocusWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from CameraFocus!\n");

    SubmitKeyPress(devCtx, CameraFocus);
}

void InterruptCameraWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from Camera!\n");

    SubmitKeyPress(devCtx, Camera);
}

BOOLEAN
OnInterruptIsr(
    IN WDFINTERRUPT Interrupt,
    IN ULONG MessageID
    )
/*++
 
  Routine Description:

    This routine responds to interrupts generated by the
    controller. If one is recognized, it queues a DPC for 
    processing. 

    This is a PASSIVE_LEVEL ISR. ACPI should specify
    level-triggered interrupts when using Synaptics 3202.

  Arguments:

    Interrupt - a handle to a framework interrupt object
    MessageID - message number identifying the device's
        hardware interrupt message (if using MSI)

  Return Value:

    TRUE if interrupt recognized.

--*/
{
    UNREFERENCED_PARAMETER(MessageID);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: EvtInterruptIsr Entry\n");

    WdfInterruptQueueWorkItemForIsr(Interrupt);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: EvtInterruptIsr Exit\n");

    return TRUE;
}


NTSTATUS
LumiaButtonsGPIOProbeResources(
    PDEVICE_EXTENSION DeviceContext,
    WDFCMRESLIST ResourcesTranslated,
    WDFCMRESLIST ResourcesRaw
)
{
    PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;

    WDF_INTERRUPT_CONFIG interruptConfigPower;
    WDF_INTERRUPT_CONFIG interruptConfigVolumeUp;
    WDF_INTERRUPT_CONFIG interruptConfigVolumeDown;
    WDF_INTERRUPT_CONFIG interruptConfigCameraFocus;
    WDF_INTERRUPT_CONFIG interruptConfigCamera;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor = NULL;

    DeviceContext->StatePower = ButtonStateUnpressed;
    DeviceContext->StateVolumeUp = ButtonStateUnpressed;
    DeviceContext->StateVolumeDown = ButtonStateUnpressed;
    DeviceContext->StateCameraFocus = ButtonStateUnpressed;
    DeviceContext->StateCamera = ButtonStateUnpressed;

    ULONG interruptFound = 0;

    ULONG InterruBTNower = 0;
    ULONG InterruptVolumeUp = 0;
    ULONG InterruptVolumeDown = 0;
    ULONG InterruptCameraFocus = 0;
    ULONG InterruptCamera = 0;

    ULONG resourceCount;

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: LumiaButtonsGPIOProbeResources Entry\n");

    resourceCount = WdfCmResourceListGetCount(ResourcesTranslated);

    for (ULONG i = 0; i < resourceCount; i++)
    {
        descriptor = WdfCmResourceListGetDescriptor(ResourcesTranslated, i);

        switch (descriptor->Type)
        {
        case CmResourceTypeInterrupt:
            // We've found an interrupt resource.

            switch (interruptFound)
            {
            case 0:
                InterruBTNower = i;
                break;
            case 1:
                InterruptVolumeUp = i;
                break;
            case 2:
                InterruptVolumeDown = i;
                break;
            case 3:
                InterruptCameraFocus = i;
                break;
            case 4:
                InterruptCamera = i;
                break;
            default:
                break;
            }

            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Found Interrupt resource id=%lu index=%lu\n", interruptFound, i);

            interruptFound++;
            break;

        default:
            // We don't care about other descriptors.
            break;
        }
    }

    if (interruptFound < 5)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Not all resources were found, Interrupts = %d\n", interruptFound);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    WDF_INTERRUPT_CONFIG_INIT(&interruptConfigPower, OnInterruptIsr, NULL);

    interruptConfigPower.PassiveHandling = TRUE;
    interruptConfigPower.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruBTNower);
    interruptConfigPower.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruBTNower);

    interruptConfigPower.EvtInterruptWorkItem = InterruptPowerWorkItem;

    status = WdfInterruptCreate(
        DeviceContext->FxDevice,
        &interruptConfigPower,
        WDF_NO_OBJECT_ATTRIBUTES,
        &DeviceContext->InterruBTNower);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for Power %x\n", status);
        goto Exit;
    }


    WDF_INTERRUPT_CONFIG_INIT(&interruptConfigVolumeUp, OnInterruptIsr, NULL);

    interruptConfigVolumeUp.PassiveHandling = TRUE;
    interruptConfigVolumeUp.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruptVolumeUp);
    interruptConfigVolumeUp.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruptVolumeUp);

    interruptConfigVolumeUp.EvtInterruptWorkItem = InterruptVolumeUpWorkItem;

    status = WdfInterruptCreate(
        DeviceContext->FxDevice,
        &interruptConfigVolumeUp,
        WDF_NO_OBJECT_ATTRIBUTES,
        &DeviceContext->InterruptVolumeUp);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for VolumeUp %x\n", status);
        goto Exit;
    }

    WDF_INTERRUPT_CONFIG_INIT(&interruptConfigVolumeDown, OnInterruptIsr, NULL);

    interruptConfigVolumeDown.PassiveHandling = TRUE;
    interruptConfigVolumeDown.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruptVolumeDown);
    interruptConfigVolumeDown.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruptVolumeDown);

    interruptConfigVolumeDown.EvtInterruptWorkItem = InterruptVolumeDownWorkItem;

    status = WdfInterruptCreate(
        DeviceContext->FxDevice,
        &interruptConfigVolumeDown,
        WDF_NO_OBJECT_ATTRIBUTES,
        &DeviceContext->InterruptVolumeDown);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for VolumeDown %x\n", status);
        goto Exit;
    }

    WDF_INTERRUPT_CONFIG_INIT(&interruptConfigCameraFocus, OnInterruptIsr, NULL);

    interruptConfigCameraFocus.PassiveHandling = TRUE;
    interruptConfigCameraFocus.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruptCameraFocus);
    interruptConfigCameraFocus.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruptCameraFocus);

    interruptConfigCameraFocus.EvtInterruptWorkItem = InterruptCameraFocusWorkItem;

    status = WdfInterruptCreate(
        DeviceContext->FxDevice,
        &interruptConfigCameraFocus,
        WDF_NO_OBJECT_ATTRIBUTES,
        &DeviceContext->InterruptCameraFocus);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for CameraFocus %x\n", status);
        goto Exit;
    }

    WDF_INTERRUPT_CONFIG_INIT(&interruptConfigCamera, OnInterruptIsr, NULL);

    interruptConfigCamera.PassiveHandling = TRUE;
    interruptConfigCamera.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruptCamera);
    interruptConfigCamera.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruptCamera);

    interruptConfigCamera.EvtInterruptWorkItem = InterruptCameraWorkItem;

    status = WdfInterruptCreate(
        DeviceContext->FxDevice,
        &interruptConfigCamera,
        WDF_NO_OBJECT_ATTRIBUTES,
        &DeviceContext->InterruptCamera);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for Camera %x\n", status);
        goto Exit;
    }

Exit:
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: LumiaButtonsGPIOProbeResources Exit: %x\n", status);

    return status;
}

NTSTATUS
OnD0Entry(
   IN WDFDEVICE Device,    
   IN WDF_POWER_DEVICE_STATE PreviousState    
   )
/*++

Routine Description:

    This routine will power on the hardware

Arguments:

    Device - WDF device to power on
    PreviousState - Prior power state

Return Value:

    NTSTATUS indicating success or failure

*/
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION devContext;
    
    devContext = GetDeviceContext(Device);

    UNREFERENCED_PARAMETER(PreviousState);
    
    //
    // N.B. This RMI chip's IRQ is level-triggered, but cannot be enabled in
    //      ACPI until passive-level interrupt handling is added to the driver.
    //      Service chip in case we missed an edge during D3 or boot-up.
    //
    devContext->ServiceInterruptsAfterD0Entry = TRUE;

    //
    // Complete any pending Idle IRPs
    //
    BtnCompleteIdleIrp(devContext);

    return status;
}

NTSTATUS
OnD0Exit(
   IN WDFDEVICE Device,    
   IN WDF_POWER_DEVICE_STATE TargetState   
   )
/*++

Routine Description:

    This routine will power down the hardware

Arguments:

    Device - WDF device to power off

    PreviousState - Prior power state

Return Value:

    NTSTATUS indicating success or failure

*/
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION devContext;

    PAGED_CODE();
    
    devContext = GetDeviceContext(Device);
    
    UNREFERENCED_PARAMETER(TargetState);    

    return status;
}
    
NTSTATUS
OnPrepareHardware(
    IN WDFDEVICE FxDevice,
    IN WDFCMRESLIST FxResourcesRaw,
    IN WDFCMRESLIST FxResourcesTranslated
    )
/*++
 
  Routine Description:

    This routine is called by the PnP manager and supplies thie device instance
    with it's SPB resources (CmResourceTypeConnection) needed to find the I2C
    driver.

  Arguments:

    FxDevice - a handle to the framework device object
    FxResourcesRaw - list of translated hardware resources that 
        the PnP manager has assigned to the device
    FxResourcesTranslated - list of raw hardware resources that 
        the PnP manager has assigned to the device

  Return Value:

    NTSTATUS indicating sucess or failure

--*/
{
    NTSTATUS status;
    PDEVICE_EXTENSION devContext;

    UNREFERENCED_PARAMETER(FxResourcesRaw);

    status = STATUS_INSUFFICIENT_RESOURCES;
    devContext = GetDeviceContext(FxDevice);

    status = LumiaButtonsGPIOProbeResources(devContext, FxResourcesTranslated, FxResourcesRaw);
    if (!NT_SUCCESS(status))
    {
        Trace(TRACE_LEVEL_ERROR,
            TRACE_INIT, 
            "LumiaButtonsGPIO: LumiaButtonsGPIOProbeResources failed %x", 
            status);
        goto exit;
    }

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error finding CmResourceTypeConnection resource - %!STATUS!",
            status);

        goto exit;
    }

exit:

    return status;
}

NTSTATUS
OnReleaseHardware(
    IN WDFDEVICE FxDevice,
    IN WDFCMRESLIST FxResourcesTranslated
    )
/*++
 
  Routine Description:

    This routine cleans up any resources provided.

  Arguments:

    FxDevice - a handle to the framework device object
    FxResourcesRaw - list of translated hardware resources that 
        the PnP manager has assigned to the device
    FxResourcesTranslated - list of raw hardware resources that 
        the PnP manager has assigned to the device

  Return Value:

    NTSTATUS indicating sucesss or failure

--*/
{
    NTSTATUS status = STATUS_SUCCESS;
    //PDEVICE_EXTENSION devContext;

    UNREFERENCED_PARAMETER(FxResourcesTranslated);
    UNREFERENCED_PARAMETER(FxDevice);
    //devContext = GetDeviceContext(FxDevice);

    return status;
}

