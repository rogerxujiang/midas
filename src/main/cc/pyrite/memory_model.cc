#include <exception>
#include "memory_model.h"
#include "mm_dramsim2.h"
#include "tokens.h"

AXI4MemoryModel::AXI4MemoryModel(MemoryModelConfig& cfg): cfg(cfg) {
  switch (cfg.kind) {
    case MMKind::DRAMSIM2:
      mem = (mm_t*)new mm_dramsim2_t;
      break;
    case MMKind::MAGIC:
      mem = (mm_t*)new mm_magic_t;
      break;
    default:
      throw std::runtime_error("Unrecognized memory model kind");
      break;
  }
  mem->init(cfg.size, cfg.data_width / 8, cfg.line_size);
}

void AXI4MemoryModel::init_memory_from_file(const char* filename) {
    fprintf(stderr, "[sw loadmem] %s\n", filename);
    void* mems[1];
    mems[0] = mem->get_data();
    ::load_mem(mems, filename, cfg.data_width / 8, 1);
}

bool AXI4MemoryModel::tick() {
  size_t MEM_CHUNKS = cfg.data_width / (8 * sizeof(uint32_t));
  mem->tick(
    reset.bits,
    axi4.in.bits.ar.valid,
    axi4.in.bits.ar.bits.addr,
    axi4.in.bits.ar.bits.id,
    axi4.in.bits.ar.bits.size,
    axi4.in.bits.ar.bits.len,

    axi4.in.bits.aw.valid,
    axi4.in.bits.aw.bits.addr,
    axi4.in.bits.aw.bits.id,
    axi4.in.bits.aw.bits.size,
    axi4.in.bits.aw.bits.len,

    axi4.in.bits.w.valid,
    axi4.in.bits.w.bits.strb,
    axi4.in.bits.w.bits.data.get_data(),
    axi4.in.bits.w.bits.last,

    axi4.in.bits.r.ready,
    axi4.in.bits.b.ready
  );

  axi4.out.bits.aw.ready = mem->aw_ready();
  axi4.out.bits.ar.ready = mem->ar_ready();
  axi4.out.bits.w.ready = mem->w_ready();

  axi4.out.bits.b.bits.id = mem->b_id();
  axi4.out.bits.b.bits.resp = mem->b_resp();
  axi4.out.bits.b.valid = mem->b_valid();

  axi4.out.bits.r.bits.id = mem->r_id();
  axi4.out.bits.r.bits.resp = mem->r_resp();
  axi4.out.bits.r.bits.last = mem->r_last();
  axi4.out.bits.r.bits.data = biguint_t((const uint32_t*)mem->r_data(), MEM_CHUNKS);
  axi4.out.bits.r.valid = mem->r_valid();
}

