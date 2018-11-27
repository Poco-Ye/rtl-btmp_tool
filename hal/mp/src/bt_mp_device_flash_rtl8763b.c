#include <stdlib.h>
#include <unistd.h>

#include "bt_mp_device_flash_rtl8763b.h"
#include "ic_config.h"
#include "bt_syslog.h"

static unsigned long gHEADER_SIZE = 0x1000;
static unsigned long Flash_page_size_len = 0x1000;

#define FLASH_WRITETEMPBUFFER_SIZE 1024*8
static unsigned char flash_WriteTempData[FLASH_WRITETEMPBUFFER_SIZE]= {0xFF};
static unsigned long flash_WriteTempData_length=0;


int
bt_Flash_Read_Buf(
    BT_DEVICE *pBtDevice,
    unsigned int Address,
    unsigned int Len,
    unsigned char *pReadingData
    )
{
    unsigned char pPayLoad[MAX_HCI_BUF_SIZE];
    unsigned char pEvent[MAX_HCI_BUF_SIZE];
    unsigned int RegReadingAddr;
    unsigned int ReadingByteNumRem, ReadingByteNum;
    unsigned int i, j;
    unsigned int EvtLen;
    unsigned int NumberOfBytesRead;

    for( i = 0 ; i <Len; i +=READ_WRITE_BYTE_NUM_MAX)
    {
        // Set register reading address.
        RegReadingAddr = Address + i;

        // Calculate remainder reading byte number.
        ReadingByteNumRem = Len - i;

        // Determine reading byte number.
        ReadingByteNum = (ReadingByteNumRem > READ_WRITE_BYTE_NUM_MAX) ? READ_WRITE_BYTE_NUM_MAX : ReadingByteNumRem;

        for( j = 0 ; j<LEN_4_BYTE; j++)
        {
            pPayLoad[j] = (unsigned char)((RegReadingAddr>>(j*8))&0xFF);
        }

        for( j = 0 ; j<LEN_4_BYTE; j++)
        {
            pPayLoad[j+LEN_4_BYTE] = (unsigned char)((ReadingByteNum>>(j*8))&0xFF);
        }

        if(bt_default_SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_EFLASH_READ_BUFFER, LEN_8_BYTE, pPayLoad, 0x0E, pEvent, &EvtLen) != BT_FUNCTION_SUCCESS)
            goto error;


        if(pEvent[EVT_STATUS] != 0)
            goto error;

        for( j = 0 ; j <LEN_4_BYTE; j++)
        {
            NumberOfBytesRead = ( (pEvent[EVT_BYTE3]<<24)+(pEvent[EVT_BYTE2]<<16)+(pEvent[EVT_BYTE1]<<8)+pEvent[EVT_BYTE0] );
        }

        if(NumberOfBytesRead != ReadingByteNum)
        {
            SYSLOGI("bee_uart_Flash_Read ERROR: NumberOfBytesRead(%d) != ReadingByteNum(%d)\n", NumberOfBytesRead, ReadingByteNum);
            goto error;
        }
        memcpy(pReadingData+i, pEvent+EVT_BYTE4, ReadingByteNum);

    }

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}



int
bt_Flash_Write_Buf(
    BT_DEVICE *pBtDevice,
    unsigned int Address,
    unsigned int Len,
    unsigned char *pWritingData
    )
{
    unsigned char pPayLoad[MAX_HCI_BUF_SIZE];
    unsigned char pEvent[MAX_HCI_BUF_SIZE];
    unsigned int RegWritingAddr;
    unsigned int WritingByteNumRem, WritingByteNum;
    unsigned int i, j;
    unsigned int EvtLen;

    for( i = 0 ; i <Len; i +=READ_WRITE_BYTE_NUM_MAX)   
    {
        // Set register writing address.
        RegWritingAddr = Address + i;

        // Calculate remainder writing byte number.
        WritingByteNumRem = Len - i;

        // Determine writing byte number.
        WritingByteNum = (WritingByteNumRem > READ_WRITE_BYTE_NUM_MAX) ? READ_WRITE_BYTE_NUM_MAX : WritingByteNumRem;
        for( j = 0 ; j <LEN_4_BYTE; j++)
        {
            pPayLoad[j] = (unsigned char)((RegWritingAddr>>(j*8))&0xFF);
        }

        for( j = 0 ; j <LEN_4_BYTE; j++)
        {
            pPayLoad[j+LEN_4_BYTE] = (unsigned char)((WritingByteNum>>(j*8))&0xFF);
        }

        memcpy(pPayLoad+LEN_8_BYTE, pWritingData+i, WritingByteNum);

        if(bt_default_SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_EFLASH_WRITE_BUFFER, (unsigned char)(LEN_8_BYTE+WritingByteNum), pPayLoad, 0x0E, pEvent, &EvtLen) != BT_FUNCTION_SUCCESS)
            goto error;

        if(pEvent[EVT_STATUS] != 0)
            goto error;

    }
    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}




