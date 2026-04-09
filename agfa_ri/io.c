#include <stdio.h>
#include <string.h>

#include "io.h"

// forward declarations
void poolUarts(void);
uint32_t waitForRSSH(void);
void executeAtpLAndS(void);
void apisWriteCommand(char *command);
void illegalMsg(void);
uint32_t getTwoByteInt(uint8_t *data);
void setLed(uint32_t led_map);
uint32_t uartInputIsDebug(void);
uint32_t uartInputGetDial(void);
void sysMain(void);
void clearSerialBuffer(SERIAL_BUFFER *buf);
uint32_t getFullBuffer(PARSER_HELPER *helper);
void uartPuts(UART_TRANSFER *uart, char *string);
void incBufferReadIndex(uint32_t serial_type);
void debugPuts(char *string);
void initParserHelper(uint32_t type, SERIAL_BUFFER *buffer, PARSER_HELPER *helper);
void readByteApis(void);
void readByteAtp(void);
void nextBufferReadIndex(SERIAL_BUFFER *buf, SERIAL_BUFFER_INDEX *buffer_index);
void nextBufferWriteIndex(SERIAL_BUFFER_INDEX *buffer_index);
void getAtpDataLength(uint32_t write_index);
void asm_copyROMtoRAM(void);
void asm_memcpy(uint8_t *dest, uint8_t *src, uint32_t len);


// uninitialized globals

// $15000 helper structure for APIS/ATP commands
PARSER_HELPER parser_helper;
// $1500E ATP status flags
uint32_t atp_mode;
// $15016 command APIS/ATP code
uint32_t command_code;
// $1501A optional APIS command param
uint32_t command_param;
// $15022 ???
uint32_t led_3_toggle;
// $15032 buffer for error string
char error_buffer[64];
// $150F2 buffer for !L&S ATP command
char las_buffer[32];
// $15112 0 if data segment is initialized ???
uint32_t is_loaded;
// $15116 whether debug mode is ON
uint32_t is_debug;
// $1511A APIS UART memory mapping
UART_TRANSFER *uart_apis;
// $1511E ATP UART memory mapping
UART_TRANSFER *uart_atp;
// $15122 debug UART memory mapping
UART_TRANSFER *uart_debug;
// $15126 serial buffers for APIS UART
SERIAL_BUFFER apis_buffers[SERIAL_BUFFER_COUNT];
// $15256 serial buffers for ATP UART
SERIAL_BUFFER atp_buffers[SERIAL_BUFFER_COUNT];
// #15386 circural buffer indexes for APIS UART
SERIAL_BUFFER_INDEX apis_buffer_index;
// #1538E circural buffer indexes for ATP UART
SERIAL_BUFFER_INDEX atp_buffer_index;
// $15396 buffer for version string
char version_string_buffer[80];
// $153E6 ATP command data length
uint32_t atp_data_length;
// $153EA initial serial transmission timeout count
uint32_t initial_timeout_count;
// $153EE in sig test (singaling test ?)
uint32_t is_sig_test;
#warning variable meaning not fully understood 'is_sig_test'
// $153F2 in sig test ordered
uint32_t is_sig_test_ordered;
#warning variable meaing not fully understood 'is_sig_test_ordered'

// $171Ac
char *atp_commands[COMMAND_COUNT_ATP] = { "!STA", "!L&S", "!BEG", "!END", "!PWR", "_GST", "_CMD", "_INF", "_SET", "_GET", "_MOD", "_NEG", "_POS", "_GPR", "_RES"};
// $17242
char *apis_commands[COMMAND_COUNT_APIS] = { "RE", "PA", "DN", "MG", "SH" };
// 172E0
UART_CONFIG *uart1_config = UART1_CFG;


