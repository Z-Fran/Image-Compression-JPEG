#include<iostream>
#include"PicReader.h"
#include<stdio.h>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cstring>
using namespace std;
#define PI 3.1415926

//Z形扫描表
int ZigZag[8][8] = { {0, 1, 5, 6,14,15,27,28} ,
					 {2, 4, 7,13,16,26,29,42},
					 {3, 8,12,17,25,30,41,43},
					 {9,11,18,24,31,40,44,53},
					 {10,19,23,32,39,45,52,54},
					 {20,22,33,38,46,51,55,60},
					 {21,34,37,47,50,56,59,61},
					 {35,36,48,49,57,58,62,63}, };
//Y量化表
int Y_quantization_table[8][8] = { {16,11,10,16,24,40,51,61} ,
								   {12,12,14,19,26,58,60,55},
								   {14,13,16,24,40,57,69,56},
								   {14,17,22,29,51,87,80,62},
								   {18,22,37,56,68,109,103,77},
								   {24,35,55,64,81,104,113,92},
								   {49,64,78,87,103,121,120,101},
								   {72,92,95,98,112,100,103,99} };
//CbCr量化表
int C_quantization_table[8][8] = { {17,18,24,47,99,99,99,99},
								   {18,21,26,66,99,99,99,99},
								   {24,26,56,99,99,99,99,99},
								   {47,66,99,99,99,99,99,99},
								   {99,99,99,99,99,99,99,99},
								   {99,99,99,99,99,99,99,99},
								   {99,99,99,99,99,99,99,99},
								   {99,99,99,99,99,99,99,99}, };
