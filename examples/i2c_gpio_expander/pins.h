#pragma once

// I2C slave interface
#define SDA_PORT            3
#define SDA_PIN             0

#define SCL_PORT            3
#define SCL_PIN             1

// Reset button input
#define RESET_BUTTON_PORT   3
#define RESET_BUTTON_PIN    2

// ESP enable output (0=disable, 1=enable)
#define ESP_EN_PORT         3
#define ESP_EN_PIN          3

// MDIO host interface
#define MDC_PORT            3
#define MDC_PIN             4

#define MDIO_PORT           3
#define MDIO_PIN            5

// FPGA reset (0=disable, 1=enable)
#define CRESET_B_PORT       1
#define CRESET_B_PIN        0

// FPGA ready input (0=not ready, 1=ready)
#define CDONE_PORT          1
#define CDONE_PIN           1

// Indicator LED output (0=enable, 1=disable)
#define INDICATOR_LED_PORT  1
#define INDICATOR_LED_PIN   5

// PHY clock enable (0=disable, 1=enable)
#define PHY_REFCLK_EN_PORT  1
#define PHY_REFCLK_EN_PIN   6

// PHY reset (0=disable, 1=enable)
#define PHY_RESET_PORT      1
#define PHY_RESET_PIN       7
