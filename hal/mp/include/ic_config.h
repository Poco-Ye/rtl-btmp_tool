//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
// Realtek Bluetooth Tool : IC Config
//
// Copyright (c) 2008 Realtek Semiconductor Corp., Taiwan
//
// $Id: ic_config.h 2017-10-19 12:47:15Z scott $
//---------------------------------------------------------------------------
#ifndef __RTKBT_IC_CONFIG_H__
#define __RTKBT_IC_CONFIG_H__

#define FLASH_PAGE_SIZE_LEN (1024*4)
#define LEN_6_BYTE 6

typedef struct IC_CONFIG_ENTRY_T{
        unsigned short offset_;
        unsigned char len_;
        unsigned char data[256];
        unsigned char mask[256];
}IC_CONFIG_ENTRY;


typedef struct IC_CONFIG_T{
    unsigned int signature_;
    IC_CONFIG_ENTRY config_entry[1024];
    unsigned int config_entry_count;
}IC_CONFIG;


void
ic_config_parse(IC_CONFIG *p_ic_config, unsigned char *p_raw_data, unsigned int raw_data_size);

void
ic_config_set_signature(IC_CONFIG *p_ic_config, const size_t signature);

void
ic_config_set_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_data, unsigned char *p_mask);

void
ic_config_append_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_data, unsigned char *p_mask);

int
ic_config_update_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_data, unsigned char *p_mask);

int
ic_config_get_entry_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_mask, unsigned char *p_data);

void
ic_config_dump_writing_data(IC_CONFIG *p_ic_config, unsigned char *p_data, unsigned int *p_len);






#endif

