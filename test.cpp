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

/*TEST(Conv, u32to8) {
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
} todo*/

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

/*TEST(Conv, u8to16) {
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
}todo */

/*
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
 todo */
TEST(Errors, on1) {
    UTF utf;
    string str = "b\xc4\x85k";
    wstring wstr = utf.u8to16(str);
    EXPECT_EQ(wstr, L"b\u0105k");
    string str1 = "b\xc4k";
    wstring wstr1 = utf.u8to16(str1);
    EXPECT_EQ(wstr1, L"b\ufffdk");
}

TEST(CorrectUtf8, len1) {
    string str = "a\106b";
    dstring expect {'a', 0106, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\106b");
}

//110xxxxx 10xxxxxx
TEST(CorrectUtf8, len2) {
    string str = "a\325\252b";
    dstring expect {'a', 02552, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\x056a\x0062");
}

//1110xxxx 10xxxxxx 10xxxxxx
TEST(CorrectUtf8, len3) {
    string str = "a\352\252\252b";
    dstring expect {'a', 0xaaaa, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\uaaaab");
}

//http://russellcottrell.com/greek/utilities/SurrogatePairCalculator.htm
//11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

TEST(AmbigUtf8, slash2) {
    string str = "a\300\257b";
    //11000000 10101111
    dstring expect {'a', 0xfffd, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    EXPECT_EQ(utf.errambig,1);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\xfffd\x0062");
}

TEST(AmbigUtf8, slash3) {
    string str = "a\340\200\257b";
    //11100000 10000000 10101111
    dstring expect {'a', 0xfffd, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    EXPECT_EQ(utf.errambig,1);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\xfffd\x0062");
}

TEST(AmbigUtf8, len4) {
    string str = "a\360\200\200\203b";
    //11110000 10000000 10000000 10000011
    dstring expect {'a', 0xfffd, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    EXPECT_EQ(utf.errambig,1);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\xfffd\x0062");
}

TEST(ExceedsUtf16, len4) {
    string str = "a\367\277\277\277b";
    //11110111 10111111 10111111 10111111
    dstring expect {'a', 0x1FFFFF, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    EXPECT_EQ(utf.errors, 0);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\xfffd\x0062");
}

/*
TEST(ExceedsUtf16, len5) {
    string str = "a\372\200\200\200b";
    //11111010 10000000 10000000 10000000 10000000
    //10000000000000000000000000
    dstring expect {'a', 0x2000000, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    EXPECT_EQ(utf.errors, 0);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\xfffd\x0062");
}

TEST(ExceedsUtf16, len6) {
    string str = "a\372\200\200\200b";
    //1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    //1000000000000000000000000000000
    dstring expect {'a', 0x40000000, 'b'};
    UTF utf;
    dstring dstr = utf.u8to32(str);
    EXPECT_EQ(utf.errors, 0);
    wstring wstr = utf.u32to16(dstr);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, L"a\xfffd\x0062");
}
*/
