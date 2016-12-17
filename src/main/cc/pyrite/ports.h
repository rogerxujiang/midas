#ifndef __PORTS_H
#define __PORTS_H

#include <queue>
#include <vector>
#include <tuple>
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

class Bidir: public Enqueuer, public Dequeuer { };

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

template <typename O, typename I>
class BidirPort : public Bidir {
  public:
    OutputPort<O> out;
    InputPort<I> in;

    std::tuple<std::queue<I>*, std::queue<O>*> biconnect(BidirPort<I,O>& that) {
      auto enq = in.connect(that.out);
      auto deq = that.in.connect(out);
      return std::make_tuple(enq, deq);
    }

    void enq_data() { out.enq_data(); };
    void deq_data() { in.deq_data(); };
    bool data_valid() { return in.data_valid(); };
};

#endif
