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
    P2DIR |= BIT5;              // P2.5 como saida
    P2SEL |= BIT5;              // Seleciona P2.5 como saida PWM

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
    TA2CCR0 = 20000 - 1;                      // PWM periodo
    TA2CCTL2 = OUTMOD_7;                      // CCR1 reset/set
    TA2CCR2 = 10000 - 1;                      // CCR1 PWM duty cycle
    TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, limpa TAR

    TB0CCR0 = 0;                              // Inicialmente, para o timer
    TB0CTL = TBSSEL__ACLK | MC__UP | TBCLR;   // ACLK, up mode, limpa TAR
}

//Timer ISR
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B_CCR0_ISR(void) {
    static const uint16_t step = 2000;  // 10% de CCR0 + 1 = 2000
    if((P2IN & BIT1) == 0) {            // Caso botao S1 esteja pressionado
        if(TA2CCR2 == 0) {
            TA2CCR2 = step - 1;         // Caso zero, CCR1 = 1999
        } else if(TA2CCR2 < TA2CCR0) {
            TA2CCR2 += step;            // Aumenta CCR1 em 10% de CCR0 = 2000
        }
    }
    if((P1IN & BIT1) == 0) {            // Caso botao S2 esteja pressionado
        if(TA2CCR2 > step) {
            TA2CCR2 -= step;            // Diminui CCR1 em 10% de CCR0 = 2000
        } else if (TA2CCR2 == step - 1) {
            TA2CCR2 = 0;                // Caso 1999, CCR1 = 0
        }
    }
    TB0CCTL0 &= ~CCIE;                  // Desabilita timerB
}


// Rotina do Servico de interrupcao da porta 2
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR() {
    volatile uint16_t debounce = 328;
    switch (P2IV) {
        case 0x4:                          // P2.1 botao S1
            TB0CCTL0 |= CCIE;              // Habilita TimerB
            TB0CCR0 = debounce;            // Adiciona ~10ms de debounce
            TB0CCTL0 &= ~CCIFG;            // Limpa flags de interrupcao
        break;
        default:
            break;
    }
}

// Rotina do Servico de interrupcao da porta 1
#pragma vector = PORT1_VECTOR
__interrupt void P1ISR() {
    volatile uint16_t debounce = 328;
    switch (P1IV) {
        case 0x4:                          // P1.1 botao S2
            TB0CCTL0 |= CCIE;              // Habilita TimerB
            TB0CCR0 = debounce;            // Adiciona ~10ms de debounce
            TB0CCTL0 &= ~CCIFG;            // Limpa flags de interrupcao
        break;
        default: break;
    }
}
