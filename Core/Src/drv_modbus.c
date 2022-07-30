/*
 * drv_modbus.c
 *
 *  Created on: 2022年7月12日
 *      Author: dev
 */
#include "common.h"
static uint8_t MODBUS_MASTER_UART = 0;
#define TIMEOUT 	5000
#define MSG_SIZE	8
#define MSG_Q_SIZE 	20
#define RX_MAX_SZ	30
typedef enum
{
	SS_IDLE, SS_SENDING
} SendState;
typedef struct
{
	uint8_t frame[MSG_SIZE];
	pFunPtr success;
	pFunPtr fail;
} ModbusMsg;
typedef struct
{
	SendState send_state;
	uint32_t timeout;
	ModbusMsg send;
	uint8_t rx_buf[RX_MAX_SZ];
	uint8_t len;
} ModbusSending;

static ModbusSending _cur_msg;
static osMessageQueueId_t q_msg;
static osEventFlagsId_t evt_id; // message queue id
#define FLAGS_SUCCESS 0x00000001ul

static const uint16_t modbus_crc_table[256] =
{
		0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1,
		0xc481, 0x0440, 0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40, 0x0a00, 0xcac1, 0xcb81, 0x0b40,
		0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40, 0x1e00, 0xdec1,
		0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41, 0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
		0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1,
		0xf281, 0x3240, 0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441, 0x3c00, 0xfcc1, 0xfd81, 0x3d40,
		0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840, 0x2800, 0xe8c1,
		0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
		0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640, 0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0,
		0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740,
		0xa501, 0x65c0, 0x6480, 0xa441, 0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01, 0x6ac0,
		0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840, 0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
		0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1,
		0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041, 0x5000, 0x90c1, 0x9181, 0x5140,
		0x9301, 0x53c0, 0x5280, 0x9241, 0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440, 0x9c01, 0x5cc0,
		0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40, 0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
		0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0,
		0x4c80, 0x8c41, 0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341,
		0x4100, 0x81c1, 0x8081, 0x4040 };

uint16_t modbus_crc(uint8_t *buffer, uint16_t size)
{
	uint16_t crc = 0xFFFFU;
	uint8_t nTemp;

	while (size--)
	{
		nTemp = *buffer++ ^ crc;
		crc >>= 8;
		crc ^= modbus_crc_table[(nTemp & 0xFFU)];
	}
	return (crc);
}

static void drv_modbus_send(uint8_t *data, pFunPtr s, pFunPtr f)
{
	ModbusMsg m = { .success = s, .fail = f };
	memcpy(m.frame, data, MSG_SIZE);
	osMessageQueuePut(q_msg, &m, 0U, 0U);
}

/*build master send frame*/
static void build_frame(uint8_t *buf, uint8_t addr, uint8_t func, uint16_t p1, uint16_t p2)
{
	buf[0] = addr;
	buf[1] = func;
	buf[2] = p1 / 256;
	buf[3] = p1 % 256;
	buf[4] = p2 / 256;
	buf[5] = p2 % 256;
	uint16_t crc16 = modbus_crc(buf, 6);
	buf[6] = crc16 % 256;
	buf[7] = crc16 / 256;
}
/*03*/
void drv_modbus_read_regs(uint8_t address, uint16_t start, uint16_t count, pFunPtr success, pFunPtr fail)
{
	uint8_t buf[10];
	build_frame(buf, address, 0x03, start, count);
	drv_modbus_send(buf, success, fail);
}
/*06*/
void drv_modbus_write_reg(uint8_t address, uint16_t start, uint16_t value, pFunPtr success, pFunPtr fail)
{
	uint8_t buf[10];
	build_frame(buf, address, 0x06, start, value);
	drv_modbus_send(buf, success, fail);
}

void drv_modbus_recv(uint8_t *buf, uint32_t len)
{
	if (len < 4)
	{
		return;
	}
	if (_cur_msg.send_state == SS_SENDING)
	{
		if (buf[0] == _cur_msg.send.frame[0] && buf[1] == _cur_msg.send.frame[1])
		{
			memcpy(_cur_msg.rx_buf,buf,len);
			_cur_msg.len = len;
			osEventFlagsSet(evt_id, FLAGS_SUCCESS);
		}
		else
		{
			log_d("modbus rx err");
		}
	}
}

void drv_modbus_loop()
{
	ModbusMsg m;
	uint32_t flag;
	while(TRUE)
	{
		if(osOK == osMessageQueueGet(q_msg, &m, NULL, osWaitForever)) //wait
		{
			memcpy(&_cur_msg.send, &m, sizeof(m));
			drv_uart_send(MODBUS_MASTER_UART, m.frame, MSG_SIZE);
			_cur_msg.send_state = SS_SENDING;
			flag = osEventFlagsWait(evt_id, FLAGS_SUCCESS, osFlagsWaitAny, TIMEOUT);
			if(FLAGS_SUCCESS == flag)
			{
				if(_cur_msg.send.success)
				{
					_cur_msg.send.success(_cur_msg.rx_buf);
				}
			}
			else if(osFlagsErrorTimeout == flag)
			{
				if(_cur_msg.send.fail)
				{
					_cur_msg.send.fail(0);
				}
			}
			_cur_msg.send_state = SS_IDLE;
		}
		osThreadYield();
	}
}
static osThreadId_t mod_th;
static const osThreadAttr_t mod_th_attr = {
  .name = "modbus",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
void drv_modbus_init()
{
	drv_uart_set_callback(MODBUS_MASTER_UART, drv_modbus_recv);
	q_msg = osMessageQueueNew(MSG_Q_SIZE, sizeof(ModbusMsg), NULL);
	evt_id = osEventFlagsNew(NULL);
	mod_th = osThreadNew(drv_modbus_loop, NULL, &mod_th_attr);
}
