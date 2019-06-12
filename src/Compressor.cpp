#include "Compressor.h"
#include "Mixer.h"
#include <iostream>
  
namespace tsal {

Compressor::Compressor() {
  setAttackTime(1.0);
  setReleaseTime(1500.0);
}

// This may not be the best implementation, maybe a circular buffer would work better, but 
// it seems to be fine
double Compressor::getOutput() {
  // If not active, just route the samples through without applying an filtering
  if (!mActive) {
    return getInput();
  }
  
  // If the end of the buffer has been reached, more audio samples need to be generated
  if (mCurrentSample == COMPRESSOR_MAX_BUFFER) {
    mCurrentSample = 0;
    // Generate new audio data
    for (unsigned i = 0; i < COMPRESSOR_MAX_BUFFER; i++) {
      mBuffer[i] = getInput();
    }
    // Filter the generated audio data
    filterAudio();
  }

  // Get an audio sample
  return mBuffer[mCurrentSample++];
}

/**
 * @brief Get the sound envelope for the sample buffer
 * 
 */
void Compressor::getEnvelope() {
  for (unsigned i = 0; i < COMPRESSOR_MAX_BUFFER; i++) {
    // Using peak detection since it is faster
    // Maybe implement RMS
    double envIn = std::abs(mBuffer[i]);

    double gain = mEnvelopeSample < envIn ? mAttackGain : mReleaseGain;
    mEnvelopeSample = envIn + gain * (mEnvelopeSample - envIn);

    mEnvelope[i] = mEnvelopeSample;
  }
}
/**
 * @brief Compress the audio in the buffer if necessary
 * 
 */
void Compressor::filterAudio() {
  double postGainAmp = dbToAmp(mPostGain);

  // If there is any pregain, apply it to the audio buffer
  if (mPreGain != 0.0) {
    double preGainAmp = dbToAmp(mPreGain);
    for (unsigned i = 0; i < COMPRESSOR_MAX_BUFFER; i++) {
      mBuffer[i] *= preGainAmp;
    }
  }

  getEnvelope();
  calculateSlope();
  
  // Apply the adjusted gain and postGain to the audio buffer
  for (unsigned i = 0; i < COMPRESSOR_MAX_BUFFER; i++) {
    mGain = mSlope * (mThreshold - mEnvelope[i]);
    mGain = std::min(0.0, mGain);
    mGain = dbToAmp(mGain);
    mBuffer[i] *= (mGain * postGainAmp);
  }
}
 
/**
 * 
 * @brief Get the slope based off the ratio
 * 
 */
void Compressor::calculateSlope() {
  mSlope = 1.0 - (1.0 / mRatio);
}

double Compressor::ampToDb(double amplitude) {
  return 20.0 * std::log(amplitude) / M_LN10;
}

double Compressor::dbToAmp(double db) {
  return std::pow(10.0, db / 20.0);
}

/**
 * @brief Set the attack time (ms)
 * 
 * @param attackTime
 */
void Compressor::setAttackTime(double attackTime) {
  if (checkParameterRange("AttackTime", attackTime, mAttackTimeRange)) {
    return;
  }
  mAttackGain = attackTime == 0.0 ? 0.0 : std::exp(-1.0 / (Mixer::getSampleRate() * attackTime/1000));
}

/**
 * @brief Set the release time (ms)
 * 
 * @param releaseTime 
 */
void Compressor::setReleaseTime(double releaseTime) {
  if (checkParameterRange("ReleaseTime", releaseTime, mReleaseTimeRange)) {
    return;
  }
  mReleaseGain = releaseTime == 0.0 ? 0.0 : std::exp(-1.0 / (Mixer::getSampleRate() * releaseTime/1000));
}

/**
 * @brief Set the threshold
 * 
 * @param threshold (dB)
 */
void Compressor::setThreshold(double threshold) {
  if (checkParameterRange("Threshold", threshold, mThresholdRange)) {
    return;
  }
  mThreshold = threshold;
}

/**
 * @brief Set the ratio
 * 
 * @param ratio (1: n)
 */
void Compressor::setRatio(double ratio) {
  if (checkParameterRange("Ratio", ratio, mRatioRange)) {
    return;
  }
  mRatio = ratio;
}

/**
 * @brief Set the pre gain
 * 
 * @param preGain (dB)
 */
void Compressor::setPreGain(double preGain) {
  if (checkParameterRange("PreGain", preGain, mGainRange)) {
    return;
  }
  mPreGain = preGain;
}

/**
 * @brief Set the post gain
 * 
 * @param postGain (dB)
 */
void Compressor::setPostGain(double postGain) {
  if (checkParameterRange("PostGain", postGain, mGainRange)) {
    return;
  }
  mPostGain = postGain;
}

// This should probably be replaced by an exception, so make an exception class
bool Compressor::checkParameterRange(std::string name, double value, ParameterRange range) {
  bool outOfRange = range.first <= value && value >= range.second;
  if (outOfRange) {
    std::cout << name << ": not in range [" 
              << range.first << ", " 
              << range.second 
              << "]" << std::endl;
  }
  return outOfRange;
}

ParameterRange Compressor::mAttackTimeRange = std::make_pair(0.0, 2000.0);
ParameterRange Compressor::mReleaseTimeRange = std::make_pair(0.0, 2000.0);
ParameterRange Compressor::mThresholdRange = std::make_pair(-30.0, 400.0);
ParameterRange Compressor::mRatioRange = std::make_pair(1.0, 20.0);
ParameterRange Compressor::mGainRange = std::make_pair(-30.0, 30.0);
}
