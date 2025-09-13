/*
 * nrf24.h
 *
 *  Created on: Sep 4, 2025
 *      Author: koppany
 */

#ifndef INC_NRF24_H_
#define INC_NRF24_H_

#include "stm32f1xx_hal.h"
#include "main.h"

// Commands
#define NRF24_CMD_R_REGISTER        	0x00
#define NRF24_CMD_W_REGISTER        	0x20
#define NRF24_CMD_R_RX_PAYLOAD      	0x61
#define NRF24_CMD_W_TX_PAYLOAD      	0xA0
#define NRF24_CMD_FLUSH_TX          	0xE1
#define NRF24_CMD_FLUSH_RX          	0xE2
#define NRF24_CMD_REUSE_TX_PL       	0xE3
#define NRF24_CMD_R_RX_PL_WID       	0x60
#define NRF24_CMD_W_ACK_PAYLOAD     	0xA8
#define NRF24_CMD_W_TX_PAYLOAD_NOACK 	0xB0
#define NRF24_CMD_NOP               	0xFF

// Registers
#define NRF24_REG_CONFIG            	0x00
#define NRF24_REG_EN_AA             	0x01
#define NRF24_REG_EN_RXADDR         	0x02
#define NRF24_REG_SETUP_AW          	0x03
#define NRF24_REG_SETUP_RETR        	0x04
#define NRF24_REG_RF_CH             	0x05
#define NRF24_REG_RF_SETUP          	0x06
#define NRF24_REG_STATUS            	0x07
#define NRF24_REG_OBSERVE_TX        	0x08
#define NRF24_REG_RPD               	0x09
#define NRF24_REG_RX_ADDR_P0        	0x0A
#define NRF24_REG_RX_ADDR_P1        	0x0B
#define NRF24_REG_RX_ADDR_P2        	0x0C
#define NRF24_REG_RX_ADDR_P3        	0x0D
#define NRF24_REG_RX_ADDR_P4        	0x0E
#define NRF24_REG_RX_ADDR_P5        	0x0F
#define NRF24_REG_TX_ADDR           	0x10
#define NRF24_REG_RX_PW_P0          	0x11
#define NRF24_REG_RX_PW_P1          	0x12
#define NRF24_REG_RX_PW_P2          	0x13
#define NRF24_REG_RX_PW_P3          	0x14
#define NRF24_REG_RX_PW_P4          	0x15
#define NRF24_REG_RX_PW_P5          	0x16
#define NRF24_REG_FIFO_STATUS       	0x17
#define NRF24_REG_DYNPD             	0x1C
#define NRF24_REG_FEATURE           	0x1D

// Config register bitmasks
#define NRF24_MASK_RX_DR            	(1 << 6)
#define NRF24_MASK_TX_DS            	(1 << 5)
#define NRF24_MASK_MAX_RT           	(1 << 4)
#define NRF24_EN_CRC                	(1 << 3)
#define NRF24_CRCO                  	(1 << 2)
#define NRF24_PWR_UP                	(1 << 1)
#define NRF24_PRIM_RX               	(1 << 0)

// Status register bitmasks
#define NRF24_RX_DR                 	(1 << 6)
#define NRF24_TX_DS                 	(1 << 5)
#define NRF24_MAX_RT                	(1 << 4)
#define NRF24_TX_FULL               	(1 << 0)


void NRF24_WriteReg(uint8_t reg, uint8_t value);
uint8_t NRF24_ReadReg(uint8_t reg);
void NRF24_Init(uint8_t isRX);
void NRF24_Transmit_IT(uint8_t *data, uint8_t len);
uint8_t NRF24_Receive_IT(uint8_t *data);


#endif /* INC_NRF24_H_ */