int
bt_Flash_Erase_Page(
    BT_DEVICE *pBtDevice,
    unsigned int Address
    )
{
    unsigned char pPayLoad[MAX_HCI_BUF_SIZE];
    unsigned char pEvent[MAX_HCI_BUF_SIZE];
    unsigned char i;
    unsigned int EvtLen;

    for( i = 0 ; i <LEN_4_BYTE; i++)
    {
        pPayLoad[i] = (unsigned char)((Address>>(i*8))&0xFF);
    }

    if(bt_default_SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_EFLASH_ERASE, LEN_4_BYTE, pPayLoad, 0x0E, pEvent, &EvtLen) != BT_FUNCTION_SUCCESS)
        goto error;

    if(pEvent[EVT_STATUS] != 0)
        goto error;

    return BT_FUNCTION_SUCCESS;

error:

    return FUNCTION_ERROR;

}

unsigned int FLASH_CONFIG_ADDR = 0;

int
BTDevice_Read_Data_from_Flash_Config_rtl8763B(
    BT_DEVICE *pBtDevice,
    unsigned char *pBuf,
    unsigned char *pReportData
    )
{
    unsigned char *pFlashReadingBuf = NULL;
    unsigned int i;
    unsigned char signature[LEN_4_BYTE]= {0xb3, 0x5b, 0x34, 0x12}; //0x12345bb3

    //unsigned char pData[12]= {0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    unsigned char pData[12] = { 0x05,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00 }; //unclock flash address 0x800000
    unsigned char bp_level[4] = { 0 };
    //[e]0x2c,0xfc,12,0x02,0x10,0x00,0x00,[0x02] if 0 is BP,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
    unsigned char pEvtBuf[LEN_512_BYTE];
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    IC_CONFIG ic_config;
    unsigned int total_len;
    unsigned long rf_data_address=0;
    //unsigned char read_signature_len[6]={0};
    unsigned int EvtLen;


    if(pBTInfo->Version > 3) //BBPro D cut version
    {
#if 1
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData, 0x0E, pEvtBuf, &EvtLen)!= BT_FUNCTION_SUCCESS)
            goto error;

        for (i=0; i < 4; i++)
        {
            bp_level[i] = pEvtBuf[i + 6]; //copy event parameter to bp_level, event code(1)+len(1)+ num_of_cmd(1) + cmd(2) + status(1)
        }
#endif
        FLASH_CONFIG_ADDR = FLASH_E_CUT_CONFIG_ADDR + gHEADER_SIZE;;
    }
    else
    {
        FLASH_CONFIG_ADDR = FLASH_C_CUT_CONFIG_ADDR;
    }

    if (bt_flash_Read_RF_Data_Address(pBtDevice,&rf_data_address) == BT_FUNCTION_SUCCESS)
    {

           FLASH_CONFIG_ADDR = rf_data_address;
    }

    pFlashReadingBuf = (unsigned char *)malloc(1024*5*sizeof(unsigned char));
    if (pFlashReadingBuf == NULL)
    {
        SYSLOGI("BTDevice_Read_Data_from_Flash_Config: allocate pFlashReadingBuf error\n");
        goto error;
    }

    memset(pFlashReadingBuf, 0xFF, FLASH_PAGE_SIZE_LEN*sizeof(unsigned char));

    //
    if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR, FLASH_PAGE_SIZE_LEN, pFlashReadingBuf) != BT_FUNCTION_SUCCESS)
        goto error;
    //step: read signature & length
//  if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR, 6, pFlashReadingBuf) != BT_FUNCTION_SUCCESS)
//      goto error;

    ///if ()

    total_len = 0;

    total_len |= pFlashReadingBuf[4];
    total_len |= pFlashReadingBuf[5] << 8;

    // check certain signature
    if (memcmp(pFlashReadingBuf, signature, 4) ){
        SYSLOGI("BTDevice_Read_Data_from_Flash_Config : signature error\n");
        total_len=0;
    }
//  if (total_len > (FLASH_PAGE_SIZE_LEN-6) )
//           total_len= FLASH_PAGE_SIZE_LEN;
//  if (total_len>0)
  //  {
 //         if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR+6, total_len, &pFlashReadingBuf[6]) != BT_FUNCTION_SUCCESS)
//              goto error;  
  //  }


    if (total_len < 3) {
        SYSLOGI("BTDevice_Read_Data_from_Flash_Config : total len error\n");
    }

    ic_config_set_signature(&ic_config, 0x12345bb3);


    ic_config_parse(&ic_config, pFlashReadingBuf+6, FLASH_PAGE_SIZE_LEN-6);


    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen);
    }


    if(ic_config_get_entry_data(&ic_config, *(unsigned short *)pBuf, *(pBuf+2), pBuf+2+1, pBuf+2+1+pBuf[2])!= BT_FUNCTION_SUCCESS)
    {
        goto error;
    }

    memcpy(pReportData, pBuf, 2+1+pBuf[2]*2);

    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen);
    }


    if(pFlashReadingBuf != NULL)
        free(pFlashReadingBuf);



    return BT_FUNCTION_SUCCESS;

error:


    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen);
    }

    if(pFlashReadingBuf != NULL)
        free(pFlashReadingBuf);


    return FUNCTION_ERROR;

}


