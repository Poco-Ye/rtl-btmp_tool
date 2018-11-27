

#include <stdio.h>
#include <string.h>
#include "ic_config.h"



//---------------------------------------------------------------------------
void
ic_config_parse(IC_CONFIG *p_ic_config, unsigned char *p_raw_data, unsigned int raw_data_size)
{
    unsigned int i, data_len, mask_len;
    unsigned short offset;
    //unsigned int total_len = 0;

    for( i = 0; i <raw_data_size; )
    {
        offset = 0;

        offset |= p_raw_data[i];
        i++;

        offset |= p_raw_data[i]<<8;
        i++;

        if (0xFFFF == offset) break;

        data_len = p_raw_data[i];
        mask_len = data_len;
        i++;

        ic_config_set_data(p_ic_config, offset, data_len, p_raw_data+i, p_raw_data+i+data_len);

        i +=(data_len*2);

    }

/*

    // parsing raw Config
    vec_byte_t pars_dt = raw_data;
    vec_byte_t::iterator it = pars_dt.begin() + CFG_HEADER_SIZE;
    //Offset (2 bytes) + Length (1 byte) + Data [n] + Mask [n] (Mask size the same as Data)
    for (; it != pars_dt.end();) {
        // offset
        ushort_t offset = 0;
        offset |= *it;
        it++;
        offset |= *it << 8;
        it++;
        if (0xFFFF == offset) break;
        // length
        byte_t data_len = *it;
        byte_t mask_len = data_len;
        it++;
        // data
        vec_byte_t::iterator dt_it = it;
        vec_byte_t::iterator dt_end = it + data_len;
        vec_byte_t data(dt_it, dt_end);
        it += data_len;
        // mask
        vec_byte_t::iterator mk_it = it;
        vec_byte_t::iterator mk_end = it + mask_len;
        vec_byte_t mask(mk_it, mk_end);
        it += mask_len;

        enties_.set_data(entry_t(offset, data, mask));
    }
*/
}
//---------------------------------------------------------------------------
void
ic_config_set_signature(IC_CONFIG *p_ic_config, const size_t signature)
{
    p_ic_config->signature_ = signature;
    p_ic_config->config_entry_count = 0;
}
//---------------------------------------------------------------------------

void
ic_config_set_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_data, unsigned char *p_mask)
{
    if (!ic_config_update_data(p_ic_config, offset, len, p_data, p_mask)) {
        ic_config_append_data(p_ic_config, offset, len, p_data, p_mask);
    }
}


//---------------------------------------------------------------------------
void
ic_config_append_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_data, unsigned char *p_mask)
{
    unsigned int config_entry_count;

    config_entry_count = p_ic_config->config_entry_count;

    p_ic_config->config_entry[config_entry_count].offset_ = offset;
    p_ic_config->config_entry[config_entry_count].len_ = len;
    memcpy(p_ic_config->config_entry[config_entry_count].data, p_data, len);
    memcpy(p_ic_config->config_entry[config_entry_count].mask, p_mask, len);
    p_ic_config->config_entry_count++;
}


