/*-----------------------------------------------------------------------------
 *
 *  convert.c - �����R�[�h�ϊ����W���[��
 *
 *    Copyright (c) 2003, Ryo.Sugahara / Algorithm Collection
 *                                        All rights reserved
 *
 *    �{���W���[����Web�y�[�W�uAlgorithm Collection�v�Ɍf�ڂ��ꂽ���̂𗬗p��
 *    �č\���������̂ł��B
 *
 *    Algorithm Collection
 *      http://alfin.mine.utsunomiya-u.ac.jp/~niy/algo/index.html
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307
 *
 *  Revision history:
 *    16-Apr-2003  ����
 *
 *----------------------------------------------------------------------------*/

/*
�����R�[�h��ʂ̔���

�֐���

whatKanji  �����R�[�h�̎�ʂ𔻒肷��

�`��

int whatKanji(unsigned char *str);

����

str  �i���́j������

�֐��l

�����̎�ʂ�\���l
�E����str�̐擪���������p�J�i�̏ꍇ�A�֐��l��1
�E����str�̐擪2�������V�t�gJIS�R�[�h�̏ꍇ�A�֐��l��2
�E����str�̐擪2������EUC���p�J�i�R�[�h�̏ꍇ�A�֐��l��4
�E����str�̐擪2������EUC�S�p�R�[�h�̏ꍇ�A�֐��l��8
�E����str�̐擪3�������VJIS(X0208-1983)�V�t�g�R�[�h�̏ꍇ�A
�@�֐��l��16
�E����str�̐擪3��������JIS(X0208-1978)�V�t�g�R�[�h�̏ꍇ�A
�@�֐��l��32
�E����str�̐擪3������JIS���[�}��(X0201)�V�t�g�R�[�h�̏ꍇ�A
�@�֐��l��64
�E����str�̐擪3������ASCII �V�t�g�R�[�h�̏ꍇ�A�֐��l��128
�E��̃P�[�X�����������ɋN����ꍇ�́A�֐��l�͂����̘_���a
�E��̂�����ł��Ȃ��ꍇ�́A�֐��l��0

���ӎ���

�p��iwhatKanji-test.c�j
    whatKanji("����");

����
    ���p�J�i�R�[�h�� 0xa0 - 0xdf �̊Ԃ��g���B

    �V�t�gJIS�R�[�h�͑�1�o�C�g�Ƃ��� 0x81 - 0x9f �܂ł� 0xe0 - 0xfc �܂ŁA��2�o�C�g�Ƃ��� 0x40 - 0xfc �܂Łi0x7f�������j���g���B���� ���߁A�V�t�gJIS�͏�L�̔��p�R�[�h�ƍ��݂ł���B�������A���䕶�� �Ƃ��Ďg���� �_�i�o�b�N�X���b�V���܂��͉~�L���j�Ȃǂ���2�o�C�g �̗̈�ɓ����Ă��邽�߂ɖ�肪�����邱�Ƃ�����B

    EUC�R�[�h�͑�1�A��2�o�C�g�Ƃ� 0xa1 - 0xfe �܂łł��邪�A���p�� �J�i�����ɂ��ẮA0x8e �𔼊p�J�i�����̑O�ɂ��A2�o�C�g�ŕ\�� ����B

    ����AJIS�R�[�h��7�r�b�g�n�����R�[�h�ł��邽�߁A���̂悤�ȃV�t�g �R�[�h���g���āA�����Z�b�g�̐؂�ւ����s���A���Ȋ�����\������B

    �@�@�@�V�t�g�R�[�h�ꗗ
    �����Z�b�g �V�t�g�R�[�h
    �VJIS X0208-1983 ESC $ B
    ��JIS X0208-1978 ESC $ @
    JIS���[�}�� X0201 ESC ( J
    ASCII ESC ( B
*/

