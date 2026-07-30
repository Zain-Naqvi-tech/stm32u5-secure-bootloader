# Project 4: Secure Bootloader

_July 26, 2026, 8:02 PM_

## Let's start off with the bootloader project

Using:
- https://www.st.com/en/evaluation-tools/b-u585i-iot02a.html
- https://os.mbed.com/platforms/ST-Discovery-B-U585I-IOT02A/

Datasheet for the STM32U585:
- https://www.st.com/resource/en/datasheet/stm32u585ai.pdf

Reference Manual:
- https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf

User Manual:
- https://www.st.com/resource/en/user_manual/um2839-discovery-kit-for-iot-node-with-stm32u5-series-stmicroelectronics.pdf

![Circuit Diagram of the STM32U585AI](image.png)

**MCU: STM32U585AI**

### Summary

Two independent security mechanisms that share a binary, plus an attack on our own work.

- Verified boot: answers the question, should I execute this image at all? We hash the app, check signature against a public key WE trust, and either jump or stop. After the jump, this mechanism is done.
- TrustZone-M: answers a different question for an entire runtime. NOW that trusted code is running, what is it allowed to touch? The hardware must be able to block non-secure apps from READING the key region at the bus level.

---

## Phase 0: Figure out how the system works

Get familiar with CubeIDE and the device I have. Blink an LED on it.

Where to plug in: CN8. This is the one we flash and debug from. Researched about the three plugins and this is the only one which allows us to do the natural flash/uart/debug out of the three.

CN8 goes straight to the ST-LINK debug chip which is somewhat a different mcu than the STM32U585 which we are using.

### GPIO on STM32 vs MSP

Ok so im used to the ONE-BIT system on msp. For stm32, we have a TWO-BIT system where each pin gets 2 bits (MODEy[1:0]).

- Pin 0 uses 0 and 1
- Pin 1 uses 2 and 3
- Pin 5 uses 10 and 11

In order to use these GPIOH_MODER things, we need to import the CMSIS header file like before.

### Let's do an LED run

LD6 and LD7 are PH6 and PH7 GPIO respectively.

1. Peripheral clock enable register 1 has a GPIOH enable bit (Bit 7).

Types of registers in this are different to what we're used to with the msp:
- Direction: GPIOx_MODER register (GPIO Port Mode Register)
- Push-pull vs open-drain: GPIOx_OTYPER
- Speed: GPIOx_OSPEEDR
- Internal pull-up/down: GPIOxPUPDR

Direction: 01 is the GP output Mode on the MODER register.

Doing it like this:

```c
GPIOH->MODER &= ~(3U << (PIN * 2));
GPIOH->MODER |= (1U << (PIN * 2));
```

We clear the two bits IN the pin first, then set it as 01 for output direction.

```c
REG &= ~(FIELD_MASK << (index * FIELD_WIDTH)); // clear
REG |= (VALUE << (index * FIELD_WIDTH));        // set
```

- FIELD_MASK: 4-bit means 0xF and 2-bit means 0x3 (2^width - 1)
- Index: Position of the field WITHIN the register
- FIELD_WIDTH: 4 or 2 or 1 or however much the width is
- Value: Value u need to write into it. Like for AF7, we need to write in 7 so we use 7U

The LEDs turn on when driven LOW and turn off when driven HIGH.

### GPIOx_ODR vs GPIOx_BSRR

- ODR: Output Data Register. This is a read/write register that stores the actual data to be output on the corresponding GPIO pins.
- BSRR (Bit Set/Reset Register): This is a 32-bit register designed to let you safely change individual bits in the ODR in a single one-shot operation without risking changing other pins.
  - Bits 0-15 (Bit Set): Writing a 1 to these lower bits forces the corresponding ODR bit to 1 HIGH.
  - Bits 16-31 (Bit Reset): Writing a 1 to these upper bits forces the ODR bit to 0 LOW.

