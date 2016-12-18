
GEN_DIR?=$(base_dir)/build

base_dir=$(abspath .)
csrc_dir=$(base_dir)/src/main/cc/pyrite
dramsim_dir=$(base_dir)/src/main/cc/dramsim2
util_dir=$(csrc_dir)/../utils

CSRCS=memory_model.cc swmodel.cc standard_models.cc modela.cc
OBJS=$(addprefix $(GEN_DIR)/, $(subst .cc,.o,$(CSRCS)))

default: simulator

include $(util_dir)/utils.mk

CXXFLAGS=-std=c++11 -lpthread -Wall -O2 -I$(csrc_dir)/../dramsim2 -I$(util_dir) -I$(csrc_dir)
LDFLAGS= -L$(GEN_DIR) -lstdc++ -lpthread

$(GEN_DIR)/%.o: $(csrc_dir)/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

simulator: $(OBJS) $(lib_o) $(dramsim_o)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf simulator $(GEN_DIR)/*


.PHONY: clean
