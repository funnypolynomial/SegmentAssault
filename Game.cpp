#include <Arduino.h>
#include <EEPROM.h>
#include "CFG.h"
#include "PINS.h"
#include "BTN.h"
#include "Wheel.h"
#include "LCD.h"
#include "Game.h"

// SegmentInvaders

namespace Game {
#define TEXT_DWELL_MS 2000  // how long text lingers
#define HIGH_SCORE_IDX 0 // EEPROM location of tag & high score
int brightnessBit = 8;  // shift value for brightness pwm, 8==255==max
int highScore = 0;
int score = 0;
int level = 1;
int waveSize = 10;
int aliensShown = 0;
bool playerHit = false;
bool levelCleared = false;
int phaserRow = 0;
unsigned long alienDelayMS = 2000;
unsigned long alienTimerMS = 0;
word displaySegments[LCD_NUM_CHARS];

byte Brightness()
{
  // return brightness pwm
  if (brightnessBit == 8)
    return 255;
  else
    return 1 << brightnessBit;
}

void ClearSegments()
{
  // blank the segment data
  ::memset(displaySegments, 0, sizeof(displaySegments));
}

void UpdatePhaser()
{
  // Build the phaser to match the aimed-at row, set 
#if CFG_PHASER_FULL    
  // phaser is all the segments on the phaser row
  word phaserSegments = 0;
  for (int col = 0; col < Segments::DIMENSION; col++)
  {
    byte seg = Segments::Layout[phaserRow][col];  
    if (seg != Segments::X)
      phaserSegments |= 1 << seg;
  }
#else
  // phaser is just one segment on the phaser row (column 1)
  word phaserSegments = LCD_SEG_BIT(Segments::A2) | LCD_SEG_BIT(Segments::G2) | LCD_SEG_BIT(Segments::D2) | LCD_SEG_BIT(Segments::Layout[phaserRow][1]);
#endif    
  displaySegments[0] = phaserSegments;
}

word GetAlien(int n)
{
  // return a word with n bits set
  word segment = 0;
  n = min(n, 16);
  for (int b = 0; b < n; b++)
  {
    word Bit = 1 << random(LCD_NUM_SEGS - 1);
    while (segment & Bit)
    {
      // next free bit
      Bit <<= 1;
      if (!Bit)
        Bit = 1;        
    }
    segment |= Bit;
  }
  return segment;
}

word RotateSegments(word segs)
{
  // return segments rotated 180
  word rotated = 0;
  const byte* pRead = &Segments::Layout[0][0]; // assumes 2D array is consecutive bytes
  const byte* pWrite = &Segments::Layout[Segments::DIMENSION - 1][Segments::DIMENSION - 1];
  for (int seg = 0; seg < Segments::DIMENSION*Segments::DIMENSION; seg++, pWrite--)
    if (segs & (1 << *pRead++))
      rotated |= (1 << *pWrite);
  return rotated;
}

void StartWave()
{
  // start the wave of aliens
  // clears levelCleared & playerHit
  // starts wave progression timer
  ClearSegments();
  displaySegments[LCD_NUM_CHARS - 1] = GetAlien(level);
  aliensShown = 1;
  levelCleared = playerHit = false;
  alienTimerMS = millis();
}

bool UpdateWave()
{
  // checks wave progression timer and scrolls wave left if due
  // may add a new alien on RHS
  // may set playerHit
  // returns true if display should update
  unsigned long nowMS = millis();
  if (nowMS - alienTimerMS >= alienDelayMS)
  {
    alienTimerMS = nowMS;
    if (displaySegments[1])
      playerHit = true; // shuffle would hit player
    else
    {
      // shuffle
      ::memmove(displaySegments + 1, displaySegments + 2, sizeof(displaySegments[0])*(LCD_NUM_CHARS - 2));
      if (aliensShown < waveSize)
      {
        displaySegments[LCD_NUM_CHARS - 1] = GetAlien(level);
        aliensShown++;
      }
      else
        displaySegments[LCD_NUM_CHARS - 1] = 0;
    }
    return true;
  }
  return false;
}

void UpdateDisplay()
{
  // repaint the LCD
#if CFG_ROTATE_LCD
  for (int ch = 0; ch < LCD_NUM_CHARS; ch++)
    LCD::SetChar(LCD_NUM_CHARS - ch - 1, RotateSegments(displaySegments[ch]));
#else
  for (int ch = 0; ch < LCD_NUM_CHARS; ch++)
    LCD::SetChar(ch, displaySegments[ch]);
#endif    
}

void UpdateChar(int ch, word wd)
{
  // repaint the single char
#if CFG_ROTATE_LCD
  LCD::SetChar(LCD_NUM_CHARS - ch - 1, RotateSegments(wd));
#else
  LCD::SetChar(ch, wd);
#endif  
}

void FirePhaser()
{
  // clear the leftmost segment of rightmost char, on the firing row
  // updates score 
  // may set levelCleared
  for (int character = 1; character < LCD_NUM_CHARS; character++)
  {
    // check each character
    word displaySegment = displaySegments[character];
#if CFG_ANIMATE_PHASER    
    word animationSegment = displaySegment;
#endif    
    for (int col = 0; col < Segments::DIMENSION; col++)
    {
      // check each segment column
      word segBit = 1 << Segments::Layout[phaserRow][col];  
      if (segBit & displaySegment)
      {
        // hit!
        displaySegments[character] &= ~segBit;
        if (!displaySegments[character])
        {
          // last one, killed char/group
          score +=  character + 1;
          if (aliensShown == waveSize)
          {
            // killed them all?
            levelCleared = true;
            for (int check = 1; check < LCD_NUM_CHARS; check++)
              if (displaySegments[check])
              {
                levelCleared = false;
                break;
              }
          }
        }
        return;
      }
#if CFG_ANIMATE_PHASER
#if CFG_PHASER_FULL
      else
      {
        animationSegment |= segBit;
        UpdateChar(character, animationSegment);
        delay(2);
      }
#else
      else if (col == 1)
      {
        animationSegment |= segBit;
        UpdateChar(character, animationSegment);
        delay(5);
      }
#endif
#endif
    }
  }
}

void DisplayText(const char* pStr, int number = -1, int delayMS = 0, int startIdx = 0)
{
  // Displays the text from char #startIdx and the number from char #6 (if not -1), max 4 digits
  // waits delayMS
  word* pSeg = displaySegments;
  ClearSegments();
  while (*pStr && startIdx < LCD_NUM_CHARS)
    pSeg[startIdx++] = LCD::GetFontSegments(*pStr++);
  if (number >= 0)
  {
    if (number > 9999)
      number = 9999;
    char buff[5];
    pStr = itoa(number, buff, 10);
    // right align
    pSeg = displaySegments + 6 + 4 - strlen(buff);
    while (*pStr)
      *pSeg++ = LCD::GetFontSegments(*pStr++);
  }
  UpdateDisplay();
  delay(delayMS);
}

void WaitForStart()
{
  // wait for press
  DisplayText("Press Fire");
  while (!btn.CheckButtonPress())
  {
    // turning the wheel while waiting adjusts the brightnes
    wheel.Update();
    brightnessBit -= wheel.GetRotation();
    brightnessBit = constrain(brightnessBit, 0, 8);
    EEPROM[HIGH_SCORE_IDX + 3] = brightnessBit;
    LCD::SetBacklight(Brightness());
  }
#if !CFG_DEBUG
  // randomize based on when fire was pressed
  unsigned long seed = 0xDECAFBAD ^ micros();
  randomSeed(seed);
#endif    
}

void Start()
{
  // Start a game, wait for a press
  WaitForStart();
  StartWave();
  UpdatePhaser();
  UpdateDisplay();
}

void NewGame()
{
  // start over
  score = 0;
  level = 1;
  waveSize = 10;
  aliensShown = 0;
  playerHit = false;
  levelCleared = false;
  phaserRow = 0;
  alienDelayMS = CFG_WAVE_SPEED_MAX_MS;  
}

void NextGame()
{
  // advance a level
  // speeds up advancing wave
  // but reduces count
  level++;
  waveSize -= CFG_WAVE_COUNT_DEC;
  waveSize = constrain(waveSize, CFG_WAVE_COUNT_MIN, 10);
  aliensShown = 0;
  playerHit = false;
  levelCleared = false;
  phaserRow = 0;
  alienDelayMS = CFG_WAVE_SPEED_NUM*alienDelayMS/CFG_WAVE_SPEED_DEN;
  alienDelayMS = max(alienDelayMS, CFG_WAVE_SPEED_MIN_MS);
}

void CheckHighScore()
{
  // checks if score is new high score, displays it  
  if (score > highScore)
  {
    highScore = score;
    EEPROM[HIGH_SCORE_IDX] = 'H';
    EEPROM[HIGH_SCORE_IDX + 1] = highScore;
    EEPROM[HIGH_SCORE_IDX + 2] = highScore/256;
    DisplayText("High", score, TEXT_DWELL_MS);
  }
  else
    DisplayText("Score", score, TEXT_DWELL_MS);
}

void GameOver()
{
  // Player died
  DisplayText("Game Over", -1, TEXT_DWELL_MS);
  CheckHighScore();
  NewGame();
  DisplayText("Level", level, TEXT_DWELL_MS);
  Start();
}

void LevelCleared()
{
  // Player won level
  CheckHighScore();
  DisplayText("Level", level + 1, TEXT_DWELL_MS);
  NextGame();
  Start();
}

void ScrollPhaser()
{
  // check moving the phaser row
  wheel.Update();
  int dirn = wheel.GetRotation();
  if (dirn != 0)
  {
    phaserRow += dirn;
#if CFG_PHASER_WRAP
    if (phaserRow >= Segments::DIMENSION)
      phaserRow -= Segments::DIMENSION;
    else if (phaserRow < 0)
      phaserRow += Segments::DIMENSION;
#else
    phaserRow = constrain(phaserRow, 0, Segments::DIMENSION - 1);
#endif
    UpdatePhaser();
    UpdateDisplay();
  }
}

void Splash()
{
  const char* pStr = "Segment"; // 7 chars and a NUL
  ClearSegments();
  UpdateDisplay();
  byte LFSR = 0x15;
  do
  {
    // reveal segments pseudo-randomly 
    // LSFR is 0b0iiissss. i is char index, s is segment number
    // visits every segment in 8 chars
    int idx = LFSR >> 4;
    word seg = 1 << (LFSR & 0x0F);
    if (seg & LCD::GetFontSegments(pStr[idx]))
    {
      displaySegments[idx] |= seg;
      UpdateDisplay();
      delay(25);
    }
    LFSR = (LFSR >> 1) | ((((LFSR >> 0) ^ (LFSR >> 1)) & 1) << 6);  // 7 bit LFSR, taps @ 7 & 6, period 127
    if (LFSR == 0x15)
      LFSR = 0; // 0 is not in the LFSR sequence, add it for 0th char's 0th segment
    else if (LFSR == 0)
      break;
  } while (true);
  delay(2000);
  
  for (int idx = LCD_NUM_CHARS - 1; idx >=0; idx--)
    DisplayText("Assault", -1, 100, idx);
  delay(1000);
}

void Init()
{
  btn.Init(PIN_ENCODER_BTN);
  if (!btn.IsDown() && EEPROM[HIGH_SCORE_IDX] == 'H')
  {
    // load if valid and btn not held at start (reset)
    highScore = EEPROM[HIGH_SCORE_IDX + 1];
    highScore += 256*EEPROM[HIGH_SCORE_IDX + 2];
    brightnessBit = EEPROM[HIGH_SCORE_IDX + 3];
    brightnessBit = constrain(brightnessBit, 0, 8);
  }
  LCD::Init();
  LCD::SetBacklight(Brightness());
  wheel.Init();
#if CFG_PHOTO
  DisplayText("PHOTO", -1);
  NewGame();
  Start();
  phaserRow = 1;
  UpdatePhaser();
  for (int idx = 0; idx < 3; idx++)
    displaySegments[LCD_NUM_CHARS - 1 - idx] = GetAlien(3);
  UpdateDisplay();
  while (1);
#endif  
  Splash();
  DisplayText("MEW   2\xFF""24", -1, 1000);
  DisplayText("High", highScore, 1000);
  NewGame();
  Start();
}

void Loop()
{
  if (btn.CheckButtonPress())
  {
    FirePhaser();
    UpdateDisplay();
  }
  if (UpdateWave())
  {
    UpdateDisplay();
    if (playerHit)
      GameOver();
    else if (levelCleared)
      LevelCleared();
  }
  ScrollPhaser();
}

};
