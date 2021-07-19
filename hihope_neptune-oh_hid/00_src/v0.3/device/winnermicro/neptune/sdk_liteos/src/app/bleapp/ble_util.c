

#include "ble_util.h"

static unsigned char BASE_UUID[16] =
{
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int uuidType(unsigned char *p_uuid)
{
    int i = 0;
    int match = 0;
    int all_zero = 1;

    for(i = 0; i != 16; ++i)
    {
        if(i == 12 || i == 13)
        {
            continue;
        }

        if(p_uuid[i] == BASE_UUID[i])
        {
            ++match;
        }

        if(p_uuid[i] != 0)
        {
            all_zero = 0;
        }
    }

    if(all_zero)
    {
        return 0;
    }

    if(match == 12)
    {
        return 32;
    }

    if(match == 14)
    {
        return 16;
    }

    return 128;
}

void app_to_btif_uuid(tls_bt_uuid_t *p_dest, BtUuid *p_src)
{
    int i = 0;

    if(p_src->uuidLen == 16 || p_src->uuidLen == 32)
    {
        for(i = 0; i != 16; ++i)
        {
            p_dest->uu[i] = BASE_UUID[i];
        }
    }

    switch(p_src->uuidLen)
    {
        case 0:
            break;

        case 16:
            p_dest->uu[12] = p_src->uuid[0]& 0xff;  //endian???
            p_dest->uu[13] = p_src->uuid[1]& 0xff;
            break;

        case 32:
            p_dest->uu[12] = p_src->uuid[0]& 0xff;
            p_dest->uu[13] = p_src->uuid[1]& 0xff;
            p_dest->uu[14] = p_src->uuid[2]& 0xff;
            p_dest->uu[15] = p_src->uuid[3]& 0xff;
            break;

        case 128:
            for(i = 0; i != 16; ++i)
            {
                p_dest->uu[i] = p_src->uuid[i];
            }

            break;

        default:
            BLE_IF_DEBUG("Unknown UUID length %d!", p_src->uuidLen);
            break;
    }
}

void btif_to_app_uuid(BtUuid *p_dest, tls_bt_uuid_t *p_src)
{
    char *p_byte = (char *)p_src;
    int i = 0;
    p_dest->uuidLen = uuidType(p_src->uu);

    switch(p_dest->uuidLen)
    {
        case 16:
            p_dest->uuid[0] = p_src->uu[12];
            p_dest->uuid[1] = p_src->uu[13];
            break;

        case 32:
            p_dest->uuid[0] = p_src->uu[12];
            p_dest->uuid[1] = p_src->uu[13];
            p_dest->uuid[2] = p_src->uu[14];
            p_dest->uuid[3] = p_src->uu[15];            
            break;

        case 128:
            for(i = 0; i != 16; ++i)
            {
                p_dest->uuid[i] = p_byte[i];
            }

            break;

        default:
            BLE_IF_DEBUG("Unknown UUID length %d!", p_dest->uuidLen);
            break;
    }
}


static tls_bt_uuid_t app_base_uuid =
{
    {
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
    	0x00, 0x10, 0x00, 0x00, 0x34, 0x12, 0x00, 0x00
    }
};

uint16_t btif_uuid128_to_uuid16(tls_bt_uuid_t *uuid)
{
    uint16_t id = 0;
    memcpy(&id, uuid->uu+12, 2);
    return id;
}
tls_bt_uuid_t * btif_uuid16_to_uuid128(uint16_t uuid16)
{
	memcpy(app_base_uuid.uu+12, &uuid16, 2);
	return &app_base_uuid;
}

