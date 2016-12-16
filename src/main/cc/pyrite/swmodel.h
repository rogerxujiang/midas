#ifndef __SWMODEL_H
#define __SWMODEL_H

#include <queue>
#include <vector>
#include <cstdint>
#include <assert.h>

// Interfaces for the ports of models. Required to as to left the SWModels
// iterate through there input and output points and remaining agnostic
// to their type paramterization
class Dequeuer {
  public:
    virtual bool data_valid(void) = 0;
    virtual void deq_data(void) = 0;
};

class Enqueuer {
  public:
    virtual void enq_data(void) = 0;
};

// Forward declaration of OutputPort so as to define the connect operator
// in the InputPort
template <typename T> class OutputPort;
template <typename T> class InputPort: public Dequeuer {
  private:
    std::queue<T>* channel_deq = nullptr;

  public:
    InputPort(){};
    ~InputPort(){
      delete channel_deq;
    };
    // The data store for a token currently presented to the model
    T bits;

    // Binds an output port to an input port, returning a ptr to the connecting
    // queue
    std::queue<T>* connect(OutputPort<T>& that) {
      assert(channel_deq == nullptr);
      auto q = new std::queue<T>();
      that.channel_enqs.push_back(q);
      channel_deq = q;
      return q;
    }

    // Returns true if there is data in the queue, and assigns it to bits
    bool data_valid(void) {
      if (!channel_deq->empty()){
        bits = channel_deq->front();
        return true;
      }
      return false;
    }
    // Called after tick. Pops the token off the queue and frees it
    void deq_data() {
      assert(!channel_deq->empty());
      channel_deq->pop();
    }
};

template <typename T> class OutputPort: public Enqueuer {
  // The InputPort is responsible for binding the channel enq to this port.
  friend class InputPort<T>;
  private:
    // This is a vector to allow the fanout of tokens to multiple consumers.
    std::vector<std::queue<T>*> channel_enqs;

  public:
    OutputPort(){};
    ~OutputPort(){};

    // The value of the token
    T bits;
    // Enqueuer the current value of bits into all of the channels bound to
    // this output port.
    void enq_data() {
      for (auto& channel: channel_enqs) {
        channel->push(bits);
      }
    }
};

class SWModel {
  private:
    std::size_t cycle = 0;
    std::vector<Dequeuer*> ins;
    std::vector<Enqueuer*> outs;

  protected:
    // The work function of the model. Called when all input tokens are available
    virtual bool tick(void) = 0;
    void register_port(Dequeuer* in) { ins.push_back(in); };
    void register_port(Enqueuer* out) { outs.push_back(out); };
  public:
    // The method the simulation environment uses to try and advance the model
    bool try_tick();
    std::size_t get_cycle(void) { return cycle; };
};

#endif // __SWMODEL_H
