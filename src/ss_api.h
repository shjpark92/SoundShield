#ifndef _SS_API_H_
#define _SS_API_H_

// orig 256 --> 8192
// new 1024 --> 32768
// try 2048 ---> 65536
#define UI_BUFFER_SIZE	32768
#define IO_BUFFER_SIZE	1024

extern float VOLUME_S[10];
extern float VOLUME_MASTER;

extern float VOLUME_S0;
extern float VOLUME_S1;
extern float VOLUME_S2;
extern float VOLUME_S3;
extern float VOLUME_S4;
extern float VOLUME_S5;
extern float VOLUME_S6;
extern float VOLUME_S7;
extern float VOLUME_S8;
extern float VOLUME_S9;

extern float IN_AMP_BUFFER[IO_BUFFER_SIZE];
extern float AMP_BUFFER[IO_BUFFER_SIZE];

extern short AMP_INDEX;
extern short AMP_READY;
extern short IN_AMP_INDEX;
extern short IN_AMP_READY;

#endif
