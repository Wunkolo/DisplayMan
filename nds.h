#ifndef _NDS_H_
#define _NDS_H_


#define BIT(n) (1 << (n))

//fixed point tools
#define floattov16(n)        ((v16)((n) * (1 << 12))) /*convert float to v16(used for 3d)*/
#define floattot16(n)        ((t16)((n) * (1 << 4))) /*convert float to t16(textures) */
#define floattov10(n)        ((n>.998) ? 0x1FF : ((v10)((n)*(1<<9)))) /*convert float to v10(Normals) */
#define floattof32(n)		 ((int32)((n) * (1 << 12)))/*Convert Float to f32(Matrices)*/

//data types
#define RGB15(r,g,b)  ((r)|((g)<<5)|((b)<<10))/*Convert color into RGB15.(components must be less than 32)*/
#define TEXTURE_PACK(u,v)    ((u & 0xFFFF) | ((v) << 16)) /*put two t16 texture coordinates into a 32 bit value*/
#define NORMAL_PACK(x,y,z)   (((x) & 0x3FF) | (((y) & 0x3FF) << 10) | ((z) << 20)) /*Pack 3 v10 Normals into a 32 bit value*/


enum TextureNds_Size
{
	Size_8 = 0,
	Size_16 = 1,
	Size_32 = 2,
	Size_64 = 3,
	Size_128 = 4,
	Size_256 = 5,
	Size_512 = 6,
	Size_1024 = 7
};

enum GeoType
{
	Triangles = 0,
	Quads = 1,
	TriangleStrips = 2,
	QuadStrip = 3,
	Triangle = 0,
	Quad = 1
};

typedef unsigned char u8;
typedef unsigned char uint8;
typedef unsigned short u16;
typedef unsigned short uint16;
typedef unsigned int u32;
typedef unsigned int uint32;
typedef unsigned short rgb15;
typedef short int t16;
typedef short int v16;
typedef short int v10;
typedef signed int int32;

typedef volatile unsigned char  vuint8;
typedef volatile unsigned short vuint16;
typedef volatile unsigned int   vuint32;
typedef volatile signed char  vint8;
typedef volatile signed short vint16;
typedef volatile signed int   vint32;



//Gfc raw commands
#define GFX_CONTROL           (*(vuint16*) 0x04000060)

#define GFX_FIFO              (*(vuint32*) 0x04000400)
#define GFX_STATUS            (*(vuint32*) 0x04000600)
#define GFX_COLOR             (*(vuint32*) 0x04000480)

#define GFX_VERTEX10          (*(vuint32*) 0x04000490)
#define GFX_VERTEX_XY         (*(vuint32*) 0x04000494)
#define GFX_VERTEX_XZ         (*(vuint32*) 0x04000498)
#define GFX_VERTEX_YZ         (*(vuint32*) 0x0400049C)
#define GFX_VERTEX_DIFF       (*(vuint32*) 0x040004A0)

#define GFX_MTX_SCALE		  (*(vuint32*) 0x0400046C)
#define GFX_MTX_IDENT		  (*(vuint32*) 0x04000454)


#define GFX_VERTEX16          (*(vuint32*) 0x0400048C)
#define GFX_VERTEX10          (*(vuint32*) 0x04000490)
#define GFX_TEX_COORD         (*(vuint32*) 0x04000488)
#define GFX_TEX_FORMAT        (*(vuint32*) 0x040004A8)
#define GFX_PAL_FORMAT        (*(vuint32*) 0x040004AC)

#define GFX_CLEAR_COLOR       (*(vuint32*) 0x04000350)
#define GFX_CLEAR_DEPTH       (*(vuint16*) 0x04000354)

#define GFX_LIGHT_VECTOR      (*(vuint32*) 0x040004C8)
#define GFX_LIGHT_COLOR       (*(vuint32*) 0x040004CC)
#define GFX_NORMAL            (*(vuint32*) 0x04000484)

#define GFX_DIFFUSE_AMBIENT   (*(vuint32*) 0x040004C0)
#define GFX_SPECULAR_EMISSION (*(vuint32*) 0x040004C4)
#define GFX_SHININESS         (*(vuint32*) 0x040004D0)

#define GFX_POLY_FORMAT       (*(vuint32*) 0x040004A4)
#define GFX_ALPHA_TEST        (*(vuint16*) 0x04000340)