int whatKanji(unsigned char *str)
{
    int val = 0;
    unsigned char b1, b2, b3;

    b1 = *str++;
    b2 = *str++;
    b3 = *str;
    if (b1 == 0x1b) {
        if (b2 == '$' && b3 == 'B') return 16;
        if (b2 == '$' && b3 == '@') return 32;
        if (b2 == '(' && b3 == 'J') return 64;
        if (b2 == '(' && b3 == 'B') return 128;
        return 0;
    }
    if ( b1 >= 0xa0 && b1 <= 0xdf) val |= 1;
    if ((b1 >= 0x81 && b1 <= 0x9f ||
         b1 >= 0xe0 && b1 <= 0xfc) && 
        (b2 >= 0x40 && b2 <= 0xfc && b2 != 0x7f)) val |= 2;
    if (b1 == 0x8e && (b2 >= 0xa0 && b2 <= 0xdf)) val |= 4; 
    if ((b1 >= 0xa1 && b1 <= 0xfe) &&
        (b2 >= 0xa1 && b1 <= 0xfe)) val |= 8;

    return val;
}


/*
���p�J�i��S�p�J�i�ɕϊ�����

�֐���

hankaku2zen  ���p�J�i��S�p�J�i�ɕϊ�����

�`��

unsigned int hankaku2zen(int hankaku);

����

hankaku  ���p�J�i�R�[�h�i0xA0-0xDF�j

�֐��l

�Ή�����S�p�J�iJIS�R�[�h�B�ϊ��ł��Ȃ��Ƃ���0�B

���ӎ���
    ���p�J�i�R�[�h�͈̔͂�0xA0����0xDF�B

�p��ihankaku2zen-test.c�j
    hankaku2zen(0xb1);
*/
unsigned int hankaku2zen(int hankaku)
{
    static unsigned int z[64] = {
        0x2121,0x2123,0x2156,0x2157,0x2122,0x2126,0x2572,0x2521,
        0x2523,0x2525,0x2527,0x2529,0x2563,0x2565,0x2567,0x2543,
        0x213c,0x2522,0x2524,0x2526,0x2528,0x252a,0x252b,0x252d,
        0x252f,0x2531,0x2533,0x2535,0x2537,0x2539,0x253b,0x253d,
        0x253f,0x2541,0x2544,0x2546,0x2548,0x254a,0x254b,0x254c,
        0x254d,0x254e,0x254f,0x2552,0x2555,0x2558,0x255b,0x255e,
        0x255f,0x2560,0x2561,0x2562,0x2564,0x2566,0x2568,0x2569,
        0x256a,0x256b,0x256c,0x256d,0x256f,0x2573,0x212b,0x212c };

    if (hankaku < 0xa0 || hankaku > 0xdf) return 0;
    return z[hankaku - 0xa0];
}

