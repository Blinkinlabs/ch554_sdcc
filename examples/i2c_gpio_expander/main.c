// I2C GPIO expander with MDIO bus

#include <ch554.h>
#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "i2c_slave.h"
#include "mdio_master.h"

#define STATUS_REG          0

#define STATUS_REG_LAST_BOOT    (1<<0)  // If 1, boot in safe mode
#define STATUS_REG_MDIO_START   (1<<1)  // Write 1 to start MDIO transaction
#define STATUS_REG_MDIO_WRITE   (1<<2)  // Write 0 for read, 1 for write
#define STATUS_REG_MDIO_ERROR   (1<<3)  // Status of last MDIO trasaction, 1=error

#define ID_REG              1

#define ID_REG_VALUE            (0xAF)  // Chip identification

#define P1_REG              2
#define P1_MOD_OC_REG       3
#define P1_DIR_PU_REG       4
#define MDIO_PHY_ADDR_REG   5
#define MDIO_PHY_REG_REG    6
#define MDIO_DATA_HIGH_REG  7
#define MDIO_DATA_LOW_REG   8
#define SRAM_REG            9

uint8_t status;
uint8_t id;
uint8_t p1_shadow;
uint8_t p1_mod_oc_shadow;
uint8_t p1_dir_pu_shadow;
uint8_t mdio_phy_addr;
uint8_t mdio_phy_reg;
uint8_t mdio_data_h;
uint8_t mdio_data_l;
uint8_t gp_mem;

//! @brief I2C device register map
//!
//! Note that the SFRs can only be addressed in direct mode, so a shadow copy
//! is placed in this table, and their values need to be copied in before a read
//! operation, or copied out after a write operation.
__idata uint8_t *regs_ptr[I2C_SLAVE_REG_COUNT] = {
    &status,                        //!< Status register
    &id,                            //!< Chip ID register
    &p1_shadow,                     //!< GPIO value register
    &p1_mod_oc_shadow,              //!< GPIO config register
    &p1_dir_pu_shadow,              //!< GPIO config register
    &mdio_phy_addr,                 //!< MDIO PHY address (5 bits)
    &mdio_phy_reg,                  //!< MDIO PHY register (5 bits)
    &mdio_data_h,                   //!< MDIO DATA_HIGH register (8 bits)
    &mdio_data_l,                   //!< MDIO DATA_LOW register (8 bits)
    &gp_mem,                        //!< General purpose storage (8 bits)
};

//! @brief Initialize the programmable GPIO outputs
//!
//! Set all P1 pins in open drain mode, with output disabled
void outputs_init() {
    P1_MOD_OC = 0xFF;
    P1_DIR_PU = 0x00;
    P1 = 0xFF;
}

//! @brief Initialize the reset button input
//!
//! Sets the pin mode for the reset button to input, and enables the
//! pin change interrupt on that pin.
void reset_button_init() {
    gpio_pin_mode(RESET_BUTTON_PIN, RESET_BUTTON_PORT, GPIO_MODE_INPUT);

    EX0 = 1;    // Enable External input 0 (reset button)
    EX1 = 1;    // Enable External input 1 (ESP_EN line)
    EA = 1;     // Global interrupt enable
}

//! @brief Button press interrupt
//!
//! This interrupt is triggered by a user pressing the reset button.
//!
//! When this happens, the BMC will immediately pull the ESP_EN pin low to
//! put the ESP32 in reset, and then reset all programmable GPIO pins, in order
//! to disable attached peripherals. It will then monitor for the reset button
//! to be released, and will then reset the BMC.
void BUTTON_ISR(void) __interrupt (INT_NO_INT0) {
    EA = 0;     // Disable further interrupts

    // Put the ESP in reset
    gpio_pin_write(ESP_EN_PIN, ESP_EN_PORT, 0);

    // Put all peripherals in their default state
    // Note: this also turns off the LED
    outputs_init();

    // TODO: Measure how many seconds the button was held down
    while(gpio_pin_read(RESET_BUTTON_PIN, RESET_BUTTON_PORT) == 0) {}

    // And reset
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG |= bSW_RESET;
}

//! @brief ESP Enabled external change interrupt
//!
//! This interrupt is triggered when an external source (probably the USB-serial
//! coverter) pulls the ESP_EN line low to put the ESP32 in reset.
//!
//! When this happens, the BMC will immediately reset all GPIO pins, in order
//! to disable attached peripherals. It will then monitor for the ESP_EN line
//! to be released, and will then reset the BMC.
void ESP_EN_ISR(void) __interrupt (INT_NO_INT1) {
    EA = 0;     // Disable further interrupts

    // Put all peripherals in their default state
    // Note: this also turns off the LED
    outputs_init();

    // Wait until the ESP_EN_PIN is released
    while(gpio_pin_read(ESP_EN_PIN, ESP_EN_PORT) == 0) {}

    // And reset
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG |= bSW_RESET;
}