//编码=码长+码字
struct Code {
	int length;
	int word;
};
//Code Y_DC[16] = {
//	{2,0b00},{3,0b001},{3,0b011},{3,0b100},{3,0b101},{3,0b110},{4,0b1110},{5,0b11110},{6,0b111110},{7,0b1111110},{8,0b11111110}, {9,0b111111110} };
//Code C_DC[16] = {
//	{2,0b00},{2,0b01},{2,0b10},{3,0b110},{4,0b1110},{5,0b11110},{6,0b111110},{7,0b1111110},{8,0b11111110},{9,0b111111110},{10,0b1111111110},{11,0b11111111110}
//};
//Code Y_AC[256] = {
//	{4,0b1010},{2,0b00},{2,0b01},{3,0b100},{4,0b1011},{5,0b11010},{7,0b1111000},{8,0b11111000},{10,0b1111110110},{16,0b1111111110000010},{16,0b1111111110000011},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{4,0b1100},{5,0b11011},{7,0b1111001},{9,0b111110110},{11,0b11111110110},{16,0b1111111110000100},{16,0b1111111110000101},{16,0b1111111110000110},{16,0b1111111110000111},{16,0b1111111110001000},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{5,0b11100},{8,0b11111001},{10,0b1111110111},{12,0b111111110100},{16,0b1111111110001001},{16,0b1111111110001010},{16,0b1111111110001011},{16,0b1111111110001100},{16,0b1111111110001101},{16,0b1111111110001110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{6,0b111010},{9,0b111110111},{12,0b111111110101},{16,0b1111111110001111},{16,0b1111111110010000},{16,0b1111111110010001},{16,0b1111111110010010},{16,0b1111111110010011},{16,0b1111111110010100},{16,0b1111111110010101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{6,0b111011},{10,0b1111111000},{16,0b1111111110010110},{16,0b1111111110010111},{16,0b1111111110011000},{16,0b1111111110011001},{16,0b1111111110011010},{16,0b1111111110011011},{16,0b1111111110011100},{16,0b1111111110011101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{7,0b1111010},{11,0b11111110111},{16,0b1111111110011110},{16,0b1111111110011111},{16,0b1111111110100000},{16,0b1111111110100001},{16,0b1111111110100010},{16,0b1111111110100011},{16,0b1111111110100100},{16,0b1111111110100101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{7,0b1111011},{12,0b111111110110},{16,0b1111111110100110},{16,0b1111111110100111},{16,0b1111111110101000},{16,0b1111111110101001},{16,0b1111111110101010},{16,0b1111111110101011},{16,0b1111111110101100},{16,0b1111111110101101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{8,0b11111010},{12,0b111111110111},{16,0b1111111110101110},{16,0b1111111110101111},{16,0b1111111110110000},{16,0b1111111110110001},{16,0b1111111110110010},{16,0b1111111110110011},{16,0b1111111110110100},{16,0b1111111110110101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111111000},{15,0b111111111000000},{16,0b1111111110110110},{16,0b1111111110110111},{16,0b1111111110111000},{16,0b1111111110111001},{16,0b1111111110111010},{16,0b1111111110111011},{16,0b1111111110111100},{16,0b1111111110111101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111111001},{16,0b1111111110111110},{16,0b1111111110111111},{16,0b1111111111000000},{16,0b1111111111000001},{16,0b1111111111000010},{16,0b1111111111000011},{16,0b1111111111000100},{16,0b1111111111000101},{16,0b1111111111000110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111111010},{16,0b1111111111000111},{16,0b1111111111001000},{16,0b1111111111001001},{16,0b1111111111001010},{16,0b1111111111001011},{16,0b1111111111001100},{16,0b1111111111001101},{16,0b1111111111001110},{16,0b1111111111001111},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{10,0b1111111001},{16,0b1111111111010000},{16,0b1111111111010001},{16,0b1111111111010010},{16,0b1111111111010011},{16,0b1111111111010100},{16,0b1111111111010101},{16,0b1111111111010110},{16,0b1111111111010111},{16,0b1111111111011000},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{10,0b1111111010},{16,0b1111111111011001},{16,0b1111111111011010},{16,0b1111111111011011},{16,0b1111111111011100},{16,0b1111111111011101},{16,0b1111111111011110},{16,0b1111111111011111},{16,0b1111111111100000},{16,0b1111111111100001},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{11,0b11111111000},{16,0b1111111111100010},{16,0b1111111111100011},{16,0b1111111111100100},{16,0b1111111111100101},{16,0b1111111111100110},{16,0b1111111111100111},{16,0b1111111111101000},{16,0b1111111111101001},{16,0b1111111111101010},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{16,0b1111111111101011},{16,0b1111111111101100},{16,0b1111111111101101},{16,0b1111111111101110},{16,0b1111111111101111},{16,0b1111111111110000},{16,0b1111111111110001},{16,0b1111111111110010},{16,0b1111111111110011},{16,0b1111111111110100},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{11,0b11111111001},{16,0b1111111111110101},{16,0b1111111111110110},{16,0b1111111111110111},{16,0b1111111111111000},{16,0b1111111111111001},{16,0b1111111111111010},{16,0b1111111111111011},{16,0b1111111111111100},{16,0b1111111111111101},{16,0b1111111111111110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0}
//};
//Code C_AC[256] = {
//	{2,0b0},{2,0b01},{3,0b100},{4,0b1010},{5,0b11000},{5,0b11001},{6,0b111000},{7,0b1111000},{9,0b111110100},{10,0b1111110110},{12,0b111111110100},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{4,0b1011},{6,0b111001},{8,0b11110110},{9,0b111110101},{11,0b11111110110},{12,0b111111110101},{16,0b1111111110001000},{16,0b1111111110001001},{16,0b1111111110001010},{16,0b1111111110001011},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{5,0b11010},{8,0b11110111},{10,0b1111110111},{12,0b111111110110},{15,0b111111111000010},{16,0b1111111110001100},{16,0b1111111110001101},{16,0b1111111110001110},{16,0b1111111110001111},{16,0b1111111110010000},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{5,0b11011},{8,0b11111000},{10,0b1111111000},{12,0b111111110111},{16,0b1111111110010001},{16,0b1111111110010010},{16,0b1111111110010011},{16,0b1111111110010100},{16,0b1111111110010101},{16,0b1111111110010110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{6,0b111010},{9,0b111110110},{16,0b1111111110010111},{16,0b1111111110011000},{16,0b1111111110011001},{16,0b1111111110011010},{16,0b1111111110011011},{16,0b1111111110011100},{16,0b1111111110011101},{16,0b1111111110011110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{6,0b111011},{10,0b1111111001},{16,0b1111111110011111},{16,0b1111111110100000},{16,0b1111111110100001},{16,0b1111111110100010},{16,0b1111111110100011},{16,0b1111111110100100},{16,0b1111111110100101},{16,0b1111111110100110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{7,0b1111001},{11,0b11111110111},{16,0b1111111110100111},{16,0b1111111110101000},{16,0b1111111110101001},{16,0b1111111110101010},{16,0b1111111110101011},{16,0b1111111110101100},{16,0b1111111110101101},{16,0b1111111110101110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{7,0b1111010},{11,0b11111111000},{16,0b1111111110101111},{16,0b1111111110110000},{16,0b1111111110110001},{16,0b1111111110110010},{16,0b1111111110110011},{16,0b1111111110110100},{16,0b1111111110110101},{16,0b1111111110110110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{8,0b11111001},{16,0b1111111110110111},{16,0b1111111110111000},{16,0b1111111110111001},{16,0b1111111110111010},{16,0b1111111110111011},{16,0b1111111110111100},{16,0b1111111110111101},{16,0b1111111110111110},{16,0b1111111110111111},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111110111},{16,0b1111111111000000},{16,0b1111111111000001},{16,0b1111111111000010},{16,0b1111111111000011},{16,0b1111111111000100},{16,0b1111111111000101},{16,0b1111111111000110},{16,0b1111111111000111},{16,0b1111111111001000},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111111000},{16,0b1111111111001001},{16,0b1111111111001010},{16,0b1111111111001011},{16,0b1111111111001100},{16,0b1111111111001101},{16,0b1111111111001110},{16,0b1111111111001111},{16,0b1111111111010000},{16,0b1111111111010001},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111111001},{16,0b1111111111010010},{16,0b1111111111010011},{16,0b1111111111010100},{16,0b1111111111010101},{16,0b1111111111010110},{16,0b1111111111010111},{16,0b1111111111011000},{16,0b1111111111011001},{16,0b1111111111011010},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{9,0b111111010},{16,0b1111111111011011},{16,0b1111111111011100},{16,0b1111111111011101},{16,0b1111111111011110},{16,0b1111111111011111},{16,0b1111111111100000},{16,0b1111111111100001},{16,0b1111111111100010},{16,0b1111111111100011},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{11,0b11111111001},{16,0b1111111111100100},{16,0b1111111111100101},{16,0b1111111111100110},{16,0b1111111111100111},{16,0b1111111111101000},{16,0b1111111111101001},{16,0b1111111111101010},{16,0b1111111111101011},{16,0b1111111111101100},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{0,0b0},{14,0b11111111100000},{16,0b1111111111101101},{16,0b1111111111101110},{16,0b1111111111101111},{16,0b1111111111110000},{16,0b1111111111110001},{16,0b1111111111110010},{16,0b1111111111110011},{16,0b1111111111110100},{16,0b1111111111110101},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0},
//	{10,0b1111111010},{15,0b111111111000011},{16,0b1111111111110110},{16,0b1111111111110111},{16,0b1111111111111000},{16,0b1111111111111001},{16,0b1111111111111010},{16,0b1111111111111011},{16,0b1111111111111100},{16,0b1111111111111101},{16,0b1111111111111110},{0,0b0},{0,0b0},{0,0b0},{0,0b0},{0,0b0}
//};
Code Y_DC[16] = { 0 };//Y的直流哈夫曼编码表
Code C_DC[16] = { 0 };//CbCr的直流哈夫曼编码表
Code Y_AC[256] = { 0 };//Y的交流哈夫曼编码表
Code C_AC[256] = { 0 };//CbCr的交流哈夫曼编码表
Code* DCTable[3] = { Y_DC,C_DC,C_DC };
Code* ACTable[3] = { Y_AC,C_AC,C_AC };
Code* Table[4] = { Y_DC,C_DC,Y_AC,C_AC };