/*
���p�J�i��S�p�J�i�ɕϊ�����i�����Ή��Łj

�֐���

han2zen  ���p�J�i��S�p�J�i�ɕϊ�����i�����Ή��Łj

�`��

int han2zen(unsigned int *zenkaku, unsigned char *str);

����

zenkaku  �i�o�́j�S�p�J�iJIS�R�[�h
str      �i���́j���p�J�i�R�[�h�i0xA0-0xDF�j���܂߂�������

�֐��l

�ϊ����ʂɂ��Ă̕t�����
�E����str�̐擪���������p�J�i�łȂ��ꍇ�A�֐��l��0
�E����str�̐擪���������p�����J�i�ł���i�܂�擪��
�@2�����������E�������J�i�̑g�����łȂ��j�ꍇ�A�֐��l��1
�E����str�̐擪2�����������E�������J�i�̑g�����ł���ꍇ�A
�@�֐��l��2

���ӎ���
    �����E�������̏ꍇ��2�������̔��p�J�i��1�������̑S�p�J�i�� �ϊ������

�p��ihan2zen-test.c�j
    unsigned int zenkaku;
    han2zen(&zenkaku, "\xc0\xde");

����
    ���p�J�i�͎g��Ȃ����Ƃɂ��悤�B���̂��Ƃ���A�S�p�J�i�� ���p�J�i�ɖ߂��֐���p�ӂ������͂Ȃ��B�ǂ����Ă��K�v�ȕ��� ���삵�ĉ������B
*/
int han2zen(unsigned int *zenkaku, unsigned char *str)
{
    static unsigned int z[64] = {
        0x2121,0x2123,0x2156,0x2157,0x2122,0x2126,0x2572,0x2521,
        0x2523,0x2525,0x2527,0x2529,0x2563,0x2565,0x2567,0x2543,
        0x213c,0x2522,0x2524,0x2526,0x2528,0x252a,0x252b,0x252d,
        0x252f,0x2531,0x2533,0x2535,0x2537,0x2539,0x253b,0x253d,
        0x253f,0x2541,0x2544,0x2546,0x2548,0x254a,0x254b,0x254c,
        0x254d,0x254e,0x254f,0x2552,0x2555,0x2558,0x255b,0x255e,
        0x255f,0x2560,0x2561,0x2562,0x2564,0x2566,0x2568,0x2569,
        0x256a,0x256b,0x256c,0x256d,0x256f,0x2573,0x212b,0x212c};
    typedef struct {
        unsigned char han;
        unsigned int zen;
    } TBL;
    static TBL daku[] = {
        {0xb3,0x2574},{0xb6,0x252c},{0xb7,0x252e},{0xb8,0x2530},
        {0xb9,0x2532},{0xba,0x2534},{0xbb,0x2536},{0xbc,0x2538},
        {0xbd,0x253a},{0xbe,0x253c},{0xbf,0x253e},{0xc0,0x2540},
        {0xc1,0x2542},{0xc2,0x2545},{0xc3,0x2547},{0xc4,0x2549},
        {0xca,0x2550},{0xcb,0x2553},{0xcc,0x2556},{0xcd,0x2559},
        {0xce,0x255c},{0,0}};
    static TBL handaku[] = {
        {0xca,0x2551},{0xcb,0x2554},{0xcc,0x2557},{0xcd,0x255a},
        {0xce,0x255d},{0,0}};
    int i;

    if (*str < 0xa0 || *str > 0xdf) return 0;
    if (*(str+1) == 0xde) {           /* ������ */
        for (i = 0; daku[i].zen != 0; i++)
            if (*str == daku[i].han) {
                *zenkaku = daku[i].zen;
                return 2;
            }
    } else if (*(str+1) == 0xdf) {    /* �������� */
        for (i = 0; handaku[i].zen != 0; i++)
            if (*str == handaku[i].han) {
                *zenkaku = handaku[i].zen;
                return 2;
            }
    }
    
    *zenkaku = z[*str - 0xa0];
    return 1;
}

/*
�V�t�gJIS�R�[�h��JIS�R�[�h�ɕϊ�����

�֐���

sjis2jis  �V�t�gJIS�R�[�h��JIS�R�[�h�ɕϊ�����

�`��

unsigned int sjis2jis(unsigned int sjis);

����

sjis  �V�t�gJIS�R�[�h

�֐��l

JIS�R�[�h

���ӎ���

�p��isjis2jis-test.c�j
    sjis2jis(0x8abf);

����
    �V�t�gJIS�R�[�h�� Microsoft�����R�[�h�Ƃ���΂�A���݂� �Ƃ���ł��L���g�p����Ă���8�r�b�g�n�����R�[�h�ł���A�p�\�R�� ��̕W�������R�[�h�ł�����B

    ����AJIS�R�[�h��7�r�b�g�n�����R�[�h�ł���AJIS�K�i�ɂ���� ��ꂽ���{�����̕W�������R�[�h�i�̂͂��j�ł���B�Ƃ��ɁA���ȁE �������܂߂��C���^�[�l�b�g�E���[���ɂ͕��ʁAJIS�R�[�h���g��� �Ă���B
*/
unsigned int sjis2jis(unsigned int sjis)
{
    unsigned int hib, lob;
    
    hib = (sjis >> 8) & 0xff;
    lob = sjis & 0xff;
    hib -= (hib <= 0x9f) ? 0x71 : 0xb1;
    hib = (hib << 1) + 1;
    if (lob > 0x7f) lob--;
    if (lob >= 0x9e) {
        lob -= 0x7d;
        hib++;
    } else lob -= 0x1f;

    return (hib << 8) | lob;
}

