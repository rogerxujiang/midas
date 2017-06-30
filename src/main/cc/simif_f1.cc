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

  int rc;
  rc = fpga_pci_init();
  check_rc(rc, "fpga_pci_init");

  /* This demo works with single FPGA slot, we pick slot #0 as it works for both f1.2xl and f1.16xl */

  slot_id = 0;

  //rc = check_afi_ready(slot_id);
  //fail_on(rc, out, "AFI not ready");

  pci_bar_handle = PCI_BAR_HANDLE_INIT;
  rc = fpga_pci_attach(0, FPGA_APP_PF, APP_PF_BAR0, 0, &pci_bar_handle);
  check_rc(rc, "fpga_pci_attach");
  //fail_on(rc, out, "Unable to attach to the AFI on slot id %d", slot_id);


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
  //f1_finish();
    fpga_shutdown();
}

void simif_f1_t::write(size_t addr, uint32_t data) {
#ifdef SIMULATION_XSIM

    // TODO: pipe comms
    //printf("HELLO WRITE addr: %x, data %x\n");

    while (!is_write_ready());
    uint64_t cmd = (((uint64_t)(0x80000000 | addr)) << 32) | (uint64_t)data;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);
    // wait for ack
/*    int gotdata = 0;
    while (gotdata == 0) {
        gotdata = ::read(xsim_to_driver_fd, buf, 8);
        if (gotdata != 0 && gotdata != 8) {
            printf("ERR GOTDATA %d\n", gotdata);
        }
    }*/
#else
    while (!is_write_ready());
    addr <<= 2;
//    fprintf(stderr, "writing addr: %x, data %x\n", addr >> 2, data);
//    __sync_synchronize();

    int rc = fpga_pci_poke(pci_bar_handle, addr, data);
//    __sync_synchronize();

//    fprintf(stderr, "writing addr: %x, data %x\n", addr >> 2, data);

    //fprintf(stderr, "writing addr: %x, data %x\n", addr, data);
//    for (int i = 0; i < 100; i++) {
//    __sync_synchronize();
//    }


    check_rc(rc, NULL);
    //fail_on(rc, out, "Unable to write to the fpga !");


#endif
}

uint32_t simif_f1_t::read(size_t addr) {
#ifdef SIMULATION_XSIM
    // TODO: pipe comms
    //printf("HELLO READ addr: %x\n");
    uint64_t cmd = addr << 1;
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
    while (!is_write_ready());

    uint32_t value;
    addr <<= 3;
//    fprintf(stderr, "wat\n");
//    fprintf(stderr, "read addr: %x, data %x\n", addr >> 3, value);
//    __sync_synchronize();

    int rc = fpga_pci_peek(pci_bar_handle, addr, &value);
//    __sync_synchronize();

//    fprintf(stderr, "read addr: %x, data %x\n", addr >> 3, value);
    //check_rc(rc, NULL);
    //fprintf(stderr, "got response\n");

    return value & 0xFFFFFFFF;
    //fail_on(rc, out, "Unable to read read from the fpga !");
#endif
}


uint32_t simif_f1_t::is_write_ready() {
#ifdef SIMULATION_XSIM
    // TODO: pipe comms
    //printf("HELLO READ addr: %x\n");
    uint64_t cmd = 0x1;
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
    uint64_t addr = 0x4;
//    __sync_synchronize();
//    fprintf(stderr, "wat\n");
    int rc = fpga_pci_peek(pci_bar_handle, addr, &value);
//    __sync_synchronize();
    check_rc(rc, NULL);
    return value & 0xFFFFFFFF;
    //fail_on(rc, out, "Unable to read read from the fpga !");
#endif
}
