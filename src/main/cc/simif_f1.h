#ifndef __SIMIF_F1_H
#define __SIMIF_F1_H

#include "simif.h"    // from midas
#include "f1.h" // from firesim/platforms/f1/f1.h

class simif_f1_t: public virtual simif_t
{
  public:
    simif_f1_t();
    virtual ~simif_f1_t();
    virtual void write(size_t addr, uint64_t data);
    virtual uint64_t read(size_t addr);
  private:
    char in_buf[MMIO_WIDTH];
    char out_buf[MMIO_WIDTH];
};

#endif // __SIMIF_F1_H