int
BTDevice_Write_Data_Entry_to_Flash_Config(
    BT_DEVICE *pBtDevice,
    unsigned char *pWritingEntry
    )
{
    unsigned char *pFlashBuf = NULL;
    unsigned int i;
    unsigned int FlashWritingLen = 0;
    unsigned int ConfigDataLen = 0;
    unsigned int WritingLen = 0;
    unsigned int EndAlignment;
    unsigned char Add_length=0;
    unsigned long rf_data_address=0;
    unsigned int EvtLen;

    unsigned int len;
    unsigned char signature[LEN_4_BYTE]= {0xb3, 0x5b, 0x34, 0x12}; //0x12345bb3

    //unsigned char pData[12]= {0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    unsigned char pData[12] = { 0x05,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00 }; //unclock flash address 0x800000
    unsigned char bp_level[4] = { 0 };
    //[e]0x2c,0xfc,12,0x02,0x10,0x00,0x00,[0x02] if 0 is BP,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
    unsigned char pEvtBuf[LEN_512_BYTE];
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if(pBTInfo->Version > 3) //BBPro D cut version
    {
#if 1
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData, 0x0E, pEvtBuf, &EvtLen)!= BT_FUNCTION_SUCCESS)
            goto error;

        for (i=0; i < 4; i++)
        {
            bp_level[i] = pEvtBuf[i + 6]; //copy event parameter to bp_level, event code(1)+len(1)+ num_of_cmd(1) + cmd(2) + status(1)
        }
#endif
//      if (pBTInfo->Version == 4)  //Dcut
//      FLASH_CONFIG_ADDR = FLASH_D_CUT_CONFIG_ADDR;
//      else
        FLASH_CONFIG_ADDR = FLASH_E_CUT_CONFIG_ADDR + gHEADER_SIZE;  // default is 0x1803000

    }
    else
    {
        FLASH_CONFIG_ADDR = FLASH_C_CUT_CONFIG_ADDR;
    }

    if (bt_flash_Read_RF_Data_Address(pBtDevice,&rf_data_address) == BT_FUNCTION_SUCCESS)
    {

           FLASH_CONFIG_ADDR = rf_data_address;
    } 
    pFlashBuf = (unsigned char *)malloc(1024*5*sizeof(unsigned char));
    if (pFlashBuf == NULL)
    {
        SYSLOGI("allocate pFlashBuf error\n");
        goto error;
    }

    memset(pFlashBuf, 0xFF, Flash_page_size_len*sizeof(unsigned char));

    if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR, Flash_page_size_len, pFlashBuf) != BT_FUNCTION_SUCCESS)
        goto error;


    if(bt_Flash_Erase_Page(pBtDevice, FLASH_CONFIG_ADDR) != BT_FUNCTION_SUCCESS)
        goto error;

    i = 0;

    //Find length
    while (i < Flash_page_size_len)
    {
        if ((pFlashBuf[i+0] == 0xff) &&
            (pFlashBuf[i+1] == 0xff) &&
            (pFlashBuf[i+2] == 0xff) &&
            (pFlashBuf[i+3] == 0xff) &&
            (pFlashBuf[i+4] == 0xff) &&
            (pFlashBuf[i+5] == 0xff) &&
            (pFlashBuf[i+6] == 0xff) &&
            (pFlashBuf[i+7] == 0xff) )
        {

            break;
        }
        else
        {
            i++;
        }
    }

    len = pWritingEntry[0]+(pWritingEntry[1]<<8);
    Add_length = len;

    //wrong signature header
    if(memcmp(pFlashBuf, signature, LEN_4_BYTE) !=0)
    {
        i=0;
    }
    SYSLOGI("-->flash current data length = 0x%x , config set lenght = 0x%x\n",i, len);
    //Handling overflow issues
    if ((i+Add_length) > (FLASH_PAGE_SIZE_LEN -16))
    {
           SYSLOGI("-->To Write to Boundary FLASH_PAGE_SIZE_LEN= 0x%x,Set flash length is 0 ,erase flash , config set lenght = 0x%x\n",FLASH_PAGE_SIZE_LEN, len);
           i=0;
    }
    SYSLOGI("-->flash current data length = 0x%x , config set lenght = 0x%x\n",i, len);
    if( i==0 ) //flash empty
    {
        ConfigDataLen = 0;
        memcpy(pFlashBuf, signature, LEN_4_BYTE);
        pFlashBuf[4] = pWritingEntry[0];
        pFlashBuf[5] = pWritingEntry[1];
        FlashWritingLen += LEN_6_BYTE;
        ConfigDataLen += len;
        SYSLOGI("-->flash empty \n");
    }
    else if ( i > (Flash_page_size_len - 16))  // reserved 16 byte ,>flash fill
    {

          SYSLOGI("-->flash fill i=%x \n" , i );
          goto error;
    }
    else //flash is not empty
    {
        ConfigDataLen = (pFlashBuf[5]<<8)+pFlashBuf[4];
        FlashWritingLen = ConfigDataLen+LEN_6_BYTE;
        ConfigDataLen += len;
        pFlashBuf[4] = ConfigDataLen&0xff;
        pFlashBuf[5] = (unsigned char)((ConfigDataLen>>8)&0xFF);

        SYSLOGI("-->flash is not empty, len:%x, configdatalen:%x\n", len, ConfigDataLen);
    }

    memcpy(pFlashBuf+FlashWritingLen, pWritingEntry+2, len);


    //flash addr & len 4 bytes alignment
    WritingLen = ConfigDataLen+LEN_6_BYTE;
    EndAlignment = WritingLen%LEN_4_BYTE;
    WritingLen = (EndAlignment == 0) ? WritingLen : WritingLen+(LEN_4_BYTE-EndAlignment);
    SYSLOGI("merged flash entry:%x, %x", ConfigDataLen, WritingLen);
    for (i = 0; i < (FlashWritingLen + len); i++)
    {
        SYSLOGI("i:%x, %x\r\n", i, pFlashBuf[i]);
    }
    if(bt_Flash_Write_Buf(pBtDevice, FLASH_CONFIG_ADDR, WritingLen, pFlashBuf) != BT_FUNCTION_SUCCESS) //Keyword(4)+total len(2)
        goto error;