double DCTCOS[8][8];
//----------
// 类：压缩类
// 主要功能：压缩tiff图像为JPEG格式
//----------
class Compress {
private:
	PicReader imread;
	BYTE* RGB_data;//RGB颜色数据
	BYTE* Y_data;//Y颜色数据
	BYTE* Cb_data;//Cb颜色数据
	BYTE* Cr_data;//Cr颜色数据
	UINT W, H;//总行数和列数
	UINT W8, H8;//编码时扩展为8的倍数
	string codeStr = "";//存放哈夫曼编码的字符串，最后写入文件中
	int LastDC[3] = { 0,0,0 };//上一个8*8分块的DC值，0-1-2分别对应Y-Cb-Cr
	ofstream fout;//输出文件流
	int qscale;//压缩力度
	int Sample;//采样因子
	unsigned char versionh = 1, versionl = 1;//版本
	BYTE codeData = 0, codeLen = 0;//用于编码,codeLen每满8位，codeData就写入文件中
	//ISO/IEC 10918-1 中推荐的缺省哈夫曼编码表
	unsigned char Y_DC_NumEveryLen[16] = { 0,1,5,1,1,1,1,1,1 };
	unsigned char Y_AC_NumEveryLen[16] = { 0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125 };
	unsigned char C_DC_NumEveryLen[16] = { 0,3,1,1,1,1,1,1,1,1,1 };
	unsigned char C_AC_NumEveryLen[16] = { 0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119 };
	unsigned char Y_DC_Pos[16] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	unsigned char Y_AC_Pos[256] = { 0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,
		0x32,0x81,0x91,0xa1,0x08,0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,
		0x1a,0x25,0x26,0x27,0x28,0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,
		0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,
		0x85,0x86,0x87,0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,
		0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
		0xd8,0xd9,0xda,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa };
	unsigned char C_DC_Pos[16] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	unsigned char C_AC_Pos[256] = { 0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
		0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,0x27,0x28,0x29,0x2a,0x35,0x36,
		0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,
		0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
		0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,
		0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9, 0xfa };

private:
	/****************************************
	Function:	BulidHufTable()
	Parameter:	无
	Return:		无
	Description:根据每个码长的码数量和每个码长对应码在哈夫曼表的位置建立哈夫曼编码表
	*****************************************/
	void BulidHufTable() {
		unsigned char* numEveryLen[4]; //每个编码表的每个码长对应码的数量
		numEveryLen[0] = Y_DC_NumEveryLen;
		numEveryLen[1] = C_DC_NumEveryLen;
		numEveryLen[2] = Y_AC_NumEveryLen;
		numEveryLen[3] = C_AC_NumEveryLen;
		unsigned char* pos[4];//每个编码表的每个码长对应码开始的位置
		pos[0] = Y_DC_Pos;
		pos[1] = C_DC_Pos;
		pos[2] = Y_AC_Pos;
		pos[3] = C_AC_Pos;
		for (int type = 0; type < 4; type++) {
			int index = 0;
			int code = 0;
			for (int i = 0; i < 16; i++) {
				for (int j = 0; j < *(numEveryLen[type] + i); j++) {
					(*(Table[type] + *(pos[type] + index))).length = i + 1;
					(*(Table[type] + *(pos[type] + index))).word = code;
					code++;
					index++;
				}
				code <<= 1; //某长度开始的第一个编码等于上一个长度的最后一个编码左移一位
			}
		}
	}

