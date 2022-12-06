#include "iodefine.h"
#include "typedefine.h"

#define	printf ((int (*)(const char *,...))0x00007c7c)
#define	SW4 (PD.DR.BIT.B16)
#define	SW6 (PD.DR.BIT.B18)
#define LED6 (PE.DR.BIT.B11)
#define LCD_RS (PA.DR.BIT.B22)
#define LCD_E (PA.DR.BIT.B23)
#define LCD_RW (PD.DR.BIT.B23)
#define LCD_DATA (PD.DR.BYTE.HH)

void main();
void forStopping();
void forWorking();
void wait_us(_UINT);
void LCD_inst(_SBYTE);
void LCD_data(_SBYTE);
void LCD_cursor(_UINT, _UINT);
void LCD_putch(_SBYTE);
void LCD_putstr(_SBYTE *);
void LCD_cls();
void LCD_init();

void main() {
	/* 変数宣言エリア */
	void (*process)() = forStopping;
	
	/* モジュールスタンバイ解除エリア */
	STB.CR4.BIT._AD0 = 0;
	STB.CR4.BIT._CMT = 0;
	
	/* SWとLEDのENABLEの設定 */
	PFC.PDIORH.BIT.B16 = 0;
	PFC.PDIORH.BIT.B18 = 0;
	PFC.PEIORL.BIT.B11 = 1;
	
	/* AD変換器設定エリア */
	AD0.ADCSR.BIT.ADM = 3;  // 2チャネルスキャンモードに設定
	AD0.ADCSR.BIT.ADCS = 1;  // 連続スキャンモードに設定
	AD0.ADCSR.BIT.CH = 1;  // スキャンするチャネルを設定
	AD0.ADCR.BIT.ADST = 1;  // スキャンの開始
	
	/* CMT1設定エリア */
	CMT1.CMCSR.BIT.CKS = 3;  // 分周比の設定
	CMT1.CMCOR = 19531 - 1;  // カウント回数の設定
	CMT.CMSTR.BIT.STR1 = 1;  // カウントの開始
	
	CMT0.CMCSR.BIT.CKS = 1;  // CMT0の分周比の設定
	
	LCD_init();  // LCDの初期化
	
	while (1) {
		/* スイッチによる処理の切り替え */
		if (SW4) process = forStopping;
		if (SW6) process = forWorking;
		
		process();  // 処理の実行
	}
}

/* 停止状態のときの処理 */
void forStopping() {
	LED6 = 0;  // LED6の点灯
}

/* 動作状態のときの処理 */
void forWorking() {
	/* 変数宣言エリア */
	_SBYTE	addr0_str[17] = {' ', ' ', 'A', 'D', 'D', 'R', '0', ' ', '=', ' ', '0', '0', '0', '0', ' ', ' ', '\0'};
	_SBYTE	addr1_str[17] = {' ', ' ', 'A', 'D', 'D', 'R', '1', ' ', '=', ' ', '0', '0', '0', '0', ' ', ' ', '\0'};
	
	LED6 = 1;  // LED6の消灯
	
	if (!AD0.ADCSR.BIT.ADF) return;  // スキャンの途中ならば処理をしない
	if (!CMT1.CMCSR.BIT.CMF) return;  // カウントの途中ならば処理をしない
	
	/* 各フラグを折る */
	AD0.ADCSR.BIT.ADF = 0;
	CMT1.CMCSR.BIT.CMF = 0;
	
	/* 表示する文字列の作成 */
	addr0_str[10] = '0' + ((AD0.ADDR0 >> 6) / 1000 % 10);
	addr0_str[11] = '0' + ((AD0.ADDR0 >> 6) / 100 % 10);
	addr0_str[12] = '0' + ((AD0.ADDR0 >> 6) / 10 % 10);
	addr0_str[13] = '0' + ((AD0.ADDR0 >> 6) % 10);
	addr1_str[10] = '0' + ((AD0.ADDR1 >> 6) / 1000 % 10);
	addr1_str[11] = '0' + ((AD0.ADDR1 >> 6) / 100 % 10);
	addr1_str[12] = '0' + ((AD0.ADDR1 >> 6) / 10 % 10);
	addr1_str[13] = '0' + ((AD0.ADDR1 >> 6) % 10);
	
	/* 文字列の表示 */
	LCD_cursor(0, 0);
	LCD_putstr(addr0_str);
	LCD_cursor(0, 1);
	LCD_putstr(addr1_str);
}

void wait_us(_UINT us) {
	_UINT val;

	val = us * 10 / 16;
	if (val >= 0xffff)
		val = 0xffff;

	CMT0.CMCOR = val;
	CMT0.CMCSR.BIT.CMF &= 0;
	CMT.CMSTR.BIT.STR0 = 1;
	while (!CMT0.CMCSR.BIT.CMF);
	CMT0.CMCSR.BIT.CMF = 0;
	CMT.CMSTR.BIT.STR0 = 0;
}

void LCD_inst(_SBYTE inst) {
	LCD_E = 0;
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_E = 1;
	LCD_DATA = inst;
	wait_us(1);
	LCD_E = 0;
	wait_us(40);
}

void LCD_data(_SBYTE data) {
	LCD_E = 0;
	LCD_RS = 1;
	LCD_RW = 0;
	LCD_E = 1;
	LCD_DATA = data;
	wait_us(1);
	LCD_E = 0;
	wait_us(40);
}

void LCD_cursor(_UINT x, _UINT y) {
	if (x > 15)
		x = 15;
	if (y > 1)
		y = 1;
	LCD_inst(0x80 | x | y << 6);
}

void LCD_putch(_SBYTE ch) {
	LCD_data(ch);
}

void LCD_putstr(_SBYTE *str) {
	_SBYTE ch;

	while (ch = *str++) 
		LCD_putch(ch);
}

void LCD_cls() {
	LCD_inst(0x01);
	wait_us(1640);
}

void LCD_init() {
	wait_us(45000);
	LCD_inst(0x30);
	wait_us(4100);
	LCD_inst(0x30);
	wait_us(100);
	LCD_inst(0x30);
	
	LCD_inst(0x38);
	LCD_inst(0x08);
	LCD_inst(0x01);
	wait_us(1640);
	LCD_inst(0x06);
	LCD_inst(0x0c);
}
