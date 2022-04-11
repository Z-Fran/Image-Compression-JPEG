#include"Compress.h"
//����JPEG��ʽ����ʾ
class Decode {
private:
	PicReader imread;
	ifstream fi;
	istringstream fin;
	BYTE* RGB_data;//����
	BYTE* Y_data;//����
	BYTE* Cb_data;//����
	BYTE* Cr_data;//����
	UINT W, H;//������������
	UINT W8, H8;//����ʱ��չΪ8�ı���

	int LastDC[3] = { 0,0,0 };//��һ��DCֵ
	unsigned char versionh, versionl;//�汾
	int Sample;//��������
	BYTE codeData = 0, codeLen = 0;//���ڰ�λ��ȡ����
	//ÿ��������ÿ���볤��Ӧ�������
	unsigned char Y_DC_NumEveryLen[16] = { 0 };
	unsigned char Y_AC_NumEveryLen[16] = { 0 };
	unsigned char C_DC_NumEveryLen[16] = { 0 };
	unsigned char C_AC_NumEveryLen[16] = { 0 };
	//ÿ��������ÿ���볤��Ӧ���ڹ��������λ��
	unsigned char Y_DC_Pos[16] = { 0 };
	unsigned char Y_AC_Pos[256] = { 0 };
	unsigned char C_DC_Pos[16] = { 0 };
	unsigned char C_AC_Pos[256] = { 0 };
	vector<vector<vector<int>>> posEveryLen; //ÿ�����ȵ����Ӧ�ڹ��������е�λ��

private:
	/****************************************
	Function:	readHead()
	Parameter:	��
	Return:		��
	Description:��ͷ��JPEG��ʽ��Ϣ�����ļ�
	*****************************************/
	bool readHead() {
		unsigned char info1; unsigned short info2; unsigned int info4; int length;
		//��ʼ���SOI(FFD8)��APPO���ͷ(FFE0)
		fin.read((char*)&info4, sizeof(info4));
		if (info4 != 0xE0FFD8FF) {
			cerr << "��ȡAPPOʧ�ܣ�" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));
		length = info1 << 8;
		fin.read((char*)&info1, sizeof(info1));
		length += info1;
		length -= 2;
		char jfif[5];
		fin.read(jfif, 5); length -= 5;
		//�汾
		fin.read((char*)&versionh, sizeof(versionh));
		fin.read((char*)&versionl, sizeof(versionl));
		length -= 2;
		//��λ���ֱ��ʣ�����ͼ
		fin.read((char*)&info1, sizeof(info1));
		fin.read((char*)&info4, sizeof(info4));
		fin.read((char*)&info2, sizeof(info2));
		length -= 7;
		if (length != 0) {
			cerr << "��ȡAPPOʧ�ܣ�" << endl;
			return false;
		}

		//������
		fin.read((char*)&info2, sizeof(info2));
		if (info2 != 0xDBFF) {
			cerr << "��ȡ������ʧ�ܣ�" << endl;
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
			cerr << "��JPEGͼƬ�������Ȳ���8�������ݲ�֧�֣�" << endl;
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
			cerr << "��ȡ������ʧ�ܣ�" << endl;
			return false;
		}

