// Copyright (C) 2016. Created 2016. All rights reserved.

/*
step 1: Append Padding Bits
	将字符串填充至 512 bits的整数倍 + 448 bits,
	填充位以 1 开始,后面都是 0;

step 2: Append Length
	将未填充前的字符串长度(64 bits,如果长度超过2的64次方,从低位开始取64 bits)附加到填充后的字符串后,
    长度是位长度。此时得到的字符串长度是512的倍数,同样也是16的倍数;

step 3: Initialize MD Buffer
	4个初始化的值(32 bits), 这里顺序是低位开始
	word A: 01 23 45 67
  	word B: 89 ab cd ef
  	word C: fe dc ba 98
  	word D: 76 54 32 10

Step 4: Process Message in 16-Word Blocks
	基础计算公式
	F(X,Y,Z) = XY v not(X) Z
    G(X,Y,Z) = XZ v Y not(Z)
    H(X,Y,Z) = X xor Y xor Z
    I(X,Y,Z) = Y xor (X v not(Z))

    循环进行以下计算
    // Process each 16-word block.
	For i = 0 to N/16-1 do

	    // Copy block i into X.
	    For j = 0 to 15 do
	       Set X[j] to M[i*16+j].
	    end // of loop on j

	    // Save A as AA, B as BB, C as CC, and D as DD. 
	    AA = A
	    BB = B
	    CC = C
     	DD = D

    // Round 1.
    // Let [abcd k s i] denote the operation
    //    a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s).
    // Do the following 16 operations.
    [ABCD  0  7  1]  [DABC  1 12  2]  [CDAB  2 17  3]  [BCDA  3 22  4]
    [ABCD  4  7  5]  [DABC  5 12  6]  [CDAB  6 17  7]  [BCDA  7 22  8]
    [ABCD  8  7  9]  [DABC  9 12 10]  [CDAB 10 17 11]  [BCDA 11 22 12]
    [ABCD 12  7 13]  [DABC 13 12 14]  [CDAB 14 17 15]  [BCDA 15 22 16]

    // Round 2.
    // Let [abcd k s i] denote the operation
    //    a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s).
    // Do the following 16 operations.
    [ABCD  1  5 17]  [DABC  6  9 18]  [CDAB 11 14 19]  [BCDA  0 20 20]
    [ABCD  5  5 21]  [DABC 10  9 22]  [CDAB 15 14 23]  [BCDA  4 20 24]
    [ABCD  9  5 25]  [DABC 14  9 26]  [CDAB  3 14 27]  [BCDA  8 20 28]
    [ABCD 13  5 29]  [DABC  2  9 30]  [CDAB  7 14 31]  [BCDA 12 20 32]

    // Round 3. 
    // Let [abcd k s t] denote the operation
    //    a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). 
    // Do the following 16 operations.
    [ABCD  5  4 33]  [DABC  8 11 34]  [CDAB 11 16 35]  [BCDA 14 23 36]
    [ABCD  1  4 37]  [DABC  4 11 38]  [CDAB  7 16 39]  [BCDA 10 23 40]
    [ABCD 13  4 41]  [DABC  0 11 42]  [CDAB  3 16 43]  [BCDA  6 23 44]
    [ABCD  9  4 45]  [DABC 12 11 46]  [CDAB 15 16 47]  [BCDA  2 23 48]

    // Round 4.
    // Let [abcd k s t] denote the operation
    //    a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s).
    // Do the following 16 operations. 
    [ABCD  0  6 49]  [DABC  7 10 50]  [CDAB 14 15 51]  [BCDA  5 21 52]
    [ABCD 12  6 53]  [DABC  3 10 54]  [CDAB 10 15 55]  [BCDA  1 21 56]
    [ABCD  8  6 57]  [DABC 15 10 58]  [CDAB  6 15 59]  [BCDA 13 21 60]
    [ABCD  4  6 61]  [DABC 11 10 62]  [CDAB  2 15 63]  [BCDA  9 21 64]

    // Then perform the following additions. (That is increment each
    //    of the four registers by the value it had before this block
    //    was started.) 
    A = A + AA
    B = B + BB
    C = C + CC
    D = D + DD

   	end // of loop on i

Step 5: Output
	从低位到高位输出 A,B,C,D
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MD5_UPDATE(A, B, C, D, M) { \
    round_1((A), (B), (C), (D), (M)     , 0xd76aa478,  7); \
    round_1((D), (A), (B), (C), (M) +  4, 0xe8c7b756, 12); \
    round_1((C), (D), (A), (B), (M) +  8, 0x242070db, 17); \
    round_1((B), (C), (D), (A), (M) + 12, 0xc1bdceee, 22); \
    round_1((A), (B), (C), (D), (M) + 16, 0xf57c0faf,  7); \
    round_1((D), (A), (B), (C), (M) + 20, 0x4787c62a, 12); \
    round_1((C), (D), (A), (B), (M) + 24, 0xa8304613, 17); \
    round_1((B), (C), (D), (A), (M) + 28, 0xfd469501, 22); \
    round_1((A), (B), (C), (D), (M) + 32, 0x698098d8,  7); \
    round_1((D), (A), (B), (C), (M) + 36, 0x8b44f7af, 12); \
    round_1((C), (D), (A), (B), (M) + 40, 0xffff5bb1, 17); \
    round_1((B), (C), (D), (A), (M) + 44, 0x895cd7be, 22); \
    round_1((A), (B), (C), (D), (M) + 48, 0x6b901122,  7); \
    round_1((D), (A), (B), (C), (M) + 52, 0xfd987193, 12); \
    round_1((C), (D), (A), (B), (M) + 56, 0xa679438e, 17); \
    round_1((B), (C), (D), (A), (M) + 60, 0x49b40821, 22); \
    round_2((A), (B), (C), (D), (M) +  4, 0xf61e2562,  5); \
    round_2((D), (A), (B), (C), (M) + 24, 0xc040b340,  9); \
    round_2((C), (D), (A), (B), (M) + 44, 0x265e5a51, 14); \
    round_2((B), (C), (D), (A), (M)     , 0xe9b6c7aa, 20); \
    round_2((A), (B), (C), (D), (M) + 20, 0xd62f105d,  5); \
    round_2((D), (A), (B), (C), (M) + 40, 0x02441453,  9); \
    round_2((C), (D), (A), (B), (M) + 60, 0xd8a1e681, 14); \
    round_2((B), (C), (D), (A), (M) + 16, 0xe7d3fbc8, 20); \
    round_2((A), (B), (C), (D), (M) + 36, 0x21e1cde6,  5); \
    round_2((D), (A), (B), (C), (M) + 56, 0xc33707d6,  9); \
    round_2((C), (D), (A), (B), (M) + 12, 0xf4d50d87, 14); \
    round_2((B), (C), (D), (A), (M) + 32, 0x455a14ed, 20); \
    round_2((A), (B), (C), (D), (M) + 52, 0xa9e3e905,  5); \
    round_2((D), (A), (B), (C), (M) +  8, 0xfcefa3f8,  9); \
    round_2((C), (D), (A), (B), (M) + 28, 0x676f02d9, 14); \
    round_2((B), (C), (D), (A), (M) + 48, 0x8d2a4c8a, 20); \
    round_3((A), (B), (C), (D), (M) + 20, 0xfffa3942,  4); \
    round_3((D), (A), (B), (C), (M) + 32, 0x8771f681, 11); \
    round_3((C), (D), (A), (B), (M) + 44, 0x6d9d6122, 16); \
    round_3((B), (C), (D), (A), (M) + 56, 0xfde5380c, 23); \
    round_3((A), (B), (C), (D), (M) +  4, 0xa4beea44,  4); \
    round_3((D), (A), (B), (C), (M) + 16, 0x4bdecfa9, 11); \
    round_3((C), (D), (A), (B), (M) + 28, 0xf6bb4b60, 16); \
    round_3((B), (C), (D), (A), (M) + 40, 0xbebfbc70, 23); \
    round_3((A), (B), (C), (D), (M) + 52, 0x289b7ec6,  4); \
    round_3((D), (A), (B), (C), (M)     , 0xeaa127fa, 11); \
    round_3((C), (D), (A), (B), (M) + 12, 0xd4ef3085, 16); \
    round_3((B), (C), (D), (A), (M) + 24, 0x04881d05, 23); \
    round_3((A), (B), (C), (D), (M) + 36, 0xd9d4d039,  4); \
    round_3((D), (A), (B), (C), (M) + 48, 0xe6db99e5, 11); \
    round_3((C), (D), (A), (B), (M) + 60, 0x1fa27cf8, 16); \
    round_3((B), (C), (D), (A), (M) +  8, 0xc4ac5665, 23); \
    round_4((A), (B), (C), (D), (M)     , 0xf4292244,  6); \
    round_4((D), (A), (B), (C), (M) + 28, 0x432aff97, 10); \
    round_4((C), (D), (A), (B), (M) + 56, 0xab9423a7, 15); \
    round_4((B), (C), (D), (A), (M) + 20, 0xfc93a039, 21); \
    round_4((A), (B), (C), (D), (M) + 48, 0x655b59c3,  6); \
    round_4((D), (A), (B), (C), (M) + 12, 0x8f0ccc92, 10); \
    round_4((C), (D), (A), (B), (M) + 40, 0xffeff47d, 15); \
    round_4((B), (C), (D), (A), (M) +  4, 0x85845dd1, 21); \
    round_4((A), (B), (C), (D), (M) + 32, 0x6fa87e4f,  6); \
    round_4((D), (A), (B), (C), (M) + 60, 0xfe2ce6e0, 10); \
    round_4((C), (D), (A), (B), (M) + 24, 0xa3014314, 15); \
    round_4((B), (C), (D), (A), (M) + 52, 0x4e0811a1, 21); \
    round_4((A), (B), (C), (D), (M) + 16, 0xf7537e82,  6); \
    round_4((D), (A), (B), (C), (M) + 44, 0xbd3af235, 10); \
    round_4((C), (D), (A), (B), (M) +  8, 0x2ad7d2bb, 15); \
    round_4((B), (C), (D), (A), (M) + 36, 0xeb86d391, 21); \
}

void add_8(unsigned char A, unsigned char B, unsigned char C[2])
{
    C[1] = A + B;
    if(C[1] < A) C[0] = 1;
    else C[0] = 0;
}

void add_16(unsigned char A[2], unsigned char B[2], unsigned char C[3])
{
    unsigned char Temp1[2];
    unsigned char Temp2[2];

    add_8(A[0], B[0], Temp1);
    add_8(A[1], B[1], Temp2);

    add_8(Temp1[1], Temp2[0], C);
    C[2] = Temp2[1];
    C[0] += Temp1[0];
}

void add_32(unsigned char A[4], unsigned char B[4], unsigned char C[4])
{
    unsigned char Temp[2];
    unsigned char Temp1[3];
    unsigned char Temp2[3];

    add_16(A, B, Temp1);
    add_16(A + 2, B + 2, Temp2);

    C[3] = Temp2[2];
    C[2] = Temp2[1];
    Temp[0] = 0;
    Temp[1] = Temp2[0];
    add_16(Temp, Temp1 + 1, Temp2);

    C[1] = Temp2[2];
    C[0] = Temp2[1];
}

void add_32_integer(unsigned char A[4], unsigned int B, unsigned char C[4])
{
    unsigned char Obj[4];
    Obj[0] = B / 16777216;
    Obj[1] = (B % 16777216) / 65536; 
    Obj[2] = (B % 65536) / 256; 
    Obj[3] = B % 256; 

    add_32(A, Obj, C);
}

void add_32_overflow(unsigned char A[4], unsigned char B[4], unsigned char C[5])
{
    unsigned char Temp[2];
    unsigned char Temp1[3];
    unsigned char Temp2[3];

    add_16(A, B, Temp1);
    add_16(A + 2, B + 2, Temp2);

    Temp[0] = 0;
    Temp[1] = Temp2[0];
    add_16(Temp, Temp1 + 1, C);

    C[4] = Temp2[2];
    C[3] = Temp2[1];
    C[0] += Temp1[0];
}

void add_32_integer_overflow(unsigned char A[4], unsigned int B, unsigned char C[5])
{
    unsigned char Obj[4];
    Obj[0] = B / 16777216;
    Obj[1] = (B % 16777216) / 65536; 
    Obj[2] = (B % 65536) / 256; 
    Obj[3] = B % 256; 

    add_32_overflow(A, Obj, C);
}

void loop_left_8(unsigned char *C, unsigned int N)
{
    unsigned char Temp;
    unsigned int move = N%8;
    if(move > 0){
        Temp = *C >> (8 - move);
        *C = *C << move;
        *C = *C ^ Temp; 
    }
}

void loop_left_32(unsigned char C[4], unsigned int N)
{
    unsigned char Temp1;
    unsigned char Temp2;
    unsigned int move = N%32;
    if(move >= 8 && move < 16){
        Temp1 = C[0];
        C[0] = C[1];
        C[1] = C[2];
        C[2] = C[3];
        C[3] = Temp1;
    }else if(move >= 16 && move < 32){
        Temp1 = C[0];
        C[0] = C[2];
        C[2] = Temp1;
        Temp1 = C[1];
        C[1] = C[3];
        C[3] = Temp1;
    }
    move = move%8;
    if(move){
        Temp1 = C[3];
        C[3] = (C[3] << move) ^ (C[0] >> (8 - move));
        Temp2 = C[2];
        C[2] = (C[2] << move) ^ (Temp1 >> (8 - move));
        Temp1 = C[1];
        C[1] = (C[1] << move) ^ (Temp2 >> (8 - move));
        C[0] = (C[0] << move) ^ (Temp1 >> (8 - move));
    }
}

void increment_8(unsigned char V[8])
{
    int i;
    int j;
    for(j = 0; j < 8; ++j){
        for(i = 7; i > -1; --i){
            if(V[i] < 255){
                ++V[i];
                break;
            }else{
                V[i] = 0;
            }
        }
    }
}

void increment(unsigned char V[8], unsigned int Value)
{
    int i;
    unsigned char Temp[5];
    add_32_integer_overflow(V + 4, Value * 8, Temp);
    add_32_integer(V, Temp[0], V);
    for(i = 1; i < 5; ++i)V[3 + i] = Temp[i];
}

void bit_and_32(unsigned char A[4], unsigned char B[4], unsigned char R[4])
{
    unsigned int i;
    for(i = 0; i < 4; ++i)R[i] = (A[i]) & (B[i]);
}

void bit_or_32(unsigned char A[4], unsigned char B[4], unsigned char R[4])
{
    unsigned int i;
    for(i = 0; i < 4; ++i)R[i] = (A[i]) | (B[i]);
}

void bit_xor_32(unsigned char A[4], unsigned char B[4], unsigned char R[4])
{
    unsigned int i;
    for(i = 0; i < 4; ++i)R[i] = (A[i]) ^ (B[i]);
}

void bit_not_32(unsigned char A[4], unsigned char R[4])
{
    unsigned int i;
    for(i = 0; i < 4; ++i)R[i] = ~(A[i]);
}

void round_1(unsigned char A[4], unsigned char B[4], unsigned char C[4], unsigned char D[4], 
    unsigned char M[4], unsigned int T, unsigned int S)
{
    unsigned char Temp1[4];
    unsigned char Temp2[4];

    bit_and_32(B, C, Temp1);
    bit_not_32(B, Temp2);
    bit_and_32(Temp2, D, Temp2);
    bit_or_32(Temp1, Temp2, Temp1);

    add_32(A, Temp1, A);
    add_32(A, M, A);
    add_32_integer(A, T, A);
    loop_left_32(A, S);
    add_32(A, B, A);

}

void round_2(unsigned char A[4], unsigned char B[4], unsigned char C[4], unsigned char D[4], 
    unsigned char M[4], unsigned int T, unsigned int S)
{
    unsigned char Temp1[4];
    unsigned char Temp2[4];

    bit_and_32(B, D, Temp1);
    bit_not_32(D, Temp2);
    bit_and_32(Temp2, C, Temp2);
    bit_or_32(Temp1, Temp2, Temp1);

    add_32(A, Temp1, A);
    add_32(A, M, A);
    add_32_integer(A, T, A);
    loop_left_32(A, S);
    add_32(A, B, A);
}

void round_3(unsigned char A[4], unsigned char B[4], unsigned char C[4], unsigned char D[4], 
    unsigned char M[4], unsigned int T, unsigned int S)
{
    unsigned char Temp[4];

    bit_xor_32(B, C, Temp);
    bit_xor_32(Temp, D, Temp);

    add_32(A, Temp, A);
    add_32(A, M, A);
    add_32_integer(A, T, A);
    loop_left_32(A, S);
    add_32(A, B, A);
}

void round_4(unsigned char A[4], unsigned char B[4], unsigned char C[4], unsigned char D[4], 
    unsigned char M[4], unsigned int T, unsigned int S)
{
    unsigned char Temp[4];

    bit_not_32(D, Temp);
    bit_or_32(Temp, B, Temp);
    bit_xor_32(Temp, C, Temp);

    add_32(A, Temp, A);
    add_32(A, M, A);
    add_32_integer(A, T, A);
    loop_left_32(A, S);
    add_32(A, B, A);
}

void md5(const char *Str, unsigned char Res[16])
{
    unsigned char A[4] = {0x67, 0x45, 0x23, 0x01};
    unsigned char B[4] = {0xef, 0xcd, 0xab, 0x89};
    unsigned char C[4] = {0x98, 0xba, 0xdc, 0xfe};
    unsigned char D[4] = {0x10, 0x32, 0x54, 0x76};
    unsigned int i = 0;
    unsigned int IsDone = 1;
    unsigned int Loop = 0;
    unsigned char Len[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int pos = 0;
    unsigned char MBuff[128];
    unsigned char Temp_A[4] = {0x67, 0x45, 0x23, 0x01};
    unsigned char Temp_B[4] = {0xef, 0xcd, 0xab, 0x89};
    unsigned char Temp_C[4] = {0x98, 0xba, 0xdc, 0xfe};
    unsigned char Temp_D[4] = {0x10, 0x32, 0x54, 0x76};
    unsigned char *MPoint;
    unsigned char Temp_Value;
    memset(MBuff, 0, 128); //初始化为0
    while(IsDone){
        pos = 0;
        while(*Str){
            MBuff[pos] = *Str;
            increment_8(Len);
            ++Str;
            ++pos;
            if(pos > 63)break;
        }
        if(pos != 64){
            IsDone = 0;
            if(pos > 56)Loop = 1;
            else Loop = 0;
            MBuff[pos] = 0x80;
            for(++pos; pos < 56 + (Loop * 64); ++pos)MBuff[pos] = 0;
            for(i = 0; i < 8; ++i){
                MBuff[pos] = Len[7 - i];
                ++pos;
            }
        }
        // init blocks
        for(i = 0; i < pos; i += 4){
            Temp_Value = MBuff[i];
            MBuff[i] = MBuff[i + 3];
            MBuff[i + 3] = Temp_Value;
            Temp_Value = MBuff[i + 2];
            MBuff[i + 2] = MBuff[i + 1];
            MBuff[i + 1] = Temp_Value;
        }

        for(i = 0; i < (Loop + 1); ++i){
            MPoint = MBuff + 64 * i;
            MD5_UPDATE(Temp_A, Temp_B, Temp_C, Temp_D, MPoint)
            add_32(A, Temp_A, Temp_A);
            add_32(B, Temp_B, Temp_B);
            add_32(C, Temp_C, Temp_C);
            add_32(D, Temp_D, Temp_D);
            for(pos = 0; pos < 4; ++pos){
                A[pos] = Temp_A[pos];
                B[pos] = Temp_B[pos];
                C[pos] = Temp_C[pos];
                D[pos] = Temp_D[pos];
            }
        }
    }
    for(i = 0; i < 4; i++) Res[3 - i] = A[i];
    for(i = 0; i < 4; i++) Res[7 - i] = B[i];
    for(i = 0; i < 4; i++) Res[11 - i] = C[i];
    for(i = 0; i < 4; i++) Res[15 - i] = D[i];
}

void md5file(const char *path, unsigned char Res[16]) 
{
    //打开文件读取数据,以读模式（"r"）打开文件
    FILE *fp;
    unsigned char A[4] = {0x67, 0x45, 0x23, 0x01};
    unsigned char B[4] = {0xef, 0xcd, 0xab, 0x89};
    unsigned char C[4] = {0x98, 0xba, 0xdc, 0xfe};
    unsigned char D[4] = {0x10, 0x32, 0x54, 0x76};
    unsigned int i = 0;
    unsigned int IsDone = 1;
    unsigned int Loop = 0;
    unsigned char Len[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int pos = 0;
    unsigned char MBuff[128];
    unsigned char Temp_A[4] = {0x67, 0x45, 0x23, 0x01};
    unsigned char Temp_B[4] = {0xef, 0xcd, 0xab, 0x89};
    unsigned char Temp_C[4] = {0x98, 0xba, 0xdc, 0xfe};
    unsigned char Temp_D[4] = {0x10, 0x32, 0x54, 0x76};
    unsigned char *MPoint;
    unsigned char Temp_Value;
    memset(MBuff, 0, 128); //初始化为0

    if(!(fp = fopen(path, "r")))
    {
        perror("fail to read");
        exit(1);
    }
    while(pos = fread(MBuff, 1, 64, fp)){
        increment(Len, pos);
        if(pos != 64){
            IsDone = 0;
            if(pos > 56)Loop = 1;
            else Loop = 0;
            MBuff[pos] = 0x80;
            for(++pos; pos < 56 + (Loop * 64); ++pos)MBuff[pos] = 0;
            for(i = 0; i < 8; ++i){
                MBuff[pos] = Len[7 - i];
                ++pos;
            }
        }
        // init blocks
        for(i = 0; i < pos; i += 4){
            Temp_Value = MBuff[i];
            MBuff[i] = MBuff[i + 3];
            MBuff[i + 3] = Temp_Value;
            Temp_Value = MBuff[i + 2];
            MBuff[i + 2] = MBuff[i + 1];
            MBuff[i + 1] = Temp_Value;
        }

        for(i = 0; i < (Loop + 1); ++i){
            MPoint = MBuff + 64 * i;
            MD5_UPDATE(Temp_A, Temp_B, Temp_C, Temp_D, MPoint)
            add_32(A, Temp_A, Temp_A);
            add_32(B, Temp_B, Temp_B);
            add_32(C, Temp_C, Temp_C);
            add_32(D, Temp_D, Temp_D);
            for(pos = 0; pos < 4; ++pos){
                A[pos] = Temp_A[pos];
                B[pos] = Temp_B[pos];
                C[pos] = Temp_C[pos];
                D[pos] = Temp_D[pos];
            }
        }
    }
    for(i = 0; i < 4; i++) Res[3 - i] = A[i];
    for(i = 0; i < 4; i++) Res[7 - i] = B[i];
    for(i = 0; i < 4; i++) Res[11 - i] = C[i];
    for(i = 0; i < 4; i++) Res[15 - i] = D[i];
}

int main(int argc,char *argv[])
{
	unsigned char Res[16];
	int i;

    if(*(argv[1]) == 49){
        md5(argv[2], Res);
        for(i = 0; i < 16; i++) printf("%02x", Res[i]);
    }
    if(*(argv[1]) == 50){
        md5file(argv[2], Res);
        for(i = 0; i < 16; i++) printf("%02x", Res[i]);
    }
    printf("\n");

    return 0;
}

