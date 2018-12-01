#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H
#include <QtGlobal>
#define PKG_SIZE 512 /*In bytes*/
#define CodeVERSION 1
//sizeof(struct DSPmem_t)
//sizeof(struct DSPmem_t)


#include "global_defines.h"

struct lowspeed_t{
    float temp;
    qint32 reserved1; // Replace with real signal
    qint32 reserved2;
    qint32 reserved3;
    qint32 reserved4;
    qint32 reserved5;
    qint32 reserved6;
    qint32 reserved7;
    qint32 reserved8;
    qint32 reserved9;
    qint32 reserved10;
};


struct midspeed_vecotr_t{
    qint32 pos[PKG_SIZE/32];
    qint32 vel[PKG_SIZE/32];
    qint32 perror[PKG_SIZE/32];
    qint32 reserved1[PKG_SIZE/32];
    qint32 reserved2[PKG_SIZE/32];
    qint32 reserved3[PKG_SIZE/32];
    qint32 reserved4[PKG_SIZE/32];
};

struct blockC_t{
   quint64 checknumber; //1-2
   quint32 version; //3
   quint32 index;  //4
   float temp; //5
   qint32 states; //6;
   quint32 reserved[16-6]; // UPDATE if new line is inserted
};

//512 bytes
union block_t{
    blockC_t lowSpeed;
    qint32 samples[PKG_SIZE/4];
};

struct hispeed_vector_t{
    qint32 IA[PKG_SIZE/4];
    qint32 IC[PKG_SIZE/4];
    qint32 QE[PKG_SIZE/4];
    qint32 Torque[PKG_SIZE/4];
    qint32 Flux[PKG_SIZE/4];
    qint32 U[PKG_SIZE/4];
    qint32 angle[PKG_SIZE/4];
};

struct USBmem_t{
    quint64 checknumber; //2
    quint32 version; //3
    quint32 index;  //4
    float temp; //5
    quint32 reserved[16-5];
    struct midspeed_vecotr_t mid;
    struct hispeed_vector_t fast;
};




#endif // DATA_STRUCT_H
