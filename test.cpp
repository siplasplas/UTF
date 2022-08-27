//
// Created by andrzej on 8/27/22.
//
#include <gtest/gtest.h>
#include "UTF.hpp"

using namespace std;

dstring fillDstring() {
    const int MAX=0x10ffff;
    dstring dstr(MAX+1);
    for (int i=0; i<=MAX; i++)
        if (UTF::isSurrogate(i))
            dstr[i] = 0;
        else
            dstr[i] = i;
    return dstr;
}

TEST(Conv, u32to8) {
    dstring dstr = fillDstring();
    UTF utf;
    string str = utf.u32to8(dstr);
    dstring dstr1 = utf.u8to32(str);
    bool fail32to8 = false;
    EXPECT_EQ(dstr.size(), dstr1.size());
    for (int i=0; i<=dstr.size(); i++) {
        if (dstr[i] != dstr1[i]){
            fail32to8 = true;
            break;
        }
    }
    EXPECT_FALSE(fail32to8);
}

TEST(Conv, u32to16) {
    dstring dstr = fillDstring();
    UTF utf;
    wstring wstr = utf.u32to16(dstr);
    dstring dstr1 = utf.u16to32(wstr);
    bool fail32to16 = false;
    EXPECT_EQ(dstr.size(), dstr1.size());
    for (int i=0; i<=dstr.size(); i++) {
        if (dstr[i] != dstr1[i]){
            fail32to16 = true;
            break;
        }
    }
    EXPECT_FALSE(fail32to16);
}

TEST(Conv, u8to16) {
    dstring dstr = fillDstring();
    UTF utf;
    string str = utf.u32to8(dstr);
    wstring wstr = utf.u8to16(str);
    string str1 = utf.u16to8(wstr);
    bool fail8to16 = false;
    EXPECT_EQ(str.size(), str1.size());
    for (int i=0; i<=str.size(); i++) {
        if (str[i] != str1[i]){
            fail8to16 = true;
            break;
        }
    }
    EXPECT_FALSE(fail8to16);
}

TEST(Conv, u16to8) {
    dstring dstr = fillDstring();
    UTF utf;
    wstring wstr = utf.u32to16(dstr);
    string str = utf.u16to8(wstr);
    wstring wstr1 = utf.u8to16(str);
    bool fail16to8 = false;
    EXPECT_EQ(wstr.size(), wstr1.size());
    for (int i=0; i<=wstr.size(); i++) {
        if (wstr[i] != wstr1[i]) {
            fail16to8 = true;
            break;
        }
    }
    EXPECT_FALSE(fail16to8);
}
