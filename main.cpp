#include"Decode.h"
int main(int argc, char* argv[]) {
    //提前计算好DCT中所需的cos函数值
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			DCTCOS[i][j] = cos((2 * (double)i + 1) * j * PI / 16.0);
		}
	}

    cout << "\n----------欢迎使用----------\n" << endl;
    if (argc < 3) {
        cerr << " 请输入正确的指令！" << endl;
        cout << "**************************************************" << endl;
        cout << "    指令集：" << endl;
        cout << "           compress   输入文件名 [采样因子1-4 [压缩力度9-0(10)]]" << endl;
        cout << "           read  输入文件名" << endl;
        cout << "**************************************************" << endl;
        return -1;
    }

    if (!strcmp(argv[1], "compress")) {
        cout << "  压 缩 中 ........." << endl;
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
            cout << "  压*缩*成*功！" << endl;
        }
        else {
            cout << "  压*缩*失*败！请查看错误提示。" << endl;
        }
    }
    else if (!strcmp(argv[1], "read")) {
        cout << "  解 码 中 ........." << endl;
        Decode dec(argv[2]);
        bool flag = dec.decode();
        if (flag) {
            cout << "  解*码*成*功！" << endl;
        }
        else {
            cout << "  解*码*失*败！请查看错误提示。" << endl;
        }
    }
    else {
        cerr << " 请输入正确的指令！" << endl;
        cout << "**************************************************" << endl;
        cout << "    指令集：" << endl;
        cout << "           compress   输入文件名 [采样因子1-4 [压缩力度9-0(10)]]" << endl;
        cout << "           read  输入文件名" << endl;
        cout << "**************************************************" << endl;
        return -1;
    }
	return 0;
}