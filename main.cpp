#include"Decode.h"
int main(int argc, char* argv[]) {
    //��ǰ�����DCT�������cos����ֵ
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			DCTCOS[i][j] = cos((2 * (double)i + 1) * j * PI / 16.0);
		}
	}

    cout << "\n----------��ӭʹ��----------\n" << endl;
    if (argc < 3) {
        cerr << " ��������ȷ��ָ�" << endl;
        cout << "**************************************************" << endl;
        cout << "    ָ���" << endl;
        cout << "           compress   �����ļ��� [��������1-4 [ѹ������9-0(10)]]" << endl;
        cout << "           read  �����ļ���" << endl;
        cout << "**************************************************" << endl;
        return -1;
    }

    if (!strcmp(argv[1], "compress")) {
        cout << "  ѹ �� �� ........." << endl;
        bool flag = false;
        if (argc == 3) {
            Compress com(argv[2]);
            flag = com.compress();
        }
        else if (argc == 4) {
            Compress com(argv[2], argv[3][0] - '0');
            flag = com.compress();
        }
        else if (argc == 5) {
            Compress com(argv[2], argv[3][0] - '0', (10 + argv[4][0] - '0') % 11);
            flag = com.compress();
        }
        if (flag) {
            cout << "  ѹ*��*��*����" << endl;
        }
        else {
            cout << "  ѹ*��*ʧ*�ܣ���鿴������ʾ��" << endl;
        }
    }
    else if (!strcmp(argv[1], "read")) {
        cout << "  �� �� �� ........." << endl;
        Decode dec(argv[2]);
        bool flag = dec.decode();
        if (flag) {
            cout << "  ��*��*��*����" << endl;
        }
        else {
            cout << "  ��*��*ʧ*�ܣ���鿴������ʾ��" << endl;
        }
    }
    else {
        cerr << " ��������ȷ��ָ�" << endl;
        cout << "**************************************************" << endl;
        cout << "    ָ���" << endl;
        cout << "           compress   �����ļ��� [��������1-4 [ѹ������9-0(10)]]" << endl;
        cout << "           read  �����ļ���" << endl;
        cout << "**************************************************" << endl;
        return -1;
    }
	return 0;
}