	/****************************************
	Function:	getBit()
	Parameter:	int data数据, int index取数索引
	Return:		对应位置的数字0/1
	Description:取出data的二进制数中index位置的值
	*****************************************/
	BYTE getBit(int data, int index) {
		data >>= index - 1;
		return data & 1;
	}

	/****************************************
	Function:	QScale()
	Parameter:	无
	Return:		无
	Description:根据压缩力度更新量化表
	*****************************************/
	void QScale() {
		//压缩力度控制在1-10
		if (qscale < 1)qscale = 1;
		if (qscale > 10)qscale = 10;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				Y_quantization_table[i][j] = Y_quantization_table[i][j] * qscale;
				if (Y_quantization_table[i][j] > 255)Y_quantization_table[i][j] = 255;
				C_quantization_table[i][j] = C_quantization_table[i][j] * qscale;
				if (C_quantization_table[i][j] > 255)C_quantization_table[i][j] = 255;
			}
		}
	}

	/****************************************
	Function:	HeadInfo()
	Parameter:	无
	Return:		无
	Description:将头部JPEG格式信息写入文件
	*****************************************/
	void HeadInfo() {
		//写入标记SOI(FFD8)和APPO标记头(FFE0)
		int info4 = 0xE0FFD8FF;
		fout.write((char*)&info4, sizeof(info4));
		//写入APPO标记内容
		unsigned short info2 = 0x1000;//标记总长16
		fout.write((char*)&info2, sizeof(info2));
		char jfif[5] = "JFIF";//JFIF串
		fout.write(jfif, 5);
		fout.write((char*)&versionh, sizeof(versionh));	//版本
		fout.write((char*)&versionl, sizeof(versionl));	//版本
		char info1 = 0;//密度单位
		fout.write((char*)&info1, sizeof(info1));
		info4 = 0x01000100;//水平和垂直的像素密度
		fout.write((char*)&info4, sizeof(info4));
		info2 = 0x0000;//缩略图水平和垂直像素计数
		fout.write((char*)&info2, sizeof(info2));

		//写入量化表
		info2 = 0xDBFF;//标记
		fout.write((char*)&info2, sizeof(info2));
		info2 = 0x8400;//长度132=0x84
		fout.write((char*)&info2, sizeof(info2));
		info1 = 0x00;//精度和类型
		fout.write((char*)&info1, sizeof(info1));
		//按Z字型写入Y量化表
		unsigned char data[64];
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				data[ZigZag[i][j]] = Y_quantization_table[i][j];
			}
		}
		fout.write((char*)data, 64);
		info1 = 0x01;//精度和类型
		fout.write((char*)&info1, sizeof(info1));
		//按Z字型写入CbCr量化表
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				data[ZigZag[i][j]] = C_quantization_table[i][j];
			}
		}
		fout.write((char*)data, 64);

		//帧		
		info2 = 0xC0FF;//基本DCT(SOF0)标记
		fout.write((char*)&info2, sizeof(info2));
		info2 = 0x1100;//长度
		fout.write((char*)&info2, sizeof(info2));
		info1 = 8;//采样精度
		fout.write((char*)&info1, sizeof(info1));
		unsigned short width = W, height = H;
		//高度(先写高位，再写低位)
		unsigned char low = height & 0x00FF;
		height >>= 8;
		unsigned char high = (unsigned char)height;
		fout.write((char*)&high, sizeof(high));
		fout.write((char*)&low, sizeof(low));
		//宽度(先写高位，再写低位)
		low = width & 0x00FF;
		width >>= 8;
		high = (unsigned char)width;
		fout.write((char*)&high, sizeof(high));
		fout.write((char*)&low, sizeof(low));

		info1 = 3;//分量数
		fout.write((char*)&info1, sizeof(info1));
		info1 = 1;//分量标志
		fout.write((char*)&info1, sizeof(info1));
		info1 = Sample;//Y采样因子
		info1 <<= 4;
		info1 += Sample;
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0;//使用的量化表类型
		fout.write((char*)&info1, sizeof(info1));
		info1 = 2;//分量标志
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x11;//Cb采样因子
		fout.write((char*)&info1, sizeof(info1));
		info1 = 1;//使用的量化表类型
		fout.write((char*)&info1, sizeof(info1));
		info1 = 3;//分量标志
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x11;//Cr采样因子
		fout.write((char*)&info1, sizeof(info1));
		info1 = 1;//使用的量化表类型
		fout.write((char*)&info1, sizeof(info1));

		info2 = 0xC4FF;//哈夫曼表标记
		fout.write((char*)&info2, sizeof(info2));
		int num[4] = { 0 };
		for (int i = 0; i < 16; i++) {
			num[0] += Y_DC_NumEveryLen[i];
			num[1] += C_DC_NumEveryLen[i];
			num[2] += Y_AC_NumEveryLen[i];
			num[3] += C_AC_NumEveryLen[i];
		}
		//计算信息长度
		unsigned short len = num[0] + num[1] + num[2] + num[3] + 2 + 4 + 4 * 16;
		unsigned short lenh = len >> 8;
		unsigned short lenl = len & 0xFF;
		lenl <<= 8;
		info2 = lenh + lenl;
		fout.write((char*)&info2, sizeof(info2));
		//Y分量
		info1 = 0x00;//DC,Y
		fout.write((char*)&info1, sizeof(info1));
		fout.write((char*)Y_DC_NumEveryLen, 16);
		fout.write((char*)Y_DC_Pos, num[0]);
		info1 = 0x10;//AC,Y
		fout.write((char*)&info1, sizeof(info1));
		fout.write((char*)Y_AC_NumEveryLen, 16);
		fout.write((char*)Y_AC_Pos, num[2]);
		//C分量
		info1 = 0x01;//DC,C
		fout.write((char*)&info1, sizeof(info1));
		fout.write((char*)C_DC_NumEveryLen, 16);
		fout.write((char*)C_DC_Pos, num[1]);
		info1 = 0x11;//AC,C
		fout.write((char*)&info1, sizeof(info1));
		fout.write((char*)C_AC_NumEveryLen, 16);
		fout.write((char*)C_AC_Pos, num[3]);

		//扫描开始(SOS)
		info2 = 0xDAFF;//扫描开始标记
		fout.write((char*)&info2, sizeof(info2));
		info2 = 0x0C00;//长度
		fout.write((char*)&info2, sizeof(info2));
		info1 = 3;//分量数
		fout.write((char*)&info1, sizeof(info1));
		info1 = 1;//分量Y
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x00;
		fout.write((char*)&info1, sizeof(info1));
		info1 = 2;//分量Cb
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x11;
		fout.write((char*)&info1, sizeof(info1));
		info1 = 3;//分量Cr
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x11;
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x00;
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x3F;
		fout.write((char*)&info1, sizeof(info1));
		info1 = 0x00;
		fout.write((char*)&info1, sizeof(info1));
	}

