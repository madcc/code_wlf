//
#include "circleBuf.h"
#include "stdlib.h"

/// Reset the buffer to its empty state.  All data in
/// the buffer will be (effectively) lost.
void circleBufInit(circleBuf* pCB)/*{{{*/
{
   	pCB->length = 0;
	pCB->start = 0;
	pCB->size = COMMAND_BUFFER_SIZE;
	//pCB->data=malloc(sizeof(BufDataType)*COMMAND_BUFFER_SIZE);
	pCB->overflow = false;
	pCB->underflow = false;
}/*}}}*/
BufDataType* circleBufGetData(circleBuf* pCB,BufSizeType index)/*{{{*/
{
	BufSizeType actualIndex=(index+pCB->start) %pCB->size;
	return &(pCB->data[actualIndex]);
}/*}}}*/
/// Append a byte to the tail of the buffer
void circleBufPush(circleBuf* pCB,BufDataType b)/*{{{*/
{
	if(pCB->length < pCB->size)	
	{
		*(circleBufGetData(pCB,pCB->length))=b;
		pCB->length++;
	}
	else
		pCB->overflow=true;
}/*}}}*/
/// Pop a byte off the head of the buffer
BufDataType circleBufPop(circleBuf* pCB)/*{{{*/
{
	BufDataType* popByte;
	if(circleBufIsEmpty(pCB))
	{
		pCB->underflow=true;
		return *circleBufGetData(pCB,0);//maybe not correct,RY
	}
	
	popByte = circleBufGetData(pCB,0);
	pCB->start=(pCB->start + 1) % pCB->size;
	pCB->length--;
	return *popByte;
}/*}}}*/

/// Pop a number of bytes off the head of the buffer.  If there
/// are not enough bytes to complete the pop, pop what we can and
/// set the underflow flag.
void circleBufPopSets(circleBuf* pCB,BufSizeType sz)/*{{{*/
{
	if(pCB->length < sz)
	{
		pCB->underflow=true;
		sz=pCB->length;
	}
	pCB->start=(pCB->start + sz) % pCB->size;
	pCB->length-=sz;
}/*}}}*/
BufSizeType  circleBufGetLength(circleBuf* pCB)/*{{{*/
{
	return pCB->length;
}/*}}}*/
BufSizeType  circleBufGetRemainCap(circleBuf* pCB)/*{{{*/
{
	return pCB->size - pCB->length;
}/*}}}*/
bool circleBufIsEmpty(circleBuf* pCB)/*{{{*/
{
	return pCB->length==0;
}/*}}}*/
bool circleBufHasOverflow(circleBuf* pCB)/*{{{*/
{
	return pCB->overflow;	
}/*}}}*/
bool circleBufHasUnderflow(circleBuf* pCB)/*{{{*/
{
	return pCB->underflow;	
}/*}}}*/









