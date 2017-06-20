#include "simif_f1.h"
#include <cassert>

simif_f1_t::simif_f1_t() {
  f1_start();
}

simif_f1_t::~simif_f1_t() {
  f1_finish();
}

void simif_f1_t::write(size_t addr, uint64_t data) {
  f1_softreg_write(addr, data);
}

uint64_t simif_f1_t::read(size_t addr) {
  return f1_softreg_read(addr);
}
