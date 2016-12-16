#ifndef __STANDARD_MODELS_H
#define __STANDARD_MODELS_H

#include "swmodel.h"

// This can probably generalized into sets of models whose stimulus
// is known apriori
class ResetModel: public SWModel {
  private:
    size_t start, end;

  public:
    ResetModel(size_t _start, size_t _end) : start(_start), end(_end) {
      assert(_start < _end);
      register_port(&reset);
    };

    OutputPort<bool> reset;

    bool tick() {
      if (get_cycle() < start) {
        reset.bits = false;
      } else if (get_cycle() < end) {
        reset.bits = true;
      } else {
        reset.bits = false;
      }
      return true;
    }
};
#endif