So, to turn ON the LED, we write a 1 to the reset bits (BR6 and BR7) in the BSRR register. Note: Reset bits are in the upper half (16-31) so remember that bit 6 is now bit 22 and bit 7 is bit 23.

Ok so the LEDs blink and WE ARE GOOD TO GO.

I am now more familiar with this new device and its things, so let's get to reading.

---

## Let's do some UART now

USART: supports both synchronous and asynchronous modes. Uses a CLOCK.

- We have 3 of these in our MCU, so let's use them. USART1/2/3.
- Bit 14 of the RCC APB2 peripheral clock enable register (RCC_APB2ENR) needs to be set for clock enable.
- USART1_TX: PA9
- USART1_RX: PA10
- AF7 for both (alternate function)

So for the setting of AF7 PIN 9 and 10, we need to use the GPIO alternate function HIGH register called GPIOx_AFHL (pin 15 to 8). This includes AFSEL for pins 0 to 7.

- So every AFSEL has 4 bits. We need AF7 which is 0111.
- GPIOx->AFR[0] is low and AFR[1] is high.
- So we can write 0x00000770 to index 1 and it should be good (use |= ofc).

---

## PLL Setup

So /Core/Inc will include all the headers. Let's make a PLL one which just has a PLL init function.

My decision for 160MHz: boot time is small and power constraints would not matter as much for this time period. We can maximize our frequency.

Ok so, for a clock to actually function and register, there are THREE important factors:

- Power: What VCORE range do I need for this target frequency? DO I need a power booster?
- Memory (Flash): How many wait states does the flash memory need to safely execute code at this speed?
- Clock: how do I configure the clock source and PLL dividers to synthesize this exact speed?

The RCC has the main PLL called PLL1.

The PLLs are controlled via RCC_PLLxDIVR, RCC_PLLxFRACR, RCC_PLLxCFGR, and RCC_CR (x = 1, 2, 3). (pasted from RM pg. 493)

- Reduce power consumption -> configure VCOx output to the lowest frequency. VCO = voltage controlled oscillator.
- Input frequency = 4 to 16MHz (Frefx_ck)
- PLLxN loop divider works to give the expected frequency at VCO output.
- PLLxFRACEN is 0 -> integer mode.
- Enabled with PLLxON=1 in RCC_CR.

> The following PLL parameters cannot be changed once the PLL is enabled: PLLxN, PLLxRGE, PLLxP, PLLxQ, and PLLxR. To ensure an optimal behavior of the PLL when one of the post-dividers (PLLxP, PLLxQ, or PLLxR) is not used, the application must clear the enable bit (PLLxPEN, PLLxQEN, PLLxREN), and configure the corresponding post-dividers to their minimum value (PLLxR = 0, PLLxP = 0, or PLLxQ = 0). If the above rules are not respected, the PLL output frequency is not guaranteed. (RM pg. 494)

Integer mode, VCO frequency:
- Fvcox = Frefx_ck * PLLxN
- 320 = 16 * 20, PLLN=20, ref=16, fvco=320 (MHz)

So for this, we obv need to choose a number from the range of 128-544MHz for the VCO frequency.

A rule is: maximize the VCO frequency to MINIMIZE the PLL jitter, while choosing a value that is an EXACT multiple of the target output frequency (160MHz).

Initialization Phase:
- Initialize the PLLs registers according to the required frequency.
- Set PLLxFRACEN to 0 in RCC_PLL1CFGR.
- PLLxON=1, wait until PLLxRDY=1 (busy wait loop).

Flow as shown in the RM:

![PLL enable sequence, integer mode (from the RM)](image-1.png)

- 0x000000100
- We need to use HSI16 clock for PLL1 entry clock source (to match the 16MHz we decided earlier).
- PLL1P: Main System Clock. Route this.
- Ok so im having a bit of a hiccup so ill watch a yt vid: just not sure about the calculations so it's good to have a second look.

![Clock sources in the MCU (Fastbit Embedded Brain Academy)](image-2.png)

