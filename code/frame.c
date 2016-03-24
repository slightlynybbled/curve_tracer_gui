#include "frame.h"
#include "uart.h"
#include <stddef.h>

#define TX_FRAME_LENGTH 256
#define RX_FRAME_LENGTH 64

#define SOF 0xf7
#define EOF 0x7f
#define ESC 0xf6
#define ESC_XOR 0x20

static uint8_t rxFrame[RX_FRAME_LENGTH];
static uint16_t rxFrameIndex = 0;

uint16_t FRM_fletcher16(uint8_t* data, size_t bytes);

void FRM_init(void){
    UART_init();
}

void FRM_push(uint8_t* data, uint16_t length){
    uint8_t txFrame[TX_FRAME_LENGTH];
    int16_t i = 0, txIndex = 0;
    
    uint16_t check = FRM_fletcher16(data, length);
    uint8_t check0 = (uint8_t)(check & 0x00ff);
    uint8_t check1 = (uint8_t)((check ^ 0xff00) >> 8);
    
    /* start the frame */
    txFrame[txIndex] = SOF;
    txIndex++;
    
    /* TODO: add the CRC16 value */
    while(i < length){
        /* add proper escape sequences */
        if((data[i] == SOF) || (data[i] == EOF) || (data[i] == ESC)){
            txFrame[txIndex] = ESC;
            txIndex++;
            txFrame[txIndex] = data[i] ^ ESC_XOR;
            txIndex++;
        }else{
            txFrame[txIndex] = data[i];
            txIndex++;
        }
        
        i++;
    }
    
    /* append the checksum */
    if((check0 == SOF) || (check0 == EOF) || (check0 == ESC)){
        txFrame[txIndex] = ESC;
        txIndex++;
        txFrame[txIndex] = check0 ^ ESC_XOR;
        txIndex++;
    }else{
        txFrame[txIndex] = check0;
        txIndex++;
    }
    
    if((check1 == SOF) || (check1 == EOF) || (check1 == ESC)){
        txFrame[txIndex] = ESC;
        txIndex++;
        txFrame[txIndex] = check1 ^ ESC_XOR;
        txIndex++;
    }else{
        txFrame[txIndex] = check1;
        txIndex++;
    }
    
    /* end the frame */
    txFrame[txIndex] = EOF;
    txIndex++;
    
    /* wait for the UART circular buffer to clear from
     * any currently in-progress transmissions */
    while(UART_writeable() < txIndex);
    UART_write(txFrame, txIndex);
}

uint16_t FRM_pull(uint8_t* data){
    uint16_t sofIndex = 0, eofIndex = 0;
    uint16_t length = 0;
    
    uint16_t i = 0;
    
    /* read the available bytes into the rxFrame */
    uint16_t numOfBytes = UART_readable();
    UART_read(&rxFrame[rxFrameIndex], numOfBytes);
    rxFrameIndex += numOfBytes;
    
    /* find the SOF */
    while(i < RX_FRAME_LENGTH){
        if(rxFrame[i] == SOF){
            sofIndex = i;
            break;
        }else{
            rxFrame[i] = 0; // clear the byte that isn't the SOF
        }
        i++;
    }
    
    /* find the EOF */
    i = sofIndex;
    while(i < RX_FRAME_LENGTH){
        if(rxFrame[i] == EOF){
            eofIndex = i;
            break;
        }
        i++;
    }
    
    /* ensure that the start of frame and the end of frame are both present */
    if(((rxFrame[0] == SOF) || (sofIndex > 0)) && (eofIndex != 0)){
        /* extract the received frame and shift the remainder of the
         * bytes into the beginning of the frame */
        i = 0;
        int16_t frameIndex = sofIndex;
        rxFrame[frameIndex] = 0;
        frameIndex++;
        while(frameIndex < eofIndex){
            if(rxFrame[frameIndex] == ESC){
                frameIndex++;
                data[i] = rxFrame[frameIndex] ^ ESC_XOR;
            }else{
                data[i] = rxFrame[frameIndex];
            }
            i++;
            frameIndex++;
        }
        
        length = i;
        
        /* a full frame was just processed, find the next SOF and
         * copy the remainder of the frame forward in preparation for
         * the next frame reception */
        i = eofIndex;
        sofIndex = 0;
        while(i < RX_FRAME_LENGTH){
            if(rxFrame[i] == SOF){
                sofIndex = i;
                break;
            }else{
                rxFrame[i] = 0;
            }
            i++;
        }
        rxFrameIndex = 0;
        
        /* copy and clear */
        if(sofIndex > 0){
            i = 0;
            while((i + sofIndex) < RX_FRAME_LENGTH){
                rxFrame[i] = rxFrame[(i + sofIndex)];
                rxFrame[(i + sofIndex)] = 0;
                i++;
            }
        }
        
        /* check the data integrity using the last two bytes as
         * the fletcher16 checksum */
        uint16_t checksum = data[length - 2] | (data[length - 1] << 8);
        length -= 2;
        uint16_t check = FRM_fletcher16(data, length);
        if(check != checksum)
            length = 0;
    }
    
    return length;
}

/* implementation from wikipedia: 
 * https://en.wikipedia.org/wiki/Fletcher%27s_checksum#Fletcher-16 */
uint16_t FRM_fletcher16(uint8_t* data, size_t bytes){
    uint16_t sum1 = 0xff, sum2 = 0xff;
    size_t tlen;

    while (bytes) {
        tlen = bytes >= 20 ? 20 : bytes;
        bytes -= tlen;
        do {
            sum2 += sum1 += *data++;
        } while (--tlen);
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
    }
    /* Second reduction step to reduce sums to 8 bits */
    sum1 = (sum1 & 0xff) + (sum1 >> 8);
    sum2 = (sum2 & 0xff) + (sum2 >> 8);
    return sum2 << 8 | sum1;
}