// $0400
// WARNING - This function was written in assebler and is presented here as C pseudocode
void asm_entryPoint(void) {
	uint32_t sp; // 68000 SP register placeholder
	uint32_t **ptr = (uint32_t **)(0x1F000);

	sp = 0x14000;
	*ptr = (uint32_t *)0x1F004;
	/* POSSIBLE BUG

	1. 0x14000 stack pointer is at beggining of adress space, and it wraps around
	after first push to to $17FFC which closes to values in initialized RAM ($17000-$17648)
	but space to 0x15000 is mot used at all.

	2. Address 0x1F000 will be overwitten by asm_copROMtoRAM writes to 0x17000.

	Both problems suggests that initialy this system has 64KB of RAM but was cost reduced to 16.
	*/

	asm_copyROMtoRAM();
	sysMain();

	while(1);
}

// $042C
void cmdATI() {
#warning TODO
}

// $04B4
uint32_t doApisReset(void) {
	uint32_t i;

	atp_mode = ATP_MODE_BOOTING;
	setLed(LED_2_3);

	apisWriteCommand("SS");
	if (!waitForRSSH()) {
		return 0;
	}

	command_code = -1;
	for (i = 0; i <= 5; i++) {
		apisWriteCommand("RE");
		poolUarts();
		if (command_code != CMD_APIS_RSRE) {
			continue;
		}
	}
	if (i > 5) {
		return 0;
	}
	setLed(LED_1_2_3);

	while(command_code != CMD_APIS_RSMG) {
		poolUarts();
	}
	setLed(LED_1_2);

	if (command_param == 1) {
		return 0;
	}

	atp_mode = ATP_MODE_IDLE;
	return 1;
}

// $0B4E
void atpWriteMode(void) {
	switch (atp_mode) {
		case ATP_MODE_IDLE:
			uartPuts(UART_ATP, "0005+idle");
			break;
		case ATP_MODE_IMAGING:
			uartPuts(UART_ATP, "0008+imaging");
			break;
		case ATP_MODE_BOOTING:
			uartPuts(UART_ATP, "0008+booting");
			break;
		case ATP_MODE_BOOT_FAILED:
			uartPuts(UART_ATP, "0012+boot failed");
			break;
		case ATP_MODE_SIG_TEST:
			uartPuts(UART_ATP, "0008+SIGtest");
			break;
	}
}

// $0BB2
void poolUarts(void) {
	uint32_t i;
	// loop until full command is fetched
start:	while(!getFullBuffer(&parser_helper));

	if (parser_helper.type == SERIAL_TYPE_APIS) {
		// match common part
		if (strncmp(parser_helper.data, "{RS", 3) == 0) {
			for (i = 0; i < COMMAND_COUNT_APIS; i++) {
				// match specific part
				if (strncmp(parser_helper.data + 3, apis_commands[i], 3) == 0) {
					// recreate APIS command code
					command_code = CMD_APIS_BASE + i;
					// fetch param if needed
					if ((command_code == CMD_APIS_RSSH) || (command_code == CMD_APIS_RSMG)) {
						command_param = getTwoByteInt(parser_helper.data + 5);
					}
					break;
				}
			}
			if (i >= COMMAND_COUNT_APIS) {
				illegalMsg();
			}
		} else {
			illegalMsg();
		}

		incBufferReadIndex(SERIAL_TYPE_APIS);
	} else if (parser_helper.type == SERIAL_TYPE_ATP) {
		for (i = 0; i < COMMAND_COUNT_ATP + 1; i++) {
			// POSSIBLE CRITICAL BUG
			// loop over 16 values but in table is only 15 !
			if (strncmp(parser_helper.data, atp_commands[i], 4) == 0) {
				// recreate ATP command code
				command_code = i;
				break;
			}
		}
		if (i >= COMMAND_COUNT_ATP + 1) {
			illegalMsg();
		}

		if (command_code == CMD_ATP_MOD) {
			atpWriteMode();
			incBufferReadIndex(SERIAL_TYPE_ATP);
			// I have no idea how achieve this in clean way without goto
			goto start;
		} else if (command_code == CMD_ATP_LaS) {
			executeAtpLAndS();
		}

		incBufferReadIndex(SERIAL_TYPE_ATP);
	} else {
		debugPuts("ATIacti - Unknown device");
	}
}

