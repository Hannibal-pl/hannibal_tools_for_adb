#ifndef __AGFA_IO_H__
#define __AGFA_IO_H__

#include <stdint.h>


// HARDWARE CONSTANTS

// UART mode register 1
#define		UART_MR1_BITS5			0x00
#define		UART_MR1_BITS6			0x01
#define		UART_MR1_BITS7			0x02
#define		UART_MR1_BITS8			0x03

#define		UART_MR1_PARITY_EVEN		0x00
#define		UART_MR1_PARITY_ODD		0x04

#define		UART_MR1_FPARITY_LOW		0x00
#define		UART_MR1_FPARITY_HIGH		0x04

#define		UART_MR1_PARITY_ENABLED		0x00
#define		UART_MR1_PARITY_FORCE		0x08
#define		UART_MR1_PARITY_DISABLED	0x10
#define		UART_MR1_PARITY_MULTIDROP	0x18

#define		UART_MR1_ERROR_CHAR		0x00
#define		UART_MR1_ERROR_BLOCK		0x20

#define		UART_MR1_RXIRQ_RXRDY		0x00
#define		UART_MR1_RXIRQ_FFULL		0x40

#define		UART_MR1_RXRTS_DISABLED		0x00
#define		UART_MR1_RXRTS_ENABLED		0x80


// UART status flags
#define		UART_STATUS_RXRDY		0x01
#define		UART_STATUS_FFULL		0x02
#define		UART_STATUS_TXRDY		0x04
#define		UART_STATUS_TMEMT		0x08
#define		UART_STATUS_ERR_OVERRUN		0x10
#define		UART_STATUS_ERR_PARITY		0x20
#define		UART_STATUS_ERR_FRAMING		0x40
#define		UART_STATUS_RECV_BREAK		0x80


// UART clock select
#define		UART_CLOCK_S1_RX_50		0x00
#define		UART_CLOCK_S1_RX_110		0x01
#define		UART_CLOCK_S1_RX_134H		0x02
#define		UART_CLOCK_S1_RX_200		0x03
#define		UART_CLOCK_S1_RX_300		0x04
#define		UART_CLOCK_S1_RX_600		0x05
#define		UART_CLOCK_S1_RX_1200		0x06
#define		UART_CLOCK_S1_RX_1050		0x07
#define		UART_CLOCK_S1_RX_2400		0x08
#define		UART_CLOCK_S1_RX_4800		0x09
#define		UART_CLOCK_S1_RX_7200		0x0A
#define		UART_CLOCK_S1_RX_9600		0x0B
#define		UART_CLOCK_S1_RX_38400		0x0C
#define		UART_CLOCK_S1_RX_TIMER		0x0D
#define		UART_CLOCK_S1_RX_IP3_16X	0x0E
#define		UART_CLOCK_S1_RX_IP3_1X		0x0F

#define		UART_CLOCK_S2_RX_75		0x00
#define		UART_CLOCK_S2_RX_110		0x01
#define		UART_CLOCK_S2_RX_134H		0x02
#define		UART_CLOCK_S2_RX_150		0x03
#define		UART_CLOCK_S2_RX_300		0x04
#define		UART_CLOCK_S2_RX_600		0x05
#define		UART_CLOCK_S2_RX_1200		0x06
#define		UART_CLOCK_S2_RX_2000		0x07
#define		UART_CLOCK_S2_RX_2400		0x08
#define		UART_CLOCK_S2_RX_4800		0x09
#define		UART_CLOCK_S2_RX_1800		0x0A
#define		UART_CLOCK_S2_RX_9600		0x0B
#define		UART_CLOCK_S2_RX_19200		0x0C
#define		UART_CLOCK_S2_RX_TIMER		0x0D
#define		UART_CLOCK_S2_RX_IP3_16X	0x0E
#define		UART_CLOCK_S2_RX_IP3_1X		0x0F

#define		UART_CLOCK_S1_TX_50		0x00
#define		UART_CLOCK_S1_TX_110		0x10
#define		UART_CLOCK_S1_TX_134H		0x20
#define		UART_CLOCK_S1_TX_200		0x30
#define		UART_CLOCK_S1_TX_300		0x40
#define		UART_CLOCK_S1_TX_600		0x50
#define		UART_CLOCK_S1_TX_1200		0x60
#define		UART_CLOCK_S1_TX_1050		0x70
#define		UART_CLOCK_S1_TX_2400		0x80
#define		UART_CLOCK_S1_TX_4800		0x90
#define		UART_CLOCK_S1_TX_7200		0xA0
#define		UART_CLOCK_S1_TX_9600		0xB0
#define		UART_CLOCK_S1_TX_38400		0xC0
#define		UART_CLOCK_S1_TX_TIMER		0xD0
#define		UART_CLOCK_S1_TX_IP3_16X	0xE0
#define		UART_CLOCK_S1_TX_IP3_1X		0xF0

