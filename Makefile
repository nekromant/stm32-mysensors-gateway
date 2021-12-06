help:
	@echo "Available make targets:"
	@echo "flash-stm32-gateway - build and flash stm32 board in gateway mode (via dfu-util)"
	@echo "rfnano-gateway - build and flash rfnano board in gateway mode (via avrdude)"
	@echo "rules-install - install udev rules"

flash-%:
	pio lib install
	pio run -t upload -e $*

install:
	cp 99-mysensors.rules /etc/udev/rules.d/
	udevadm control --reload-rules
	udevadm trigger

clean:
	-rm -Rfv .pio 