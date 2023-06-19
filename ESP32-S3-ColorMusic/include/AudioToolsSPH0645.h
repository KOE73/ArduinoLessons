#pragma once

#include "AudioTools.h" // Для микрофона

template <typename T>
class MicrophoneSPH0645Reducer : public BaseConverter<T>
{
public:
    MicrophoneSPH0645Reducer(int channels = 2)
    {
        this->channels = channels;
    }

    size_t convert(uint8_t *src, size_t byte_count)
    {
        T *sample = (T *)src;
        int size = byte_count / sizeof(T);
        for (size_t j = 0; j < size; j++)
        {
            T s = (*sample >> 14) & ((T)0x3FFFF); // 0x3FFFF;
            if (s & (1 << 17))
                s |= 0xFFFC0000;

            // if ((s & 0b111111000000000000) == 0b111110000000000000)
            //     s &= 0b000001111111111111;
            // else if ((s & 0b111110000000000000) == 0b111100000000000000)
            //     s &= 0b000011111111111111;
            // else if ((s & 0b111100000000000000) == 0b111000000000000000)
            //     s &= 0b000111111111111111;
            // else if ((s & 0b111000000000000000) == 0b110000000000000000)
            //     s &= 0b001111111111111111;
            // else if ((s & 0b110000000000000000) == 0b100000000000000000)
            //     s &= 0b011111111111111111;

            *sample = s;
            //(~s) & ((T)0x3FFFF);

             //Serial.print("\t\t");
             //Serial.println(*sample, BIN);

            sample++;
        }
        return byte_count;
    }

protected:
    int channels;
};

// 11100011000001/0000