// $0D6C
uint32_t waitForRSSH(void) {
	uint32_t i;

	poolUarts();
	for (i = 0; i <= 5; i++) {
		if (command_code == CMD_APIS_RSSH) {
			break;
		}
		poolUarts();
	}

	if (i > 5) {
		sprintf(error_buffer, "SH timed out");
		debugPuts(error_buffer);
		return 0;
	}

	return 1;
}

// $0E4C
void apisWriteCommand(char *command) {
	char buf[64];

	sprintf(buf, "{SR%s", command);
	uartPuts(UART_APIS, buf);
}

// $0EAA
void illegalMsg(void) {
	command_code = -1;
	command_param = -1;
	sprintf(error_buffer, "Illegal msg : %s", parser_helper.data);
	debugPuts(error_buffer);
}

// $0F84
uint32_t getTwoByteInt(uint8_t *data) {
	return (data[0] - '0') * 10 + (data[1] = '0');
}

// $1014
// POSSIBLE BUG
// This function is called by poolUarts() and can also call indirectly poolUarts() witch can cause
// probably undesirable recurrency causing stack overflow
void executeAtpLAndS(void) {
	uint32_t led = LED_NO_LED;
	uint8_t ch;

	switch (parser_helper.data[10]) {
		case '0':
			if (led_3_toggle) {
				led = LED_3;
				led_3_toggle = 0;
			} else {
				led = LED_NO_LED;
				led_3_toggle = 1;
			}
			break;
		case '1':
			led = LED_1_2_3;
			break;
		case '3':
			led = LED_2_3;
			break;
	}

	if (parser_helper.data[5] == 'W') {
		led = LED_3;
	}

	if (parser_helper.data[6] == 'H') {
		led = LED_1;
	}

	setLed(led);
	strcpy(las_buffer, "0004+");
#warning TODO

	ch = uartInputGetDial() + '0';
	if (ch == '0') {
		ch = '6';
	}
	las_buffer[6] = ch;
	las_buffer[7] = ch + 0x20;
	las_buffer[8] = 0;
	uartPuts(UART_ATP, las_buffer);
}

