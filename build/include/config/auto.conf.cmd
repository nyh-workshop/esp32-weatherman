deps_config := \
	/c/esp/esp-idf/components/app_trace/Kconfig \
	/c/esp/esp-idf/components/aws_iot/Kconfig \
	/c/esp/esp-idf/components/bt/Kconfig \
	/c/esp/esp-idf/components/esp32/Kconfig \
	/c/esp/esp-idf/components/ethernet/Kconfig \
	/c/esp/esp-idf/components/fatfs/Kconfig \
	/c/esp/esp-idf/components/freertos/Kconfig \
	/c/esp/esp-idf/components/heap/Kconfig \
	/c/esp/esp-idf/components/libsodium/Kconfig \
	/c/esp/esp-idf/components/log/Kconfig \
	/c/esp/esp-idf/components/lwip/Kconfig \
	/c/esp/esp-idf/components/mbedtls/Kconfig \
	/c/esp/esp-idf/components/openssl/Kconfig \
	/c/esp/esp-idf/components/pthread/Kconfig \
	/c/esp/esp-idf/components/spi_flash/Kconfig \
	/c/esp/esp-idf/components/spiffs/Kconfig \
	/c/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/c/esp/esp-idf/components/wear_levelling/Kconfig \
	/c/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/c/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/d/esp32-projects/http_request/main/Kconfig.projbuild \
	/c/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/c/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
