#include "standard_models.h"

bool ResetModel::tick() {
  if (get_cycle() < start) {
    reset.bits = false;
  } else if (get_cycle() < end) {
    reset.bits = true;
  } else {
    reset.bits = false;
  }
  return true;
}

