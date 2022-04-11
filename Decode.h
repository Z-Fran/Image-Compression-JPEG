#include"Compress.h"
//解码JPEG格式并显示
class Decode {
private:
	PicReader imread;
	ifstream fi;
	istringstream fin;
	BYTE* RGB_data;//数据
	BYTE* Y_data;//数据
	BYTE* Cb_data;//数据
	BYTE* Cr_data;//数据
	UINT W, H;//总行数和列数
	UINT W8, H8;//编码时扩展为8的倍数

	int LastDC[3] = { 0,0,0 };//上一个DC值
	unsigned char versionh, versionl;//版本
	int Sample;//采样银子
	BYTE codeData = 0, codeLen = 0;//用于按位读取数据
	//每个编码表的每个码长对应码的数量
	unsigned char Y_DC_NumEveryLen[16] = { 0 };
	unsigned char Y_AC_NumEveryLen[16] = { 0 };
	unsigned char C_DC_NumEveryLen[16] = { 0 };
	unsigned char C_AC_NumEveryLen[16] = { 0 };
	//每个编码表的每个码长对应码在哈夫曼表的位置
	unsigned char Y_DC_Pos[16] = { 0 };
	unsigned char Y_AC_Pos[256] = { 0 };
	unsigned char C_DC_Pos[16] = { 0 };
	unsigned char C_AC_Pos[256] = { 0 };
	vector<vector<vector<int>>> posEveryLen; //每个长度的码对应在哈夫曼表中的位置

private:
	/****************************************
	Function:	readHead()
	Parameter:	无
	Return:		无
	Description:将头部JPEG格式信息读入文件
	*****************************************/
	bool readHead() {
		unsigned char info1; unsigned short info2; unsigned int info4; int length;
		//开始标记SOI(FFD8)和APPO标记头(FFE0)
		fin.read((char*)&info4, sizeof(info4));
		if (info4 != 0xE0FFD8FF) {
			cerr << "读取APPO失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));
		length = info1 << 8;
		fin.read((char*)&info1, sizeof(info1));
		length += info1;
		length -= 2;
		char jfif[5];
		fin.read(jfif, 5); length -= 5;
		//版本
		fin.read((char*)&versionh, sizeof(versionh));
		fin.read((char*)&versionl, sizeof(versionl));
		length -= 2;
		//单位，分辨率，缩略图
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info4, sizeof(info4));
		fin.read((char*)&info2, sizeof(info2));
		length -= 7;
		if (length != 0) {
			cerr << "读取APPO失败！" << endl;
			return false;
		}

		//量化表
		fin.read((char*)&info2, sizeof(info2));
		if (info2 != 0xDBFF) {
			cerr << "读取量化表失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));
		length = info1 << 8;
		fin.read((char*)&info1, sizeof(info1));
		length += info1;
		length -= 2;
		fin.read((char*)&info1, sizeof(info1));
		length -= 1;
		unsigned char data[64];
		unsigned char samplesize = info1 >> 4;
		if (samplesize != 0) {
			cerr << "此JPEG图片采样精度不是8，程序暂不支持！" << endl;
			return false;
		}
		if (info1 == 0x00) {
			fin.read((char*)data, 64);
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					Y_quantization_table[i][j] = data[ZigZag[i][j]];
				}
			}
			length -= 64;
		}
		else if (info1 == 0x01) {
			fin.read((char*)data, 64);
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					C_quantization_table[i][j] = data[ZigZag[i][j]];
				}
			}
			length -= 64;
		}
		fin.read((char*)&info1, sizeof(info1));
		length -= 1;
		if (info1 == 0x00) {
			fin.read((char*)data, 64);
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					Y_quantization_table[i][j] = data[ZigZag[i][j]];
				}
			}
			length -= 64;
		}
		else if (info1 == 0x01) {
			fin.read((char*)data, 64);
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					C_quantization_table[i][j] = data[ZigZag[i][j]];
				}
			}
			length -= 64;
		}
		if (length != 0) {
			cerr << "读取量化表失败！" << endl;
			return false;
		}

		//帧	
		fin.read((char*)&info2, sizeof(info2)); // 基本DCT(SOF0)标记
		if (info2 != 0xC0FF) {
			cerr << "读取帧失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//长度
		length = info1 << 8;
		fin.read((char*)&info1, sizeof(info1));
		length += info1;
		length -= 2;
		fin.read((char*)&info1, sizeof(info1));//采样精度
		if (info1 != 8) {
			cerr << "此JPEG图片采样精度不是8，程序暂不支持！" << endl;
			return false;
		}
		length -= 1;
		//高度
		unsigned char low, high;
		fin.read((char*)&high, sizeof(high));
		fin.read((char*)&low, sizeof(low));
		H = high;
		H <<= 8;
		H += low;
		//宽度
		fin.read((char*)&high, sizeof(high));
		fin.read((char*)&low, sizeof(low));
		W = high;
		W <<= 8;
		W += low;
		length -= 4;
		W8 = W + (8 - W % 8) % 8;
		H8 = H + (8 - H % 8) % 8;

		fin.read((char*)&info1, sizeof(info1));//分量数
		fin.read((char*)&info1, sizeof(info1));//分量标志
		fin.read((char*)&info1, sizeof(info1));//Y采样因子
		switch (info1) {
		case 0x11:Sample = 1; break;
		case 0x22:Sample = 2; break;
		case 0x33:Sample = 3; break;
		case 0x44:Sample = 4; break;
		default:cerr << "暂不支持该图片的采样因子" << endl;
			return false;
		}
		if ((W8 % (8 * Sample) != 0) || (H8 % (8 * Sample) != 0)) {
			cerr << "该图片采样因子与图片尺寸不成比例关系，暂不支持！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//使用的量化表类型
		fin.read((char*)&info1, sizeof(info1));//分量标志
		fin.read((char*)&info1, sizeof(info1));//Cb采样因子
		fin.read((char*)&info1, sizeof(info1));//使用的量化表类型
		fin.read((char*)&info1, sizeof(info1));//分量标志
		fin.read((char*)&info1, sizeof(info1));//Cr采样因子
		fin.read((char*)&info1, sizeof(info1));//使用的量化表类型
		length -= 10;
		if (length != 0) {
			cerr << "读取帧失败！" << endl;
			return false;
		}

		//哈夫曼表
		fin.read((char*)&info2, sizeof(info2));
		if (info2 != 0xC4FF) {
			cerr << "读取哈夫曼表失败！" << endl;
			return false;
		}
		fin.read((char*)&info2, sizeof(info2));//长度
		fin.read((char*)&info1, sizeof(info1));//分量标志
		if (!ReadHufTable(info1)) {
			cerr << "读取哈夫曼表失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//分量标志
		if (!ReadHufTable(info1)) {
			cerr << "读取哈夫曼表失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//分量标志
		if (!ReadHufTable(info1)) {
			cerr << "读取哈夫曼表失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//分量标志
		if (!ReadHufTable(info1)) {
			cerr << "读取哈夫曼表失败！" << endl;
			return false;
		}

		//扫描开始(SOS)
		fin.read((char*)&info2, sizeof(info2));
		if (info2 != 0xDAFF) {
			cerr << "读取SOS失败！" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));
		length = info1 << 8;
		fin.read((char*)&info1, sizeof(info1));
		length += info1;
		length -= 2;
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info1, sizeof(info1));
		length -= 10;
		if (length != 0) {
			cerr << "读取SOS失败！" << endl;
			return false;
		}
		return true;
	}
	/****************************************
	Function:	ReadHufTable()
	Parameter:	info:哈夫曼表类型
				0-Y_DC,1-C_DC,2-Y_AC,3-C_AC
	Return:		读取成功标志
	Description:读取并构建某类型哈夫曼表
	*****************************************/
	bool ReadHufTable(char info) {
		int type = -1;
		switch (info) {
		case 0x00:type = 0; break;
		case 0x01:type = 1; break;
		case 0x10:type = 2; break;
		case 0x11:type = 3; break;
		}
		if (type < 0 || type>3) {
			return false;
		}
		unsigned char* numEveryLen[4];
		numEveryLen[0] = Y_DC_NumEveryLen;
		numEveryLen[1] = C_DC_NumEveryLen;
		numEveryLen[2] = Y_AC_NumEveryLen;
		numEveryLen[3] = C_AC_NumEveryLen;
		unsigned char* pos[4];
		pos[0] = Y_DC_Pos;
		pos[1] = C_DC_Pos;
		pos[2] = Y_AC_Pos;
		pos[3] = C_AC_Pos;
		Code* table[4];
		table[0] = Y_DC;
		table[1] = C_DC;
		table[2] = Y_AC;
		table[3] = C_AC;
		fin.read((char*)numEveryLen[type], 16);
		//统计总个数
		int num = 0;
		for (int i = 0; i < 16; i++) {
			num += *(numEveryLen[type] + i);
		}
		//判断总个数是否符合规则
		if ((type < 2 && num>16) || (num > 256)) {
			return false;
		}
		fin.read((char*)pos[type], num);
		//构造编码表
		int index = 0;
		int code = 0;
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < *(numEveryLen[type] + i); j++) {
				posEveryLen[type][i].push_back(*(pos[type] + index));
				(*(table[type] + *(pos[type] + index))).length = i + 1;
				(*(table[type] + *(pos[type] + index))).word = code;
				code++;
				index++;
			}
			code <<= 1;
		}
		return true;
	}

	/****************************************
	Function:	getNextBit()
	Parameter:	无
	Return:		读取到的数值
	Description:从文件中读取下一位数据
	*****************************************/
	int getNextBit() {
		//没使用完8位继续读取一字节
		if (codeLen == 0) {
			fin.read((char*)&codeData, 1);
			codeLen = 8;
			if (codeData == 0xFF) {
				char temp;
				fin.read((char*)&temp, 1);
			}//FF后面跟了一个无用的0x00
		}
		int bit = codeData & 0b10000000;
		bit >>= 7;
		codeData <<= 1;
		codeLen--;
		return bit;
	}

	/****************************************
	Function:	GetNextCode()
	Parameter:	type：哈夫曼表类型
				0-Y_DC,1-C_DC,2-Y_AC,3-C_AC
	Return:		获取到的编码在哈夫曼表中的位置
	Description:从文件中读取数据并解析出下一个哈夫曼编码
	*****************************************/
	int GetNextCode(int type) {
		int len = 0;
		int word = 0;
		unsigned int i;
		int flag = 1;
		while (flag) {
			word <<= 1;
			word += getNextBit();
			len++;
			for (i = 0; i < posEveryLen[type][len - 1].size(); i++) {
				if (word == Table[type][posEveryLen[type][len - 1][i]].word) {
					flag = 0;
					break;
				}
			}
		}
		return posEveryLen[type][len - 1][i];
	}