// $1116
void initUarts(void) {
	UART_CONFIG *uart1_cfg = UART1_CFG;
	UART_TRANSFER *uart1_pa = UART_ATP;
	UART_TRANSFER *uart1_pb = UART_APIS;
	UART_TRANSFER *uart2_pa = UART_DEBUG;

	// disable & reset ATP UART
	uart1_pa->command = UART_CMD_RESET_RX | UART_CMD_RX_DISABLE;
	uart1_pa->command = UART_CMD_RESET_TX | UART_CMD_TX_DISABLE;

	// disable & reset APIS UART
	uart1_pb->command = UART_CMD_RESET_RX | UART_CMD_RX_DISABLE;
	uart1_pb->command = UART_CMD_RESET_TX | UART_CMD_TX_DISABLE;

	// config ATP UART
	uart1_pa->command = UART_CMD_RESET_TO_MR1;
	uart1_pa->mr1 = UART_MR1_PARITY_DISABLED | UART_MR1_PARITY_EVEN | UART_MR1_BITS8;
	uart1_pa->mr1 = UART_MR1_PARITY_DISABLED | UART_MR1_PARITY_ODD | UART_MR1_BITS8;
	uart1_pa->clock = UART_CLOCK_S1_RX_9600 | UART_CLOCK_S1_TX_9600;

	// config APIS UART
	uart1_pa->command = UART_CMD_RESET_TO_MR1;
	uart1_pa->mr1 = UART_MR1_PARITY_DISABLED | UART_MR1_PARITY_EVEN | UART_MR1_BITS8;
	uart1_pa->mr1 = UART_MR1_PARITY_ENABLED | UART_MR1_PARITY_ODD | UART_MR1_BITS8;
	uart1_pa->clock = UART_CLOCK_S1_RX_2400 | UART_CLOCK_S1_TX_2400;

	// UART1 general config
	uart1_cfg->output_port_config = UART_OUT_CFG_O3_CT_OUTPUT | UART_OUT_CFG_04_RXRDYA;
	uart1_cfg->output_port_reset = UART_OUTP_LED2 | UART_OUTP_LED3;
	// POSSIBLE BUG
	// this confilcts with output port configuration, but this is like source code looks like
	uart1_cfg->output_port_set = (uint8_t)(~UART_OUTP_LED_MASK);
	uart1_cfg->aux_control = UART_AUX_TIMER_CLK_DIV16;
	uart1_cfg->int_mask = 0;
	uart1_cfg->counter_hi = 0xFF;
	uart1_cfg->counter_lo = 0xFF; //assuming 3.6864 MHz clock, above configuration gives output on port 3 ~ 3.5 Hz pulses

	// enable ATP UART
	uart1_pa->command = UART_CMD_RX_ENABLE;
	uart1_pa->command = UART_CMD_TX_ENABLE;

	// enable APIS UART
	uart1_pb->command = UART_CMD_RX_ENABLE;
	uart1_pb->command = UART_CMD_TX_ENABLE;

	// initialize debug uart only if debug input port is enabled
	if (uartInputIsDebug()) {
		// disable & reset debug UART
		uart2_pa->command = UART_CMD_RESET_RX | UART_CMD_RX_DISABLE;
		uart2_pa->command = UART_CMD_RESET_TX | UART_CMD_TX_DISABLE;

		// config debug UART
		uart2_pa->command = UART_CMD_RESET_TO_MR1;
		uart2_pa->mr1 = UART_MR1_PARITY_DISABLED | UART_MR1_PARITY_EVEN | UART_MR1_BITS8;
		uart2_pa->mr1 = UART_MR1_PARITY_DISABLED | UART_MR1_PARITY_ODD | UART_MR1_BITS8;
		uart2_pa->clock = UART_CLOCK_S1_RX_9600 | UART_CLOCK_S1_TX_9600;

		// enable debug UART
		uart2_pa->command = UART_CMD_RX_ENABLE;
		uart2_pa->command = UART_CMD_TX_ENABLE;
	}

	// delay loop, may need disablig loop optimizations to keep it
	for (uint32_t i = 0; i < 10000; i++);
}

// $1206
uint32_t uartReadByte(UART_TRANSFER *uart, uint8_t *buffer) {
	if (uart->status & UART_STATUS_RXRDY) {
		*buffer = uart->buffer;
		return 1;
	}

	return 0;
}

// $1222
uint32_t uartWriteByte(UART_TRANSFER *uart, uint8_t buffer) {
	if (uart->status & UART_STATUS_TXRDY) {
		uart->buffer = buffer;
		return 1;
	}

	return 0;
}

// $123E
void setLed(uint32_t led_map) {
	uint8_t led_mask;

	switch (led_map) {
		case LED_NO_LED:
			led_mask = 0;
			break;
		case LED_1:
			led_mask = UART_OUTP_LED1;
			break;
		case LED_1_2:
			led_mask = UART_OUTP_LED1 | UART_OUTP_LED2;
			break;
		case LED_3:
			led_mask = UART_OUTP_LED3;
			break;
		case LED_2_3:
			led_mask = UART_OUTP_LED2 | UART_OUTP_LED3;
			break;
		case LED_1_2_3:
			led_mask = UART_OUTP_LED1 | UART_OUTP_LED2 | UART_OUTP_LED3;
			break;
		default:
			// POSSIBLE BUG
			// improper value, but this is how source looks like
			led_mask = led_map;
			break;
	}

	uart1_config->output_port_reset = led_mask;
	uart1_config->output_port_set = (~led_mask) & UART_OUTP_LED_MASK;
}

