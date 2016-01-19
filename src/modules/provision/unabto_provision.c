/*
 * Copyright (C) 2008-2015 Nabto - All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unabto_config.h"

#if NABTO_ENABLE_PROVISIONING

#include "unabto_provision.h"
#include "unabto_provision_http.h"
#include "unabto_provision_file.h"

bool unabto_provision_execute(nabto_main_setup* nms, provision_context_t* context) {
    uint8_t key[KEY_BUFFER_SIZE];
    unabto_provision_status_t status = unabto_provision_http(nms, context, key);
    if (status != UPS_OK) {
        switch (status) {
        case UPS_PROV_ALREADY_PROVISIONED:
            NABTO_LOG_FATAL(("Device is already provisioned - customer service must be contacted"));
            break;
        case UPS_PROV_INVALID_TOKEN:
            NABTO_LOG_FATAL(("Invalid user token specified to provisioning service - check token or contact customer service"));
            break;
        default:
            NABTO_LOG_FATAL(("Provisoning failed with status [%d]", status));
            break;
        }
        return false;
    } else {
        return true;
    }
}

bool unabto_provision_set_key(nabto_main_setup *nms, char *key)
{
    size_t i;
    size_t pskLen = strlen(key);

    if (!key || pskLen != PRE_SHARED_KEY_SIZE * 2) {
        NABTO_LOG_ERROR(("Invalid key: %s", key));
        return false;
    }

    // Read the pre shared key as a hexadecimal string.
    for (i = 0; i < pskLen/2 && i < 16; i++) {
        sscanf(key+(2*i), "%02hhx", &nms->presharedKey[i]);
    }
    nms->secureAttach= true;
    nms->secureData = true;
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    return true;
}

bool unabto_provision_try_existing(nabto_main_setup *nms,
                                   provision_context_t* context,
                                   const char* path)
{
    char text[256];
    // If persistent file exists, use it
    if (unabto_provision_read_file(path, text, sizeof(text))) {
        return unabto_provision_parse_data(nms, text, nms->presharedKey);
    }

    NABTO_LOG_INFO(("Provisioning file not found, creating"));

    if (!unabto_provision_execute(nms, context)) {
        return false;
    }

    return unabto_provision_write_file(path, nms);
    
}

#endif
