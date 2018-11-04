#pragma once

#include <cinttypes>

#include <stdio.h>

std::uint8_t modulo(std::uint16_t dividen, std::uint8_t d)
{
    std::uint16_t divisor = d;
    int count_sig = 0;
    while (dividen >> count_sig) {
        count_sig++;
    }

//    for (;count_sig > 0; count_sig --) {
//        if (dividen & (1 << count_sig))
//    }

}

constexpr int multiplication(std::uint8_t a, std::uint8_t b)
{
    std::uint16_t r = 0;
    std::uint16_t mul = a;
    do {
        if (b & 1)
            r ^= mul;
        b >>= 1;
        mul <<= 1;
    } while (b);
    printf("r %x\n", r);
    return r % 0x11b;
}

//void Round(State, RoundKey)
//{
//    ByteSub(State);
//    ShiftRow(State);
//    MixColumn(State);
//    AddRoundKey(State, RoundKey);
//}

//void FinalRound(State, RoundKey)
//{
//    ByteSub(State);
//    ShiftRow(State);
//    AddRoundKey(State, RoundKey);

//}