// $12AA
uint32_t uartInputIsReset(void) {
	UART_CONFIG *uart = UART1_CFG;

	if (!(uart->input_port & UART_INP_RESET)) { // active LO ?
		return 1;
	}

	return 0;
}

// $12C0
uint32_t uartInputIsDebug(void) {
	UART_CONFIG *uart = UART1_CFG;

	if (!(uart->input_port & UART_INP_DEBUG)) { // active LO ?
		return 1;
	}

	return 0;
}

// $1340
uint32_t uartInputGetDial(void) {
	return (uart1_config->input_port >> 2) & 0x7;
}

// $1804
void sysMain(void) {
	is_loaded = 0;
	cmdATI();
}

// $181A
void initATI(void) {
	initial_timeout_count = 100000;

	// clear all serial buffers
	for (uint32_t i = 0; i < SERIAL_BUFFER_COUNT; i++) {
		clearSerialBuffer(&apis_buffers[i]);
		clearSerialBuffer(&atp_buffers[i]);
	}

	// initialize circural buffer indexes
	apis_buffer_index.write_index = 0;
	apis_buffer_index.read_index = 0;
	atp_buffer_index.write_index = 0;
	atp_buffer_index.read_index = 0;

	// disable debugging
	is_debug = 0;

	// assingn memory mapped UARTs to variables;
	uart_apis = UART_APIS;
	uart_atp = UART_ATP;
	uart_debug = UART_DEBUG;

	// reset sig test variables
	is_sig_test = 0;
	is_sig_test_ordered = 0;

	//check whether debug line in enabled
	if (uartInputIsDebug()) {
		is_debug = 1;
	}

	sprintf(version_string_buffer, "Agfa T9400PS ATI v%s\n", AGFA_VERSION);
	debugPuts(version_string_buffer);
}

// $18F8
uint32_t getFullBuffer(PARSER_HELPER *helper) {
	// read and chek if first in line APIS buffer is full and ready to read
	readByteApis();
	if (apis_buffers[apis_buffer_index.read_index].flags == BUFFER_FULL) {
		initParserHelper(SERIAL_TYPE_APIS, &apis_buffers[apis_buffer_index.read_index], helper);
		return 1;
	}

	// read and chek if first in line ATP buffer is full and ready to read
	readByteAtp();
	if (atp_buffers[atp_buffer_index.read_index].flags == BUFFER_FULL) {
		initParserHelper(SERIAL_TYPE_ATP, &atp_buffers[atp_buffer_index.read_index], helper);
		return 1;
	}

	return 0;
}

// #19BC
void uartPuts(UART_TRANSFER *uart, char *string) {
	debugPuts(string);

	// write string into debug UART
	for (; *string != 0; string++) {
		// try till success
		while(!uartWriteByte(uart_debug, *string)) {
			// if failed try read data form main UARTs to avoid chip buffer overflow
			readByteApis();
			readByteAtp();
		}
	}
}

// $1A04
void incBufferReadIndex(uint32_t serial_type) {
	if (serial_type == SERIAL_TYPE_APIS) {
		nextBufferReadIndex(&apis_buffers[apis_buffer_index.read_index], &apis_buffer_index);
	} else if (serial_type == SERIAL_TYPE_ATP) {
		nextBufferReadIndex(&atp_buffers[atp_buffer_index.read_index], &atp_buffer_index);
	}

}

// $1A70
void debugPuts(char *string) {
	if (is_debug) {
		// write string into debug UART
		for (; *string != 0; string++) {
			// try till success
			while(!uartWriteByte(uart_debug, *string)) {
				// if failed try read data form main UARTs to avoid chip buffer overflow
				readByteApis();
				readByteAtp();
			}
		}

		// put CR LF (new line)
		// POSSIBLE BUG
		// accest to debug UART in different way
		while(!uartWriteByte(UART_DEBUG, '\r'));
		while(!uartWriteByte(UART_DEBUG, '\n'));
	}
}

