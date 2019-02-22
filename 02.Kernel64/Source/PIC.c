/*
 * PIC.c
 *
 *  Created on: 2019. 2. 13.
 *      Author: DEWH
 */
#include "PIC.h"

void kInitializePIC(void){
	//master initialize

	//ICW1 Port:0x20
	kOutPortByte(PIC_MASTER_PORT1, 0x11);
	//ICW2 Port:0x21
	kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);
	//ICW3 Port:0x21
	kOutPortByte(PIC_MASTER_PORT2, 0x04);
	//ICW4 Port:0x21
	kOutPortByte(PIC_MASTER_PORT2, 0x01);

	//slave initialize

	//ICW1 Port:0x20
	kOutPortByte(PIC_SLAVE_PORT1, 0x11);
	//ICW2 Port:0x21
	kOutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR+8);
	//ICW3 Port:0x21
	kOutPortByte(PIC_SLAVE_PORT2, 0x02);
	//ICW4 Port:0x21
	kOutPortByte(PIC_SLAVE_PORT2, 0x01);

}
void kMaskPICInterrupt(WORD wIRQBitmask){
	kOutPortByte(PIC_MASTER_PORT2, (BYTE)wIRQBitmask);
	kOutPortByte(PIC_SLAVE_PORT2, (BYTE)(wIRQBitmask>>8));
}
void kSendEOIToPIC(int iIRQNumber){
	kOutPortByte(PIC_MASTER_PORT1, 0x20);
	if(iIRQNumber>=8){
		kOutPortByte(PIC_SLAVE_PORT1, 0x20);
	}
}
