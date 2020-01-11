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

VOID SendReport(
    IN PDEVICE_EXTENSION deviceContext,
    IN BTN_REPORT hidReportFromDriver
)
{
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

VOID EvaluateButtonAction(
    IN PDEVICE_EXTENSION deviceContext,
    IN BUTTON_TYPE ButtonType
)
{
    BTN_REPORT hidReportFromDriver = { 0 };

    int RelevantButtonActiveCount = (int)deviceContext->StateCamera + (int)deviceContext->StateCameraFocus +
                                    (int)deviceContext->StatePower +
                                    (int)deviceContext->StateVolumeUp + (int)deviceContext->StateVolumeDown;

    if (RelevantButtonActiveCount <= 2)
    {
        // Trigger on Volume Up being high
        if (deviceContext->StatePower && deviceContext->StateVolumeUp && ButtonType == VolumeUp)
        {
            // Power + Volume Up (High)
            // WIN + F15

            hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
            hidReportFromDriver.KeysData.Keyboard.LeftWin = ButtonStatePressed;
            hidReportFromDriver.KeysData.Keyboard.F15 = ButtonStatePressed;
            SendReport(deviceContext, hidReportFromDriver);

            // Unpress the keys immediately
            hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
            hidReportFromDriver.KeysData.Keyboard.LeftWin = ButtonStateUnpressed;
            hidReportFromDriver.KeysData.Keyboard.F15 = ButtonStateUnpressed;
            SendReport(deviceContext, hidReportFromDriver);

            deviceContext->IgnoreButtonPresses = TRUE;
        }
        // Trigger on Volume Down being high
        else if (deviceContext->StatePower && deviceContext->StateVolumeDown && ButtonType == VolumeDown)
        {
            // Power + Volume Down (High)
            // CTRL + ALT + DEL

            hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
            hidReportFromDriver.KeysData.Keyboard.LeftCtrl = ButtonStatePressed;
            hidReportFromDriver.KeysData.Keyboard.LeftAlt = ButtonStatePressed;
            hidReportFromDriver.KeysData.Keyboard.Del = ButtonStatePressed;
            SendReport(deviceContext, hidReportFromDriver);

            // Unpress the keys immediately
            hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
            hidReportFromDriver.KeysData.Keyboard.LeftCtrl = ButtonStateUnpressed;
            hidReportFromDriver.KeysData.Keyboard.LeftAlt = ButtonStateUnpressed;
            hidReportFromDriver.KeysData.Keyboard.Del = ButtonStateUnpressed;
            SendReport(deviceContext, hidReportFromDriver);

            deviceContext->IgnoreButtonPresses = TRUE;
        }
        //
        // Only one key should be active at a time after checking the above combinations
        // The only exception is the slider where we can expect it to be enabled with other keys
        //
        else if (RelevantButtonActiveCount <= 1)
        {
            // Trigger on power being low
            if (ButtonType == Power && !deviceContext->StatePower && !deviceContext->IgnoreButtonPresses)
            {
                // Power
                hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONTROL;
                hidReportFromDriver.KeysData.Control.SystemPowerDown = ButtonStatePressed;
                SendReport(deviceContext, hidReportFromDriver);

                // Unpress the key immediately
                hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONTROL;
                hidReportFromDriver.KeysData.Control.SystemPowerDown = ButtonStateUnpressed;
                SendReport(deviceContext, hidReportFromDriver);
            }

            if (ButtonType == VolumeUp && !deviceContext->IgnoreButtonPresses)
            {
                // Volume Up

                hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONSUMER;
                hidReportFromDriver.KeysData.Consumer.VolumeUp = deviceContext->StateVolumeUp;
                SendReport(deviceContext, hidReportFromDriver);
            }

            if (ButtonType == VolumeDown && !deviceContext->IgnoreButtonPresses)
            {
                // Volume Down

                hidReportFromDriver.ReportID = REPORTID_CAPKEY_CONSUMER;
                hidReportFromDriver.KeysData.Consumer.VolumeDown = deviceContext->StateVolumeDown;
                SendReport(deviceContext, hidReportFromDriver);
            }

            if (ButtonType == CameraFocus && !deviceContext->IgnoreButtonPresses)
            {
                // Camera Focus
                // Placeholder

                hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
                hidReportFromDriver.KeysData.Keyboard.LeftWin = deviceContext->StateCameraFocus;
                SendReport(deviceContext, hidReportFromDriver);
            }

            if (ButtonType == Camera && !deviceContext->IgnoreButtonPresses)
            {
                // Camera
                // Placeholder

                hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
                hidReportFromDriver.KeysData.Keyboard.LeftWin = deviceContext->StateCamera;
                SendReport(deviceContext, hidReportFromDriver);
            }

            if (deviceContext->StateSlider && ButtonType == Slider)
            {
                // Slider on
                // WIN + F14

                hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
                hidReportFromDriver.KeysData.Keyboard.LeftWin = ButtonStatePressed;
                hidReportFromDriver.KeysData.Keyboard.F14 = ButtonStatePressed;
                SendReport(deviceContext, hidReportFromDriver);

                // Unpress the keys immediately
                hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
                hidReportFromDriver.KeysData.Keyboard.LeftWin = ButtonStateUnpressed;
                hidReportFromDriver.KeysData.Keyboard.F14 = ButtonStateUnpressed;
                SendReport(deviceContext, hidReportFromDriver);
            }
            else if (!deviceContext->StateSlider && ButtonType == Slider)
            {
                // Slider off
                // WIN + F14

                hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
                hidReportFromDriver.KeysData.Keyboard.LeftWin = ButtonStatePressed;
                hidReportFromDriver.KeysData.Keyboard.F14 = ButtonStatePressed;
                SendReport(deviceContext, hidReportFromDriver);

                // Unpress the keys immediately
                hidReportFromDriver.ReportID = REPORTID_CAPKEY_KEYBOARD;
                hidReportFromDriver.KeysData.Keyboard.LeftWin = ButtonStateUnpressed;
                hidReportFromDriver.KeysData.Keyboard.F14 = ButtonStateUnpressed;
                SendReport(deviceContext, hidReportFromDriver);
            }
        }
    }

    if (RelevantButtonActiveCount == 0)
    {
        deviceContext->IgnoreButtonPresses = FALSE;
    }
}

VOID HandleButtonPress(
    IN PDEVICE_EXTENSION deviceContext,
    IN BUTTON_TYPE ButtonType)
{
    if (!deviceContext->ProcessInterrupts)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Cancelling interrupt processing because we are not done initializing yet.\n");
        return;
    }

    switch (ButtonType)
    {
    case VolumeUp:
    {
        deviceContext->StateVolumeUp = !deviceContext->StateVolumeUp;
        break;
    }
    case VolumeDown:
    {
        deviceContext->StateVolumeDown = !deviceContext->StateVolumeDown;
        break;
    }
    case Power:
    {
        deviceContext->StatePower = !deviceContext->StatePower;
        break;
    }
    case Camera:
    {
        deviceContext->StateCamera = !deviceContext->StateCamera;
        break;
    }
    case CameraFocus:
    {
        deviceContext->StateCameraFocus = !deviceContext->StateCameraFocus;
        break;
    }
    case Slider:
    {
        deviceContext->StateSlider = !deviceContext->StateSlider;
        break;
    }
    }

    EvaluateButtonAction(deviceContext, ButtonType);
}

void InterruptPowerWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from Power!\n");

    if (devCtx->InitializationOk == 2)
        HandleButtonPress(devCtx, Power);

    devCtx->InitializationOk++;
}

void InterruptVolumeUpWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from VolumeUp!\n");

    if (devCtx->InitializationOk == 2)
        HandleButtonPress(devCtx, VolumeUp);

    devCtx->InitializationOk = TRUE;
}

void InterruptVolumeDownWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from VolumeDown!\n");

    if (devCtx->InitializationOk == 2)
        HandleButtonPress(devCtx, VolumeDown);

    devCtx->InitializationOk++;
}

void InterruptCameraFocusWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from CameraFocus!\n");

    if (devCtx->InitializationOk == 2)
        HandleButtonPress(devCtx, CameraFocus);

    devCtx->InitializationOk++;
}

void InterruptCameraWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from Camera!\n");

    if (devCtx->InitializationOk == 2)
        HandleButtonPress(devCtx, Camera);

    devCtx->InitializationOk++;
}

void InterruptSliderWorkItem(
    WDFINTERRUPT Interrupt,
    WDFOBJECT AssociatedObject
)
{
    UNREFERENCED_PARAMETER(Interrupt);

    PDEVICE_EXTENSION devCtx = GetDeviceContext(AssociatedObject);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Got an interrupt from Slider!\n");

    if (devCtx->InitializationOk == 2)
        HandleButtonPress(devCtx, Slider);

    devCtx->InitializationOk++;
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

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: EvtInterruptIsr Entry\n");

    WdfInterruptQueueWorkItemForIsr(Interrupt);

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: EvtInterruptIsr Exit\n");

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
    WDF_INTERRUPT_CONFIG interruptConfigSlider;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor = NULL;

    DeviceContext->StatePower = ButtonStateUnpressed;
    DeviceContext->StateVolumeUp = ButtonStateUnpressed;
    DeviceContext->StateVolumeDown = ButtonStateUnpressed;
    DeviceContext->StateCameraFocus = ButtonStateUnpressed;
    DeviceContext->StateCamera = ButtonStateUnpressed;
    DeviceContext->StateSlider = ButtonStateUnpressed;

    DeviceContext->ProcessInterrupts = FALSE;

    ULONG interruptFound = 0;

    ULONG InterruptPower = 0;
    ULONG InterruptVolumeUp = 0;
    ULONG InterruptVolumeDown = 0;
    ULONG InterruptCameraFocus = 0;
    ULONG InterruptCamera = 0;
    ULONG InterruptSlider = 0;

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
                InterruptPower = i;
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
            case 5:
                InterruptSlider = i;
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

    if (interruptFound < 3)
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Not all resources were found, Interrupts = %d\n", interruptFound);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Beginning to create interrupts\n");

    WDF_INTERRUPT_CONFIG_INIT(&interruptConfigPower, OnInterruptIsr, NULL);

    interruptConfigPower.PassiveHandling = TRUE;
    interruptConfigPower.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruptPower);
    interruptConfigPower.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruptPower);

    interruptConfigPower.EvtInterruptWorkItem = InterruptPowerWorkItem;

    status = WdfInterruptCreate(
        DeviceContext->FxDevice,
        &interruptConfigPower,
        WDF_NO_OBJECT_ATTRIBUTES,
        &DeviceContext->InterruptPower);
    if (!NT_SUCCESS(status))
    {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for Power %x\n", status);
        goto Exit;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Created Interrupt\n");

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

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Created Interrupt\n");

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

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Created Interrupt\n");

    if (interruptFound >= 5)
    {
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

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Created Interrupt\n");

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

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Created Interrupt\n");

        if (interruptFound >= 6)
        {

            WDF_INTERRUPT_CONFIG_INIT(&interruptConfigSlider, OnInterruptIsr, NULL);

            interruptConfigSlider.PassiveHandling = TRUE;
            interruptConfigSlider.InterruptTranslated = WdfCmResourceListGetDescriptor(ResourcesTranslated, InterruptSlider);
            interruptConfigSlider.InterruptRaw = WdfCmResourceListGetDescriptor(ResourcesRaw, InterruptSlider);

            interruptConfigSlider.EvtInterruptWorkItem = InterruptSliderWorkItem;

            status = WdfInterruptCreate(
                DeviceContext->FxDevice,
                &interruptConfigSlider,
                WDF_NO_OBJECT_ATTRIBUTES,
                &DeviceContext->InterruptSlider);
            if (!NT_SUCCESS(status))
            {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: WdfInterruptCreate failed for Slider %x\n", status);
                goto Exit;
            }

            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: Created Interrupt\n");

        }
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

NTSTATUS OnD0EntryPostInterruptsEnabled
(
    IN WDFDEVICE Device,
    IN WDF_POWER_DEVICE_STATE PreviousState
)
{
    UNREFERENCED_PARAMETER(PreviousState);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: OnD0EntryPostInterruptsEnabled Entry\n");

    PDEVICE_EXTENSION DeviceContext = GetDeviceContext(Device);

    DeviceContext->ProcessInterrupts = TRUE;

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "LumiaButtonsGPIO: OnD0EntryPostInterruptsEnabled Exit\n");

    return 0;
}