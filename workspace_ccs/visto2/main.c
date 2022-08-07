/* Leonam Christian Silva Gomes, 190043318
 */
#include <msp430.h>
#include <stdint.h>
#include <intrinsics.h>
/**
 * Sensor ultrassonico HC-SR04 com mostrador analogico utilizando
 * servo motor SG90 Tower Pro e led1 e led2.
 */
static const uint16_t lowlimit = 500 - 1; //  0 graus
static volatile uint16_t newdistance;
static volatile uint16_t PRONTO = 0;

void IOconfig(void);

void initTimers(void);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Para WDT

    static const uint16_t uplimit = 2500 - 1; //  180 gaus
    static volatile uint16_t newTA2CCR2;

    IOconfig();                             // Configura pinos de entrada e saida
    initTimers();                           // Inicia temporizadores
    __enable_interrupt();                   // Habilita interrupcoes, GIE bit em SR;
    for(;;) {

        while(!PRONTO);                     // Aguarda sinal de TA0CCR2
        PRONTO = 0;

        newTA2CCR2 = newdistance / 1.45 + lowlimit;

        // Atualiza PWM P2.5 com nova distancia
        if(newTA2CCR2 >= uplimit) {          // Caso CCR1 maior que limite superior
            TA2CCR2 = uplimit;               // Limitando maior valor de CCR2
        } else {
            TA2CCR2 = newTA2CCR2;            // Atualiza TACCR2 com novo tempo
        }

        // LEDs
        if(TA2CCR2 >= uplimit) {             // Maior que 2500
            P1OUT &= ~BIT0;      // Desliga Led1
            P4OUT &= ~BIT7;      // Desliga Led2
        } else if (TA2CCR2 >= 1700 - 1) {    // Entre 2500 e 1700
            P1OUT &= ~BIT0;      // Desliga Led1
            P4OUT |= BIT7;       // Liga Led2
        } else if (TA2CCR2 >= 900 - 1){      // Entre 900 e 1700
            P1OUT |= BIT0;       // Liga Led1
            P4OUT &= ~BIT7;      // Desliga Led2
        } else {
            P1OUT |= BIT0;       // Liga Led1
            P4OUT |= BIT7;       // Liga Led2
        }
    }
}

void IOconfig(void) {
    P1OUT &= ~BIT0;             // Inicia LED2 desligado
    P4OUT &= ~BIT7;             // Inicia LED1 desligado
    P4DIR |= BIT7;              // P4.7(LED2) como saida
    P1DIR |= BIT0;              // P1.0(LED1) como saida

    P2DIR |= BIT5;              // P2.5 como saida
    P2SEL |= BIT5;              // Seleciona P2.5 como saida PWM

    P1DIR |= BIT2;              // Sinal trigger(saida)
    P1DIR &= ~BIT3;             // Sinal echo(captura)
    P1SEL |= BIT2 | BIT3;       // P1.2 como TA0CCR1 e P1.3 como TA0CCR2
}

void initTimers(void) {

    // TimerA2 PWM P2.5
    TA2CCR0 = 20000 - 1;                      // PWM periodo
    TA2CCTL2 = OUTMOD_7;                      // CCR2 reset/set
    TA2CCR2 = lowlimit;                       // CCR2 PWM duty cycle
    TA2CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, limpa TAR

    // TimerA0 Sensor Ultrassonico, Trigger -> P1.2(PWM), Echo -> P1.2(CAP)
    TA0CTL |= TASSEL__SMCLK |                 // SMCLK = 1Mhz
              MC__UP        |                 // Modo "up"
              TACLR;                          // Limpa TA0R

    TA0CCR0 = 12000 - 1;                      // Periodo total, trigger ate tempo maximo 12ms
    TA0CCR1 = 10 - 1;                         // Pulso trigger dura 10us

    TA0CCTL1 |= OUTMOD_7;                     // Modo "set/reset"

    TA0CCTL2 |= CAP  |                        // Modo captura
                CM_3 |                        // Modo de subida e descida do CLK
                CCIE;                         // Habilita interrupcao TA0CCR2
}

// Servico de inrrupcao de rotina TimerA0
#pragma vector = TIMER0_A1_VECTOR
__interrupt void timerA0_TA0IV_ISR(void) {
    static volatile uint16_t count = 0;
    static volatile uint16_t lastcaptured = 0;

    switch (TA0IV) {                          // Leitura limpa flag de interrupcao
        case 0:
            break;

        case TA0IV_TA0CCR2:                   // Se CCR2 ativou interrupcao
            if(count != 0) {                  // Conta subida e descida
                PRONTO = 1;                   // Sinaliza processo para main()
                count = 0;
            } else {
                count = 1;                    // Contagem
            }
            newdistance = TA0CCR2 - lastcaptured; // 0 a 12ms
            lastcaptured = TA0CCR2;

            TA0CCTL2 &= ~CCIFG;                // Limpa flag de interrupcao de CCR2
            break;

        default:
            break;
    }
}

