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
uint16_t mdio_data;
uint8_t gp_mem;

uint8_t temp_var;

// I2C device register map
__idata uint8_t *regs_ptr[I2C_SLAVE_REG_COUNT] = {
    &status,                        // Status register
    &id,                            // Chip ID register
    &p1_shadow,                     // GPIO value register
    &p1_mod_oc_shadow,              // GPIO config register
    &p1_dir_pu_shadow,              // GPIO config register
    &mdio_phy_addr,                 // MDIO PHY address (5 bits)
    &mdio_phy_reg,                  // MDIO PHY register (5 bits)
    (uint8_t *)&mdio_data + 0,      // MDIO DATA_HIGH register (8 bits)
    (uint8_t *)&mdio_data + 1,      // MDIO DATA_LOW register (8 bits)
    &gp_mem,                        // General purpose storage (8 bits)
};

void main() {
    i2c_slave_transaction_t result;

    CfgFsys();
//    mInitSTDIO();   // debug output on P3.1

    i2c_slave_init();
    mdio_master_init();

    while (1) {
        // Re-write the status and read-only registers
        status = 0; // reset_status | mdio_dir

        p1_shadow = P1;                 // TODO: Setting these here gives stale values
        p1_mod_oc_shadow = P1_MOD_OC;
        p1_dir_pu_shadow = P1_DIR_PU;

        id = ID_REG_VALUE;

        // Wait for an I2C transaction
        result = i2c_slave_poll();

        // And handle any write callbacks
        if(result == I2C_SLAVE_WRITE) {
            switch(i2c_slave_reg) {
                case STATUS_REG:
                    if(status && (STATUS_REG_MDIO_START | STATUS_REG_MDIO_WRITE))
                        mdio_master_write(mdio_phy_addr, mdio_phy_reg, mdio_data);
                    else if(status && (STATUS_REG_MDIO_START))
                        mdio_master_read(mdio_phy_addr, mdio_phy_reg, &mdio_data);
                    break;
                case P1_REG:            P1 = i2c_slave_val;             break;
                case P1_MOD_OC_REG:     P1_MOD_OC = i2c_slave_val;      break;
                case P1_DIR_PU_REG:     P1_DIR_PU = i2c_slave_val;      break;
                //case MDIO_PHY_ADDR_REG: mdio_phy_addr = i2c_slave_val;  break;
                //case MDIO_PHY_REG_REG:  mdio_phy_reg = i2c_slave_val;   break;
                //case MDIO_DATA_HIGH_REG: 
                //case MDIO_DATA_LOW_REG: 
                case SRAM_REG:          gp_mem = i2c_slave_val;         break;
                default:
                    break;
            }
        }
    }
}
