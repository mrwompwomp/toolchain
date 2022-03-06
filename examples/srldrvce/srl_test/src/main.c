/*
 *--------------------------------------
 * Program Name:
 * Author:
 * License:
 * Description:
 *--------------------------------------
*/

#include <srldrvce.h>

#include <debug.h>
#include <keypadc.h>
#include <stdbool.h>
#include <string.h>
#include <tice.h>

void get_device_type_(srl_device_t *srl);
void get_endpoint_addresses_(srl_device_t *srl);
size_t ring_buf_contig_avail_(ring_buf_ctrl_t *rbuf);
bool ring_buf_has_consecutive_region_(ring_buf_ctrl_t *rbuf, uint8_t size);
size_t ring_buf_push_(ring_buf_ctrl_t *rbuf, void *data, size_t size);
size_t ring_buf_pop_(ring_buf_ctrl_t *rbuf, void *data, size_t size);
void ring_buf_update_read_(ring_buf_ctrl_t *rbuf, size_t size, uint8_t region);
void ring_buf_update_write_(ring_buf_ctrl_t *rbuf, size_t size);

void print_ring_buf(const ring_buf_ctrl_t *rbuf) {
    printf("  buf_start: %p\n", rbuf->buf_start);
    printf("  buf_end: %p\n", rbuf->buf_end);
    printf("  data_start: %p\n", rbuf->data_start);
    printf("  data_break: %p\n", rbuf->data_break);
    printf("  data_end: %p\n", rbuf->data_end);
    printf("  dma_active: %i\n", rbuf->dma_active);
}

void print_srl_dev(const srl_device_t *srl) {
    printf("dev: %p\n", srl->dev);
    printf("rx_addr: %x\n", srl->rx_addr);
    printf("tx_addr: %x\n", srl->tx_addr);
    printf("type: %i\n", srl->type);
    printf("subtype: %i\n", srl->subtype);
    printf("tx_buf:\n");
    print_ring_buf(&srl->tx_buf);
    printf("rx_buf:\n");
    print_ring_buf(&srl->rx_buf);
}

srl_device_t srl;

usb_device_t device = NULL;

bool has_srl_device = false;

uint8_t srl_buf[512];

static usb_error_t handle_usb_event(usb_event_t event, void *event_data,
                                    usb_callback_data_t *callback_data __attribute__((unused))) {
    /* Enable newly connected devices */
    if(event == USB_DEVICE_CONNECTED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) {
        usb_device_t device = event_data;
        printf("device connected\n");
        usb_ResetDevice(device);
    }
    /* When a device is connected, or when connected to a computer */
    if((event == USB_DEVICE_ENABLED_EVENT && !(usb_GetRole() & USB_ROLE_DEVICE)) || event == USB_HOST_CONFIGURE_EVENT) {
        device = event_data;
    }
    if(event == USB_DEVICE_DISCONNECTED_EVENT) {
        srl_Close(&srl);
        has_srl_device = false;
        device = NULL;
    }

    return USB_SUCCESS;
}

int main(void) {
    os_ClrHome();
    const usb_standard_descriptors_t *desc = srl_GetCDCStandardDescriptors();
    /* Initialize the USB driver with our event handler and the serial device descriptors */
    //usb_error_t usb_error = 
    usb_Init(handle_usb_event, NULL, desc, USB_DEFAULT_INIT_FLAGS);
    //if(usb_error) {
    //    usb_Cleanup();
    //    printf("usb init error %u\n", usb_error);
    //    do kb_Scan(); while(!kb_IsDown(kb_KeyClear));
    //    return 1;
    //}

    do {
        kb_Scan();
        
        usb_HandleEvents();

        if(device && !has_srl_device) {
            //printf("device enabled\n");

            /* Initialize the serial library with the newly attached device */
            srl_error_t error = srl_Open(&srl, device, srl_buf, sizeof srl_buf, SRL_INTERFACE_ANY, 9600);

            if(error) {
                /* Print the error code to the homescreen */
                printf("Error %u initting serial\n", error);
            }
            has_srl_device = true;

            print_srl_dev(&srl);
        }

        if(has_srl_device) {
            char in_buf[64];

            if(kb_IsDown(kb_KeyMath)) {
                size_t bytes_read = srl_Read(&srl, in_buf, sizeof in_buf);
                printf("r %u\n", bytes_read);
                while(kb_IsDown(kb_KeyMath)) kb_Scan();
            }

            if(kb_IsDown(kb_KeyApps)) {
                srl_Write(&srl, "yeet\r\n", 6);
                while(kb_IsDown(kb_KeyApps)) kb_Scan();
            }

            if(kb_IsDown(kb_KeyPrgm)) {
                printf("ca %u\n", ring_buf_contig_avail_(&srl.rx_buf));
                while(kb_IsDown(kb_KeyPrgm)) kb_Scan();
            }

            if(kb_IsDown(kb_KeyVars)) {
                print_srl_dev(&srl);
                while(kb_IsDown(kb_KeyVars)) kb_Scan();
            }

            if(kb_IsDown(kb_KeySin)) {
                printf("%s\n", (char*)srl.tx_buf.data_start);
                while(kb_IsDown(kb_KeySin)) kb_Scan();
            }

            //size_t bytes_read = srl_Read(&srl, in_buf, sizeof in_buf);
//
            //if(bytes_read) {
            //    //srl_Write(&srl, in_buf, bytes_read);
            //}
        }

    } while(!kb_IsDown(kb_KeyClear));

    usb_Cleanup();
    return 0;
}
