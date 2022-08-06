#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>

/**
 * Mede o intervalo entre pressionamento de S1 e S2
 * Utilizando TimerA em modo de captura. T < 1s led1
 * 1 < T < 3s Led1 e Led2, 3s < T Led2.
 */

volatile uint16_t lasts1 = 1;

void initTimers(void);

int IOconfig(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida
    initTimers();

    __enable_interrupt();               // Habilita interrupcoes, GIE bit em SR

    __bis_SR_register(0xD8);            // Somente ACLK ligado
    __no_operation();                   // Aguarda interrupcao

}

void initTimers(void) {
    //TIMER_A - principal
    TA0CTL |= TACLR        | // Clock auxiliar = 32768 Hz
              MC_2         | // Modo continuo
              TASSEL__ACLK | // Limpa TA0R
                      ID_1;  // TACLK = 32768/2 = 16384 Hz


    TA0CCTL0 |= CAP   |       // Modo de captura
                CM_3;         // Captura na subida e descida do clk


    TA0CCTL1 = OUTMOD_1;      // Modo "set"
    TA0CCR0 = 49152 - 1;      // 3 segundos

    // TIMER_B - captura de evento dos botoes
    TB0CTL = TBSSEL__ACLK | MC__UP | TBCLR;   // ACLK, up mode, limpa TAR
    TB0CCTL0 &= ~CCIE;
    TB0CCR0 = 0;
    TB0CCTL0 &= ~CCIFG;
}

int IOconfig(void) {
    P1OUT &= ~(BIT0);           // inicia led desligado (ativo-alto)
    P1DIR |= (BIT0);            // pino P1.0 como saida

    P4OUT &= ~(BIT7);           // inicia led desligado (ativo-alto)
    P4DIR |= (BIT7);            // pino P2.0 como saida

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up

    P2IE  |= (BIT1);            // P2.1 interrupcao habilitada
    P2IES |= (BIT1);            // P2.1 Hi/lo edge
    P2IFG &= ~(BIT1);           // P2.1 IFG = 0

    // Botao ligado a CCR0
    P1REN |= (BIT1);            // habilita resistor
    P1OUT |= (BIT1);            // resistor pull-up
    P1DIR &= ~(BIT1);           // Pino P1.1 como entrada
    P1SEL = BIT1;               // Como CCR0

    return 0;
}

//TimerA ISR
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A_CCR0_ISR(void) {

    if ((TA0CCTL1 & CCIFG) != 0) { // houve overflow
        P1OUT &= ~BIT0;            // Desliga led1
        P4OUT |= BIT7;             // Liga led 2
    } else if (TA0CCR0 < 16384 - 1) {
        P1OUT |= BIT0;             // Liga led1
        P4OUT &= ~BIT7;            // Desliga led 2
    } else {
        P1OUT |= BIT0;             // Liga Led1
        P4OUT |= BIT7;             // Liga Led2
    }

    TA0CCTL0 &= ~CCIE;             // Desabilita timerB
}


//TimerB ISR
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B_CCR0_ISR(void) {
    volatile uint16_t s1 = (P2IN & BIT1);

    if(s1 == 0) {               // Caso botao esteja pressionado

        P1OUT &= ~BIT0;           // Desliga led 1
        P4OUT &= ~BIT7;           // Desliga led 2

        // Inicia TimerA0
        TA0CTL |= TACLR        | // Clock auxiliar = 32768 Hz
                  MC_2         | // Modo continuo
                          ID_1;  // TACLK = 32768/2 = 16384 Hz

        TA0CCTL0 |= CCIE;        // Habilita interrupcao CCR0
        TA0CCTL0 &= ~CCIFG;      // Limpa flags de interrupcao
        TA0CCTL1 &= ~CCIFG;      // Limpa flags de overflow
        //FIM

        lasts1 = 0;
    } else {                    // Botao solto mas houve interrupcao
        if(lasts1 == 1) {       // E estava solto antes da interrupcao

            P1OUT &= ~BIT0;           // Desliga led 1
            P4OUT &= ~BIT7;           // Desliga led 2

            // Inicia TimerA0
            TA0CTL |= TACLR        | // Clock auxiliar = 32768 Hz
                      MC_2         | // Modo continuo
                              ID_1;  // TACLK = 32768/2 = 16384 Hz

            TA0CCTL0 |= CCIE;        // Habilita interrupcao CCR0
            TA0CCTL0 &= ~CCIFG;      // Limpa flags de interrupcao
            TA0CCTL1 &= ~CCIFG;      // Limpa flags de overflow
            //FIM
        }
        lasts1 = 1;
    }

    TB0CCTL0 &= ~CCIE;          // Desabilita timerB
}

// Rotina do Servico de interrupcao da porta 2
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR() {
    volatile uint16_t debounce = 328;  // MCLOCK em 1MHz, delay ~= 10ms
    switch (P2IV) {
        case 0x4:                       // P2.1 botao S1
            TB0CCTL0 |= CCIE;           // Habilita TimerB
            TB0CCR0 = debounce;         // Adiciona ~10ms de debounce
            TB0CCTL0 &= ~CCIFG;         // Limpa flags de interrupcao
            TB0CTL |= TBCLR;            // Zera contador
        break;
        default: break;
    }
}

