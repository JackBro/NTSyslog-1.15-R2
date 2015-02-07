/*-----------------------------------------------------------------------------
 *
 *  convert.h - �����R�[�h�ϊ����W���[�� �w�b�_�t�@�C��
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

/* �����R�[�h��ʂ̔���*/
extern int whatKanji(unsigned char *str);
/* ���p�J�i��S�p�J�i�ɕϊ����� */
extern unsigned int hankaku2zen(int hankaku);
/* ���p�J�i��S�p�J�i�ɕϊ�����i�����Ή��Łj*/
extern int han2zen(unsigned int *zenkaku, unsigned char *str);
/* �V�t�gJIS�R�[�h��JIS�R�[�h�ɕϊ����� */
extern unsigned int sjis2jis(unsigned int sjis);
/* �V�t�gJIS�R�[�h��EUC�R�[�h�ɕϊ����� */
extern unsigned int sjis2euc(unsigned int sjis);
/* JIS�R�[�h���V�t�gJIS�R�[�h�ɕϊ����� */
extern unsigned int jis2sjis(unsigned int jis);
/* JIS�R�[�h��EUC�R�[�h�ɕϊ����� */
extern unsigned int jis2euc(unsigned int jis);
/* EUC�R�[�h��JIS�R�[�h�ɕϊ����� */
extern unsigned int euc2jis(unsigned int euc);
/* ��_�R�[�h��JIS�R�[�h�ɕϊ����� */
extern unsigned int kuten2jis(unsigned int kuten);
/* JIS�R�[�h����_�R�[�h�ɕϊ����� */
extern unsigned int jis2kuten(unsigned int jis);
