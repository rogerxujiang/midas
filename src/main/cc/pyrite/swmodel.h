#ifndef __SWMODEL_H
#define __SWMODEL_H

#include <vector>
#include "ports.h"

class SWModel {
  private:
    std::size_t cycle = 0;
    std::vector<Dequeuer*> ins;
    std::vector<Enqueuer*> outs;

  protected:
    // The work function of the model. Called when all input tokens are available
    virtual bool tick(void) = 0;
    void register_port(Dequeuer* in) { ins.push_back(in); };
    void register_port(Enqueuer* out) { outs.push_back(out); };
    void register_port(Bidir* test) { ins.push_back(test); outs.push_back(test); }
  public:
    // The method the simulation environment uses to try and advance the model
    bool try_tick();
    std::size_t get_cycle(void) { return cycle; };
};

#endif // __SWMODEL_H
