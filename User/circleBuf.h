//

#ifndef CIRCLE_BUF_H
#define CIRCLE_BUF_H

#include "globalDef.h"

//typedef u8 BufDataType;
#define BufDataType u8
//typedef u16 BufSizeType;
#define BufSizeType u16

typedef struct cicleBufStruct
{
	BufSizeType size; /// Size of this buffer
	volatile BufSizeType length; /// Current circleBufLength of valid buffer data
	volatile BufSizeType start; /// Current circleBufStart point of valid bufffer data
	BufDataType data[COMMAND_BUFFER_SIZE]; /// Pointer to buffer data
	volatile bool overflow; /// Overflow indicator
	volatile bool underflow; /// Underflow indicator
}circleBuf;

/// Reset the buffer to its empty state.  All data in
/// the buffer will be (effectively) lost.
void circleBufInit(circleBuf* pCB);
BufDataType* circleBufGetData(circleBuf* pCB,BufSizeType index);
/// Append a byte to the tail of the buffer
void circleBufPush(circleBuf* pCB,BufDataType b);
/// Pop a byte off the head of the buffer
BufDataType circleBufPop(circleBuf* pCB);

/// Pop a number of bytes off the head of the buffer.  If there
/// are not enough bytes to complete the pop, pop what we can and
/// set the underflow flag.
void circleBufPopSets(circleBuf* pCB,BufSizeType sz);
BufSizeType  circleBufGetLength(circleBuf* pCB);
BufSizeType  circleBufGetRemainCap(circleBuf* pCB);
bool circleBufIsEmpty(circleBuf* pCB);
bool circleBufHasOverflow(circleBuf* pCB);
bool circleBufHasUnderflow(circleBuf* pCB);

#endif

