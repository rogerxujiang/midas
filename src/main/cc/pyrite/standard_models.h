#ifndef __STANDARD_MODELS_H
#define __STANDARD_MODELS_H

#include "swmodel.h"
#include "tokens.h"

// This can probably generalized into sets of models whose stimulus
// is known apriori
class ResetModel: public SWModel {
  private:
    size_t start, end;
    bool tick(void);

  public:
    ResetModel(size_t _start, size_t _end) : start(_start), end(_end) {
      assert(_start < _end);
      register_port(&reset);
    };
    OutputPort<bool> reset;
};

class AbstractAXI4Slave: public SWModel {
  public:
    BidirPort<AXI4SToken, AXI4MToken> axi4;
    InputPort<bool> reset;

    AbstractAXI4Slave() {
      register_port(&reset);
      register_port(&axi4);
    }
};
#endif
