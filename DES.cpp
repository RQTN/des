#include <stdlib.h>
#include <stdio.h>
#include "tables.h"


void char2bit(char* charInput, int* bitOutput) {
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			bitOutput[7 * (i + 1) - j + i] = (charInput[i] >> j) & 1;
		}
	}
}

void bit2char(int* bitInput, char* charOutput) {
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			charOutput[i] = charOutput[i] * 2 + bitInput[j + 8 * i];
		}
	}
}

void IP(int* input, int* output) {
	for (int i = 0; i < 64; ++i) {
		output[i] = input[IP_Table[i] - 1];
	}
}

void PC_1(int* bitKey, int* pc1Key) {
	for (int i = 0; i < 56; ++i) {
		pc1Key[i] = bitKey[PC1_Table[i] - 1];
	}
}

void LeftShift(int* input, int* output, int n) {
	for (int i = 0; i < 28; ++i) {
		output[i] = input[(i + n) % 28];
	}
}

void PC_2(int* cd, int* subKey) {
	for (int i = 0; i < 48; ++i) {
		subKey[i] = cd[PC2_Table[i] - 1];
	}
}

void getSubKeys(int* bitKey, int subKeys[16][48]) {
	int pc1Key[56] = {};
	int c[28] = {};
	int d[28] = {};
	int LeftShiftC[16][28] = {};
	int LeftShiftD[16][28] = {};
	int cd[16][56] = {};
	PC_1(bitKey, pc1Key);
	for (int i = 0; i < 28; ++i) {
		c[i] = pc1Key[i];
		d[i] = pc1Key[i + 28];
	}

	for (int i = 0; i < 16; ++i) {
		if (i == 0) {
			LeftShift(c, LeftShiftC[i], Left_Shift_Table[i]);
			LeftShift(d, LeftShiftD[i], Left_Shift_Table[i]);
		}
		else {
			LeftShift(LeftShiftC[i-1], LeftShiftC[i], Left_Shift_Table[i]);
			LeftShift(LeftShiftD[i-1], LeftShiftD[i], Left_Shift_Table[i]);
		}

	}

	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 28; ++j) {
			cd[i][j] = LeftShiftC[i][j];
			cd[i][j + 28] = LeftShiftD[i][j];
		}
	}

	for (int i = 0; i < 16; ++i) {
		PC_2(cd[i], subKeys[i]);
	}
}

void E(int* input, int* Einput) {
	for (int i = 0; i < 48; ++i) {
		Einput[i] = input[E_Table[i] - 1];
	}
}

void Xor(int* res, int* lhs, int* rhs, int len) {
	for (int i = 0; i < len; ++i) {
		res[i] = lhs[i] ^ rhs[i];
	}
}

void S(int *Einput, int* Sinput) {
	int BoxValues[8] = {};
	for (int i = 0, j = 0; i < 48; i = i + 6) {
		BoxValues[j] = S_Box[j]
			[(Einput[i] << 1) + (Einput[i + 5])]
		[(Einput[i + 1] << 3) + (Einput[i + 2] << 2) + (Einput[i + 3] << 1) + (Einput[i + 4])];
		j++;
	}

	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 4; ++j) {
			Sinput[3 * (i + 1) - j + i] = (BoxValues[i] >> j) & 1;
		}
	}
}

void P(int* input, int* output) {
	for (int i = 0; i < 32; ++i) {
		output[i] = input[P_Table[i] - 1];
	}
}

void F(int* input, int* output, int* subKey) {
	int Einput[48] = {};
	int Sinput[32] = {};
	int toBePushInS[48] = {};
	E(input, Einput);
	Xor(toBePushInS, Einput, subKey, 48);
	S(toBePushInS, Sinput);
	P(Sinput, output);
}

void IPR(int *input, int* output) {
	for (int i = 0; i < 64; ++i) {
		output[i] = input[IPR_Table[i] - 1];
	}
}

void DES_E(char* msg, char* key, int* output) {
	int ip[64] = {};
	int bitMsg[64] = {};
	int bitKey[64] = {};
	int subKeys[16][48] = {};
	char2bit(msg, bitMsg);
	IP(bitMsg, ip);
	char2bit(key, bitKey);
	getSubKeys(bitKey, subKeys);
	int l[17][32] = {};
	int r[17][32] = {};
	int R16L16[64] = {};

	for (int i = 0; i < 32; ++i) {
		l[0][i] = ip[i];
		r[0][i] = ip[i + 32];
	}

	for (int i = 1; i < 17; ++i) {
		for (int j = 0; j < 32; ++j) {
			l[i][j] = r[i - 1][j];
		}
		F(r[i - 1], r[i], subKeys[i - 1]);
		Xor(r[i], r[i], l[i - 1], 32);
	}
	
	for (int i = 0; i < 32; ++i) {
		R16L16[i] = r[16][i];
		R16L16[i + 32] = l[16][i];
	}

	IPR(R16L16, output);	
}

void DES_D(int* input, char* key, char* output) {
	int ip[64] = {};
	int bitKey[64] = {};
	int subKeys[16][48] = {};
	int bitMsg[64] = {};
	IP(input, ip);
	char2bit(key, bitKey);
	getSubKeys(bitKey, subKeys);

	int l[17][32] = {};
	int r[17][32] = {};
	int R16L16[64] = {};

	for (int i = 0; i < 32; ++i) {
		l[0][i] = ip[i];
		r[0][i] = ip[i + 32];
	}

	for (int i = 1; i < 17; ++i) {
		for (int j = 0; j < 32; ++j) {
			l[i][j] = r[i - 1][j];
		}
		F(r[i - 1], r[i], subKeys[16 - i]);
		Xor(r[i], r[i], l[i - 1], 32);
	}

	for (int i = 0; i < 32; ++i) {
		R16L16[i] = r[16][i];
		R16L16[i + 32] = l[16][i];
	}

	IPR(R16L16, bitMsg);
	bit2char(bitMsg, output);
}

int main() {
	char msg[9] = {};
	char key[9] = {};
	int EmsgInBit[64] = {};
	char EmsgInChar[9] = {};
	int DmsgInBit[64] = {};
	char DmsgInChar[9] = {};

	printf("Input the message:\n");
	scanf("%s", msg);

	printf("Input the key:\n");
	scanf("%s", key);

	printf("\nBegin Encryption!\n");
	DES_E(msg, key, EmsgInBit);
	bit2char(EmsgInBit, EmsgInChar);

	printf("The Encryption message(bit form):\n");
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			printf("%d", EmsgInBit[i * 8 + j]);
		}
		printf(" ");
	}
	printf("\n");
	printf("The Encryption message(char form):\n");
	printf("%s\n\n", EmsgInChar);

	printf("Begin Decryption!\n");
	DES_D(EmsgInBit, key, DmsgInChar);
	char2bit(DmsgInChar, DmsgInBit);
	printf("The Decryption message(bit form):\n");
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			printf("%d", DmsgInBit[i * 8 + j]);
		}
		printf(" ");
	}
	printf("\n");
	printf("The Decryption message(char form):\n");
	printf("%s\n", DmsgInChar);

}
