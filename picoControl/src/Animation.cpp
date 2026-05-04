#include "config.h"
#include "Animation.h"
#include "crsf_channels.h"

extern int channels[];

Animation::Animation(const animationStep* animationData, int totalSteps) {
  this->animationData = animationData;
  this->totalSteps    = totalSteps;
  this->currentStep   = 0;
  this->playing       = false;
  this->paused        = false;
  this->animationTimer = 0;
}

void Animation::start() {
  if (!playing) {
    playing   = true;
    paused    = false;
    startTime = millis();
    currentStep = 0;
  }
}

void Animation::stop() {
  playing  = false;
  paused   = false;
  currentStep   = 0;
  animationTimer = 0;
}

void Animation::update() {
  if (!playing) { animationTimer = 0; return; }

  // Read current step from PROGMEM
  uint8_t axis_x       = pgm_read_byte(&animationData[currentStep].axis_x);
  uint8_t axis_y       = pgm_read_byte(&animationData[currentStep].axis_y);
  uint8_t nunchuck     = pgm_read_byte(&animationData[currentStep].nunchuck);
  uint8_t keypad_lo    = pgm_read_byte(&animationData[currentStep].keypad_lo);
  uint8_t analog1      = pgm_read_byte(&animationData[currentStep].analog1);
  uint8_t sw_mux_0_3   = pgm_read_byte(&animationData[currentStep].sw_mux_0_3);
  uint8_t sw_mux_4_7   = pgm_read_byte(&animationData[currentStep].sw_mux_4_7);
  uint8_t sw_mux_8_11  = pgm_read_byte(&animationData[currentStep].sw_mux_8_11);
  uint8_t sw_mux_12_15 = pgm_read_byte(&animationData[currentStep].sw_mux_12_15);

  // Playback readback debug (shows step + values during replay)
  #ifdef ANIMATION_DEBUG
  if (!paused) {
    Serial.print("->"); Serial.print(currentStep); Serial.print("-{");
    Serial.print(axis_x);       Serial.print(',');
    Serial.print(axis_y);       Serial.print(',');
    Serial.print(nunchuck);     Serial.print(',');
    Serial.print(keypad_lo);    Serial.print(',');
    Serial.print(analog1);      Serial.print(',');
    Serial.print(sw_mux_0_3);  Serial.print(',');
    Serial.print(sw_mux_4_7);  Serial.print(',');
    Serial.print(sw_mux_8_11); Serial.print(',');
    Serial.print(sw_mux_12_15);
    Serial.println("},");
  }
  #endif

  // Write to channels[] in new protocol positions
  channels[CRSF_CH_AXIS_X1]       = axis_x;
  channels[CRSF_CH_AXIS_Y1]       = axis_y;
  channels[CRSF_CH_AXIS_X2]       = 127;        // not recorded — keep centre
  channels[CRSF_CH_AXIS_Y2]       = 127;        // not recorded — keep centre
  channels[CRSF_CH_ARM]           = 0;           // not recorded
  channels[CRSF_CH_ANALOG1]       = analog1;
  channels[CRSF_CH_ANALOG2]       = 0;
  channels[CRSF_CH_ANALOG3]       = 0;
  channels[CRSF_CH_NUNCHUCK_BTN]  = nunchuck;
  channels[CRSF_CH_KEYPAD_LO]     = keypad_lo;
  channels[CRSF_CH_KEYPAD_HI]     = 0;           // not recorded (12 keys fit in lo byte)
  channels[CRSF_CH_SW_MUX_0_3]    = sw_mux_0_3;
  channels[CRSF_CH_SW_MUX_4_7]    = sw_mux_4_7;
  channels[CRSF_CH_SW_MUX_8_11]   = sw_mux_8_11;
  channels[CRSF_CH_SW_MUX_12_15]  = sw_mux_12_15;

  // Advance step and handle loop/pause
  animationTimer++;
  if (currentStep < (totalSteps - 1)) currentStep++;

  if (animationTimer > (totalSteps - 1)) {
    paused = true;
    #ifdef ANIMATION_DEBUG
    Serial.print("-> pause "); Serial.println((totalSteps + PAUSE - animationTimer) / 20.0);
    #endif
  }

  if (currentStep == (totalSteps - 1) && animationTimer > (totalSteps + PAUSE)) {
    currentStep    = 0;
    animationTimer = 0;
    startTime      = millis();
    paused         = false;
    #ifdef ANIMATION_DEBUG
    Serial.println("---------- Restarting Animation ----------");
    #endif
  }
}

bool Animation::isPlaying() { return playing; }
bool Animation::isPaused()  { return paused; }