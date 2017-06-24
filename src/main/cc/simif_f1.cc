#include "simif_f1.h"
#include <cassert>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

simif_f1_t::simif_f1_t() {
  //f1_start();
  mkfifo(driver_to_xsim, 0666);
  printf("opening driver to xsim\n");
  driver_to_xsim_fd = open(driver_to_xsim, O_WRONLY);
  printf("opening xsim to driver\n");
  xsim_to_driver_fd = open(xsim_to_driver, O_RDONLY);
}

simif_f1_t::~simif_f1_t() {
  //f1_finish();
}

void simif_f1_t::write(size_t addr, uint32_t data) {
    // TODO: pipe comms
    //printf("HELLO WRITE addr: %x, data %x\n");
    uint64_t cmd = ((uint64_t) 0x80000000 | addr) << 32 | (uint64_t)data;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);
    // wait for ack
    ::read(xsim_to_driver_fd, buf, 8);
}

uint32_t simif_f1_t::read(size_t addr) {
    // TODO: pipe comms
    //printf("HELLO READ addr: %x\n");
    uint64_t cmd = addr;
    char * buf = (char*)&cmd;
    ::write(driver_to_xsim_fd, buf, 8);
    ::read(xsim_to_driver_fd, buf, 8);
    return *(uint64_t*)buf;
}
