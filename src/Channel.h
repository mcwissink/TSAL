#include "Effect.h"
#include "Instrument.h"
#include "OutputDevice.h"

#ifndef CHANNEL_H
#define CHANNEL_H

namespace tsal {

/** @class Channel
 * @brief Routes instruments and other channels through an effect chain and to the master channel
 * on the mixer
 * 
 * Instruments, effects, and channels can all be added to a channel. The mixer has one default master channel
 * but more channels can be added to the mixer to increase flexibility
 */
class Channel : public OutputDevice {
  public:
    virtual double getOutput() override;
    void add(Channel* channel);
    void remove(Channel* channel);
    void add(Effect* effect);
    void remove(Effect* effect);
    void add(Instrument* instrument);
    void remove(Instrument* instrument);
  private:
    RouteDevice<OutputDevice> mChannelIn;
    Effect* mEffectChainEnd = nullptr;
};

};

#endif