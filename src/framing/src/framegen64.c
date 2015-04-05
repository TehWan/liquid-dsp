/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// framegen64.c
//
// frame64 generator: 8-byte header, 64-byte payload, 1340-sample frame
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <complex.h>

#include "liquid.internal.h"

struct framegen64_s {
    qpacketmodem    enc;                // packet encoder/modulator
    qpilotgen       pilotgen;           // pilot symbol generator
    float complex   pn_sequence[64];    // 64-symbol p/n sequence
    unsigned char   payload_dec[150];   // 600 = 150 bytes * 8 bits/bytes / 2 bits/symbol
    float complex   payload_sym[600];   // modulated payload symbols
    float complex   payload_tx[630];    // modulated payload symbols with pilots
    unsigned int    m;                  // filter delay (symbols)
    float           beta;               // filter excess bandwidth factor
    firinterp_crcf interp;              // pulse-shaping filter
};

// create framegen64 object
framegen64 framegen64_create()
{
    framegen64 q = (framegen64) malloc(sizeof(struct framegen64_s));
    q->m    = 7;
    q->beta = 0.3f;

    unsigned int i;

    // generate pn sequence
    msequence ms = msequence_create(7, 0x0089, 1);
    for (i=0; i<64; i++) {
        q->pn_sequence[i] = (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2) +
                            (msequence_advance(ms) ? M_SQRT1_2 : -M_SQRT1_2)*_Complex_I;
    }
    msequence_destroy(ms);

    // create payload encoder/modulator object
    int check      = LIQUID_CRC_24;
    int fec0       = LIQUID_FEC_NONE;
    int fec1       = LIQUID_FEC_GOLAY2412;
    int mod_scheme = LIQUID_MODEM_QPSK;
    q->enc         = qpacketmodem_create();
    qpacketmodem_configure(q->enc, 72, check, fec0, fec1, mod_scheme);
    //qpacketmodem_print(q->enc);
    assert( qpacketmodem_get_frame_len(q->enc)==600 );

    // create pilot generator
    q->pilotgen = qpilotgen_create(600, 21);
    assert( qpilotgen_get_frame_len(q->pilotgen)==630 );

    // create pulse-shaping filter (k=2)
    q->interp = firinterp_crcf_create_rnyquist(LIQUID_FIRFILT_ARKAISER,2,q->m,q->beta,0);

    // return main object
    return q;
}

// destroy framegen64 object
void framegen64_destroy(framegen64 _q)
{
    // destroy internal objects
    qpacketmodem_destroy(_q->enc);
    qpilotgen_destroy(_q->pilotgen);

    // free main object memory
    free(_q);
}

// print framegen64 object internals
void framegen64_print(framegen64 _q)
{
    float eta = (float) (8*(64 + 8)) / (float) (LIQUID_FRAME64_LEN/2);
    printf("framegen64 [m=%u, beta=%4.2f]:\n", _q->m, _q->beta);
    printf("  preamble/etc.\n");
    printf("    * ramp/up symbols       :   %u\n", _q->m);
    printf("    * p/n symbols           :   64\n");
    printf("    * ramp\\down symbols     :   %u\n", _q->m);
    printf("  payload\n");
#if 0
    printf("    * payload crc           :   %s\n", crc_scheme_str[_q->check][1]);
    printf("    * fec (inner)           :   %s\n", fec_scheme_str[_q->fec0][1]);
    printf("    * fec (outer)           :   %s\n", fec_scheme_str[_q->fec1][1]);
#endif
    printf("    * payload len, uncoded  :   %u bytes\n", 64);
    printf("    * payload len, coded    :   %u bytes\n", 150);
    printf("    * modulation scheme     :   %s\n", modulation_types[LIQUID_MODEM_QPSK].name);
    printf("    * payload symbols       :   600\n");
    printf("    * pilot symbols         :    30\n");
    printf("  summary\n");
    printf("    * total symbols         :   %u\n", LIQUID_FRAME64_LEN/2);
    printf("    * spectral efficiency   :   %6.4f b/s/Hz\n", eta);
}

// execute frame generator (creates a frame)
//  _q          :   frame generator object
//  _header     :   8-byte header data
//  _payload    :   64-byte payload data
//  _frame      :   output frame samples [size: LIQUID_FRAME64_LEN x 1]
void framegen64_execute(framegen64      _q,
                        unsigned char * _header,
                        unsigned char * _payload,
                        float complex * _frame)
{
    unsigned int i;

    // concatenate header and payload
    memmove(&_q->payload_dec[0], _header,   8*sizeof(unsigned char));
    memmove(&_q->payload_dec[8], _payload, 64*sizeof(unsigned char));

    // run packet encoder and modulator
    qpacketmodem_encode(_q->enc, _q->payload_dec, _q->payload_sym);

    // add pilot symbols
    qpilotgen_execute(_q->pilotgen, _q->payload_sym, _q->payload_tx);

    unsigned int n=0;

    // reset interpolator
    firinterp_crcf_reset(_q->interp);

    // p/n sequence
    for (i=0; i<64; i++) {
        firinterp_crcf_execute(_q->interp, _q->pn_sequence[i], &_frame[n]);
        n+=2;
    }

    // frame payload
    for (i=0; i<630; i++) {
        firinterp_crcf_execute(_q->interp, _q->payload_tx[i], &_frame[n]);
        n+=2;
    }

    // interpolator settling
    for (i=0; i<2*_q->m; i++) {
        firinterp_crcf_execute(_q->interp, 0.0f, &_frame[n]);
        n+=2;
    }

    assert(n==LIQUID_FRAME64_LEN);
}


