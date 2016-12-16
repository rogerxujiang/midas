#include "swmodel.h"

using namespace std;

bool SWModel::try_tick() {
  bool inputs_valid = true;
  for (auto& in: ins) {
    inputs_valid &= in->data_valid();
  }

  if (inputs_valid) {
    tick();
    cycle++;

    for (auto& in: ins) {
      in->deq_data();
    }
    for (auto& out: outs) {
      out->enq_data();
    }
  }

  return inputs_valid;
}
