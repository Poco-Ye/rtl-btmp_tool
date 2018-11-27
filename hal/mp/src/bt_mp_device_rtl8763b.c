#include "bt_mp_device_rtl8763b.h"
#include "bt_syslog.h"

int
BTDevice_ReadThermal_RTL8763B(
    BT_DEVICE *pBtDevice,
    BT_PARAMETER *pParam,
    unsigned char *pThermalValue
    )
{

    unsigned char pPayload[HCI_CMD_LEN_MAX];
    unsigned char pEvent[HCI_EVT_LEN_MAX];

    unsigned short Value =0 ;
    unsigned short reg_value=0 ;
    int n=0;
    unsigned int EvtLen;

    unsigned long Tmeter_avgx4=0; // (no unit)
    unsigned long Tmeter_acc =0;

    SYSLOGI(" BTDevice_ReadThermal_RTL8763B : 0x%x\n", pParam->ExeMode);
    for (n=0;n<8;n++)
    {
        if (bt_default_SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_READ_THERMAL_METER_DATA, LEN_0_BYTE, pPayload, 0x0E, pEvent, &EvtLen))
        {
        // FW NO Support
            if(bt_default_GetRFRegMaskBits(pBtDevice, 0x00, 15, 0, &reg_value))
                goto error;

            reg_value = reg_value & 0x3000;
            if (reg_value == 0x0000)
            {
                if(bt_default_SetRFRegMaskBits(pBtDevice, 0x00, 15, 0, 0x1000))
                    goto error;
            }


            if(bt_default_SetRFRegMaskBits(pBtDevice, 0x04, 6, 6, 1))
                goto error;

            if (bt_default_GetRFRegMaskBits(pBtDevice, 0x04, 5, 0, &Value))
                goto error;

            if (reg_value == 0x0000)
            {
                if(bt_default_SetRFRegMaskBits(pBtDevice, 0x00, 15, 0, 0x0000))
                    goto error;
            }

        }
        else
        {
            Value = *(pEvent+EVT_BYTE0);
        }

     Tmeter_acc += Value;

    }
    Tmeter_avgx4 = (Tmeter_acc>>1) + (Tmeter_acc&0x0001);

    *pThermalValue = (unsigned char)Tmeter_avgx4;

    return BT_FUNCTION_SUCCESS;

error:
    return FUNCTION_ERROR;
}