#define		UART_CLOCK_S2_TX_75		0x00
#define		UART_CLOCK_S2_TX_110		0x10
#define		UART_CLOCK_S2_TX_134H		0x20
#define		UART_CLOCK_S2_TX_150		0x30
#define		UART_CLOCK_S2_TX_300		0x40
#define		UART_CLOCK_S2_TX_600		0x50
#define		UART_CLOCK_S2_TX_1200		0x60
#define		UART_CLOCK_S2_TX_2000		0x70
#define		UART_CLOCK_S2_TX_2400		0x80
#define		UART_CLOCK_S2_TX_4800		0x90
#define		UART_CLOCK_S2_TX_1800		0xA0
#define		UART_CLOCK_S2_TX_9600		0xB0
#define		UART_CLOCK_S2_TX_19200		0xC0
#define		UART_CLOCK_S2_TX_TIMER		0xD0
#define		UART_CLOCK_S2_TX_IP3_16X	0xE0
#define		UART_CLOCK_S2_TX_IP3_1X		0xF0


// UART auxiliary configuration
#define		UART_AUX_I0_DELTA_IRQ_DISABLED	0x00
#define		UART_AUX_I0_DELTA_IRQ_ENABLED	0x01

#define		UART_AUX_I1_DELTA_IRQ_DISABLED	0x00
#define		UART_AUX_I1_DELTA_IRQ_ENABLED	0x02

#define		UART_AUX_I2_DELTA_IRQ_DISABLED	0x00
#define		UART_AUX_I2_DELTA_IRQ_ENABLED	0x04

#define		UART_AUX_I3_DELTA_IRQ_DISABLED	0x00
#define		UART_AUX_I3_DELTA_IRQ_ENABLED	0x08

#define		UART_AUX_COUNTER_EXTERNAL	0x00
#define		UART_AUX_COUNTER_TXCA		0x10
#define		UART_AUX_COUNTER_TXCB		0x20
#define		UART_AUX_COUNTER_CLK_DIV16	0x30
#define		UART_AUX_TIMER_EXTERNAL		0x40
#define		UART_AUX_TIMER_EXTERNAL_DIV16	0x50
#define		UART_AUX_TIMER_CLK		0x60
#define		UART_AUX_TIMER_CLK_DIV16	0x70


// UART commands
#define		UART_CMD_RX_ENABLE		0x01
#define		UART_CMD_RX_DISABLE		0x02
#define		UART_CMD_TX_ENABLE		0x04
#define		UART_CMD_TX_DISABLE		0x08
#define		UART_CMD_RESET_TO_MR1		0x10
#define		UART_CMD_RESET_RX		0x20
#define		UART_CMD_RESET_TX		0x30
#define		UART_CMD_RESET_ERROR		0x40
#define		UART_CMD_RESET_BREAK_CHG_INT	0x50
#define		UART_CMD_BREAK_START		0x60
#define		UART_CMD_BREAK_STOP		0x70


// UART output port configuration
#define		UART_OUT_CFG_O2_OUT		0x00
#define		UART_OUT_CFG_O2_TXCA16X		0x01
#define		UART_OUT_CFG_O2_TXCA		0x02
#define		UART_OUT_CFG_O2_RXCA		0x03

#define		UART_OUT_CFG_O3_OUT		0x00
#define		UART_OUT_CFG_O3_CT_OUTPUT	0x04
#define		UART_OUT_CFG_O3_TXCB		0x08
#define		UART_OUT_CFG_O3_RXCB		0x0C

#define		UART_OUT_CFG_O4_OUT		0x00
#define		UART_OUT_CFG_04_RXRDYA		0x10

#define		UART_OUT_CFG_O5_OUT		0x00
#define		UART_OUT_CFG_05_RXRDYB		0x20

#define		UART_OUT_CFG_O6_OUT		0x00
#define		UART_OUT_CFG_06_TXRDYA		0x40

#define		UART_OUT_CFG_O7_OUT		0x00
#define		UART_OUT_CFG_07_TXRDYB		0x80


// Port meaning is specific to this code

// UART 1 output port bits
#define		UART_OUTP_APIS_BAUD_RATE	0x01	// on - 2400, off - 1200
#define		UART_OUTP_APIS_POLARITY		0x02	// on - positive, off - negative
#define		UART_OUTP_APIS_VIDEO		0x04	// on - disabled, off - enabled
#define		UART_OUTP_CT_OUTPUT		0x08
#define		UART_OUTP_RXRDY			0x10
#define		UART_OUTP_LED1			0x20
#define		UART_OUTP_LED2			0x40
#define		UART_OUTP_LED3			0x80
#define		UART_OUTP_LED_MASK		0xE0


// UART 1 input port bits
#define		UART_INP_0			0x01
#define		UART_INP_RESET			0x02
#define		UART_INP_DIAL1			0x04
#define		UART_INP_DIAL2			0x08
#define		UART_INP_DIAL3			0x10
#define		UART_INP_DEBUG			0x20
#define		UART_INP_IACK			0x40
#define		UART_INP_ALWAYS_ONE		0x80


// SOFTWARE CONSTANTS

