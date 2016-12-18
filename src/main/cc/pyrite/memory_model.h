#ifndef __MEMORY_MODEL_H
#define __MEMORY_MODEL_H

#include "standard_models.h"
#include "mm.h"

enum class MMKind {
  DRAMSIM2,
  MAGIC
};

struct MemoryModelConfig {
  MMKind kind;
  size_t size;
  int data_width;
  int line_size = 64;
};

class AXI4MemoryModel: public AbstractAXI4Slave {
  private:
    mm_t* mem;
    MemoryModelConfig cfg;

    bool tick();

  public:
    AXI4MemoryModel(MemoryModelConfig& cfg);
    void init_memory_from_file(const char* filename);
};

#endif
