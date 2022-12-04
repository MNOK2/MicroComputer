#include "iodefine.h"
#include "typedefine.h"

#define	printf ((int (*)(const char *,...))0x00007c7c)
#define	SW6 (PD.DR.BIT.B18)
#define	SW4 (PD.DR.BIT.B16)
#define LCD_RS (PA.DR.BIT.B22)
#define LCD_E (PA.DR.BIT.B23)
#define LCD_RW (PD.DR.BIT.B23)
#define LCD_DATA (PD.DR.BYTE.HH)

void main();
void wait_us(_UINT);
void LCD_inst(_SBYTE);
void LCD_data(_SBYTE);
void LCD_cursor(_UINT, _UINT);
void LCD_putch(_SBYTE);
void LCD_putstr(_SBYTE *);
void LCD_cls();
void LCD_init();

void main() {
	/* �ϐ��錾�G���A */
	_SBYTE	addr0_str[17] = {' ', ' ', 'A', 'D', 'D', 'R', '0', ' ', '=', ' ', '0', '0', '0', '0', ' ', ' ', '\0'};
	_SBYTE	addr1_str[17] = {' ', ' ', 'A', 'D', 'D', 'R', '1', ' ', '=', ' ', '0', '0', '0', '0', ' ', ' ', '\0'};
	_UINT isStopping = 1;
	_UINT i;
	
	/* ���W���[���X�^���o�C�����G���A */
	STB.CR4.BIT._AD0 = 0;
	STB.CR4.BIT._CMT = 0;
	
	/* AD�ϊ���ݒ�G���A */
	AD0.ADCSR.BIT.ADM = 3;  // 2�`���l���X�L�������[�h�ɐݒ�
	AD0.ADCSR.BIT.ADCS = 1;  // �A���X�L�������[�h�ɐݒ�
	AD0.ADCSR.BIT.CH = 1;  // �X�L��������`���l����ݒ�
	AD0.ADCR.BIT.ADST = 1;  // �X�L�����̊J�n
	
	/* CMT1�ݒ�G���A */
	CMT1.CMCSR.BIT.CKS = 3;  // ������̐ݒ�
	CMT1.CMCOR = 19531 - 1;  // �J�E���g�񐔂̐ݒ�
	CMT.CMSTR.BIT.STR1 = 1;  // �J�E���g�̊J�n
	
	LCD_init();  // LCD�̏�����
	
	while (1) {
		/* �X�C�b�`�ɂ���~��ԂƓ����Ԃ�؂�ւ��� */
		if (SW6) isStopping = 0;
		if (SW4) isStopping = 1;
		
		/* ��Ԃɂ���ď�����؂�ւ��� */
		if (isStopping) continue;  // ��~��ԂȂ�Ώ��������Ȃ�
		if (!AD0.ADCSR.BIT.ADF) continue;  // �X�L�����̓r���Ȃ�Ώ��������Ȃ�
		if (!CMT1.CMCSR.BIT.CMF) continue;  // �J�E���g�̓r���Ȃ�Ώ��������Ȃ�
		
		/* �e�t���O��܂� */
		AD0.ADCSR.BIT.ADF = 0;
		CMT1.CMCSR.BIT.CMF = 0;
		
		/* �\�����镶����̍쐬 */
		addr0_str[10] = '0' + ((AD0.ADDR0 >> 6) / 1000 % 10);
		addr0_str[11] = '0' + ((AD0.ADDR0 >> 6) / 100 % 10);
		addr0_str[12] = '0' + ((AD0.ADDR0 >> 6) / 10 % 10);
		addr0_str[13] = '0' + ((AD0.ADDR0 >> 6) % 10);
		addr1_str[10] = '0' + ((AD0.ADDR1 >> 6) / 1000 % 10);
		addr1_str[11] = '0' + ((AD0.ADDR1 >> 6) / 100 % 10);
		addr1_str[12] = '0' + ((AD0.ADDR1 >> 6) / 10 % 10);
		addr1_str[13] = '0' + ((AD0.ADDR1 >> 6) % 10);
		
		/* ������̕\�� */
		LCD_cursor(0, 0);
		LCD_putstr(addr0_str);
		LCD_cursor(0, 1);
		LCD_putstr(addr1_str);
	}
}

void wait_us(_UINT us) {
	_UINT val;

	val = us * 10 / 16;
	if (val >= 0xffff)
		val = 0xffff;

	CMT0.CMCSR.BIT.CKS = 1;
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