		//֡	
		fin.read((char*)&info2, sizeof(info2)); // ����DCT(SOF0)���
		if (info2 != 0xC0FF) {
			cerr << "��ȡ֡ʧ�ܣ�" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//����
		length = info1 << 8;
		fin.read((char*)&info1, sizeof(info1));
		length += info1;
		length -= 2;
		fin.read((char*)&info1, sizeof(info1));//��������
		if (info1 != 8) {
			cerr << "��JPEGͼƬ�������Ȳ���8�������ݲ�֧�֣�" << endl;
			return false;
		}
		length -= 1;
		//�߶�
		unsigned char low, high;
		fin.read((char*)&high, sizeof(high));
		fin.read((char*)&low, sizeof(low));
		H = high;
		H <<= 8;
		H += low;
		//���
		fin.read((char*)&high, sizeof(high));
		fin.read((char*)&low, sizeof(low));
		W = high;
		W <<= 8;
		W += low;
		length -= 4;
		W8 = W + (8 - W % 8) % 8;
		H8 = H + (8 - H % 8) % 8;

		fin.read((char*)&info1, sizeof(info1));//������
		fin.read((char*)&info1, sizeof(info1));//������־
		fin.read((char*)&info1, sizeof(info1));//Y��������
		switch (info1) {
		case 0x11:Sample = 1; break;
		case 0x22:Sample = 2; break;
		case 0x33:Sample = 3; break;
		case 0x44:Sample = 4; break;
		default:cerr << "�ݲ�֧�ָ�ͼƬ�Ĳ�������" << endl;
			return false;
		}
		if ((W8 % (8 * Sample) != 0) || (H8 % (8 * Sample) != 0)) {
			cerr << "��ͼƬ����������ͼƬ�ߴ粻�ɱ�����ϵ���ݲ�֧�֣�" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//ʹ�õ�����������
		fin.read((char*)&info1, sizeof(info1));//������־
		fin.read((char*)&info1, sizeof(info1));//Cb��������
		fin.read((char*)&info1, sizeof(info1));//ʹ�õ�����������
		fin.read((char*)&info1, sizeof(info1));//������־
		fin.read((char*)&info1, sizeof(info1));//Cr��������
		fin.read((char*)&info1, sizeof(info1));//ʹ�õ�����������
		length -= 10;
		if (length != 0) {
			cerr << "��ȡ֡ʧ�ܣ�" << endl;
			return false;
		}

		//��������
		fin.read((char*)&info2, sizeof(info2));
		if (info2 != 0xC4FF) {
			cerr << "��ȡ��������ʧ�ܣ�" << endl;
			return false;
		}
		fin.read((char*)&info2, sizeof(info2));//����
		fin.read((char*)&info1, sizeof(info1));//������־
		if (!ReadHufTable(info1)) {
			cerr << "��ȡ��������ʧ�ܣ�" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//������־
		if (!ReadHufTable(info1)) {
			cerr << "��ȡ��������ʧ�ܣ�" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//������־
		if (!ReadHufTable(info1)) {
			cerr << "��ȡ��������ʧ�ܣ�" << endl;
			return false;
		}
		fin.read((char*)&info1, sizeof(info1));//������־
		if (!ReadHufTable(info1)) {
			cerr << "��ȡ��������ʧ�ܣ�" << endl;
			return false;
		}

		//ɨ�迪ʼ(SOS)
		fin.read((char*)&info2, sizeof(info2));
		if (info2 != 0xDAFF) {
			cerr << "��ȡSOSʧ�ܣ�" << endl;
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
			cerr << "��ȡSOSʧ�ܣ�" << endl;
			return false;
		}
		return true;
	}
	/****************************************
	Function:	ReadHufTable()
	Parameter:	info:������������
				0-Y_DC,1-C_DC,2-Y_AC,3-C_AC
	Return:		��ȡ�ɹ���־
	Description:��ȡ������ĳ���͹�������
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
		//ͳ���ܸ���
		int num = 0;
		for (int i = 0; i < 16; i++) {
			num += *(numEveryLen[type] + i);
		}
		//�ж��ܸ����Ƿ���Ϲ���
		if ((type < 2 && num>16) || (num > 256)) {
			return false;
		}
		fin.read((char*)pos[type], num);
		//��������
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
	Parameter:	��
	Return:		��ȡ������ֵ
	Description:���ļ��ж�ȡ��һλ����
	*****************************************/
	int getNextBit() {
		//ûʹ����8λ������ȡһ�ֽ�
		if (codeLen == 0) {
			fin.read((char*)&codeData, 1);
			codeLen = 8;
			if (codeData == 0xFF) {
				char temp;
				fin.read((char*)&temp, 1);
			}//FF�������һ�����õ�0x00
		}
		int bit = codeData & 0b10000000;
		bit >>= 7;
		codeData <<= 1;
		codeLen--;
		return bit;
	}

	/****************************************
	Function:	GetNextCode()
	Parameter:	type��������������
				0-Y_DC,1-C_DC,2-Y_AC,3-C_AC
	Return:		��ȡ���ı����ڹ��������е�λ��
	Description:���ļ��ж�ȡ���ݲ���������һ������������
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
	//���캯��
	Decode(char *finname) {
		//�����ļ�ȫ������
		fi.open(finname, ios::binary);
		if (!fin) {
			cerr << "������Ϣ�����ļ�ʧ�ܣ�" << endl;
			exit(1);
		}
		istreambuf_iterator<char> beg(fi), end;// ���������ļ�ָ�룬ָ��ʼ�ͽ�������char(һ�ֽ�)Ϊ����
		string content(beg, end);               // ���ļ�ȫ������string�ַ���
		fi.close();
		fin.str(content);

		//��ʼ����ÿ�����ȵ����Ӧ�ڹ��������е�λ��
		posEveryLen.resize(4);
		posEveryLen[0].resize(16);
		posEveryLen[1].resize(16);
		posEveryLen[2].resize(16);
		posEveryLen[3].resize(16);

		//��ȡJPEG�ļ�ͷ����Ϣ
		if (!readHead()) {
			cerr << "��ȡ�ļ���ʽʧ�ܣ�" << endl;
			exit(1);
		};

		//�����ڴ�ռ�
		Y_data = new(nothrow) BYTE[H8 * W8];
		Cb_data = new(nothrow) BYTE[H8 * W8];
		Cr_data = new(nothrow) BYTE[H8 * W8];
		if (Y_data == nullptr || Cb_data == nullptr || Cr_data == nullptr) {
			cerr << "������Ϣ�������ڴ�ռ�ʧ�ܣ�" << endl;
			exit(1);
		}
		RGB_data = new(nothrow) BYTE[4 * H * W];
		if (RGB_data == nullptr) {
			cerr << "������Ϣ�������ڴ�ռ�ʧ�ܣ�" << endl;
			exit(1);
		}
		memset((char*)RGB_data, 255, 4 * H * W);
	}
	//��������
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
	Parameter:	flag����ʾY/Cb/Cr, row:ͼƬ�к�, col:ͼƬ�к�,
				data:����ɢ���ұ任�������
	Return:		��
	Description:��8*8�ֿ���з������ͷ���ɢ���ұ任
	*****************************************/
	void DctAndQuantization(int flag, int row, int col, int* zdata) {
		//������
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
		//����ɢ���ұ任
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
	Parameter:	flag����ʾY/Cb/Cr, data:����������
	Return:		��
	Description:��8*8�ֿ�ı�����н���
	*****************************************/
	void HuffmanDecode(int flag, int* zdata) {
		memset((char*)zdata, 0, 4 * 64);
		int pos;
		//DC����
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

		//AC����
		int EOB = 0, ZRL = 15 * 16;
		for (int i = 1; i < 64; i++) {
			pos = GetNextCode((flag + 1) / 2 + 2);
			if (pos == EOB) {
				break;
			}
			//pos��4λ��0�ĳ��ȣ���4λ��length
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
	Parameter:	��
	Return:		��
	Description:����JPEG��ʽͼƬ��ת��ΪRGB���ݲ���ʾ
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
		//תRGB
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
		cout << "�汾��" << (unsigned int)versionh << "." << (unsigned int)versionl << endl;
		cout << "�߶ȣ�" << H << "     " << "��ȣ�" << W << endl;
		imread.showPic(RGB_data, W, H);
		return true;
	}
};