/* ---------------------------------------------------------------------------
 * Truevision Targa Reader/Writer
 * $Id: targa.h,v 1.8 2004/10/09 09:30:26 emikulic Exp $
 *
 * Copyright (C) 2001-2003 Emil Mikulic.
 *
 * Source and binary redistribution of this code, with or without
 * changes, for free or for profit, is allowed as long as this copyright
 * notice is kept intact.  Modified versions have to be clearly marked
 * as modified.
 *
 * This code is provided without any warranty.  The copyright holder is
 * not liable for anything bad that might happen as a result of the
 * code.
 * -------------------------------------------------------------------------*/

/*
 * Modified by Stanislav Podgorskiy
 * stanislav@podgorskiy.com
 */

#pragma once

#include <fstream>

#ifndef _MSC_VER
# include <inttypes.h>
#else /* MSVC */
 typedef unsigned __int8  uint8_t;
 typedef unsigned __int16 uint16_t;
 typedef unsigned __int32 uint32_t;
#endif

template< uint32_t index >
class BIT
{
public:
    enum
    {
        RESULT = 1 << index
    };
};

class ImageDescriptorBits
{
public:
    /* bits 0,1,2,3 - attribute bits per pixel
     * bit  4       - set if image is stored right-to-left
     * bit  5       - set if image is stored top-to-bottom
     * bits 6,7     - unused (must be set to zero)
     */
    enum
    {
        TGA_ATTRIB_BITS = BIT<0>::RESULT | BIT<1>::RESULT | BIT<2>::RESULT | BIT<3>::RESULT,
        TGA_R_TO_L_BIT = BIT<4>::RESULT,
        TGA_T_TO_B_BIT = BIT<5>::RESULT,
        TGA_UNUSED_BITS = BIT<6>::RESULT | BIT<7>::RESULT
    /* Note: right-to-left order is not honored by some Targa readers */
    };
};

/* Targa image and header fields -------------------------------------------*/
class tga_image
{
public:
    /* color map = palette */
    enum ColorMap
    {
        TGA_COLOR_MAP_ABSENT = 0,
        TGA_COLOR_MAP_PRESENT = 1
    };

    enum ImageType
    {
        TGA_IMAGE_TYPE_NONE         = 0, /* no image data */
        TGA_IMAGE_TYPE_COLORMAP     = 1, /* uncompressed, color-mapped */
        TGA_IMAGE_TYPE_BGR          = 2, /* uncompressed, true-color */
        TGA_IMAGE_TYPE_MONO         = 3, /* uncompressed, black and white */
        TGA_IMAGE_TYPE_COLORMAP_RLE = 9, /* run-length, color-mapped */
        TGA_IMAGE_TYPE_BGR_RLE     = 10, /* run-length, true-color */
        TGA_IMAGE_TYPE_MONO_RLE    = 11 /* run-length, black and white */
    };

    /* Note that Targa is stored in little-endian order */
    uint8_t     image_id_length;
    uint8_t     color_map_type;
    uint8_t     image_type;

    /* color map specification */
    uint16_t    color_map_origin;   /* index of first entry */
    uint16_t    color_map_length;   /* number of entries included */
    uint8_t     color_map_depth;    /* number of bits per entry */

    /* image specification */
    uint16_t    origin_x;
    uint16_t    origin_y;
    uint16_t    width;
    uint16_t    height;
    uint8_t     pixel_depth;
    uint8_t     image_descriptor;

    uint8_t *image_id;
    /* The length of this field is given in image_id_length, it's read raw
     * from the file so it's not not guaranteed to be zero-terminated.  If
     * it's not NULL, it needs to be deallocated.  see: tga_free_buffers()
     */

    uint8_t *color_map_data;
    /* See the "color map specification" fields above.  If not NULL, this
     * field needs to be deallocated.  see: tga_free_buffers()
     */

    uint8_t *image_data;
    /* Follows image specification fields (see above) */

    /* Extension area and developer area are silently ignored.  The Targa 2.0
     * spec says we're not required to read or write them.
     */

    /* For decoding header bits ------------------------------------------------*/
    uint8_t tga_get_attribute_bits(const tga_image *tga);
    int tga_is_right_to_left(const tga_image *tga);
    int tga_is_top_to_bottom(const tga_image *tga);
    int tga_is_colormapped(const tga_image *tga);
    int tga_is_rle(const tga_image *tga);
    int tga_is_mono(const tga_image *tga);


    /* Error handling ----------------------------------------------------------*/
    enum tga_result
    {
        TGA_NOERR,
        TGAERR_FOPEN,
        TGAERR_EOF,
        TGAERR_WRITE,
        TGAERR_CMAP_TYPE,
        TGAERR_IMG_TYPE,
        TGAERR_NO_IMG,
        TGAERR_CMAP_MISSING,
        TGAERR_CMAP_PRESENT,
        TGAERR_CMAP_LENGTH,
        TGAERR_CMAP_DEPTH,
        TGAERR_ZERO_SIZE,
        TGAERR_PIXEL_DEPTH,
        TGAERR_NO_MEM,
        TGAERR_NOT_CMAP,
        TGAERR_RLE,
        TGAERR_INDEX_RANGE,
        TGAERR_MONO
    };

    const char *tga_error(const tga_result errcode);

    /* Load/save ---------------------------------------------------------------*/
    tga_result tga_read(tga_image *dest, std::istream& inputStream);
    tga_result tga_write(std::ostream& outputStream, const tga_image *src);

    tga_result tga_write_rgb(std::ostream& outputStream, uint8_t *image,
        const uint16_t width, const uint16_t height, const uint8_t depth);

    tga_result tga_write_rgb_rle(std::ostream& outputStream, uint8_t *image,
        const uint16_t width, const uint16_t height, const uint8_t depth);

    /* Manipulation ------------------------------------------------------------*/
    tga_result tga_flip_horiz(tga_image *img);
    tga_result tga_flip_vert(tga_image *img);
    tga_result tga_color_unmap(tga_image *img);

    uint8_t *tga_find_pixel(const tga_image *img, uint16_t x, uint16_t y);
    tga_result tga_unpack_pixel(const uint8_t *src, const uint8_t bits,
        uint8_t *b, uint8_t *g, uint8_t *r, uint8_t *a);
    tga_result tga_pack_pixel(uint8_t *dest, const uint8_t bits,
        const uint8_t b, const uint8_t g, const uint8_t r, const uint8_t a);

    tga_result tga_desaturate(tga_image *img,
            const int cr, const int cg, const int cb, const int dv);
    tga_result tga_desaturate_rec_601_1(tga_image *img);
    tga_result tga_desaturate_rec_709(tga_image *img);
    tga_result tga_desaturate_itu(tga_image *img);
    tga_result tga_desaturate_avg(tga_image *img);
    tga_result tga_convert_depth(tga_image *img, const uint8_t bits);
    tga_result tga_swap_red_blue(tga_image *img);

};



