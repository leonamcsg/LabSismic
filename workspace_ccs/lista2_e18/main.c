#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>

/**
 * Mede o tempo de "bouncing" do botao S1 P2.1
 */
#define INDEX 100

volatile uint16_t captures[INDEX] = {0};  // Endereco 0x2400
volatile uint16_t captures_index = 0;
volatile uint16_t last_captured = 0;
volatile uint16_t first_capture = 1;


void initTimers(void);

void IOconfig(void);

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
    TA1CTL |= TACLR         | // Limpa TA0R
              MC_3          | // Modo "Up/Down"
              TASSEL__SMCLK | // CLK = 1Mhz
                      ID_3;   // TA1CLK = 1Mhz/8 = 125Khz


    TA1CCTL0 |= OUTMOD_1;     // Modo "set"
    TA1CCR0  = 62500 - 1;     // 500 ms x 2 = 1 segundo

    TA1CCTL2 |= CAP   |       // Modo de captura
                CCIE  |       // Habilita interrupcao
                CM_3;         // Captura na subida e descida do clk

    TA1CCTL2 &= ~CCIFG;       // Limpa flag de interrupcao

}

void IOconfig(void) {

    P2DIR &= ~(BIT1);           // pino P2.1 como entrada
    P2REN |= (BIT1);            // habilita resistor
    P2OUT |= (BIT1);            // resistor pull-up

    P2IE  |= (BIT1);            // P2.1 interrupcao habilitada
    P2SEL |= (BIT1);            // P2.1 Como TA1CCR2
    P2IFG &= ~(BIT1);           // P2.1 IFG = 0

}

// Rotina do Servico de Interrupcao do TimerA1
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMERA1_TA1IV_ISR() {
    switch (TA1IV) {
        case TA1IV_TACCR2:
            if (first_capture) {

                TA1CTL |= TACLR | // Limpa TA0R
                          MC_3  | // Modo "Up/Down"
                            ID_3; // TA1CLK = 1Mhz/8 = 125Khz

                TA1CCTL2 &= ~CCIFG;       // Limpa flag de interrupcao
                TA1CCTL0 &= ~CCIFG;       // Limpa flag de interrupcao

                first_capture = 0;
            } else {
                if ((captures_index < INDEX) &
                  ((TA1CCTL0 & CCIFG) == 0)) {

                    captures[captures_index++] = TA1CCR2 - last_captured;
                    last_captured = TA1CCR2;
                    TA1CTL &= ~CCIFG;
                }
            }
            break;
        default:
            break;
    }
}
