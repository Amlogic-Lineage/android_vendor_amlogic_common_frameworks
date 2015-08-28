/** @file RGBPicture.h
 *  @par Copyright:
 *  - Copyright 2011 Amlogic Inc as unpublished work
 *  All Rights Reserved
 *  - The information contained herein is the confidential property
 *  of Amlogic.  The use, copying, transfer or disclosure of such information
 *  is prohibited except by express written agreement with Amlogic Inc.
 *  @author   Tellen Yu
 *  @version  1.0
 *  @date     2014/04/26
 *  @par function description:
 *  - 1 save rgb data to picture
 *  @warning This class may explode in your face.
 *  @note If you inherit anything from this class, you're doomed.
 */

#ifndef _RGB_PICTURE_H_
#define _RGB_PICTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned int width;
    unsigned int height;
#define FB_FORMAT_RGB565    0
#define FB_FORMAT_ARGB8888  1
    unsigned int format;
    char* data;
}rc_fb_t;

typedef struct rgb888 {
    char r;
    char g;
    char b;
} rc_rgb888_t;

typedef rc_rgb888_t rc_rgb24_t;

typedef struct rgb565 {
    short b:5;
    short g:6;
    short r:5;
} rc_rgb565_t;

#pragma pack(1) // ����1�ֽڷ�ʽ���ж���
typedef struct
{
    unsigned short  bf_type;/*λͼ�ļ������ͣ�����ΪBMP(0-1�ֽ�)*/
    unsigned long   bf_size;/*λͼ�ļ��Ĵ�С�����ֽ�Ϊ��λ(2-5�ֽ�)*/
    unsigned short  bf_reserved1;/*λͼ�ļ������֣�����Ϊ0(6-7�ֽ�)*/
    unsigned short  bf_reserved2; /*λͼ�ļ������֣�����Ϊ0(8-9�ֽ�)*/
    unsigned long   bf_offbits; /* λͼ���ݵ���ʼλ�ã��������λͼ(10-13�ֽ�)*/
}BmpFileHeader_t;

typedef struct
{
    unsigned long   bi_size; /* ���ṹ��ռ���ֽ���(14-17�ֽ�)*/
    unsigned long   bi_width;/*λͼ�Ŀ�ȣ�������Ϊ��λ(18-21�ֽ�)*/
    unsigned long   bi_height;/* λͼ�ĸ߶ȣ�������Ϊ��λ(22-25�ֽ�)*/

    unsigned short  bi_planes;/* Ŀ���豸�ļ��𣬱���Ϊ1(26-27�ֽ�)*/
    unsigned short  bi_bitcount;/* ÿ�����������λ����������1(˫ɫ),(28-29�ֽ�)
                                4(16ɫ)��8(256ɫ)��24(���ɫ)֮һ*/

    unsigned long   bi_compression; /*λͼѹ�����ͣ������� 0(��ѹ��),(30-33�ֽ�)
                                    1(BI_RLE8ѹ������)��2(BI_RLE4ѹ������)����3(BI_BITFIELDS)*/

    unsigned long   bi_sizeimage;/*λͼ�Ĵ�С�����ֽ�Ϊ��λ(34-37�ֽ�)*/
    unsigned long   bi_xpelspermeter;/* λͼˮƽ�ֱ��ʣ�ÿ��������(38-41�ֽ�)*/
    unsigned long   bi_ypelspermeter;/*λͼ��ֱ�ֱ��ʣ�ÿ��������(42-45�ֽ�)*/
    unsigned long   bi_clrused;/*λͼʵ��ʹ�õ���ɫ���е���ɫ��(46-49�ֽ�)*/
    unsigned long   bi_clrimportant;/* λͼ��ʾ��������Ҫ����ɫ��(50-53�ֽ�)*/
}BmpInfoHeader_t;

/*
��ɫ����rgb_color_table_t�ṹ���ݵĸ�����bi_bitcount��ȷ��:
��biBitCount=1,4,8ʱ���ֱ���2,16,256������;
��biBitCount=16ʱ��bi_compression = BI_BITFIELDSʱ����������:0xf800, 0x7e0, 0x1f
    PhotoRGBColorTable_t bmp_colors[3];

    bmp_colors[0].rgb_blue      =   0;
    bmp_colors[0].rgb_green     =   0xF8;
    bmp_colors[0].rgb_red       =   0;
    bmp_colors[0].rgb_reserved  =   0;
    bmp_colors[1].rgb_blue      =   0xE0;
    bmp_colors[1].rgb_green     =   0x07;
    bmp_colors[1].rgb_red       =   0;
    bmp_colors[1].rgb_reserved  =   0;
    bmp_colors[2].rgb_blue      =   0x1F;
    bmp_colors[2].rgb_green     =   0;
    bmp_colors[2].rgb_red       =   0;
    bmp_colors[2].rgb_reserved  =   0;
��biBitCount=24ʱ��û����ɫ���
*/
typedef struct//color table
{
    unsigned char rgb_blue;// ��ɫ������(ֵ��ΧΪ0-255)
    unsigned char rgb_green; // ��ɫ������(ֵ��ΧΪ0-255)
    unsigned char rgb_red; // ��ɫ������(ֵ��ΧΪ0-255)
    unsigned char rgb_reserved;// ����������Ϊ0
}BmpColorTable_t;
#pragma pack() // ȡ��1�ֽڶ��뷽ʽ

int RGBA2bmp(char *buf, int width, int height, char* filePath);

#ifdef __cplusplus
}
#endif

#endif/*_RGB_PICTURE_H_*/
