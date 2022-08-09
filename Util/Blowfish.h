
#ifndef BLOWFISH_H
#define BLOWFISH_H


typedef struct 
{
   unsigned int p[18];
   unsigned int s[4][256];

} BLOWFISH_KEY; 


extern void BlowfishSetKey( const BYTE* pUserKey, int len, BLOWFISH_KEY* pKeyStructOut );

extern void BlowfishEncrypt(uchar in[], uchar out[], BLOWFISH_KEY *keystruct);
extern void BlowfishDecrypt(uchar in[], uchar out[], BLOWFISH_KEY *keystruct);





#endif