// @brief Measure the system voltage
//
// This function measures the current system voltage, by sampling an assumed 1.2V
// reference and inverting the result.
//
// \return System voltage, in 10ths of a volt
uint8_t measure_system_voltage() {
    uint8_t val;

    // Disable the ADC interrupt
    IE_ADC = 0;

    // Enable the ADC, using the slow clock setting
    ADC_CFG = bADC_EN;

    // Select the input channel 00=P1.1, 01=P1.4, 10=P1.5, 11=P3.2
    ADC_CHAN1 = 0;  
    ADC_CHAN0 = 1; 

    // Start a conversion
    ADC_START = 1;

    // Wait for the result
    while(ADC_START == 1) {}

    // Read the 1.2V input, in terms of the system voltage
    // val = 1.2/Vcc*255
    val = ADC_DATA;

    // Then VCC (in 1/10s of V) can be calculated as:
    // Vcc = (1.2*255*10)/val = 3060/val
    return 3060/val;
}

#define STABLE_VOLTAGE 31           // System voltage to wait for, (10ths of V)
#define STABLE_VOLTAGE_DELAY 50     // Amount of time to delay after the system voltage has become high enough (ms)

// @brief Wait for the system voltage to rise to a given setpoint
void wait_for_stable_voltage() {
    // If the voltage is already high enough, return immediately
    // This path is for cases where the board is reset via the front button, or the programming interface
    if(measure_system_voltage() >= STABLE_VOLTAGE)
        return;

    // Otherwise, wait for the system voltage to become stable
    //  TODO: Does this need averaging?
    while(measure_system_voltage() < 31) {}

    // Then, delay for a short time to ensure stability
    mDelaymS(STABLE_VOLTAGE_DELAY);
}

void main() {
    i2c_slave_transaction_t result;

    // Put ESP_EN in open-drain mode, pulled down (disable the ESP32)
    gpio_pin_mode(ESP_EN_PIN, ESP_EN_PORT, GPIO_MODE_OPEN_DRAIN);
//    gpio_pin_write(ESP_EN_PIN, ESP_EN_PORT, 0);

    CfgFsys();
//    mInitSTDIO();   // debug output on P3.1

    // Initialize the programmable GPIO
    // TODO: These ended up being fixed-function
    outputs_init();

    // Wait for the system voltage to settle
//    wait_for_stable_voltage();

    // Release ESP_EN (enable the ESP32)
//    gpio_pin_write(ESP_EN_PIN, ESP_EN_PORT, 1);

    // Turn on the front panel LED
    gpio_pin_write(INDICATOR_LED_PIN, INDICATOR_LED_PORT, 0);

    reset_button_init();
    i2c_slave_init();
    mdio_master_init();

    // I2C slave listen routine
    while (1) {
        // Re-write the status and read-only registers
        status = 0; // reset_status | mdio_dir
        id = ID_REG_VALUE;

        p1_shadow = P1;                 // TODO: Setting these here gives stale values
        p1_mod_oc_shadow = P1_MOD_OC;
        p1_dir_pu_shadow = P1_DIR_PU;

        // Wait for an I2C transaction
        result = i2c_slave_poll();

        // And handle any write callbacks
        /*
        P1 = p1_shadow;
        P1_MOD_OC = p1_mod_oc_shadow;
        P1_DIR_PU = p1_dir_pu_shadow;
        if(status & STATUS_REG_MDIO_START) {
            if (status & STATUS_REG_MDIO_WRITE)
                mdio_master_write(mdio_phy_addr, mdio_phy_reg, mdio_data_h, mdio_data_l);
            else
                mdio_master_read(mdio_phy_addr, mdio_phy_reg, &mdio_data_h, &mdio_data_l);
        }
        */
        
        if(result == I2C_SLAVE_WRITE) {
            switch(i2c_slave_reg) {
                case STATUS_REG:
                    if(i2c_slave_val & STATUS_REG_MDIO_START) {
                        if (i2c_slave_val & STATUS_REG_MDIO_WRITE)
                            mdio_master_write(mdio_phy_addr, mdio_phy_reg, mdio_data_h, mdio_data_l);
                        else
                            mdio_master_read(mdio_phy_addr, mdio_phy_reg, &mdio_data_h, &mdio_data_l);
                    }
                    break;
                case P1_REG:            P1 = i2c_slave_val;             break;
                case P1_MOD_OC_REG:     P1_MOD_OC = i2c_slave_val;      break;
                case P1_DIR_PU_REG:     P1_DIR_PU = i2c_slave_val;      break;
                case MDIO_PHY_ADDR_REG: mdio_phy_addr = i2c_slave_val;  break;
                case MDIO_PHY_REG_REG:  mdio_phy_reg = i2c_slave_val;   break;
                case MDIO_DATA_HIGH_REG: mdio_data_h = i2c_slave_val;   break;
                case MDIO_DATA_LOW_REG: mdio_data_l = i2c_slave_val;    break;
                case SRAM_REG:          gp_mem = i2c_slave_val;         break;
                default:
                    break;
            }
        }
    }
}