public:
	//构造函数
	//finname:输入文件名 foutname:输出文件名 s:采样因子(1-4) qs:压缩力度(1-10)
	Compress(char* finname, int s = 1, int qs = 1) {
		//设置采样因子和压缩力度
		if (s < 1)s = 1;
		if (s > 4)s = 4;
		Sample = s;
		qscale = qs;
		QScale();
		//打开文件
		char foutname[100];
		int i;
		if (strlen(finname) > 90) {
			cerr << "错误信息：输入文件名太长！" << endl;
			exit(1);
		}
		for (i = (int)strlen(finname); i >= 0; i--) {
			if (finname[i] == '.') {
				break;
			}
		}
		memcpy(foutname, finname, i + 1);
		foutname[i + 1] = 'j'; foutname[i + 2] = 'p'; foutname[i + 3] = 'g'; foutname[i + 4] = '\0';
		fout.open(foutname, ios::binary);
		if (!fout) {
			cerr << "错误信息：打开文件失败！" << endl;
			exit(1);
		}
		RGB_data = nullptr;
		Y_data = nullptr;
		Cb_data = nullptr;
		Cr_data = nullptr;

		//读取RGB数据
		imread.readPic(finname);
		imread.getData(RGB_data, W, H);
		//使图像尺寸为8的倍数
		W8 = W + (8 - W % 8) % 8;
		H8 = H + (8 - H % 8) % 8;
		if ((W8 % (8 * Sample) != 0) || (H8 % (8 * Sample) != 0)) {
			Sample = 1;
		}
		//RGB转换为YCbCr
		Y_data = new(nothrow) BYTE[H8 * W8 + 1];
		Cb_data = new(nothrow) BYTE[H8 * W8 + 1];
		Cr_data = new(nothrow) BYTE[H8 * W8 + 1];
		if (Y_data == nullptr || Cb_data == nullptr || Cr_data == nullptr) {
			cerr << "错误信息：申请内存空间失败！" << endl;
			exit(1);
		}
		for (unsigned int r = 0; r < H; r++) {
			for (unsigned int c = 0; c < W; c++) {
				int index = r * W * 4 + c * 4;
				BYTE R = RGB_data[index];
				BYTE G = RGB_data[index + 1];
				BYTE B = RGB_data[index + 2];
				Y_data[r * W8 + c] = BYTE(0.299 * R + 0.587 * G + 0.114 * B);
				Cb_data[r * W8 + c] = BYTE(-0.1687 * R - 0.3313 * G + 0.5 * B + 128);
				Cr_data[r * W8 + c] = BYTE(0.5 * R - 0.4187 * G - 0.0813 * B + 128);
			}
		}
		//扩充为8的倍数
		int index = H * W * 4 + W * 4;
		BYTE r = RGB_data[index];
		BYTE g = RGB_data[index + 1];
		BYTE b = RGB_data[index + 2];
		for (unsigned int r = H; r < H8; r++) {
			for (unsigned int c = W; c < W8; c++) {
				Y_data[r * W8 + c] = (char)(0.299 * r + 0.587 * g + 0.114 * b - 128);
				Cb_data[r * W8 + c] = (char)(-0.1687 * r - 0.3313 * g + 0.5 * b);
				Cr_data[r * W8 + c] = (char)(0.5 * r - 0.4187 * g - 0.0813 * b);
			}
		}
		//建立哈夫曼编码表
		BulidHufTable();
	}
	//析构函数
	~Compress() {		
		delete[] RGB_data;
		delete[] Y_data;
		delete[] Cr_data;
		delete[] Cb_data;
		RGB_data = nullptr;
		Y_data = nullptr;
		Cb_data = nullptr;
		Cr_data = nullptr;
	}

	/****************************************
	Function:	DctAndQuantization()
	Parameter:	flag：表示Y/Cb/Cr, row:图片行号, col:图片列号, 
				data:量化后的数据
	Return:		无
	Description:对8*8分块进行离散余弦变换并量化
	*****************************************/
	void DctAndQuantization(int flag, int row, int col, int* zdata) {
		double data[8][8];//DCT变换后的数据
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				double CI = (i == 0) ? 1 / sqrt(8.0) : 0.5;
				double CJ = (j == 0) ? 1 / sqrt(8.0) : 0.5;
				double p = 0;
				for (int x = 0; x < 8; x++)
				{
					for (int y = 0; y < 8; y++)
					{
						//DCTCOS 提前计算好DCT变换中涉及的cos函数值
						if (flag == 0) {
							p += (Y_data[(row + y) * W8 + (col + x)] - 128) * DCTCOS[x][j] * DCTCOS[y][i];
						}
						else if (flag == 1) {
							p += (Cb_data[(row + y) * W8 + (col + x)] - 128) * DCTCOS[x][j] * DCTCOS[y][i];
						}
						else if (flag == 2) {
							p += (Cr_data[(row + y) * W8 + (col + x)] - 128) * DCTCOS[x][j] * DCTCOS[y][i];
						}
					}
				}
				data[i][j] = p * CI * CJ;
			}
		}
		//量化
		if (flag == 0) {
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					zdata[ZigZag[i][j]] = (int)round((double)data[i][j] / (double)Y_quantization_table[i][j]);
				}
			}
		}
		else {
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					zdata[ZigZag[i][j]] = (int)round((double)data[i][j] / (double)C_quantization_table[i][j]);
				}
			}
		}
	}

	/****************************************
	Function:	HuffmanEncode()
	Parameter:	flag：表示Y/Cb/Cr, zdata:量化后的数据
	Return:		无
	Description:对量化后的数据进行哈夫曼编码并写入文件
	*****************************************/
	void HuffmanEncode(int flag, const int* zdata) {
		//DC编码
		Code hufCode[128];
		int d = zdata[0] - LastDC[flag];//和上一个DC差值
		LastDC[flag] = zdata[0];
		int index = 0;
		if (d == 0) {
			//差值为0
			hufCode[index++] = DCTable[flag][0];
		}
		else {
			//差值不为0
			Code Loc = { 0 };
			int D = abs(d);
			while (D > 0) {
				D >>= 1;
				Loc.length++;
			}
			if (d > 0) {
				Loc.word = d;
			}
			else {
				Loc.word = (int)pow(2, Loc.length) - 1 + d;
			}
			hufCode[index++] = DCTable[flag][Loc.length];//在编码表中的行位置
			hufCode[index++] = Loc;//在编码表中的列位置

		}

		//AC编码
		Code EOB = ACTable[flag][0], ZRL = ACTable[flag][15 * 16];
		//找到EOB位置
		int i = 63;
		while (zdata[i] == 0)i--;
		int z = i;
		for (i = 1; i <= z; i++) {
			int zeroPos = i;
			while (zdata[i] == 0 && i <= z) {
				i++;
			}//统计连续的0
			int zeroNum = i - zeroPos;
			while (zeroNum >= 16) {
				hufCode[index++] = ZRL;
				zeroNum -= 16;
			}//16个0可编码为ZRL
			Code Loc = { 0 };
			int D = abs(zdata[i]);
			while (D > 0) {
				D >>= 1;
				Loc.length++;
			}
			if (zdata[i] > 0) {
				Loc.word = zdata[i];
			}
			else {
				Loc.word = (int)pow(2, Loc.length) - 1 + zdata[i];
			}
			hufCode[index++] = ACTable[flag][zeroNum * 16 + Loc.length];//在编码表中的行位置
			hufCode[index++] = Loc;//在编码表中的列位置
		}
		if (z < 63) {
			hufCode[index++] = EOB;
		}

		//写入文件
		for (int i = 0; i < index; i++) {
			int code = hufCode[i].word;
			int length = hufCode[i].length;
			while (length > 0) {
				//每满8位就写入文件
				if (codeLen == 8) {
					codeStr += codeData;
					if (codeData == 0xFF) {
						codeStr += '\0';
					}//与JPEG的FF标记区分
					codeData = 0;
					codeLen = 0;
				}
				BYTE bit = getBit(code, length);
				codeData <<= 1;
				codeData |= bit;
				codeLen++;
				length--;
			}
		}
	}

	/****************************************
	Function:	compress()
	Parameter:	无
	Return:		是否成功标志
	Description:压缩图像为JPEG格式
	*****************************************/
	bool compress() {
		//写入文件头信息
		HeadInfo();
		//对每个8*8分块编码
		for (unsigned int row = 0; row < H8; row += Sample * 8) {
			for (unsigned int col = 0; col < W8; col += Sample * 8) {
				int zdata[64];//离散余弦变换及量化后的数据
				//Y
				for (int i = 0; i < Sample; i++) {
					for (int j = 0; j < Sample; j++) {
						DctAndQuantization(0, row + i * 8, col + j * 8, zdata);
						HuffmanEncode(0, zdata);
					}
				}
				//Y采样Sample*Sample次，Cb和Cr采样一次
				//Cb
				DctAndQuantization(1, row, col, zdata);
				HuffmanEncode(1, zdata);
				//Cr
				DctAndQuantization(2, row, col, zdata);
				HuffmanEncode(2, zdata);
			}
		}

		fout << codeStr;//写入编码

		//写入JPEG文件结束结束标志
		unsigned short info2 = 0xD9FF;//EOI标记
		fout.write((char*)&info2, sizeof(info2));
		fout.close();
		return true;
	}	
};