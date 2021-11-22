#include "mpu.h"
#include <iostream>
/**
 * (Moritz)
 *
 * To Dos:
 *  - BCD Variants
 *  - Fix Flags
 */

void MPU::tick() {
    if (cycle == 0) {
        instr = mem->read(pc());
        pc_inc();
        cycle++;
    } else {
        // ADC, immediate
        if (instr == 0x69) {
            if (cycle == 1) {
                A += mem->read(pc()) + (P & Flag::C);
                pc_inc();
                cycle = 0;
            }
        }
        // ADC, absolute
        if (instr == 0x6D) {
            if (cycle == 1) get_lower_abs_address();
            if (cycle == 2) get_higher_abs_address();
            if (cycle == 3) {
                A += mem->read(abs_address) + (P & Flag::C);
                cycle = 0;
            }
        }
        // ADC, zero page
        if (instr == 0x65) {
            if (cycle == 1) {
                get_lower_abs_address();
            }
            if (cycle == 2) {
                A += mem->read(abs_address) + (P & Flag::C);
                cycle = 0;
            }
        }
        // ADC, indirect X (Indexed Indirect Addressing)
        if (instr == 0x61) {
            if (cycle == 1) {
                get_lower_abs_address();
                cycle++;
            }
            if (cycle == 2) {
                indexed_address = abs_address + X;
                cycle++;
            }
            if (cycle == 3) {
                abs_address = mem->read(indexed_address);
                cycle++;
            }
            if (cycle == 4) {
                abs_address |= (mem->read(indexed_address + 1) << 8);
                cycle++;
            }
            if (cycle == 5) {
                A += mem->read(abs_address) + (P & Flag::C);
                cycle = 0;
            }
        }
    }
}

void MPU::get_lower_abs_address() {
    abs_address = mem->read(pc()); // low part of address
    pc_inc();
    cycle++;
}

void MPU::get_higher_abs_address() {
    abs_address |= (mem->read(pc()) << 8); // high part of address
    pc_inc();
    cycle++;
}