#if 1
    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen))
            goto error;
    }
#endif
    usleep(20000);
    // add
    if (BTDevice_Write_SECUREDATA_to_Flash_Config(pBtDevice,Add_length)!= BT_FUNCTION_SUCCESS)
     goto error;

    SYSLOGI("-BTDevice_Write_Data_Entry_to_Flash_Config()....\n");
    return BT_FUNCTION_SUCCESS;

error:

    free(pFlashBuf);

    return FUNCTION_ERROR;

}


int
BTDevice_Erase_ConfigPage_Entry_bp_level(
    BT_DEVICE *pBtDevice
    )
{
    //unsigned char *pFlashBuf = NULL;
    unsigned int i;
    //unsigned int FlashWritingLen = 0;
    //unsigned int ConfigDataLen = 0;
    //unsigned int WritingLen = 0;
    unsigned long flashcurrentlength=0;
    unsigned long rf_data_address=0;
    unsigned int EvtLen;

    //unsigned char signature[LEN_4_BYTE]= {0xb3, 0x5b, 0x34, 0x12}; //0x12345bb3

    //unsigned char pData[12]= {0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    unsigned char pData[12] = { 0x05,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00 }; //unclock flash address 0x800000
    unsigned char bp_level[4] = { 0 };
    //[e]0x2c,0xfc,12,0x02,0x10,0x00,0x00,[0x02] if 0 is BP,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
    unsigned char pEvtBuf[LEN_512_BYTE];
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if(pBTInfo->Version > 3) //BBPro D cut version
    {
#if 1
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData, 0x0E, pEvtBuf, &EvtLen)!= BT_FUNCTION_SUCCESS)
            goto error;

        for (i=0; i < 4; i++)
        {
            bp_level[i] = pEvtBuf[i + 6]; //copy event parameter to bp_level, event code(1)+len(1)+ num_of_cmd(1) + cmd(2) + status(1)
        }
#endif
    //  FLASH_CONFIG_ADDR = FLASH_D_CUT_CONFIG_ADDR;
        FLASH_CONFIG_ADDR = FLASH_E_CUT_CONFIG_ADDR + gHEADER_SIZE;
    }
    else
    {
        FLASH_CONFIG_ADDR = FLASH_C_CUT_CONFIG_ADDR;
    }
    if (bt_flash_Read_RF_Data_Address(pBtDevice,&rf_data_address) == BT_FUNCTION_SUCCESS)
    {

           FLASH_CONFIG_ADDR = rf_data_address;
    }
    //add
    if (BTDevice_Find_Config_Length(pBtDevice,&flashcurrentlength)!= BT_FUNCTION_SUCCESS)
            goto error;


    if(bt_Flash_Erase_Page(pBtDevice, FLASH_CONFIG_ADDR) != BT_FUNCTION_SUCCESS)
        goto error;

    //modify header
    if (flashcurrentlength !=0 )
    {
        flashcurrentlength = (-1)*flashcurrentlength;
        if (BTDevice_Write_SECUREDATA_to_Flash_Config(pBtDevice,flashcurrentlength)!= BT_FUNCTION_SUCCESS)
                goto error;
    }
#if 1
    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen))
            goto error;
    }
#endif

    SYSLOGI("-BTDevice_Erase_ConfigPage_Entry_bp_level SUCCESS.. \n");
    usleep(20000);
    return BT_FUNCTION_SUCCESS;

error:
    SYSLOGI("-BTDevice_Erase_ConfigPage_Entry_bp_level Error.. \n");

    return FUNCTION_ERROR;

}
//----------------------------------------------------------------------------
/*
typedef struct _IMG_CTRL_HEADER_FORMAT
{
    uint8_t ic_type;
    uint8_t secure_version;
    union
    {
        uint16_t value;
        struct
        {
            uint16_t xip: 1; // payload is executed on flash
            uint16_t enc: 1; // all the payload is encrypted
            uint16_t load_when_boot: 1; // load image when boot
            uint16_t enc_load: 1; // encrypt load part or not
            uint16_t enc_key_select: 3; // referenced to ENC_KEY_SELECT
            uint16_t rsvd: 9;
        };
    } ctrl_flag;
    uint16_t image_id;
    uint16_t crc16;
    uint32_t payload_len;
} T_IMG_CTRL_HEADER_FORMAT;
*/