// ATP & APIS serial commands
#define		CMD_ATP_STA			0
#define		CMD_ATP_LaS			1
#define		CMD_ATP_BEG			2
#define		CMD_ATP_END			3
#define		CMD_ATP_PWR			4
#define		CMD_ATP_GST			5
#define		CMD_ATP_CMD			6
#define		CMD_ATP_INF			7
#define		CMD_ATP_SET			8
#define		CMD_ATP_GET			9
#define		CMD_ATP_MOD			10
#define		CMD_ATP_NEG			11
#define		CMD_ATP_POS			12
#define		CMD_ATP_GPR			13
#define		CMD_ATP_RES			14
#define		CMD_APIS_BASE			100
#define		CMD_APIS_RSRE			100
#define		CMD_APIS_RSPA			101
#define		CMD_APIS_RSDN			102
#define		CMD_APIS_RSMG			103
#define		CMD_APIS_RSSH			104

// memory mapped UARTs
#define		UART1_CFG			((UART_CONFIG *)   (0x40000))
#define		UART_ATP			((UART_TRANSFER *) (0x40000))
#define		UART_APIS			((UART_TRANSFER *) (0x40010))
#define		UART2_CFG			((UART_CONFIG *)   (0x50000))
#define		UART_DEBUG			((UART_TRANSFER *) (0x50000))

// number of individual setial buffers per UART
#define		SERIAL_BUFFER_COUNT		4

// serial type
#define		SERIAL_TYPE_APIS		1 // APIS - Agfa Printer Interface Specification ?
#define		SERIAL_TYPE_ATP			2 // ATP  - Agfa Technical Protocol ?

// serial buffer flags
#define		BUFFER_EMPTY			0
#define		BUFFER_IN_PROCES		1
#define		BUFFER_APIS_DATA		2
#define		BUFFER_ATP_LENGTH		3
#define		BUFFER_ATP_DATA			4
#define		BUFFER_FULL			5

// ATP modes
#define		ATP_MODE_IDLE			1
#define		ATP_MODE_IMAGING		2
#define		ATP_MODE_BOOTING		3
#define		ATP_MODE_BOOT_FAILED		4
#define		ATP_MODE_SIG_TEST		15

// LED on/off
#define		LED_NO_LED			0
#define		LED_1				1
#define		LED_1_2				2
#define		LED_3				4
#define		LED_2_3				5
#define		LED_1_2_3			6

// commandline char types
#define		CHAR_NONE			0
#define		CHAR_NEW_LINE			1
#define		CHAR_DEL			2
#define		CHAR_UP				3
#define		CHAR_DOWN			4
#define		CHAR_BACKWARD			5
#define		CHAR_FORWARD			6
#define		CHAR_NORMAL			7
#define		CHAR_DEL_LINE			8


// version string
#define		AGFA_VERSION		"2.2"

#define		SERIAL_BUFFER_SIZE	0x40

#define		CMDLINE_BUFFER_SIZE	0x50
#define		CMDLINE_BUFFER_COUNT	10

#define		COMMAND_COUNT_APIS	5
#define		COMMAND_COUNT_ATP	15

#define		DEBUG_COMMAND_MAGIC	0xBAFBAF11


#pragma pack(1)
// UART tranfer specific registers
typedef struct {
	uint8_t			pad0;
	union {
		uint8_t		mr1;
		uint8_t		mr2;
	};
	uint8_t			pad1;
	union {
		uint8_t		status;
		uint8_t		clock;
	};
	uint8_t			pad2;
	uint8_t			command;
	uint8_t			pad3;
	uint8_t			buffer;
} UART_TRANSFER;

// UART general configuration specific registers
typedef struct {
	uint8_t			pad0[9];
	union {
		uint8_t		input_port_change;
		uint8_t		aux_control;
	};
	uint8_t			pad1;
	union {
		uint8_t		int_status;
		uint8_t		int_mask;
	};
	uint8_t			pad2;
	uint8_t			counter_hi;
	uint8_t			pad3;
	uint8_t			counter_lo;
	uint8_t			pad4[9];
	uint8_t			int_vector;
	uint8_t			pad5;
	union {
		uint8_t		input_port;
		uint8_t		output_port_config;
	};
	uint8_t			pad6;
	uint8_t			output_port_set;
	uint8_t			pad7;
	uint8_t			output_port_reset;
} UART_CONFIG;

#pragma pack()


typedef struct {
	uint32_t		flags;
	uint32_t		bytes_read;
	int32_t			timeout_count;
	uint8_t			data[SERIAL_BUFFER_SIZE];
} SERIAL_BUFFER;

typedef struct {
	uint32_t		write_index;
	uint32_t		read_index;
} SERIAL_BUFFER_INDEX;

typedef struct {
	uint32_t		unk0;
	uint32_t		type;
	uint8_t*		data;
	uint32_t		length;
} PARSER_HELPER;

typedef struct {
	uint32_t		magic;
	int32_t			(*func)();
	char			name[8];
} DEBUG_COMMAND;

#endif
