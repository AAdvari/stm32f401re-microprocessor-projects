#ifndef __DTMF_H
#define __DTMF_H

#define DTMFsz  256               // DTMF Input Buffer

typedef struct DTMF  {
  unsigned int   AIindex;         // Input Data Index
  unsigned int   AIcheck;         // Index Window Trigger for DTMF check
  unsigned char  digit;           // detected digit
  unsigned char  early;           // early detected digit
  unsigned char  new;             // set to 1 when new digit detected
  unsigned char  d[4];			  // last four detected digits
  unsigned int   d_i;             // index
  unsigned short AInput[DTMFsz];  // A/D Input Data
} DTMF;

extern DTMF dail1;                // DTMF info of one input

extern void DTMF_Detect (DTMF *t);// check for valid DTMF tone

#endif // __DTMF_H


////  [
  //   697,  770,  852,  941,
  //  1209, 1336, 1477, 1633,
  //  1394, 1540, 1704, 1882,
  //  2418, 2672, 2954, 3266
  //  ]