#define FLASH_E_CUT_AMZM_CONFIG_SECURE_ADDR   0X01802000
int
BTDevice_Write_SECUREDATA_to_Flash_Config(
    BT_DEVICE *pBtDevice,
    int Add_length
    )
{
    unsigned char *pFlashBuf = NULL;
    unsigned int i;
    int n=0;
    //unsigned int WritingLen = 0;
    unsigned long payload_length=0;
    int rtn = FUNCTION_ERROR;
    unsigned int EvtLen;

    unsigned int len;
    //unsigned char signature[LEN_4_BYTE]= {0xb3, 0x5b, 0x34, 0x12}; //0x12345bb3

    //unsigned char pData[12]= {0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    unsigned char pData[12] = { 0x05,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00 }; //unclock flash address 0x800000
    unsigned char bp_level[4] = { 0 };
    //[e]0x2c,0xfc,12,0x02,0x10,0x00,0x00,[0x02] if 0 is BP,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
    unsigned char pEvtBuf[LEN_512_BYTE];
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;
    SYSLOGI("+BTDevice_Write_SECUREDATA_to_Flash_Config()....\n");

    if(pBTInfo->Version > 3) //BBPro D cut version
    {
#if 1
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData, 0x0E, pEvtBuf, &EvtLen)!= BT_FUNCTION_SUCCESS)
            goto exit;

        for (i=0; i < 4; i++)
        {
            bp_level[i] = pEvtBuf[i + 6]; //copy event parameter to bp_level, event code(1)+len(1)+ num_of_cmd(1) + cmd(2) + status(1)
        }
#endif
        //define secure header address :0x1802000
        FLASH_CONFIG_ADDR = FLASH_E_CUT_AMZM_CONFIG_SECURE_ADDR;

    }
    else
    {
        SYSLOGI("Skip BTDevice_Write_SECUREDATA_to_Flash_Config()\n");
        rtn = BT_FUNCTION_SUCCESS;
        goto exit;
    }

    pFlashBuf = (unsigned char *)malloc(1024*5*sizeof(unsigned char));
    if (pFlashBuf == NULL)
    {
        SYSLOGI("allocate pFlashBuf error\n");
        goto exit;
    }

    memset(pFlashBuf, 0xFF, Flash_page_size_len*sizeof(unsigned char));

    //read 0x1802000
    if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR, Flash_page_size_len, pFlashBuf) != BT_FUNCTION_SUCCESS)
        goto exit;

       //erase 0x1802000
    if(bt_Flash_Erase_Page(pBtDevice, FLASH_CONFIG_ADDR) != BT_FUNCTION_SUCCESS)
        goto exit;

    i = 0;

    //Find length
    while (i < Flash_page_size_len)
    {
        if ((pFlashBuf[i+0] == 0xff) &&
            (pFlashBuf[i+1] == 0xff) &&
            (pFlashBuf[i+2] == 0xff) &&
            (pFlashBuf[i+3] == 0xff) &&
            (pFlashBuf[i+4] == 0xff) &&
            (pFlashBuf[i+5] == 0xff) &&
            (pFlashBuf[i+6] == 0xff) &&
            (pFlashBuf[i+7] == 0xff) )
        {

            break;
        }
        else
        {
            i++;
        }
    }


    SYSLOGI("-->flash current data length = 0x%x , config set lenght = 0x%x\n",i, len);
    if( i==0 ) //flash empty
    {
        SYSLOGI("-->flash empty \n");
        rtn = BT_FUNCTION_SUCCESS;
        goto exit;
    }
    else if ( i > (Flash_page_size_len - 16))  // reserved 16 byte ,>flash fill
    {

          SYSLOGI("-->flash fill i=%x \n" , i );
          goto exit;
    }

    //Set 0xEE0 & 0xFF0
    memset(&pFlashBuf[0xee0],0xFF,16);
    memset(&pFlashBuf[0xFF0],0xFF,16);


       //get current payload  length
    payload_length=0;
    for(n=0;n<4;n++)
    {
        payload_length = payload_length<<8;
        payload_length = payload_length+(unsigned long)(pFlashBuf[11-n]);
    }
    //set new payload length
    if (Add_length <0 )
    {
        if ((unsigned long)abs(Add_length) > payload_length )
             payload_length =0;
        else
            payload_length -=(abs(Add_length));
    }
    else
        payload_length += Add_length;

    pFlashBuf[8]  =(unsigned char) ( payload_length      &0xFF);
    pFlashBuf[9]  =(unsigned char) ((payload_length>>8)  &0xFF);
    pFlashBuf[10] =(unsigned char) ((payload_length>>16) &0xFF);
    pFlashBuf[11] =(unsigned char) ((payload_length>>24) &0xFF);


    //recove data
    if(bt_Flash_Write_Buf(pBtDevice, FLASH_CONFIG_ADDR, Flash_page_size_len, pFlashBuf) != BT_FUNCTION_SUCCESS) //Keyword(4)+total len(2)
        goto exit;