/*
�V�t�gJIS�R�[�h��EUC�R�[�h�ɕϊ�����

�֐���

sjis2euc  �V�t�gJIS�R�[�h��EUC�R�[�h�ɕϊ�����

�`��

unsigned int sjis2euc(unsigned int sjis);

����

sjis  �V�t�gJIS�R�[�h

�֐��l

EUC�R�[�h

���ӎ���

�p��isjis2euc-test.c�j
    sjis2euc(0x8abf);

����
    �V�t�gJIS�R�[�h�� Microsoft�����R�[�h�Ƃ���΂�A���݂� �Ƃ���ł��L���g�p����Ă���8�r�b�g�n�����R�[�h�ł���A�p�\�R�� ��̕W�������R�[�h�ł�����B

    ����AEUC�R�[�h��UNIX��ōL���g���Ă���8�r�b�g�n�����R�[�h �ł���AJIS�R�[�h�̑�1�o�C�g�A��2�o�C�g�̗�����0x80�����Z���� ���̂��g�p����B
*/
unsigned int sjis2euc(unsigned int sjis)
{
    unsigned int hib, lob;
    
    hib = (sjis >> 8) & 0xff;
    lob = sjis & 0xff;
    hib -= (hib <= 0x9f) ? 0x71 : 0xb1;
    hib = (hib << 1) + 1;
    if (lob >= 0x9f) {
        lob -= 0x7e;
        hib++;
    } else if (lob > 0x7f) lob -= 0x20;
    else lob -= 0x1f;

    hib |= 0x80;
    lob |= 0x80;

    return (hib << 8) | lob;
}

/*
JIS�R�[�h���V�t�gJIS�R�[�h�ɕϊ�����

�֐���

jis2sjis  JIS�R�[�h���V�t�gJIS�R�[�h�ɕϊ�����

�`��

unsigned int jis2sjis(unsigned int jis);

����

jis  JIS�R�[�h

�֐��l

�V�t�gJIS�R�[�h

���ӎ���

�p��ijis2sjis-test.c�j
    jis2sjis(0x3441);

����
    JIS�R�[�h��7�r�b�g�n�����R�[�h�ł���AJIS�K�i�ɂ���� ��ꂽ���{�����̕W�������R�[�h�i�̂͂��j�ł���B�Ƃ��ɁA���ȁE �������܂߂��C���^�[�l�b�g�E���[���ɂ͕��ʁAJIS�R�[�h���g��� �Ă���B

    ����A�V�t�gJIS�R�[�h�� Microsoft�����R�[�h�Ƃ���΂�A���݂� �Ƃ���ł��L���g�p����Ă���8�r�b�g�n�����R�[�h�ł���A�p�\�R�� ��̕W�������R�[�h�ł�����B
*/
unsigned int jis2sjis(unsigned int jis)
{
    unsigned int hib, lob;
    
    hib = (jis >> 8) & 0xff;
    lob = jis & 0xff;
    lob += (hib & 1) ? 0x1f : 0x7d;
    if (lob >= 0x7f) lob++;
    hib = ((hib - 0x21) >> 1) + 0x81;
    if (hib > 0x9f) hib += 0x40;

    return (hib << 8) | lob;
}

/*
JIS�R�[�h��EUC�R�[�h�ɕϊ�����

�֐���

jis2euc  JIS�R�[�h��EUC�R�[�h�ɕϊ�����

�`��

unsigned int jis2euc(unsigned int jis);

����

jis  JIS�R�[�h

�֐��l

EUC�R�[�h

���ӎ���

�p��ijis2euc-test.c�j
    jis2euc(0x3441);

����
    JIS�R�[�h��7�r�b�g�n�����R�[�h�ł���AJIS�K�i�ɂ���� ��ꂽ���{�����̕W�������R�[�h�i�̂͂��j�ł���B�Ƃ��ɁA���ȁE �������܂߂��C���^�[�l�b�g�E���[���ɂ͕��ʁAJIS�R�[�h���g��� �Ă���B

    ����AEUC�R�[�h��UNIX��ōL���g���Ă���8�r�b�g�n�����R�[�h �ł���AJIS�R�[�h�̑�1�o�C�g�A��2�o�C�g�̗�����0x80�����Z���� ���̂��g�p����B
*/
unsigned int jis2euc(unsigned int jis)
{
    return jis | 0x8080;
}

