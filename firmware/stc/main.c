#include "hal.h"
#include "petcargo.h"

/*
 * SDCC builds the 8051 interrupt vector table while compiling the translation
 * unit that contains main().  The handlers themselves live in hal.c, so their
 * interrupt-qualified declarations must be visible here; plain external
 * linkage is not enough for SDCC to emit the vector jumps.
 */
void timer0_isr(void) __interrupt (1);
void uart1_isr(void) __interrupt (4);
void uart2_isr(void) __interrupt (8);
void pca_isr(void) __interrupt (7);

void main(void)
{
    hal_init();
    petcargo_init();
    while (1) {
        petcargo_run_once();
    }
}