#if 1
    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen))
            goto exit;
    }
#endif
    usleep(20000);

    SYSLOGI("-BTDevice_Write_SECUREDATA_to_Flash_Config()....\n");
    rtn = BT_FUNCTION_SUCCESS;

exit:

    free(pFlashBuf);

    return  rtn ;

}
int
BTDevice_Find_Config_Length(
    BT_DEVICE *pBtDevice,
    unsigned long *length
    )
{
    unsigned char *pFlashBuf = NULL;
    unsigned int i;
    unsigned char signature[LEN_4_BYTE]= {0xb3, 0x5b, 0x34, 0x12}; //0x12345bb3
    unsigned long rf_data_address=0;
    //unsigned int EvtLen;
    //unsigned char pData[12]= {0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    //unsigned char pData[12] = { 0x05,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00 }; //unclock flash address 0x800000
    //unsigned char bp_level[4] = { 0 };
    //[e]0x2c,0xfc,12,0x02,0x10,0x00,0x00,[0x02] if 0 is BP,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
    //unsigned char pEvtBuf[LEN_512_BYTE];
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;
    SYSLOGI("+BTDevice_Find_Config_Length()....\n");

    if(pBTInfo->Version > 3) //BBPro D cut version
    {
#if 0
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData, 0x0E, pEvtBuf, &EvtLen)!= BT_FUNCTION_SUCCESS)
            goto error;

        for (i=0; i < 4; i++)
        {
            bp_level[i] = pEvtBuf[i + 6]; //copy event parameter to bp_level, event code(1)+len(1)+ num_of_cmd(1) + cmd(2) + status(1)
        }
#endif
    //  FLASH_CONFIG_ADDR = FLASH_D_CUT_CONFIG_ADDR;
        FLASH_CONFIG_ADDR = FLASH_E_CUT_CONFIG_ADDR + gHEADER_SIZE;
    }
    else
    {
        FLASH_CONFIG_ADDR = FLASH_C_CUT_CONFIG_ADDR;
    }

    /*--------------------------------------------------------*/
    // for new flow : 20180822
    //
    if (bt_flash_Read_RF_Data_Address(pBtDevice,&rf_data_address) == BT_FUNCTION_SUCCESS)
    {

           FLASH_CONFIG_ADDR = rf_data_address;
    } 


    /**********************************************************/
    pFlashBuf = (unsigned char *)malloc(1024*5*sizeof(unsigned char));
    if (pFlashBuf == NULL)
    {
        SYSLOGI("allocate pFlashBuf error\n");
        goto error;
    }

    memset(pFlashBuf, 0xFF, Flash_page_size_len*sizeof(unsigned char));

    if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR, Flash_page_size_len, pFlashBuf) != BT_FUNCTION_SUCCESS)
        goto error;


    //if(bt_Flash_Erase_Page(pBtDevice, FLASH_CONFIG_ADDR) != BT_FUNCTION_SUCCESS)
    //  goto error;

    i = 0;

    //Find length
    while (i < Flash_page_size_len)
    {
        if ((pFlashBuf[i+0] == 0xff) &&
            (pFlashBuf[i+1] == 0xff) &&
            (pFlashBuf[i+2] == 0xff) &&
            (pFlashBuf[i+3] == 0xff) &&
            (pFlashBuf[i+4] == 0xff) &&
            (pFlashBuf[i+5] == 0xff) &&
            (pFlashBuf[i+6] == 0xff) &&
            (pFlashBuf[i+7] == 0xff) )
        {
            break;
        }
        else
        {
            i++;
        }
    }


    //wrong signature header
    if(memcmp(pFlashBuf, signature, LEN_4_BYTE) !=0)
    {
        i=0;
    }
      *length = i;
    SYSLOGI("-->flash current data length = 0x%x\n",i);

#if 0
    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen))
            goto error;
    }
#endif

    SYSLOGI("-BTDevice_Find_Config_Length()....\n");
    return BT_FUNCTION_SUCCESS;

error:

    free(pFlashBuf);

    return FUNCTION_ERROR;

}
int
bt_Flash_Set_Header_Size(
    BT_DEVICE *pBtDevice,
    unsigned long size
    )
{
    SYSLOGI("bt_Flash_Set_Header_Size() %d",pBtDevice->InterfaceType);
    gHEADER_SIZE = size;
    Flash_page_size_len = size;
    return BT_FUNCTION_SUCCESS;
}

