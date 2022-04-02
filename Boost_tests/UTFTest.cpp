#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "../UTF.hpp"
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

BOOST_AUTO_TEST_SUITE(UTFCheckSuite)

    BOOST_AUTO_TEST_CASE(u32to8)
    {
        dstring dstr = fillDstring();
        UTF utf;
        string str = utf.u32to8(dstr);
        dstring dstr1 = utf.u8to32(str);
        bool fail32to8 = false;
        for (int i=0; i<=dstr.size(); i++) {
            if (dstr[i] != dstr1[i]){
                fail32to8 = true;
                break;
            }
        }
        BOOST_REQUIRE_EQUAL(fail32to8 , false);
    }

    BOOST_AUTO_TEST_CASE(u32to16)
    {
        dstring dstr = fillDstring();
        UTF utf;
        wstring wstr = utf.u32to16(dstr);
        dstring dstr1 = utf.u16to32(wstr);
        bool fail32to16 = false;
        for (int i=0; i<=dstr.size(); i++) {
            if (dstr[i] != dstr1[i]){
                fail32to16 = true;
                break;
            }
        }
        BOOST_REQUIRE_EQUAL(fail32to16 , false);
    }

    BOOST_AUTO_TEST_CASE(u8to16)
    {
        dstring dstr = fillDstring();
        UTF utf;
        string str = utf.u32to8(dstr);
        wstring wstr = utf.u8to16(str);
        string str1 = utf.u16to8(wstr);
        bool fail8to16 = false;
        for (int i=0; i<=str.size(); i++) {
            if (str[i] != str1[i]){
                fail8to16 = true;
                break;
            }
        }
        BOOST_REQUIRE_EQUAL(fail8to16 , false);
    }

    BOOST_AUTO_TEST_CASE(u16to8)
    {
        dstring dstr = fillDstring();
        UTF utf;
        wstring wstr = utf.u32to16(dstr);
        string str = utf.u16to8(wstr);
        wstring wstr1 = utf.u8to16(str);
        bool fail16to8 = false;
        for (int i=0; i<=wstr.size(); i++) {
            if (wstr[i] != wstr1[i]) {
                fail16to8 = true;
                break;
            }
        }
        BOOST_REQUIRE_EQUAL(fail16to8 , false);
    }

BOOST_AUTO_TEST_SUITE_END()