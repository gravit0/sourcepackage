/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   basefunctions.cpp
 * Author: gravit
 *
 * Created on 3 октября 2017 г., 17:02
 */

#include "basefunctions.h"

std::string IntToByte(int integer) {
    std::stringstream result;
    char* zen = (char*) ((void*) &integer);
    if (integer == 0)result << (char) 0;
    if (integer > (256 * 256 * 256 - 1) || integer < 0)result << (char) *(zen + 3);
    if (integer > (256 * 256 - 1) || integer < 0)result << (char) *(zen + 2);
    if (integer > 255 || integer < 0)result << (char) *(zen + 1);
    result << *(zen);
    std::string abc = result.str();
    return abc;
}
//std::string IntToByte(int paramInt)
//{
//     std::string arrayOfByte;
//     char id=0;
//     for (int i = 0; i < 4; i++)
//     {
//         unsigned char z = (paramInt >> (i * 8));
//         if(z!=0)
//         {
//             arrayOfByte[id] = z;
//             id++;
//         }
//     }
//     return arrayOfByte;
//}

int byteToInt(std::string buffer) {
    if (buffer.size() > 4 || buffer.size() == 0)
        return 0;
    unsigned int resu = 0;
    if (buffer.size() == 1) {
        resu = resu + ((unsigned char) (buffer.at(0)));
    } else if (buffer.size() == 2) {
        resu = resu + ((unsigned char) (buffer.at(0)))*256;
        resu = resu + ((unsigned char) (buffer.at(1)));
    } else if (buffer.size() == 3) {
        resu = resu + ((unsigned char) (buffer.at(0)))*256 * 256;
        resu = resu + ((unsigned char) (buffer.at(1)))*256;
        resu = resu + ((unsigned char) (buffer.at(2)));
    } else if (buffer.size() == 4) {
        resu = resu + ((unsigned char) (buffer.at(0)))*256 * 256 * 256;
        resu = resu + ((unsigned char) (buffer.at(1)))*256 * 256;
        resu = resu + ((unsigned char) (buffer.at(2)))*256;
        resu = resu + ((unsigned char) (buffer.at(3)));
    }
    //    if(buffer.size()<=4) resu=resu+((unsigned char)(buffer.at(0)));
    //    if(buffer.size()<=3) resu=resu+((unsigned char)(buffer.at(1)))*256;
    //    if(buffer.size()<=2) resu=resu+((unsigned char)(buffer.at(2)))*256*256;
    //    if(buffer.size()==1) resu=resu+((unsigned char)(buffer.at(3)))*256*256*256;

    return *((int*) (&resu));
}

long long int byteToLong(std::string buffer) {
    unsigned int size = buffer.size();
    if (size == 1) {
        return (long long int) (buffer[0] & 0xff);

    }
    if (size == 0) {
        return 0;

    }
    if (size == 2) {
        return (long long int) (buffer[0] & 0xff) << 8 | (long long int) (buffer[1] & 0xff);
    }
    if (size == 3) {
        return (buffer[0] & 0xff) << 16 | (long long int) (buffer[1] & 0xff) << 8 | (long long int) (buffer[2] & 0xff);
    }
    if (size == 4) {
        return (long long int) (buffer[0] & 0xff) << 24 | (long long int) (buffer[1] & 0xff) << 16
                | (long long int) (buffer[2] & 0xff) << 8 | (long long int) (buffer[3] & 0xff);
    }
    if (size == 5) {
        return (long long int) (buffer[0] & 0xff) << 32
                | (long long int) (buffer[1] & 0xff) << 24 | (long long int) (buffer[2] & 0xff) << 16
                | (long long int) (buffer[3] & 0xff) << 8 | (long long int) (buffer[4] & 0xff);

    }
    if (size == 6) {
        return (long long int) (buffer[0] & 0xff) << 40 | (long long int) (buffer[1] & 0xff) << 32
                | (long long int) (buffer[2] & 0xff) << 24 | (long long int) (buffer[3] & 0xff) << 16
                | (long long int) (buffer[4] & 0xff) << 8 | (long long int) (buffer[5] & 0xff);
    }
    if (size == 7) {
        return (long long int) (buffer[0] & 0xff) << 48
                | (long long int) (buffer[1] & 0xff) << 40 | (long long int) (buffer[2] & 0xff) << 32
                | (long long int) (buffer[3] & 0xff) << 24 | (long long int) (buffer[4] & 0xff) << 16
                | (long long int) (buffer[5] & 0xff) << 8 | (long long int) (buffer[6] & 0xff);
    }
    if (size == 8) {
        return (long long int) (buffer[0] & 0xff) << 56 | (long long int) (buffer[1] & 0xff) << 48
                | (long long int) (buffer[2] & 0xff) << 40 | (long long int) (buffer[3] & 0xff) << 32
                | (long long int) (buffer[4] & 0xff) << 24 | (long long int) (buffer[5] & 0xff) << 16
                | (long long int) (buffer[6] & 0xff) << 8 | (long long int) (buffer[7] & 0xff);
    }
    return 0;

}

int findNoSlash(const std::string& str, const char ch, const unsigned int frist_pos, bool* isReplace) {
    bool NoAdept = false;
    unsigned int size = str.size();
    for (unsigned int i = frist_pos; i < size; ++i) {
        char thch = str[i];
        if (thch == '\\') {
            *isReplace = true;
            NoAdept = !NoAdept;
        } else {
            if (!NoAdept) {
                if (thch == ch) {
                    return (int) i;
                }
            } else {
                NoAdept = false;
            }
        }
    }
    return -1;
}

void SlashReplace(std::string* str, const unsigned int frist_pos) {
    bool NoAdept = false;
    unsigned int size = str->size();
    for (unsigned int i = frist_pos; i < size; ++i) {
        char thch = (*str)[i];
        if (thch == '\\') {
            NoAdept = !NoAdept;
            if (i + 1 < size) {
                char nextch = (*str)[i + 1];
                str->replace(i, 2, 1, nextch);
            }
        } else {
            if (NoAdept) {
                NoAdept = false;
            }
        }
    }
}