#define GFX_BEGIN			(*(vuint32*) 0x04000500)
#define GFX_END				(*(vuint32*) 0x04000504)
#define GFX_FLUSH			(*(vuint32*) 0x04000540)
#define GFX_VIEWPORT		(*(vuint32*) 0x04000580)
#define GFX_TOON_TABLE		((vuint16*)  0x04000380)
#define GFX_EDGE_TABLE		((vuint16*)  0x04000330)
#define GFX_FOG_COLOR		(*(vuint32*) 0x04000358)
#define GFX_FOG_OFFSET		(*(vuint32*) 0x0400035C)
#define GFX_FOG_TABLE		((vuint8*)   0x04000360)
#define GFX_BOX_TEST		(*(vint32*)  0x040005C0)
#define GFX_POS_TEST		(*(vuint32*) 0x040005C4)
#define GFX_POS_RESULT		((vint32*)   0x04000620)

#define GFX_BUSY (GFX_STATUS & BIT(27))

#define GFX_VERTEX_RAM_USAGE	(*(uint16*)  0x04000606)
#define GFX_POLYGON_RAM_USAGE	(*(uint16*)  0x04000604)

#define GFX_CUTOFF_DEPTH		(*(uint16*)0x04000610)




//Gfx Fifo Commands

#define FIFO_COMMAND_PACK(c1,c2,c3,c4) (((c4) << 24) | ((c3) << 16) | ((c2) << 8) | (c1))/*Combines 4 GFXcommands into an int32 commands for packed command lists*/
#define REG2ID(r)				(u8)( ( ((u32)(&(r)))-0x04000400 ) >> 2 )/*Converts a GFX command for use in a packed command list.*/

#define FIFO_NOP				REG2ID(GFX_FIFO)/*Packed command for nothing*/
#define FIFO_STATUS				REG2ID(GFX_STATUS)/*Packed command for geometry engine status*/
#define FIFO_COLOR				REG2ID(GFX_COLOR)/*Packed command for vertex color*/
#define FIFO_VERTEX16			REG2ID(GFX_VERTEX16)/*Packed command for vertex with 3 16 bit params(and another 16 bits for padding) -7 to 7*/
#define FIFO_VERTEX10			REG2ID(GFX_VERTEX10)/*Packed command for vertex with 3 16 bit params(and another 16 bits for padding) 0.0 - 0.999*/
#define FIFO_TEX_COORD			REG2ID(GFX_TEX_COORD)/*Packed command for texture coordinates*/
#define FIFO_TEX_FORMAT			REG2ID(GFX_TEX_FORMAT)/*Packed command for texture format*/
#define FIFO_PAL_FORMAT			REG2ID(GFX_PAL_FORMAT)/*Packed command for texture palette attributes*/
#define FIFO_MTX_SCALE			REG2ID(GFX_MTX_SCALE)/*Command for setting the scale matrix*/
#define FIFO_CLEAR_COLOR		REG2ID(GFX_CLEAR_COLOR)/*Packed command for setting the clear color*/
#define FIFO_CLEAR_DEPTH		REG2ID(GFX_CLEAR_DEPTH)/*Packed command for setting the clear depth*/
#define FIFO_LIGHT_VECTOR		REG2ID(GFX_LIGHT_VECTOR)/*Packed command for direction of a light source*/
#define FIFO_LIGHT_COLOR		REG2ID(GFX_LIGHT_COLOR)/*Packed command for the color of a light*/
#define FIFO_NORMAL				REG2ID(GFX_NORMAL)/*Packed command for setting the normals of the following vertices*/
#define FIFO_DIFFUSE_AMBIENT	REG2ID(GFX_DIFFUSE_AMBIENT)/*Packed command for setting the diffuse and ambient color of the following vertices*/
#define FIFO_SPECULAR_EMISSION	REG2ID(GFX_SPECULAR_EMISSION)/*Packed command for setting the specular and emission color of the following vertices*/
#define FIFO_SHININESS			REG2ID(GFX_SHININESS)/*Packed command for setting the shininess table to be used for the following vertices*/
#define FIFO_POLY_FORMAT		REG2ID(GFX_POLY_FORMAT)/*Packed command for setting polygon attributes*/
#define FIFO_BEGIN				REG2ID(GFX_BEGIN)/*Packed command that starts a vertex list*/
#define FIFO_END				REG2ID(GFX_END)/*Packed command that has no discernible effect. The NDS hardware does not need this, computer emulation software may however.*/
#define FIFO_FLUSH				REG2ID(GFX_FLUSH)/*Packed command that waits for VBlank*/
#define FIFO_VIEWPORT			REG2ID(GFX_VIEWPORT)/*Packed command for setting viewport size*/




#endif