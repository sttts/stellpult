#include <Arduino.h>

// Set up the WDT to perform a system reset if the loop() blocks for more than 1 second
void watchdog_setup() {
 // Set up the generic clock (GCLK2) used to clock the watchdog timer at 1.024kHz
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(4) |            // Divide the 32.768kHz clock source by divisor 32, where 2^(4 + 1): 32.768kHz/32=1.024kHz
                    GCLK_GENDIV_ID(2);              // Select Generic Clock (GCLK) 2
  while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_DIVSEL |          // Set to divide by 2^(GCLK_GENDIV_DIV(4) + 1)
                     GCLK_GENCTRL_IDC |             // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |           // Enable GCLK2
                     GCLK_GENCTRL_SRC_OSCULP32K |   // Set the clock source to the ultra low power oscillator (OSCULP32K)
                     GCLK_GENCTRL_ID(2);            // Select GCLK2         
  while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

  // Feed GCLK2 to WDT (Watchdog Timer)
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |           // Enable GCLK2 to the WDT
                     GCLK_CLKCTRL_GEN_GCLK2 |       // Select GCLK2
                     GCLK_CLKCTRL_ID_WDT;           // Feed the GCLK2 to the WDT
  while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

  REG_WDT_CONFIG = WDT_CONFIG_PER_4K;               // Set the WDT reset timeout to 1 second
  while(WDT->STATUS.bit.SYNCBUSY);                  // Wait for synchronization
  REG_WDT_CTRL = WDT_CTRL_ENABLE;                   // Enable the WDT in normal mode
  while(WDT->STATUS.bit.SYNCBUSY);                  // Wait for synchronization
}

// Set up the WDT to perform a system reset if the loop() blocks for more than 1 second
void watchdog_setup2() {
  // Feed GCLK2 to WDT
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_ID_WDT |        // Identify it as the WDT clock
                     GCLK_CLKCTRL_CLKEN |         // Enable the generic clock by setting the Clock Enabler bit
                     GCLK_CLKCTRL_GEN_GCLK2;      // Select GCLK2
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_WDT_EWCTRL = WDT_EWCTRL_EWOFFSET_8K;        // Set the Early Warning Interrupt Time Offset to 8 seconds 
  REG_WDT_INTENSET = WDT_INTENSET_EW;             // Enable the Early Warning interrupt
  REG_WDT_CONFIG = WDT_CONFIG_PER_16K;            // Set the WDT reset timeout to 16 seconds
  REG_WDT_CTRL = WDT_CTRL_ENABLE;                 // Enable the WDT in normal mode
  while (WDT->STATUS.bit.SYNCBUSY);               // Await synchronization of registers between clock domains

  // Configure and enable WDT interrupt
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);                  // Top priority
  NVIC_EnableIRQ(WDT_IRQn);
}

void watchdog_loop() {
  if (!WDT->STATUS.bit.SYNCBUSY)                // Check if the WDT registers are synchronized
  {
    REG_WDT_CLEAR = WDT_CLEAR_CLEAR_KEY;        // Clear the watchdog timer
  }
}
