#include "simif_f1.h"
#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>



simif_f1_t::simif_f1_t() {
#ifdef SIMULATION_XSIM
  mkfifo(driver_to_xsim, 0666);
  printf("opening driver to xsim\n");
  driver_to_xsim_fd = open(driver_to_xsim, O_WRONLY);
  printf("opening xsim to driver\n");
  xsim_to_driver_fd = open(xsim_to_driver, O_RDONLY);
#else
  rc = fpga_pci_init();
  //fail_on(rc, out, "Unable to initialize the fpga_pci library");

  /* This demo works with single FPGA slot, we pick slot #0 as it works for both f1.2xl and f1.16xl */

  slot_id = 0;

  //rc = check_afi_ready(slot_id);
  //fail_on(rc, out, "AFI not ready");

  printf("===== Starting with peek_poke_example =====\n");
  pci_bar_handle = PCI_BAR_HANDLE_INIT;
  rc = fpga_pci_attach(0, FPGA_APP_PF, APP_PF_BAR0, 0, &pci_bar_handle);
  printf("attached %d\n", rc);
  //fail_on(rc, out, "Unable to attach to the AFI on slot id %d", slot_id);


#endif
}

simif_f1_t::~simif_f1_t() {
  //f1_finish();
#ifndef SIMULATION_XSIM
        rc = fpga_pci_detach(pci_bar_handle);
        if (rc) {
            printf("Failure while detaching from the fpga.\n");
        }
#endif
}

void simif_f1_t::write(size_t addr, uint32_t data) {
#ifdef SIMULATION_XSIM

    // TODO: pipe comms
    //printf("HELLO WRITE addr: %x, data %x\n");
    uint64_t cmd = (((uint64_t)(0x80000000 | addr)) << 32) | (uint64_t)data;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);
    // wait for ack
    int gotdata = 0;
    while (gotdata == 0) {
        gotdata = ::read(xsim_to_driver_fd, buf, 8);
        if (gotdata != 0 && gotdata != 8) {
            printf("ERR GOTDATA %d\n", gotdata);
        }
    }
#else

    rc = fpga_pci_poke(pci_bar_handle, addr >> 2, data);
    printf("writing addr: %x, data %x, rc: %x\n", addr, data, rc);
    //fail_on(rc, out, "Unable to write to the fpga !");


#endif
}

uint32_t simif_f1_t::read(size_t addr) {
#ifdef SIMULATION_XSIM
    // TODO: pipe comms
    //printf("HELLO READ addr: %x\n");
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
    rc = fpga_pci_peek(pci_bar_handle, addr >> 2, &value);
    printf("read addr: %x, data %x, rc: %x\n", addr, value, rc);
    return value & 0xFFFFFFFF;
    //fail_on(rc, out, "Unable to read read from the fpga !");
#endif
}
