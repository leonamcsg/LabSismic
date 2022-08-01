#include <msp430.h>
#include <stdint.h>

/**
 * Leonam C. S. Gomes, 2022
 *
 * Gerando PWM de 50Hz em TA0.1, com T0 = CCR0 e DC = CCR1.
 * Com DC regulavel por S1 e S2.Timer a partir de SMCLOCK 1Mhrz, CCR0 = 20000,
 * T0 = 20ms, ciclo de trabalho regulavel, modo reset/set.
 *
 */

void IOconfig(void);

void initTimerA0(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                 // Para WDT

    IOconfig();                               // Configura pinos de entrada e saida
    initTimerA0();
    __enable_interrupt();                     // Habilita interrupcoes, GIE bit em SR

    __bis_SR_register(LPM0_bits);             // Modo de baixo consumo
    __no_operation();
}

void IOconfig(void) {
    P1DIR |= BIT2;              // P1.2 como saida
    P1SEL |= BIT2;              // Seleciona P1.2 como saida PWM

    P1DIR &= ~(BIT1);           // pino P1.1 como entrada
    P1REN |= (BIT1);            // habilita resistor
    P1OUT |= (BIT1);            // resistor pull-up

    P1IE  |= (BIT1);            // P1.1 interrupcao habilitada
    P1IES |= (BIT1);            // P1.1 Hi/lo edge
    P1IFG &= ~(BIT1);           // P1.1 IFG = 0

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up

    P2IE  |= (BIT1);            // P2.1 interrupcao habilitada
    P2IES |= (BIT1);            // P2.1 Hi/lo edge
    P2IFG &= ~(BIT1);           // P2.1 IFG = 0

}

void initTimerA0(void) {
    TA0CCR0 = 20000 - 1;                      // PWM periodo
    TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA0CCR1 = 10000 - 1;                      // CCR1 PWM duty cycle
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, limpa TAR
}

// Rotina do Servico de interrupcao da porta 2
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR() {
    volatile uint16_t debounce = 10000;
    static const uint16_t step = 2000;     // 10% de CCR0 + 1 = 2000
    switch (P2IV) {
        case 0x4:                          // P2.1 botao S1
            while(--debounce);
            if((P2IN & BIT1) == 0) {       // Caso botao esteja pressionado
                if(TA0CCR1 == 0) {
                    TA0CCR1 = step - 1;    // Caso zero, CCR1 = 1999
                } else if(TA0CCR1 < TA0CCR0) {
                    TA0CCR1 += step;       // Aumenta CCR1 em 2000 contagens
                }
            }
        break;
        default: break;
    }
}

// Rotina do Servico de interrupcao da porta 1
#pragma vector = PORT1_VECTOR
__interrupt void P1ISR() {
    volatile uint16_t debounce = 10000;
    static const uint16_t step = 2000;     // 10% de CCR0 + 1 = 2000
    switch (P1IV) {
        case 0x4:                          // P1.1 botao S2
            while(--debounce);
            if((P1IN & BIT1) == 0) {       // Caso botao esteja pressionado
                if(TA0CCR1 > step) {
                    TA0CCR1 -= step;       // Diminui CCR1 em 10% de CCR0
                } else if (TA0CCR1 == step - 1) {
                    TA0CCR1 = 0;           // Caso 1999, CCR1 = 0
                }
            }
        break;
        default: break;
    }
}

