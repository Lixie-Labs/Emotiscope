#define BUF_SIZE (1024)
uint8_t uart_data[BUF_SIZE];
uint16_t uart_data_index = 0;

void check_serial(){
	
	/*
	unsigned char c;
	if (usb_serial_jtag_read_bytes(&c, 1, 0)){
		if (c == '\n'){
			uart_data[uart_data_index] = 0;
			usb_serial_jtag_write_bytes(uart_data, uart_data_index, 0);
			printf("testing printf\n");
			ESP_LOGI(TAG, "Received: %s", uart_data);
			uart_data_index = 0;
		} else {
			uart_data[uart_data_index] = c;
			uart_data_index++;
		}
	}
	*/
}

void init_serial(){
	/*
	ESP_LOGI(TAG, "init_serial()");

    // Configure USB SERIAL JTAG
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
        .rx_buffer_size = BUF_SIZE,
        .tx_buffer_size = BUF_SIZE,
    };

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
    ESP_LOGI("usb_serial_jtag echo", "USB_SERIAL_JTAG init done");
	*/
}