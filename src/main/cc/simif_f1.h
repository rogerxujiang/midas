#ifndef __SIMIF_F1_H
#define __SIMIF_F1_H

#include "simif.h"    // from midas
//#include "f1.h" // from firesim/platforms/f1/f1.h

#define SIMULATION_XSIM

#ifndef SIMULATION_XSIM
#include <fpga_pci.h>
#include <fpga_mgmt.h>
#endif


class simif_f1_t: public virtual simif_t
{
  public:
    simif_f1_t();
    virtual ~simif_f1_t();
    virtual void write(size_t addr, uint32_t data);
    virtual uint32_t read(size_t addr);
  private:
    char in_buf[MMIO_WIDTH];
    char out_buf[MMIO_WIDTH];
#ifdef SIMULATION_XSIM
    char * driver_to_xsim = "/tmp/driver_to_xsim";
    char * xsim_to_driver = "/tmp/xsim_to_driver";
    int driver_to_xsim_fd;
    int xsim_to_driver_fd;
#else
    int rc;
    int slot_id;
    pci_bar_handle_t pci_bar_handle;
#endif
};

#endif // __SIMIF_F1_H
