#pragma once

// I2C slave interface
#define SDA_PORT            3
#define SDA_PIN             0

#define SCL_PORT            3
#define SCL_PIN             1

// Reset button input
#define SYS_RESET_PORT      3
#define SYS_RESET_PIN       2

// ESP reset output
#define ESP_RESET_PORT      3
#define ESP_RESET_PIN       3

// MDIO host interface
#define MDC_PORT            3
#define MDC_PIN             4

#define MDIO_PORT           3
#define MDIO_PIN            5

// Fixed GPIO pins
#define CRESET_B_PORT       1
#define CRESET_B_PIN        0

#define CDONE_PORT          1
#define CDONE_PIN           1

// TODO: PHY clock enable
// TODO: PHY reset
