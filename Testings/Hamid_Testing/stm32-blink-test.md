
# Day 01 — STM32 Blink Test

Project: RehabGrid
Board: STM32 NUCLEO-G0B1RE
IDE: SEGGER Embedded Studio

## Objective
Verify the STM32 can execute firmware and control the onboard LED.

## Configuration
- LED: PA5
- System clock: 64 MHz
- Timer: TIM3
- ON duration: 500 ms
- OFF duration: 500 ms

## Result
PASS — The onboard LED blinked successfully.

## Libraries
- CLOCK
- GPIO
- TIMER
