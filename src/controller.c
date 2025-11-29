#include "ps1/registers.h"
#include "controller.h"

static void delayMicroseconds(int time) {
	time = ((time * 271) + 4) / 8;

	__asm__ volatile(
		".set push\n"
		".set noreorder\n"
		"bgtz  %0, .\n"
		"addiu %0, -2\n"
		".set pop\n"
		: "+r"(time)
	);
}

void initControllerBus(void) {
	SIO_CTRL(0) = SIO_CTRL_RESET;
	SIO_MODE(0) = 0
		| SIO_MODE_BAUD_DIV1
		| SIO_MODE_DATA_8;
	SIO_BAUD(0) = F_CPU / 250000;
	SIO_CTRL(0) = 0
		| SIO_CTRL_TX_ENABLE
		| SIO_CTRL_RX_ENABLE
		| SIO_CTRL_DSR_IRQ_ENABLE;
}

static bool waitForAcknowledge() {
	for (int timeout = 120; timeout > 0; timeout -= 10) {
		if (IRQ_STAT & (1 << IRQ_SIO0)) {
			IRQ_STAT = ~(1 << IRQ_SIO0);
			SIO_CTRL(0) |= SIO_CTRL_ACKNOWLEDGE;
			return true;
		}
		delayMicroseconds(10);
	}
	return false;
}

static void selectPort(const int port) {
    if (port)
        SIO_CTRL(0) |= SIO_CTRL_CS_PORT_2;
    else
        SIO_CTRL(0) &= ~SIO_CTRL_CS_PORT_2;
}

static uint8_t exchangeByte(uint8_t value) {
	while (!(SIO_STAT(0) & SIO_STAT_TX_NOT_FULL))
		__asm__ volatile("");

	SIO_DATA(0) = value;

	while (!(SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY))
		__asm__ volatile("");

	return SIO_DATA(0);
}

static int exchangeControllerPacket(
	const uint8_t *request,
	uint8_t *response
) {
	IRQ_STAT = ~(1 << IRQ_SIO0);
	SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
	delayMicroseconds(DTR_DELAY);

	int respLength = 0;

	SIO_DATA(0) = ADDR_CONTROLLER;

	if (waitForAcknowledge()) {
		int reqLength = 4;
		const int maxRespLength = 8;
		while (SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY)
			SIO_DATA(0);

		while (respLength < maxRespLength) {
			if (reqLength > 0) {
				*(response++) = exchangeByte(*(request++));
				reqLength--;
			} else {
				*(response++) = exchangeByte(0);
			}

			respLength++;

			if (!waitForAcknowledge())
				break;
		}
	}

	delayMicroseconds(DTR_DELAY);
	SIO_CTRL(0) &= ~SIO_CTRL_DTR;

	return respLength;
}

uint16_t readControllerButtons(const int port) {
    uint8_t request[4], response[8];

    request[0] = CMD_POLL;
    request[1] = 0x00;
    request[2] = 0x00;
    request[3] = 0x00;

    selectPort(port);
    const int respLength = exchangeControllerPacket(
        request,
        response
    );

    if (respLength < 4) {
        return 0;
    }

    return (response[2] | (response[3] << 8)) ^ 0xffff;
}