//---------------------------------------------------------------------------
int
ic_config_update_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_data, unsigned char *p_mask)
{

/*
    vec_entry_t::iterator e_begin = enties_.begin();
    vec_entry_t::iterator e_end = enties_.end();
    vec_entry_t::iterator e;

    for( e = e_begin; e!=e_end; e++)
    {
        if (e->offset_ == offset && e->mask_ == mask) {
            e->data_ = data;
            return true;
        }
    }
    return false;
*/
    unsigned int i;
    unsigned int config_entry_count;

    config_entry_count = p_ic_config->config_entry_count;

    for( i = 0; i < config_entry_count; i++)
    {
        if( (p_ic_config->config_entry[i].offset_ == offset) &&
            (p_ic_config->config_entry[i].len_ == len) &&
            memcmp(p_ic_config->config_entry[i].mask, p_mask, len) == 0 )
        {
            memcpy(p_ic_config->config_entry[i].data, p_data, len);
            return 1;
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
int
ic_config_get_entry_data(IC_CONFIG *p_ic_config, const unsigned short offset, unsigned char len, unsigned char *p_mask, unsigned char *p_data)
{
/*
    vec_entry_t::iterator e_begin = enties_.begin();
    vec_entry_t::iterator e_end = enties_.end();
    vec_entry_t::iterator e;

    for( e = e_begin; e!=e_end; e++)
    {
        if (offset == e->offset_ && mask == e->mask_) {
            return *e;
        }
    }
    return entry_t();
*/
    unsigned int i;
    unsigned int config_entry_count;

    config_entry_count = p_ic_config->config_entry_count;

    for( i = 0; i < config_entry_count; i++)
    {
        if( (p_ic_config->config_entry[i].offset_ == offset) &&
            (p_ic_config->config_entry[i].len_ == len) &&
            memcmp(p_ic_config->config_entry[i].mask, p_mask, len) == 0 )
        {
            memcpy(p_data, p_ic_config->config_entry[i].data, len);
            return 0;

        }
    }

    return 1;

}
//---------------------------------------------------------------------------

void
ic_config_dump_writing_data(IC_CONFIG *p_ic_config, unsigned char *p_data, unsigned int *p_len)
{
/*
    vec_entry_t::iterator e_begin = enties_.begin();
    vec_entry_t::iterator e_end = enties_.end();
    vec_entry_t::iterator e;

    vec_byte_t total_data;
    ushort_t total_len = 0;
    for( e = e_begin; e!=e_end; e++)
    {
        const vec_byte_t bytes = e->dump_data_bytes();
        total_len += bytes.size();
        total_data.insert(total_data.end(), bytes.begin(), bytes.end());
    }

    vec_byte_t rbuf;
    rbuf.push_back(byte_t(signature_));
    rbuf.push_back(byte_t(signature_ >> 8));
    rbuf.push_back(byte_t(signature_ >> 16));
    rbuf.push_back(byte_t(signature_ >> 24));

    rbuf.push_back(byte_t(total_len));
    rbuf.push_back(byte_t(total_len >> 8));

    rbuf.insert(rbuf.end(), total_data.begin(), total_data.end());

    return rbuf;
*/

    unsigned int i, j;
    unsigned int config_entry_count;
    unsigned char tmp[FLASH_PAGE_SIZE_LEN];

    config_entry_count = p_ic_config->config_entry_count;

    p_data[0] = (unsigned char) (p_ic_config->signature_ & 0xff);
    p_data[1] = (unsigned char) (p_ic_config->signature_>>8) & 0xff;
    p_data[2] = (unsigned char) (p_ic_config->signature_>>16) & 0xff;
    p_data[3] = (unsigned char) (p_ic_config->signature_>>24) & 0xff;

    j = 0;
    for( i = 0; i < config_entry_count; i++)
    {
        tmp[j++] = (unsigned char) (p_ic_config->config_entry[i].offset_ & 0xff);
        tmp[j++] = (unsigned char) (p_ic_config->config_entry[i].offset_>>8 & 0xff);
        tmp[j++] = p_ic_config->config_entry[i].len_;

        memcpy(tmp+j, p_ic_config->config_entry[i].data, p_ic_config->config_entry[i].len_);
        j += p_ic_config->config_entry[i].len_;

        memcpy(tmp+j, p_ic_config->config_entry[i].mask, p_ic_config->config_entry[i].len_);
        j += p_ic_config->config_entry[i].len_;

    }

    p_data[4] = (unsigned char) (j & 0xff);
    p_data[5] = (unsigned char)( (j>>8) & 0xff);

    memcpy(p_data+6, tmp, j);

    *p_len = j+LEN_6_BYTE;

    return;

}