// $1AE0 unreachable
void setDebugFlag(uint32_t flag) {
	is_debug = flag;
}

// $1AEC unreachable
void setAlternateUart(char uartNr) {
	if (uartNr == '1') {
		uart_apis = UART_DEBUG;
	} else if (uartNr == '0') {
		uart_atp = UART_DEBUG;
	} else {
		debugPuts("ERROR : illegal scanSwitch parameter");
	}
}

// $1B2A ureachable
void setInitialTimeoutCount(uint32_t timeout) {
	initial_timeout_count = timeout;
}

// $1B4E
void doSigTest(void) {
#warning TODO
}

// $1B76
void clearSerialBuffer(SERIAL_BUFFER *buf) {
	buf->flags = BUFFER_EMPTY;
	buf->bytes_read = 0;
	buf->timeout_count= initial_timeout_count;
}

// $1B8A
void nextBufferReadIndex(SERIAL_BUFFER *buf, SERIAL_BUFFER_INDEX *buffer_index) {

	// error if write index was catched
	if (buffer_index->write_index == buffer_index->read_index) {
		debugPuts("UNLOCK-ERROR : too many unlocks");
		clearSerialBuffer(buf);
	} else {
		clearSerialBuffer(buf);
		if (++buffer_index->read_index >= SERIAL_BUFFER_COUNT) {
			buffer_index->read_index = 0;
		}
	}
}

// $1BD0
void initParserHelper(uint32_t type, SERIAL_BUFFER *buffer, PARSER_HELPER *helper) {
	buffer->flags = BUFFER_IN_PROCES;
	helper->unk0 = 1;
	helper->type = type;
	helper->data = buffer->data;
	helper->length = buffer->bytes_read;

	if (is_debug) {
		debugPuts(buffer->data);
	}
}

// $1C0E
void readByteApis(void) {
	uint32_t write_index;
	uint8_t buf;

	doSigTest();

	write_index = apis_buffer_index.write_index;
	// try read byte form hardware ...
	if (uartReadByte(uart_apis, &buf)) { // ...success
		apis_buffers[write_index].timeout_count = initial_timeout_count;

		switch (apis_buffers[write_index].flags) {
			case BUFFER_EMPTY:
				if (buf == '{') {
					apis_buffers[write_index].data[apis_buffers[write_index].bytes_read++] = buf;
					apis_buffers[write_index].flags = BUFFER_APIS_DATA;
				}
				return;
			case BUFFER_IN_PROCES:
				debugPuts("APIS-ERROR : illegal buffer flags");
				return;
			case BUFFER_APIS_DATA:
				// POSSIBLE BUG
				// no buffer overflow check
				if (buf == '{') {
					apis_buffers[write_index].bytes_read = 0;
				} else if (buf == '}') {
					apis_buffers[write_index].flags = BUFFER_FULL;
					nextBufferWriteIndex(&apis_buffer_index);
				}

				apis_buffers[write_index].data[apis_buffers[write_index].bytes_read++] = buf;
				apis_buffers[write_index].data[apis_buffers[write_index].bytes_read] = 0;
				return;
		}
	} else { // ... failure
		if (apis_buffers[write_index].flags == BUFFER_APIS_DATA) {
			if (--apis_buffers[write_index].timeout_count < 0) {
				clearSerialBuffer(&apis_buffers[write_index]);
				debugPuts("APIS-ERROR : timeout");
			}
		}
	}
}

