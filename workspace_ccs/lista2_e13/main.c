#include <msp430.h>
#include <stdint.h>

/**
 * Leonam C. S. Gomes, 2022
 *
 * Gerando PWM de 50Hz e DC = 50% na saida TA0.1, com T0 = CCR0 e DC = CCR1.
 *
 */

//
//-------     -------     -------
//|     |     |     |     |         Periodo do ciclo, Timer a partir de SMCLOCK
//      -------     -------         1Mhrz, CCR0 = 20000, T0 = 20ms,
//
//----  ----  ----  ----  ----  -
//|  |  |  |  |  |  |  |  |  |  |   50% do ciclo de trabalho, 50Hz, modo reset/set,
//   ----  ----  ----  ----  ----   T1 = 10ms.
//

void IOconfig(void);

void initTimerA0(void);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

    IOconfig();                               // Configura pinos de entrada e saida
    initTimerA0();

    __bis_SR_register(LPM0_bits);             // Modo de baixo consumo
    __no_operation();
}

void IOconfig(void) {
    P1DIR |= BIT2;                            // P1.2 como saida
    P1SEL |= BIT2;                            // Seleciona P1.2 como saida PWM
}

void initTimerA0(void) {
    TA0CCR0 = 20000 - 1;                      // PWM periodo
    TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA0CCR1 = 10000 - 1;                      // CCR1 PWM duty cycle
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, limpa TAR
}
