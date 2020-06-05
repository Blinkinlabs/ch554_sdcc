#include <stdio.h>
#include <stdint.h>

union {
    uint8_t reg_mem[10];
    struct {
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
    } regs;
} test;

int main(int argc, void** argv) {
    test.regs.status = 1;
    test.regs.id = 2;
    test.regs.p1_shadow = 3;
    test.regs.gp_mem = 10;

    for(int i = 0; i < 10; i++) {
        printf("reg=%i val=%i\n", i, test.reg_mem[i]);
    }
    return 0;
}
