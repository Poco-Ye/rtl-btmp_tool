#ifndef __BT_FLASH_RTL8763B_H
#define __BT_FLASH_RTL8763B_H


#include "bt_mp_base.h"

#define MAX_HCI_BUF_SIZE 260
#define READ_WRITE_BYTE_NUM_MAX 244

#define HCI_VENDOR_EFLASH_READ_BUFFER           0xFC2D
#define HCI_VENDOR_EFLASH_WRITE_BUFFER          0xFC2E
#define HCI_VENDOR_EFLASH_WRITE_READ_BUFFER     0xFC2F
#define HCI_VENDOR_EFLASH_ERASE 0xFC2B
#define HCI_VENDOR_GET_RF_DATA_FLASH_ADDR                    0xFD85

#define FLASH_PAGE_SIZE_LEN (1024*4)
#define FLASH_C_CUT_CONFIG_ADDR 0x00801000
#define FLASH_D_CUT_CONFIG_ADDR 0x01803000   //0x00803000
#define FLASH_E_CUT_CONFIG_ADDR 0x01802000







int
bt_Flash_Read_Buf(
    BT_DEVICE *pBtDevice,
    unsigned int Address,
    unsigned int Len,
    unsigned char *pReadingData
    );

int
bt_Flash_Write_Buf(
    BT_DEVICE *pBtDevice,
    unsigned int Address,
    unsigned int Len,
    unsigned char *pWritingData
    );

int
bt_Flash_Erase_Page(
    BT_DEVICE *pBtDevice,
    unsigned int Address
    );



int
BTDevice_Read_Data_from_Flash_Config_rtl8763B(
    BT_DEVICE *pBtDevice,
    unsigned char *pBuf,
    unsigned char *pReportData
    );

int
BTDevice_Write_Data_Entry_to_Flash_Config(
    BT_DEVICE *pBtDevice,
    unsigned char *pWritingEntry
    );

int
BTDevice_Erase_ConfigPage_Entry_bp_level(
    BT_DEVICE *pBtDevice
    );
int
BTDevice_Write_SECUREDATA_to_Flash_Config(
    BT_DEVICE *pBtDevice,
    int Add_length
    );

int
BTDevice_Find_Config_Length(
    BT_DEVICE *pBtDevice,
    unsigned long *length
    );
int
bt_Flash_Set_Header_Size(
    BT_DEVICE *pBtDevice,
    unsigned long size
    );

/*
int
BTDevice_Write_Data_Entry_to_Flash_Config_8K(
    BT_DEVICE *pBtDevice,
    unsigned char *pWritingEntry
    );
*/

int  BT_Device_FlashtempBuf_WriteToDevice(BT_DEVICE *pBtDevice);
void  BT_Device_FlashtempBuf_CleanBuffer(BT_DEVICE *pBtDevice);
int   BT_Device_FlashtempBuf_AddEntry(BT_DEVICE *pBtDevice,unsigned char *pWritingEntry,unsigned long WritingEntry_Length );
int  bt_flash_Read_RF_Data_Address(BT_DEVICE *pBtDevice,unsigned long *rf_data_addr);
#endif