/*
#define  FLASH_SIZE_COUNT          8
#define  FALSH_CONFIG_SET_SIZE 1024*FLASH_SIZE_COUNT
#define  DUMMY_FALSH_CONFIG_SET_SIZE 1024*(FLASH_SIZE_COUNT+1)

int
BTDevice_Write_Data_Entry_to_Flash_Config_8k(
    BT_DEVICE *pBtDevice,
    unsigned char *pWritingEntry
    )
{
    unsigned char *pFlashBuf = NULL;
    unsigned int i;
    unsigned int FlashWritingLen = 0;
    unsigned int ConfigDataLen = 0;
    unsigned int WritingLen = 0;
    unsigned int EndAlignment;
    unsigned char Add_length=0;
    unsigned int EvtLen;

    unsigned int len;
    unsigned char signature[LEN_4_BYTE]= {0xb3, 0x5b, 0x34, 0x12}; //0x12345bb3

    //unsigned char pData[12]= {0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    unsigned char pData[12] = { 0x05,0x10,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00 }; //unclock flash address 0x800000
    unsigned char bp_level[4] = { 0 };
    //[e]0x2c,0xfc,12,0x02,0x10,0x00,0x00,[0x02] if 0 is BP,0x00,0x00,0x00,0x00,0x00,0x00,0x00;
    unsigned char pEvtBuf[LEN_512_BYTE];
    BT_CHIPINFO *pBTInfo = pBtDevice->pBTInfo;

    if(pBTInfo->Version > 3) //BBPro D cut version
    {
#if 1
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if(pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData, 0x0E, pEvtBuf, &EvtLen)!= BT_FUNCTION_SUCCESS)
            goto error;

        for (i=0; i < 4; i++)
        {
            bp_level[i] = pEvtBuf[i + 6]; //copy event parameter to bp_level, event code(1)+len(1)+ num_of_cmd(1) + cmd(2) + status(1)
        }
#endif
//      if (pBTInfo->Version == 4)  //Dcut
//      FLASH_CONFIG_ADDR = FLASH_D_CUT_CONFIG_ADDR;
//      else
        FLASH_CONFIG_ADDR = FLASH_E_CUT_CONFIG_ADDR ;

    }
    else
    {
        FLASH_CONFIG_ADDR = FLASH_C_CUT_CONFIG_ADDR;
    }

    pFlashBuf = (unsigned char *)malloc(DUMMY_FALSH_CONFIG_SET_SIZE*sizeof(unsigned char));
    if (pFlashBuf == NULL)
    {
        SYSLOGI("allocate pFlashBuf error\n");
        goto error;
    }

    memset(pFlashBuf, 0xFF, DUMMY_FALSH_CONFIG_SET_SIZE*sizeof(unsigned char));

    if(bt_Flash_Read_Buf(pBtDevice, FLASH_CONFIG_ADDR, FALSH_CONFIG_SET_SIZE, pFlashBuf) != BT_FUNCTION_SUCCESS)
        goto error;


    if(bt_Flash_Erase_Page(pBtDevice, FLASH_CONFIG_ADDR) != BT_FUNCTION_SUCCESS)
        goto error;

    i = 0;

    //Find length
    while (i < Flash_page_size_len)
    {
        if ((pFlashBuf[i+0] == 0xff) &&
            (pFlashBuf[i+1] == 0xff) &&
            (pFlashBuf[i+2] == 0xff) &&
            (pFlashBuf[i+3] == 0xff) &&
            (pFlashBuf[i+4] == 0xff) &&
            (pFlashBuf[i+5] == 0xff) &&
            (pFlashBuf[i+6] == 0xff) &&
            (pFlashBuf[i+7] == 0xff) )
        {
            break;
        }
        else
        {
            i++;
        }
    }

    len = pWritingEntry[0]+(pWritingEntry[1]<<8);
    Add_length = len;

    //wrong signature header
    if(memcmp(pFlashBuf, signature, LEN_4_BYTE) !=0)
    {
        i=0;
    }

    SYSLOGI("-->flash current data length = 0x%x , config set lenght = 0x%x\n",i, len);
    if( i==0 ) //flash empty
    {
        ConfigDataLen = 0;
        memcpy(pFlashBuf, signature, LEN_4_BYTE);
        pFlashBuf[4] = pWritingEntry[0];
        pFlashBuf[5] = pWritingEntry[1];
        FlashWritingLen += LEN_6_BYTE;
        ConfigDataLen += len;
        SYSLOGI("-->flash empty \n");
    }
    else if ( i > (Flash_page_size_len - 16))  // reserved 16 byte ,>flash fill
    {
          SYSLOGI("-->flash fill i=%x \n" , i );
          goto error;
    }
    else //flash is not empty
    {
        ConfigDataLen = (pFlashBuf[5]<<8)+pFlashBuf[4];
        FlashWritingLen = ConfigDataLen+LEN_6_BYTE;
        ConfigDataLen += len;
        pFlashBuf[4] = ConfigDataLen&0xff;
        pFlashBuf[5] = (unsigned char)((ConfigDataLen>>8)&0xFF);

        SYSLOGI("-->flash is not empty, len:%x, configdatalen:%x\n", len, ConfigDataLen);
    }

    memcpy(pFlashBuf+FlashWritingLen, pWritingEntry+2, len);


    //flash addr & len 4 bytes alignment
    WritingLen = ConfigDataLen+LEN_6_BYTE;
    EndAlignment = WritingLen%LEN_4_BYTE;
    WritingLen = (EndAlignment == 0) ? WritingLen : WritingLen+(LEN_4_BYTE-EndAlignment);
    SYSLOGI("merged flash entry:%x, %x", ConfigDataLen, WritingLen);
    for (i = 0; i < (FlashWritingLen + len); i++)
    {
        SYSLOGI("i:%x, %x\r\n", i, pFlashBuf[i]);
    }
    if(bt_Flash_Write_Buf(pBtDevice, FLASH_CONFIG_ADDR, WritingLen, pFlashBuf) != BT_FUNCTION_SUCCESS) //Keyword(4)+total len(2)
        goto error;


#if 1
    if (pBTInfo->Version > 3) //BBPro D cut version
    {
        unsigned char pData_lock[12] = { 0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
        for (i = 0; i < 4; i++)
        {
            pData_lock[4 + i] = bp_level[i]; //copy event parameter to bp_level
        }
        //0x2c,0xfc,12,0x02,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  -->KEY
        if (pBtDevice->SendHciCommandWithEvent(pBtDevice, 0xfc2c, 12, pData_lock, 0x0E, pEvtBuf, &EvtLen))
            goto error;
    }
#endif
    usleep(20000);
    // add
    if (BTDevice_Write_SECUREDATA_to_Flash_Config(pBtDevice,Add_length)!= BT_FUNCTION_SUCCESS)
     goto error;

    SYSLOGI("-BTDevice_Write_Data_Entry_to_Flash_Config()....\n");
    return BT_FUNCTION_SUCCESS;

error:

    free(pFlashBuf);

    return FUNCTION_ERROR;

}
*/