/*
EUC�R�[�h��JIS�R�[�h�ɕϊ�����

�֐���

euc2jis  EUC�R�[�h��JIS�R�[�h�ɕϊ�����

�`��

unsigned int euc2jis(unsigned int euc);

����

euc  EUC�R�[�h

�֐��l

JIS�R�[�h

���ӎ���
    ���p�J�i�Ή���EUC�R�[�h��S�p�J�iJIS�R�[�h�ɕϊ����Ă���B ���̂��߂ɁA�֐� euc2jis-test.c�j euc2jis(0xb4c1);

����
    EUC�R�[�h��UNIX��ōL���g���Ă���8�r�b�g�n�����R�[�h �ł���AJIS�R�[�h�̑�1�o�C�g�A��2�o�C�g�̗�����0x80�����Z���� ���̂��g�p����B����ɁAANK�̔��p�J�i�����ɂ��ẮA�O��1�o�C�g (0x8e) �����A2�o�C�g�Ŕ��p�J�i������\���B

    ����AJIS�R�[�h��7�r�b�g�n�����R�[�h�ł���AJIS�K�i�ɂ���� ��ꂽ���{�����̕W�������R�[�h�i�̂͂��j�ł���B�Ƃ��ɁA���ȁE �������܂߂��C���^�[�l�b�g�E���[���ɂ͕��ʁAJIS�R�[�h���g��� �Ă���B
*/
unsigned int euc2jis(unsigned int euc)
{
    unsigned int jis;

    if ((euc & 0xff00) == 0x8e00)
        jis = hankaku2zen(euc & 0xff);
    else jis = euc & ~0x8080;
    return jis;
}

/*
��_�R�[�h��JIS�R�[�h�ɕϊ�����

�֐���

kuten2jis  JIS�R�[�h����_�R�[�h�ɕϊ�����

�`��

unsigned int kuten2jis(unsigned int kuten);

����

kuten  ��_�R�[�h

�֐��l

JIS�R�[�h

���ӎ���

�p��ikuten2jis-test.c�j
    kuten2jis(0x3441);

����
    ��_�R�[�h�͐̂̃~�j�R�����ɗ��p����銿���R�[�h�ŁA�R�[�h ���犿������͂���Ƃ��Ɏg����ꍇ������B

    ����AJIS�R�[�h��7�r�b�g�n�����R�[�h�ł���AJIS�K�i�ɂ���� ��ꂽ���{�����̕W�������R�[�h�i�̂͂��j�ł���B�Ƃ��ɁA���ȁE �������܂߂��C���^�[�l�b�g�E���[���ɂ͕��ʁAJIS�R�[�h���g��� �Ă���B
*/
unsigned int kuten2jis(unsigned int kuten)
{
    unsigned int hib, lob;
    
    hib = kuten / 100 + 32;
    lob = kuten % 100 + 32;

    return (hib << 8) | lob;
}

/*
JIS�R�[�h����_�R�[�h�ɕϊ�����

�֐���

jis2kuten  JIS�R�[�h����_�R�[�h�ɕϊ�����

�`��

unsigned int jis2kuten(unsigned int jis);

����

jis  JIS�R�[�h

�֐��l

��_�R�[�h

���ӎ���

�p��ijis2kuten-test.c�j
    jis2kuten(0x3441);

����
    JIS�R�[�h��7�r�b�g�n�����R�[�h�ł���AJIS�K�i�ɂ���� ��ꂽ���{�����̕W�������R�[�h�i�̂͂��j�ł���B�Ƃ��ɁA���ȁE �������܂߂��C���^�[�l�b�g�E���[���ɂ͕��ʁAJIS�R�[�h���g��� �Ă���B

    ����A��_�R�[�h�͐̂̃~�j�R�����ɗ��p����銿���R�[�h�ŁA�R�[�h ���犿������͂���Ƃ��Ɏg����ꍇ������B
*/
unsigned int jis2kuten(unsigned int jis)
{
    unsigned int hib, lob;
    
    hib = (jis >> 8) & 0xff;
    lob = jis & 0xff;

    return (hib-32) * 100 + (lob-32);
}
