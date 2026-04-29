/*
 *    bit_tools.h
 *    Brent Edstrom, 2020
 */

#include <Arduino.h>

template <typename T> 
bool isBitSet(const T val, const int bit)
{
    if(val & (1 << bit))
        return true;
    return false;
}

template <typename T> 
void setBit(T &val, const int bit)
{
    val |= (1 << bit);
}

template <typename T> 
void clearBit(T &val, const int bit)
{
    val &= ~(1 << bit);
}

template <typename T> 
void toggleBit( T &val, const int bit)
{
    val ^= (1 << bit);
}

template <typename T> 
void invert(T &val)
{
    int bits = sizeof(T) * 8;
    for(int i = 0; i < bits; i++)
    {
        toggleBit(val, i);
    }
}

template <typename T> 
void crossover(T &val1, T &val2, const int crossover)
{
    int bits = sizeof(T) * 8;
    T temp1 = val1;
    T temp2 = val2;
    
    for(int i = crossover; i < bits; i++)
    {
        if(bitSet(val1, i))
        {
            setBit(temp2, i);
        }else{
            clearBit(temp2, i);
        }
        if(bitSet(val2, i))
        {
            setBit(temp1, i);
        }else{
            clearBit(temp1, i);
        }
    }
    val1 = temp1;
    val2 = temp2;
}

template <typename T> 
T getValue(const T number, const int start_bit, const int end_bit)
{
    T val = 0;
    int val_bit = 0;
    for(int i = start_bit; i <= end_bit; i++)
    {
        if(isBitSet(number, i))
        {
            setBit(val, val_bit);
        }
        val_bit++;
    }
    
    return val;
}

template <typename T> 
void printBinary(const T val)
{
   int bits = sizeof(T) * 8;
   for(int i = 0; i < bits; i++)
   {
      if(val & (1 << i))
      {
          Serial.print("1");
      }else{
          Serial.print("0");
      }
   }
}