//--------------------------------------------  temp flash Coding ------------------------------------

int  BT_Device_FlashtempBuf_AddEntry(   BT_DEVICE *pBtDevice,unsigned char *pWritingEntry,unsigned long WritingEntry_Length )
{
       int rtn = BT_FUNCTION_SUCCESS;
    SYSLOGI(" BT_Device_FlashtempBuf_AddEntry %d",pBtDevice->InterfaceType);
    if ((WritingEntry_Length + flash_WriteTempData_length+2)   >  FLASH_WRITETEMPBUFFER_SIZE)
    {
        rtn =FUNCTION_ERROR;
        goto exit;
    }

    memcpy(&flash_WriteTempData[flash_WriteTempData_length+2], pWritingEntry, WritingEntry_Length);

    flash_WriteTempData_length += WritingEntry_Length;

exit:

    return rtn;
}
void  BT_Device_FlashtempBuf_CleanBuffer(BT_DEVICE *pBtDevice)
{
       SYSLOGI(" BT_Device_FlashtempBuf_CleanBuffer %d",pBtDevice->InterfaceType);
       memset(flash_WriteTempData,0xFF,FLASH_WRITETEMPBUFFER_SIZE);
       flash_WriteTempData[0] = 0x00;
       flash_WriteTempData[1]  =0x00;

       flash_WriteTempData_length=0;

}
int  BT_Device_FlashtempBuf_WriteToDevice(BT_DEVICE *pBtDevice)
{

        flash_WriteTempData[0]=flash_WriteTempData_length &0xFF;
        flash_WriteTempData[1]=(flash_WriteTempData_length>>8)&0xFF;
        if (BTDevice_Write_Data_Entry_to_Flash_Config(pBtDevice,flash_WriteTempData) != BT_FUNCTION_SUCCESS)
            return FUNCTION_ERROR;

       return  BT_FUNCTION_SUCCESS;
}
int  bt_flash_Read_RF_Data_Address(BT_DEVICE *pBtDevice,unsigned long *rf_data_addr)
{
    int rtn= FUNCTION_ERROR;
    unsigned char pPayLoad[MAX_HCI_BUF_SIZE];
    unsigned char pEvent[MAX_HCI_BUF_SIZE];
    unsigned long addr = 0;
    //int n=0;
    unsigned int EvtLen;

    memset(pPayLoad,0,MAX_HCI_BUF_SIZE);
    memset(pEvent,0,MAX_HCI_BUF_SIZE);

    if(bt_default_SendHciCommandWithEvent(pBtDevice, HCI_VENDOR_GET_RF_DATA_FLASH_ADDR, LEN_8_BYTE, pPayLoad, 0x0E, pEvent, &EvtLen) == BT_FUNCTION_SUCCESS)
    {

            addr= (pEvent[6]) |  (pEvent[7]<<8) | (pEvent[8]<<16) | (pEvent[9]<<24) ;

          //  hci_event_pkt_ptr->event_parameter[param_length] = rf_data_addr & 0xff;

          //  hci_event_pkt_ptr->event_parameter[param_length + 1] = (rf_data_addr >> 8) & 0xff;

          //  hci_event_pkt_ptr->event_parameter[param_length + 2] =  (rf_data_addr >> 16) & 0xff;

          //  hci_event_pkt_ptr->event_parameter[param_length + 3] = (rf_data_addr >> 24) & 0xff;
            *rf_data_addr = addr;
            rtn = BT_FUNCTION_SUCCESS;
    }
    *rf_data_addr = addr;
     return rtn;
}
