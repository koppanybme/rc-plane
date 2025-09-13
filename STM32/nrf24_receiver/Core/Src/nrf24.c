/*
 * nrf24.c
 *
 *  Created on: Sep 4, 2025
 *      Author: koppany
 */

#include "nrf24.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;

#define NRF24_PAYLOAD_SIZE 32

static uint8_t spiTxBuf[NRF24_PAYLOAD_SIZE+1];		// +1 for command byte
static uint8_t spiRxBuf[NRF24_PAYLOAD_SIZE+1];		// +1 for command byte

volatile uint8_t nrf24_tx_done = 0;
volatile uint8_t nrf24_rx_done = 0;

static inline void NRF24_CE_H(void)  { HAL_GPIO_WritePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin, GPIO_PIN_SET); }
static inline void NRF24_CE_L(void)  { HAL_GPIO_WritePin(NRF24_CE_GPIO_Port, NRF24_CE_Pin, GPIO_PIN_RESET); }
static inline void NRF24_CSN_H(void) { HAL_GPIO_WritePin(SPI2_CSN_GPIO_Port, SPI2_CSN_Pin, GPIO_PIN_SET); }
static inline void NRF24_CSN_L(void) { HAL_GPIO_WritePin(SPI2_CSN_GPIO_Port, SPI2_CSN_Pin, GPIO_PIN_RESET); }

void NRF24_WriteReg(uint8_t reg, uint8_t value) {
    spiTxBuf[0] = NRF24_CMD_W_REGISTER | (reg & 0x1F);
    spiTxBuf[1] = value;
    NRF24_CSN_L();
    HAL_SPI_Transmit(&hspi2, spiTxBuf, 2, 1);
    NRF24_CSN_H();
}

uint8_t NRF24_ReadReg(uint8_t reg) {
    spiTxBuf[0] = NRF24_CMD_R_REGISTER | (reg & 0x1F);
    spiTxBuf[1] = 0xff;
    NRF24_CSN_L();
    HAL_SPI_TransmitReceive(&hspi2, spiTxBuf, spiRxBuf, 2, 1);
    NRF24_CSN_H();
    return spiRxBuf[1];
}

void NRF24_Init(uint8_t isRX){
	HAL_Delay(100);	// Wait 100ms for power on
	NRF24_WriteReg(NRF24_REG_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT);
	NRF24_WriteReg(NRF24_REG_RF_CH, 100);		// Channel 100
	NRF24_WriteReg(NRF24_REG_RF_SETUP, 0x26);	// 250kbps air rate, 0dBm output power
	NRF24_WriteReg(NRF24_REG_RX_PW_P0, NRF24_PAYLOAD_SIZE);
	NRF24_WriteReg(NRF24_REG_EN_AA, 0x00);

    if(isRX) {
        NRF24_WriteReg(NRF24_REG_CONFIG, NRF24_PWR_UP | NRF24_EN_CRC | NRF24_CRCO | NRF24_PRIM_RX | NRF24_MASK_TX_DS | NRF24_MASK_MAX_RT);
    } else {
    	NRF24_WriteReg(NRF24_REG_FEATURE, 0x01);
        NRF24_WriteReg(NRF24_REG_CONFIG, NRF24_PWR_UP | NRF24_EN_CRC | NRF24_CRCO | NRF24_MASK_RX_DR | NRF24_MASK_MAX_RT);
    }

    HAL_Delay(2);	// Wait 1.5ms for power up
    NRF24_CE_H();
}

void NRF24_Transmit_IT(uint8_t *data, uint8_t len) {
    nrf24_tx_done = 0;

    // Clear STATUS flags
    NRF24_WriteReg(NRF24_REG_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT);

    // Prepare TX payload command
    spiTxBuf[0] = NRF24_CMD_W_TX_PAYLOAD_NOACK;
    for(uint8_t i=0; i<len; i++) spiTxBuf[i+1] = data[i];
    uint32_t time = HAL_GetTick();
    spiTxBuf[1] = (time & 0xff000000) >> 24;
    spiTxBuf[2] = (time & 0x00ff0000) >> 16;
    spiTxBuf[3] = (time & 0x0000ff00) >> 8;
    spiTxBuf[4] = (time & 0x000000ff) >> 0;

    NRF24_CSN_L();
    HAL_SPI_Transmit(&hspi2, spiTxBuf, len+1, 10);
    NRF24_CSN_H();
    // CE pulse after SPI finishes (in callback)
}

uint8_t NRF24_Receive_IT(uint8_t* data) {
    nrf24_rx_done = 0;
    NRF24_WriteReg(NRF24_REG_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT);
    // Prepare RX payload read
    spiTxBuf[0] = NRF24_CMD_R_RX_PAYLOAD;
    for(int i=1;i<=NRF24_PAYLOAD_SIZE;i++) spiTxBuf[i] = 0xFF;

    NRF24_CSN_L();
    HAL_SPI_TransmitReceive(&hspi2, spiTxBuf, spiRxBuf, NRF24_PAYLOAD_SIZE+1, 10);
    NRF24_CSN_H();
    for(int i = 0; i<NRF24_PAYLOAD_SIZE; i++) data[i] = spiRxBuf[i+1];
    return 0;
}

/*
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if(hspi->Instance == SPI2) {
        NRF24_CSN_H();

        // TX payload done
        if(spiTxBuf[0] == NRF24_CMD_W_TX_PAYLOAD) {
            NRF24_CE_H();
            nrf24_tx_done = 1;
        }

        // RX payload done
        if(spiTxBuf[0] == NRF24_CMD_R_RX_PAYLOAD) {
            nrf24_rx_done = 1;
            // Data is in spiRxBuf[1:32]
        }
    }
}
*/
