/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

// Test soft-decoding of a particular coding scheme
// (helper function to keep code base small)
void fec_test_soft_codec(fec_scheme _fs,
                         unsigned int _n,
                         void * _opts)
{
#if !LIBFEC_ENABLED
    if ( _fs == LIQUID_FEC_CONV_V27    ||
         _fs == LIQUID_FEC_CONV_V29    ||
         _fs == LIQUID_FEC_CONV_V39    ||
         _fs == LIQUID_FEC_CONV_V615   ||
         _fs == LIQUID_FEC_CONV_V27P23 ||
         _fs == LIQUID_FEC_CONV_V27P34 ||
         _fs == LIQUID_FEC_CONV_V27P45 ||
         _fs == LIQUID_FEC_CONV_V27P56 ||
         _fs == LIQUID_FEC_CONV_V27P67 ||
         _fs == LIQUID_FEC_CONV_V27P78 ||
         _fs == LIQUID_FEC_CONV_V29P23 ||
         _fs == LIQUID_FEC_CONV_V29P34 ||
         _fs == LIQUID_FEC_CONV_V29P45 ||
         _fs == LIQUID_FEC_CONV_V29P56 ||
         _fs == LIQUID_FEC_CONV_V29P67 ||
         _fs == LIQUID_FEC_CONV_V29P78 ||
         _fs == LIQUID_FEC_RS_M8)
    {
        AUTOTEST_WARN("convolutional, Reed-Solomon codes unavailable (install libfec)\n");
        return;
    }
#endif

    // generate fec object
    fec q = fec_create(_fs,_opts);

    // create arrays
    unsigned int n_enc = fec_get_enc_msg_length(_fs,_n);
    unsigned char msg[_n];              // original message
    unsigned char msg_enc[n_enc];       // encoded message
    unsigned char msg_soft[8*n_enc];    // encoded message (soft bits)
    unsigned char msg_dec[_n];          // decoded message

    // initialze message
    unsigned int i;
    for (i=0; i<_n; i++) {
        msg[i] = rand() & 0xff;
        msg_dec[i] = 0;
    }

    // encode message
    fec_encode(q, msg, _n, msg_enc);

    // convert to soft bits
    for (i=0; i<n_enc; i++) {
        msg_soft[8*i+0] = (msg_enc[i] & 0x80) ? 255 : 0;
        msg_soft[8*i+1] = (msg_enc[i] & 0x40) ? 255 : 0;
        msg_soft[8*i+2] = (msg_enc[i] & 0x20) ? 255 : 0;
        msg_soft[8*i+3] = (msg_enc[i] & 0x10) ? 255 : 0;
        msg_soft[8*i+4] = (msg_enc[i] & 0x08) ? 255 : 0;
        msg_soft[8*i+5] = (msg_enc[i] & 0x04) ? 255 : 0;
        msg_soft[8*i+6] = (msg_enc[i] & 0x02) ? 255 : 0;
        msg_soft[8*i+7] = (msg_enc[i] & 0x01) ? 255 : 0;
    }

    // channel: add single error
    msg_soft[0] = 255 - msg_soft[0];

    // decode message
    fec_decode_soft(q, msg_soft, _n, msg_dec);

    // validate output
    CONTEND_SAME_DATA(msg,msg_dec,_n);

    // clean up objects
    fec_destroy(q);
}

// 
// AUTOTESTS: basic encode/decode functionality
//

// repeat codes
void autotest_fecsoft_r3()     { fec_test_soft_codec(LIQUID_FEC_REP3,        64, NULL); }
void autotest_fecsoft_r5()     { fec_test_soft_codec(LIQUID_FEC_REP5,        64, NULL); }

// Hamming block codes
void autotest_fecsoft_h74()    { fec_test_soft_codec(LIQUID_FEC_HAMMING74,   64, NULL); }
void autotest_fecsoft_h84()    { fec_test_soft_codec(LIQUID_FEC_HAMMING84,   64, NULL); }
void autotest_fecsoft_h128()   { fec_test_soft_codec(LIQUID_FEC_HAMMING128,  64, NULL); }

// convolutional codes
void autotest_fecsoft_v27()    { fec_test_soft_codec(LIQUID_FEC_CONV_V27,    64, NULL); }
void autotest_fecsoft_v29()    { fec_test_soft_codec(LIQUID_FEC_CONV_V29,    64, NULL); }
void autotest_fecsoft_v39()    { fec_test_soft_codec(LIQUID_FEC_CONV_V39,    64, NULL); }
void autotest_fecsoft_v615()   { fec_test_soft_codec(LIQUID_FEC_CONV_V615,   64, NULL); }

// convolutional codes (punctured)
void autotest_fecsoft_v27p23() { fec_test_soft_codec(LIQUID_FEC_CONV_V27P23, 64, NULL); }
void autotest_fecsoft_v27p34() { fec_test_soft_codec(LIQUID_FEC_CONV_V27P34, 64, NULL); }
void autotest_fecsoft_v27p45() { fec_test_soft_codec(LIQUID_FEC_CONV_V27P45, 64, NULL); }
void autotest_fecsoft_v27p56() { fec_test_soft_codec(LIQUID_FEC_CONV_V27P56, 64, NULL); }
void autotest_fecsoft_v27p67() { fec_test_soft_codec(LIQUID_FEC_CONV_V27P67, 64, NULL); }
void autotest_fecsoft_v27p78() { fec_test_soft_codec(LIQUID_FEC_CONV_V27P78, 64, NULL); }

void autotest_fecsoft_v29p23() { fec_test_soft_codec(LIQUID_FEC_CONV_V29P23, 64, NULL); }
void autotest_fecsoft_v29p34() { fec_test_soft_codec(LIQUID_FEC_CONV_V29P34, 64, NULL); }
void autotest_fecsoft_v29p45() { fec_test_soft_codec(LIQUID_FEC_CONV_V29P45, 64, NULL); }
void autotest_fecsoft_v29p56() { fec_test_soft_codec(LIQUID_FEC_CONV_V29P56, 64, NULL); }
void autotest_fecsoft_v29p67() { fec_test_soft_codec(LIQUID_FEC_CONV_V29P67, 64, NULL); }
void autotest_fecsoft_v29p78() { fec_test_soft_codec(LIQUID_FEC_CONV_V29P78, 64, NULL); }

// Reed-Solomon block codes
void autotest_fecsoft_rs8()    { fec_test_soft_codec(LIQUID_FEC_RS_M8,       64, NULL); }