public:
	//构造函数
	Decode(char *finname) {
		//读入文件全部内容
		fi.open(finname, ios::binary);
		if (!fin) {
			cerr << "错误信息：打开文件失败！" << endl;
			exit(1);
		}
		istreambuf_iterator<char> beg(fi), end;// 设置两个文件指针，指向开始和结束，以char(一字节)为步长
		string content(beg, end);               // 将文件全部读入string字符串
		fi.close();
		fin.str(content);

		//初始化：每个长度的码对应在哈夫曼表中的位置
		posEveryLen.resize(4);
		posEveryLen[0].resize(16);
		posEveryLen[1].resize(16);
		posEveryLen[2].resize(16);
		posEveryLen[3].resize(16);

		//读取JPEG文件头部信息
		if (!readHead()) {
			cerr << "读取文件格式失败！" << endl;
			exit(1);
		};

		//申请内存空间
		Y_data = new(nothrow) BYTE[H8 * W8];
		Cb_data = new(nothrow) BYTE[H8 * W8];
		Cr_data = new(nothrow) BYTE[H8 * W8];
		if (Y_data == nullptr || Cb_data == nullptr || Cr_data == nullptr) {
			cerr << "错误信息：申请内存空间失败！" << endl;
			exit(1);
		}
		RGB_data = new(nothrow) BYTE[4 * H * W];
		if (RGB_data == nullptr) {
			cerr << "错误信息：申请内存空间失败！" << endl;
			exit(1);
		}
		memset((char*)RGB_data, 255, 4 * H * W);
	}
	//析构函数
	~Decode() {
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
				data:反离散余弦变换后的数据
	Return:		无
	Description:对8*8分块进行反量化和反离散余弦变换
	*****************************************/
	void DctAndQuantization(int flag, int row, int col, int* zdata) {
		//反量化
		double data[8][8];
		if (flag == 0) {
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					data[i][j] = (double)zdata[ZigZag[i][j]] * (double)Y_quantization_table[i][j];
				}
			}
		}
		else {
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					data[i][j] = (double)zdata[ZigZag[i][j]] * (double)C_quantization_table[i][j];
				}
			}
		}
		//反离散余弦变换
		for (int x = 0; x < 8; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				double p = 0;
				for (int i = 0; i < 8; i++)
				{
					for (int j = 0; j < 8; j++)
					{
						double CI = (i == 0) ? 1 / sqrt(8.0) : 0.5;
						double CJ = (j == 0) ? 1 / sqrt(8.0) : 0.5;
						p += CI * CJ * data[i][j] * DCTCOS[x][i] * DCTCOS[y][j];
					}
				}
				p = p + 128;
				if (flag == 0) {
					Y_data[(row + x) * W8 + (col + y)] = (BYTE)round(p);
				}
				else if (flag == 1) {
					Cb_data[(row + x) * W8 + (col + y)] = (BYTE)round(p);
				}
				else if (flag == 2) {
					Cr_data[(row + x) * W8 + (col + y)] = (BYTE)round(p);
				}
			}
		}
	}

	/****************************************
	Function:	HuffmanDecode()
	Parameter:	flag：表示Y/Cb/Cr, data:解码后的数据
	Return:		无
	Description:对8*8分块的编码进行解码
	*****************************************/
	void HuffmanDecode(int flag, int* zdata) {
		memset((char*)zdata, 0, 4 * 64);
		int pos;
		//DC解码
		pos = GetNextCode((flag + 1) / 2);
		if (pos != 0) {
			int word = 0;
			for (int i = 0; i < pos; i++) {
				word <<= 1;
				word += getNextBit();
			}
			if (word >= pow(2, pos - 1)) {
				LastDC[flag] += word;
			}
			else {
				LastDC[flag] += word - (int)pow(2, pos) + 1;
			}
		}
		zdata[0] = LastDC[flag];

		//AC解码
		int EOB = 0, ZRL = 15 * 16;
		for (int i = 1; i < 64; i++) {
			pos = GetNextCode((flag + 1) / 2 + 2);
			if (pos == EOB) {
				break;
			}
			//pos高4位是0的长度，低4位是length
			int word = 0;
			int len = pos & 0xf;
			int num = pos >> 4;
			i += num;
			for (int j = 0; j < len; j++) {
				word <<= 1;
				word += getNextBit();
			}
			if (word >= pow(2, len - 1)) {
				zdata[i] = word;
			}
			else {
				zdata[i] = word - (int)pow(2, len) + 1;
			}
		}
	}
	
	/****************************************
	Function:	decode()
	Parameter:	无
	Return:		无
	Description:解码JPEG格式图片，转换为RGB数据并显示
	*****************************************/
	bool decode() {
		for (unsigned int row = 0; row < H8; row += Sample * 8) {
			for (unsigned int col = 0; col < W8; col += Sample * 8) {
				int zdata[64];
				//Y
				for (int i = 0; i < Sample; i++) {
					for (int j = 0; j < Sample; j++) {
						HuffmanDecode(0, zdata);
						DctAndQuantization(0, row + i * 8, col + j * 8, zdata);
					}
				}
				//Cb
				HuffmanDecode(1, zdata);
				for (int i = 0; i < Sample; i++) {
					for (int j = 0; j < Sample; j++) {
						DctAndQuantization(1, row + i * 8, col + j * 8, zdata);
					}
				}
				//Cr
				HuffmanDecode(2, zdata);
				for (int i = 0; i < Sample; i++) {
					for (int j = 0; j < Sample; j++) {
						DctAndQuantization(2, row + i * 8, col + j * 8, zdata);
					}
				}
			}
		}
		//转RGB
		for (unsigned int r = 0; r < H; r++) {
			for (unsigned int c = 0; c < W; c++) {
				int index = r * W * 4 + c * 4;
				int R = (int)round(Y_data[r * W8 + c] + 1.371 * (Cr_data[r * W8 + c] - 128));
				int G = (int)round(Y_data[r * W8 + c] - 0.698 * (Cr_data[r * W8 + c] - 128) - 0.336 * (Cb_data[r * W8 + c] - 128));
				int B = (int)round(Y_data[r * W8 + c] + 1.732 * (Cb_data[r * W8 + c] - 128));
				if (R < 0)R = 0;
				if (G < 0)G = 0;
				if (B < 0)B = 0;
				if (R > 255)R = 255;
				if (G > 255)G = 255;
				if (B > 255)B = 255;
				RGB_data[index] = R;
				RGB_data[index + 1] = G;
				RGB_data[index + 2] = B;
			}
		}
		cout << "版本：" << (unsigned int)versionh << "." << (unsigned int)versionl << endl;
		cout << "高度：" << H << "     " << "宽度：" << W << endl;
		imread.showPic(RGB_data, W, H);
		return true;
	}
};