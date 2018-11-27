#include "fifo.h"
#include <string.h>
#include <QDebug>

fifo::fifo(QObject *parent) : QObject(parent)
{
}

int fifo::getSize(){
    return (wrPos - rdPos)&(SIZE-1);
}

void fifo::write(void *src){
    memcpy(&data[wrPos] , src , PKG_SIZE);
    wrPos = (wrPos+1)&(SIZE-1);
    //size++;
}

 union block_t* fifo::read(){
     union block_t* ptr = &data[rdPos];
     rdPos = (rdPos+1)&(SIZE-1);
     return ptr;
     //size--;
 }

 void fifo::reset(){
     rdPos=0;
     wrPos=0;
 }

 void fifo::checkSize(){
     int size = (wrPos - rdPos)&(SIZE-1);
     if( size  > (3*SIZE/4))
         rdPos = (wrPos + (3*SIZE/4))&(SIZE-1);
 }
