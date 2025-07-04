/*
 * mt6835_stm32_spi_port.h
 *
 *  Created on: Feb 24, 2025
 *      Author: user
 */

#ifndef INC_MT6835_STM32_SPI_PORT_H_
#define INC_MT6835_STM32_SPI_PORT_H_

#define MT6835_STM32_SPI_PORT_ENABLE  (0u)

#ifdef __cplusplus
extern "C" {
#endif

#include "mt6835.h"

mt6835_t *mt6835_stm32_spi_port_init(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* INC_MT6835_STM32_SPI_PORT_H_ */
