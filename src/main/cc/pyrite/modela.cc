#include <iostream>
#include <queue>
#include <vector>
#include <cstdint>
#include <stdio.h>
#include <string>

#include "swmodel.h"

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
      register_port(&in);
      register_port(&out);
    }
    ModelA(const ModelA&) = delete; // no copy

    // Port definitions
    InputPort<uint64_t> in;
    OutputPort<uint64_t> out;

    bool tick(void) {
      cout << "Ticking " << name << endl;
      uint64_t input = in.bits;
      cout << "  Got input " << input << "!\n";
      out.bits = input + incAmt;
      cout << "  Pushing output " << out.bits << "!\n";
      return true;
    }
};

int main(int argc, char** argv) {
  printf("Hello World!\n");

  size_t runtime = 10;

  // Create Models
  ModelA* m1 = new ModelA("Model 1", 1);
  ModelA* m2 = new ModelA("Model 2", 2);
  vector<SWModel*> models = {m1, m2};
  cout << "models created!" << endl;

  // Connect
  auto m1enq = m1->in.connect(m2->out);
  auto m2enq = m2->in.connect(m1->out);

  cout << "models connected!" << endl;

  // Start with a number
  m1enq->push(1L);
  m2enq->push(0L);
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
