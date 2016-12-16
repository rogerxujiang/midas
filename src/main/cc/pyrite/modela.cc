#include <iostream>
#include <queue>
#include <tuple>
#include <vector>
#include <cstdint>
#include <stdio.h>
#include <string>

#include "swmodel.h"
#include "standard_models.h"

using namespace std;

class ModelA : public SWModel {
  private:
    string name;
    long incAmt;

  public:
    ModelA(string _name, long _incAmt) : name(_name), incAmt(_incAmt) {
      std::cout << "Creating ModelA named " << name << "!\n";
      // Admittedtly, this is a pretty big wart... We shouldn't do this in
      // child class' constructor if would be nice if it was just done
      // implicitly
      register_port(&interface);
      register_port(&reset);
    }
    ModelA(const ModelA&) = delete; // no copy

    // Port definitions
    BidirPort<uint64_t, uint64_t> interface;
    InputPort<bool> reset;

    bool tick(void) {
      cout << "Ticking " << name << endl;
      if (!reset.bits) {
        cout << "  Got input " << interface.in.bits << "!\n";
        interface.out.bits = interface.in.bits + incAmt;
        cout << "  Pushing output " << interface.out.bits << "!\n";
      } else {
        cout << "  Held in reset" << endl;
        interface.out.bits = 0;
      }
      return true;
    }
};

int main(int argc, char** argv) {
  printf("Hello World!\n");

  size_t runtime = 10;

  // Create Models
  ModelA* m1 = new ModelA("Model 1", 1);
  ModelA* m2 = new ModelA("Model 2", 2);
  ResetModel * resetter = new ResetModel(0, 2);
  
  std::vector<SWModel*> models = {m1, m2, resetter};
  cout << "models created!" << endl;

  // Connect
  auto channel = m1->interface.biconnect(m2->interface);
  auto m1reset = m1->reset.connect(resetter->reset);
  auto m2reset = m2->reset.connect(resetter->reset);

  cout << "models connected!" << endl;

  // Start with a number
  get<0>(channel)->push(1L);
  get<1>(channel)->push(0L);
  m1reset->push(true);
  m2reset->push(true);
  cout << "initial tokens pushed!" << endl;

  bool done = 0;
  // Simulation loop
  while (!done) {
    done = true;
    for(auto& model: models) {
      if (model->get_cycle() < runtime) {
        model->try_tick();
        done = false;
      }
    }
  }

  return 0;
}
