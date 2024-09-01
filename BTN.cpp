#include <Arduino.h>
#include "BTN.h"

// buttons

BTN btn;

#define HOLD_TIME_MS 50

void BTN::Init(int Pin)
{
  m_iPin = Pin;
  m_iPrevReading = HIGH;
  m_iPrevState = LOW;
  m_iTransitionTimeMS = millis();
  pinMode(m_iPin, INPUT_PULLUP);
}

bool BTN::CheckButtonPress()
{
  // debounced button, true if button pressed
  int ThisReading = digitalRead(m_iPin);
  if (ThisReading != m_iPrevReading)
  {
    // state change, reset the timer
    m_iPrevReading = ThisReading;
    m_iTransitionTimeMS = millis();
  }
  else if (ThisReading != m_iPrevState &&
           (millis() - m_iTransitionTimeMS) >= HOLD_TIME_MS)
  {
    // a state other than the last one and held for long enough
    m_iPrevState = ThisReading;
    return (ThisReading == LOW);
  }
  return false;
}

bool BTN::IsDown()
{
  // non-debounced, instantaneous reading
  return digitalRead(m_iPin) == LOW;
}
