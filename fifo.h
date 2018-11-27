#ifndef FIFO_H
#define FIFO_H

#include <QObject>
#include "data_struct.h"

#define SIZE 4096

class fifo : public QObject
{
    Q_OBJECT
public:
    explicit fifo(QObject *parent = nullptr);

public:
    void write(void *src);
    union block_t* read();
    void reset();
    int getSize();
    void checkSize();

private:
    int rdPos=0;
    int wrPos=0;
    union block_t data[SIZE]={0};

};



#endif // FIFO_H
