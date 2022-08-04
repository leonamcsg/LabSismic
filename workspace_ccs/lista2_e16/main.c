#include <msp430.h>
#include <stdint.h>

/**
 * Leonam C. S. Gomes, 2022
 *
 * Gerando PWM de 50Hz em TA0.1, com T0 = CCR0 e DC = CCR1.
 * Com DC regulavel por S1 e S2, dentro do intervalo 0,5ms - 2,5ms.
 * Timer a partir de SMCLOCK 1Mhrz, CCR0 = 20000,
 * T0 = 20ms, ciclo de trabalho regulavel, modo reset/set.
 *
 */

void IOconfig(void);

void initTimers(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Para WDT

    IOconfig();                             // Configura pinos de entrada e saida
    initTimers();                           // Inicia temporizadores
    __enable_interrupt();                   // Habilita interrupcoes, GIE bit em SR

    TB0CCTL0 |= CCIE;
    TB0CCR0 = 0;
    TB0CCTL0 &= ~CCIFG;

    __bis_SR_register(LPM0_bits);           // Modo de baixo consumo
    __no_operation();
}

void IOconfig(void) {
    P4DIR |= BIT7;              // P4.7 como saida
    P1DIR |= BIT2 | BIT0;       // P1.2 e P1.0(LED1) como saida
    P1SEL |= BIT2;              // Seleciona P1.2 como saida PWM

    P1DIR &= ~(BIT1);           // Pino P1.1 como entrada
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

void initTimers(void) {
    TA0CCR0 = 20000 - 1;                      // PWM periodo
    TA0CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA0CCR1 = 1499 - 1;                       // CCR1 PWM duty cycle
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, limpa TAR

    TB0CTL = TBSSEL__ACLK | MC__UP | TBCLR;   // ACLK, up mode, limpa TAR
}

//Timer ISR
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B_CCR0_ISR(void) {
    static const uint16_t step = 111;         // 2000/18 = 111, ~9,99 graus
    static const uint16_t uplimit = 2498 - 1; // ~180 gaus
    static const uint16_t lowlimit = 500 - 1; //  0 graus

    if((P2IN & BIT1) == 0) {            // Caso botao S1 esteja pressionado
        if(TA0CCR1 < uplimit) {         // Caso CCR1 maior que limite superior
            TA0CCR1 += step;            // Aumenta CCR1 em ~10 graus
        } else if (TA0CCR1 > uplimit) {
            TA0CCR1 = uplimit;          // Limitando maior valor de CCR1
        }
    }
    if((P1IN & BIT1) == 0) {            // Caso botao S2 esteja pressionado
        if(TA0CCR1 > lowlimit) {        // Se CCR1 > 499
            TA0CCR1 -= step;            // Diminui CCR1 em 10 graus
        } else if(TA0CCR1 < lowlimit) {
            TA0CCR1 = lowlimit;         // Limitando menor valor de CCR1
        }
    }

    //Leds
    switch(TA0CCR1) {
    case 500 - 1:
        P1OUT |= BIT0;    // Liga Led1
        P4OUT &= ~BIT7;   // Desliga Led2
        break;
    case 2498 - 1:        // Limite superior
        P1OUT &= ~BIT0;   // Desliga Led1
        P4OUT |= BIT7;    // Liga Led2
        break;
    case 1499 - 1:
        P1OUT |= BIT0;    // Liga Led1
        P4OUT |= BIT7;    // Liga Led2
        break;
    default:
        break;
    }
    TB0CCTL0 &= ~CCIE;                  // Desabilita timerB
}

// Rotina do Servico de interrupcao da porta 2
#pragma vector = PORT2_VECTOR
__interrupt void P2ISR() {
    volatile uint16_t debounce = 328;
    switch (P2IV) {
        case 0x4:                       // P2.1 botao S1
            TB0CCTL0 |= CCIE;           // Habilita TimerB
            TB0CCR0 = debounce;         // Adiciona ~10ms de debounce
            TB0CCTL0 &= ~CCIFG;         // Limpa flags de interrupcao
            TB0CTL |= TBCLR;            // Zera contador
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
        case 0x4:                       // P1.1 botao S2
            TB0CCTL0 |= CCIE;           // Habilita TimerB
            TB0CCR0 = debounce;         // Adiciona ~10ms de debounce
            TB0CCTL0 &= ~CCIFG;         // Limpa flags de interrupcao
            TB0CTL |= TBCLR;            // Zera contador
        break;
        default: break;
    }
}
