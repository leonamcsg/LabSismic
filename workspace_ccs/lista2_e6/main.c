#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>

/**
 * Contador binario utilizando led1 e led2, incrementado por S2
 * e decrementado por S1
 */

volatile uint16_t lasts1 = 1;
volatile uint16_t lasts2 = 1;
volatile uint16_t s1request = 0;
volatile uint16_t s2request = 0;

void initTimers(void);

int IOconfig(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           // stop watchdog timer

    IOconfig();                         // Configura pinos de entrada e saida
    initTimers();

    __enable_interrupt();               // Habilita interrupcoes, GIE bit em SR

    for(;;) {                           // Loop principal
    }

}

void initTimers(void) {
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

    P1DIR &= ~(BIT1);           // pino P1.1 como entrada
    P1REN |= (BIT1);            // habilita resistor
    P1OUT |= (BIT1);            // resistor pull-up

    P1IE  |= (BIT1);            // P1.1 interrupcao habilitada
    P1IES |= (BIT1);            // P1.1 Hi/lo edge
    P1IFG &= ~(BIT1);           // P1.1 IFG = 0

    return 0;
}

//Timer ISR
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B_CCR0_ISR(void) {
    volatile uint16_t s2 = (P1IN & BIT1);
    volatile uint16_t s1 = (P2IN & BIT1);

    if (s2request) {
        if(s2 == 0) {                 // Caso botao esteja pressionado
           if((~P4OUT & BIT7) == 0)   // LED1 alterna de estado se LED2 = 1
               P1OUT ^= BIT0;
           P4OUT ^= BIT7;             // LED2 alterna de estado
           lasts2 = 0;
        } else {                      // Botao solto mas houve interrupcao
            if(lasts2 == 1) {         // Estava solto antes da interrupcao
                if((~P4OUT & BIT7) == 0) // LED1 alterna de estado se LED2 = 1
                   P1OUT ^= BIT0;
                P4OUT ^= BIT7;          // LED2 alterna de estado
            }
           lasts2 = 1;
        }
        s2request = 0;
    }

    if (s1request) {                // S1 causou interrupcao
        if(s1 == 0) {               // Caso botao esteja pressionado
            if((P4OUT & BIT7) == 0) // LED1 alterna de estado se LED2 = 0
                P1OUT ^= BIT0;
            P4OUT ^= BIT7;          // LED2 alterna de estado
            lasts1 = 0;
        } else {                    // Botao solto mas houve interrupcao
            if(lasts1 == 1) {         // Estava solto antes da interrupcao
                if((P4OUT & BIT7) == 0) // LED1 alterna de estado se LED2 = 0
                    P1OUT ^= BIT0;
                P4OUT ^= BIT7;          // LED2 alterna de estado
            }
            lasts2 = 1;
        }
        s1request = 0;
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
            s1request = 1;
        break;
        default: break;
    }
}

// Rotina do Servico de interrupcao da porta 1
#pragma vector = PORT1_VECTOR
__interrupt void P1ISR() {
    volatile uint16_t debounce = 328;  // MCLOCK em 1MHz, 1000/1Mhz ~= 10ms
    switch (P1IV) {
        case 0x4:                       // P1.1 botao S2
            TB0CCTL0 |= CCIE;           // Habilita TimerB
            TB0CCR0 = debounce;         // Adiciona ~10ms de debounce
            TB0CCTL0 &= ~CCIFG;         // Limpa flags de interrupcao
            TB0CTL |= TBCLR;            // Zera contador
            s2request = 1;
        break;
        default: break;
    }
}