- Ok so this person targets a 168MHz clock OUTPUT.
- VCO boosts the frequency.
- Ok yeah this makes a lot more sense. Let's do the calcs again and see.

Redo the calcs:
- We don't need PLLQ for sure.
- We want SysClk to be 160MHz.
- We are using HSI for sure. Our VCO has a range of 128-544MHz.
- The input frequency range to VCO is 4-16MHz. So we can divide HSI by one and keep the VCO input the same as 16MHz.
- So PLLM=1.
- 16*20=320 which divided by 2 makes 160 which is what I want.
- Fref = 16MHz. VCO output is 320MHz. Sysclk is now 160MHz. PLLP is 2.

![Figure 38. Clock tree for STM32U5 series](image-3.png)

- Ok so I just figured out from this diagram that only the pll1_r_ck goes to the system clock. So all we need to do is swap the things we did for P with R and we good.

### Power for the 160MHz output

Next step for PLL is working with power. We need enough power for the 160MHz output to function properly.

Dynamic Voltage Scaling Management:
- Increasing/decreasing voltage used for Vcore according to the application performance and power consumption needs.
- Dynamic voltage scaling to increase Vcore is overvolting, opposite is undervolting which is used to save power in stuff like phones and laptops.
- We need Range 1L high output voltage at 1.2V. Used when the sysclk f is upto 160MHz which it is for us.
- Section 10.5.4 page 410 has a map for setting this voltage. Let's get to the flow. I will code using the flow, then check back in with the flow and notes on what happened during the process.

Notes for the voltage scaling settings:
- PLL1MBOOST is the prescalar for EPOD booster input clock. Configures the prescalar of the PLL1, used for the EPOD booster. The EPOD booster input frequency is: PLL1 input clock frequency / PLL1MBOOST.
- Important note on the BOOST stuff: it can only be written when the PLL1 is disabled (PLL1ON=0 and PLL1RDY=0).
- We need to do PLL1MBOOST=0000 because we need 16MHz/1 = 16MHz.
- In order to clear those bits, we first need to make sure that the PLL1 is disabled and the EPODBoost mode is disabled in the PWR_VOSR (power voltage scaling register).
- Just learned something new: In order to use a clock source, one must TURN IT ON before selecting it for the PLL.

### Wait States

Ok just got stuck on Wait States:
- The reason we need wait states is because at times memory cant return data in one cycle. We need wait states to transfer it in the next data cycle.
- The number of wait states is related to the HCLK frequency and the flash's extra time.
- So we need to find a value which says that for this voltage range, for this HCLK band, use this many wait states.
- Found the table, so we need 0 wait states.

Ok while programming, I came across a better difference definition for Flash memory and SRAM:
- Flash Memory: non-volatile. Uses floating-gate transistors which are physically slower to read. At 160MHz, a CPU cycle is only 6.25ns which requires a 4-wait-state pause (from table 54. on chapter 7.3.3 page 295 of the RM).
- SRAM: volatile and integrated on the high-speed CPU bus matrix. Incredibly fast and operates at true CPU core speed. Therefore, it needs 0 wait states.

### Switch the system clock

- Now all that is left is switching the system clock to the PLL r.
- This can be done from the RCC_CFGR1 register.
- DONEEEEE

---

## USART programming

Now we need to program the usart for our specific message delivery use.
- Set the Baud Rate.
- Configure Stop bits.
- Configure Word Length (we can make it 8 bits and one stop bit).
- Enable the Peripheral AND the RX TX enable and Parity.

Baud Rate:
- Equation: Baud Rate = usart_ker_ck_pres / USARTDIV
- In the reset state where the USART's internal clock prescaler register is set to divide-by-1: usart_ker_ck_pres = System Clock Frequency.

Basic Steps I did:
1. Enable USART Clock.
2. Enable GPIO Port A Clock.
3. Make Port A Pins to AF mode using the MODER register.
4. Set the AFR Register indices to the required Afn number.

---

## Phase 1: Memory Map

Readings: