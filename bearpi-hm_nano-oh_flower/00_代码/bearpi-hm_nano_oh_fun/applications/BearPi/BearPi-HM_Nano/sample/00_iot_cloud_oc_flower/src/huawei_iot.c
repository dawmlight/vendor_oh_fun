#include "huawei_iot.h"

void huawei_iot_init(void)
{
    dtls_al_init();
    mqtt_al_init();
    oc_mqtt_init();   
}
