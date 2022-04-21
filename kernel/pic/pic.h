#ifndef PIC_H
#define PIC_H

void PIC_remap();
void PIC_sendEOI(unsigned char irq);
void IRQ_clear_mask(unsigned char IRQline);
void IRQ_set_mask(unsigned char IRQline);


#endif