// $1D98
void readByteAtp(void) {
	uint32_t write_index;
	uint8_t buf;

	doSigTest();

	write_index = atp_buffer_index.write_index;
	// try read byte form hardware ...
	if (uartReadByte(uart_atp, &buf)) { // ...success
		atp_buffers[write_index].timeout_count = initial_timeout_count;

		switch (atp_buffers[write_index].flags) {
			case BUFFER_EMPTY:
				if (buf == '0') {
					atp_buffers[write_index].data[atp_buffers[write_index].bytes_read++] = buf;
					atp_buffers[write_index].flags = BUFFER_ATP_LENGTH;
				}
				return;
			case BUFFER_ATP_LENGTH:
				if (atp_buffers[write_index].bytes_read == 4) {
					atp_buffers[write_index].flags = BUFFER_ATP_DATA;
					getAtpDataLength(write_index);
					atp_buffers[write_index].bytes_read = 0;
					if (atp_data_length == 1) {
						break;
					}
				}
				apis_buffers[write_index].data[apis_buffers[write_index].bytes_read++] = buf;
				return;
			case BUFFER_ATP_DATA:
				if (buf != '!') {
					getAtpDataLength(write_index);
					atp_buffers[write_index].bytes_read = 0;
					atp_buffers[write_index].data[atp_buffers[write_index].bytes_read++] = buf;
					--atp_data_length;
				} else {
					if (--atp_data_length != 0) {
						apis_buffers[write_index].data[apis_buffers[write_index].bytes_read++] = buf;
					} else {
						break;
					}
				}
				return;
		}

		atp_buffers[write_index].data[atp_buffers[write_index].bytes_read++] = buf;
		atp_buffers[write_index].data[atp_buffers[write_index].bytes_read] = 0;
		atp_buffers[write_index].flags = BUFFER_FULL;
		nextBufferWriteIndex(&atp_buffer_index);

	} else { // ... failure
		if ((atp_buffers[write_index].flags == BUFFER_ATP_LENGTH) || (atp_buffers[write_index].flags == BUFFER_ATP_DATA)) {
			if (--atp_buffers[write_index].timeout_count < 0) {
				clearSerialBuffer(&atp_buffers[write_index]);
				debugPuts("ATP-ERROR : timeout");
			}
		}
	}
}

// $1F88
void nextBufferWriteIndex(SERIAL_BUFFER_INDEX *buffer_index) {
	uint32_t last_index = buffer_index->write_index;

	// increment circural write index
	if (++buffer_index->write_index >= SERIAL_BUFFER_COUNT) {
		buffer_index->write_index = 0;
	}

	// error if read index was catched
	if (buffer_index->write_index == buffer_index->read_index) {
		debugPuts("SCAN-ERROR : buffer full");
		buffer_index->write_index = last_index;
	}
}

// $1FB8
void getAtpDataLength(uint32_t write_index) {
	uint32_t length = atp_buffers[write_index].bytes_read - 1;

	// check if data lenght is encoded in at least 3 bytes
	if (length < 3) {
		if (is_debug) {
			debugPuts("ATP-ERROR : illegal count received");
		}
		clearSerialBuffer(&atp_buffers[write_index]);
	} else {
		//decode hexadecimal char encoded data lenght
		atp_data_length = 0;
		for (uint32_t i = length - 3; i < length; i++) {
			uint8_t ch = atp_buffers[write_index].data[i];
			atp_data_length <<= 4;
			atp_data_length += (ch >= '9' ? ch - 0x37 : ch - 0x30);
		}

		if ((atp_data_length > SERIAL_BUFFER_SIZE) || (atp_data_length < 1)) {
			if (is_debug) {
				debugPuts("ATP-ERROR : received count too big");
			}
			clearSerialBuffer(&atp_buffers[write_index]);
		}
	}

}

// $C0BC
// WARNING - This function was written in assebler and is presented here as C pseudocode
void asm_copyROMtoRAM(void) {
	// initialize RAM by copying parts of ROM
	asm_memcpy((uint8_t *)0x17000, (uint8_t *)0xF000, 0x648);
}

// $C0DA
// WARNING - This function was written in assebler and is presented here as C pseudocode
void asm_memcpy(uint8_t *dest, uint8_t *src, uint32_t len) {
	while (len--) {
		*(dest++) = *(src++);
	}
}
