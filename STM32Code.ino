/* Created by Amit Kumar
 * github.com/abtAmit/5-Channel-AC-Dimmer-and-Switch-Module-with-STM32
 * STM32F103C8T6 5-Channel AC Dimmer 
 * Framework: Arduino IDE (STM32duino Official Core)
 * * Inputs: PA3, PA4, PA5, PA6, PA7 (Analog)
 * Outputs: PB12, PB13, PB14, PB15, PA8 (TRIAC Gates via Optocouplers)
 * ZCD Input: PB10 (Physical)
 */

#define ZCD_PIN PB10

const int analogPins [] = {PA3, PA4, PA5, PA6, PA7};
const int triacPins [] = {PB12, PB13, PB14, PB15, PA8};

volatile int tick_counter = 0;
volatile int dim_delays []= {95, 95, 95, 95, 95};

HardwareTimer *DimTimer;
HardwareTimer *FakeZcdTimer; // Timer for simulation

void setup() {
  // Unlock the full 12-bit resolution of the STM32 ADC
  analogReadResolution(12);

  for (int i = 0; i < 5; i++) {
    pinMode(triacPins[i], OUTPUT);
    digitalWrite(triacPins[i], LOW);
    pinMode(analogPins[i], INPUT_ANALOG);
  }

  // ZDC interrupt
  pinMode(ZCD_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ZCD_PIN), zcd_isr, RISING); 

  // Triac TIMER (Timer 2) 
  DimTimer = new HardwareTimer(TIM2);
  DimTimer->setOverflow(10000, MICROSEC_FORMAT); // 100us ticks
  DimTimer->attachInterrupt(dimmer_timer_isr);
  DimTimer->resume();

}

void loop() {
  for (int i = 0; i < 5; i++) {
    // Dummy read to switch multiplexer and prevent crosstalk
    analogRead(analogPins[i]); 
    delay(1); 
    
    // 2. Actual read 
    int adc_val = analogRead(analogPins[i]); 
    
    // 3. Map to phase ticks (95 = almost off, 5 = almost fully on)
    dim_delays[i] = map(adc_val, 0, 4095, 95, 5); 
  }
  delay(10);
}

// --- INTERRUPT SERVICE ROUTINES ---


//  ZCD logic (resets the phase angle)
void zcd_isr() {
  tick_counter = 0; 
  for (int i = 0; i < 5; i++) {
    digitalWrite(triacPins[i], LOW);
  }
}

// Phase control timer (fires every 100us)
void dimmer_timer_isr() {
  tick_counter++; 
  
  for (int i = 0; i < 5; i++) {
    if (tick_counter == dim_delays[i]) {
      digitalWrite(triacPins[i], HIGH);
    } 
    else if (tick_counter == dim_delays[i] + 1) {
      digitalWrite(triacPins[i], LOW);
    }
  }
}
