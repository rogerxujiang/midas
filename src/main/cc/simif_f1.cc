#include "simif_f1.h"
#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

simif_f1_t::simif_f1_t() {
#ifdef SIMULATION_XSIM
    mkfifo(driver_to_xsim, 0666);
    fprintf(stderr, "opening driver to xsim\n");
    driver_to_xsim_fd = open(driver_to_xsim, O_WRONLY);
    fprintf(stderr, "opening xsim to driver\n");
    xsim_to_driver_fd = open(xsim_to_driver, O_RDONLY);
#else
    fprintf(stderr, "===== FPGA setup =====\n");
    slot_id = 0; // TODO: make this a cmd line arg
    int rc = fpga_pci_init();
    check_rc(rc, "fpga_pci_init");

    // TODO: add back AFI checking
    //rc = check_afi_ready(slot_id);
    // check_rc(rc, "fpga_pci_init");

    pci_bar_handle = PCI_BAR_HANDLE_INIT;
    rc = fpga_pci_attach(0, FPGA_APP_PF, APP_PF_BAR0, 0, &pci_bar_handle);
    check_rc(rc, "fpga_pci_attach");
#endif
}

void simif_f1_t::check_rc(int rc, char * infostr) {
#ifndef SIMULATION_XSIM
    if (infostr) {
        fprintf(stderr, "%s\n", infostr);
    }
    if (rc) {
        fprintf(stderr, "INVALID RETCODE: %d\n", rc, infostr);
        fpga_shutdown();
        exit(1);
    }
#endif
}

void simif_f1_t::fpga_shutdown() {
#ifndef SIMULATION_XSIM
    int rc = fpga_pci_detach(pci_bar_handle);
    // don't call check_rc because of fpga_shutdown call. do it manually:
    if (rc) {
        fprintf(stderr, "Failure while detaching from the fpga: %d\n", rc);
    }
#endif
}

simif_f1_t::~simif_f1_t() {
    fpga_shutdown();
}

void simif_f1_t::write(size_t addr, uint32_t data) {
    // addr is really a (32-byte) word address because of zynq implementation
    addr <<= 2;
#ifdef SIMULATION_XSIM
    uint64_t cmd = (((uint64_t)(0x80000000 | addr)) << 32) | (uint64_t)data;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);
    while (!is_write_ready());
#else
    int rc = fpga_pci_poke(pci_bar_handle, addr, data);
    while (!is_write_ready());
    check_rc(rc, NULL);
#endif
}

uint32_t simif_f1_t::read(size_t addr) {
    addr <<= 3;
#ifdef SIMULATION_XSIM
    uint64_t cmd = addr;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);

    int gotdata = 0;
    while (gotdata == 0) {
        gotdata = ::read(xsim_to_driver_fd, buf, 8);
        if (gotdata != 0 && gotdata != 8) {
            printf("ERR GOTDATA %d\n", gotdata);
        }
    }
    return *((uint64_t*)buf);
#else
    uint32_t value;
    int rc = fpga_pci_peek(pci_bar_handle, addr, &value);
    return value & 0xFFFFFFFF;
#endif
}


uint32_t simif_f1_t::is_write_ready() {
    uint64_t addr = 0x4;
#ifdef SIMULATION_XSIM
    uint64_t cmd = addr;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);

    int gotdata = 0;
    while (gotdata == 0) {
        gotdata = ::read(xsim_to_driver_fd, buf, 8);
        if (gotdata != 0 && gotdata != 8) {
            printf("ERR GOTDATA %d\n", gotdata);
        }
    }
    return *((uint64_t*)buf);
#else
    uint32_t value;
    int rc = fpga_pci_peek(pci_bar_handle, addr, &value);
    check_rc(rc, NULL);
    return value & 0xFFFFFFFF;
#endif
}
