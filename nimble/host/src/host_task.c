/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include "os/os.h"
#include "host/attr.h"
#include "host/host_task.h"
#ifdef ARCH_sim
#include "itf.h"
#endif


int ble_host_listen_enabled;

struct ble_host_connection ble_host_connections[BLE_HOST_MAX_CONNECTIONS];
int ble_host_num_connections;

static struct os_eventq host_task_evq;
//static struct ble_att_chan default_attr_chan; 
static struct os_callout ble_host_task_timer;

/**
 * Initialize the host portion of the BLE stack.
 */
int 
host_init(void)
{
    os_eventq_init(&host_task_evq);
    os_callout_init(&ble_host_task_timer, &host_task_evq, NULL);

#ifdef ARCH_sim
    if (ble_host_listen_enabled) {
        int rc;

        rc = ble_sim_listen(1);
        assert(rc == 0);
    }
#endif

    return 0;
}

struct ble_host_connection *
ble_host_find_connection(uint16_t con_handle)
{
    int i;

    for (i = 0; i < BLE_HOST_MAX_CONNECTIONS; i++) {
        if (ble_host_connections[i].bc_handle == con_handle) {
            return ble_host_connections + i;
        }
    }

    return NULL;
}

int
ble_host_send_data_connectionless(uint16_t con_handle, uint16_t cid,
                                  uint8_t *data, uint16_t len)
{
    int rc;

#ifdef ARCH_sim
    rc = ble_host_sim_send_data_connectionless(con_handle, cid, data, len);
#else
    rc = -1;
#endif

    return rc;
}

/**
 * XXX: This is only here for testing.
 */
int 
ble_host_poll(void)
{
    int rc;

#ifdef ARCH_sim
    rc = ble_host_sim_poll();
#else
    rc = -1;
#endif

    return rc;
}

void
ble_host_task_handler(void *arg)
{
    struct os_event *ev;

    os_callout_reset(&ble_host_task_timer, 50);

    /**
     * How do we decide what channels to listen on for data?  This must be 
     * configured to the host task.  Maintain a list of channels to 
     *
     */
    while (1) {
        ev = os_eventq_get(&host_task_evq);
        switch (ev->ev_type) {
            case OS_EVENT_T_TIMER:
                /* Poll the attribute channel */
                //ble_att_chan_poll(&default_attr_chan, &host_task_evq);            
                /* Reset callout, wakeup every 50ms */
                os_callout_reset(&ble_host_task_timer, 50);
                break;
            case BLE_HOST_EVENT_NEW_ATTR_CONN: 
                break;
        }
    }
}