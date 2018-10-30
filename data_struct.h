#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H
#include <QtGlobal>
#define PKG_SIZE 512 /*In bytes*/
//sizeof(struct DSPmem_t)
//sizeof(struct DSPmem_t)

struct lowspeed_t{
    qint32 temp;
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
    qint32 reserved11;
    qint32 reserved12;
    qint32 reserved13;
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
    quint32 checknumber;
    quint32 index;
    struct lowspeed_t slow;
    struct midspeed_vecotr_t mid;
    struct hispeed_vector_t fast;
};



#endif // DATA_STRUCT_H
