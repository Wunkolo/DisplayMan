#include "DisplayList.h"


DisplayList::DisplayList(void)
{
	SetTextureSize(Size_128,Size_128);//Default texture size
	dl_displayList_filled,dl_commands_filled,dl_attributes_filled = 0;
}

void DisplayList::SetTextureSize(TextureNds_Size width,TextureNds_Size height)
{
	textureWidth = width;
	textureHeight = height;
	dl_displayList_filled = 0;
	dl_commands_filled = 0;
	dl_attributes_filled = 0;
}

void DisplayList::PackCommands()
{
	u32 i;

	//padding
	while (dl_commands_filled < 4)
	{
		dl_commands_buffer[dl_commands_filled] = FIFO_NOP;
		dl_commands_filled++;
	}

	dl_displayList[dl_displayList_filled] = FIFO_COMMAND_PACK(dl_commands_buffer[0],dl_commands_buffer[1],dl_commands_buffer[2],dl_commands_buffer[3]);
	dl_displayList_filled++;

	for (i = 0; i < dl_attributes_filled; i++)
	{
		dl_displayList[dl_displayList_filled] = dl_attributes_buffer[i];
		dl_displayList_filled++;
	}
	dl_commands_filled = 0;
	dl_attributes_filled = 0;
	if (dl_displayList_filled > Temp_size)
	{
		printf("Display list overflow");
	}
}


//Begin Vertex Definitions
void DisplayList::BeginVertexList(GeoType geotype)
{
	dl_commands_buffer[dl_commands_filled] = FIFO_BEGIN;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = geotype;
	dl_attributes_filled++;
	if (dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}

//End Vertex List
void DisplayList::EndVertexList()
{
	dl_commands_buffer[dl_commands_filled] = FIFO_END;
	dl_commands_filled++;
	if (dl_commands_filled == 4)
	{
		PackCommands();
	}
}

//Begin DisplayList
u32* DisplayList::BeginDisplayList()
{
	dl_displayList_filled_start = dl_displayList_filled;
	dl_displayList_filled++;
	return (dl_displayList + dl_displayList_filled_start);
}

//End DisplayList
//returns display list size in bytes
unsigned int DisplayList::EndList()
{
	PackCommands();
	dl_displayList[dl_displayList_filled_start] = (dl_displayList_filled - dl_displayList_filled_start) - 1;
	return (unsigned int)dl_displayList_filled;
}

//Add Vertexv16 to display list
void DisplayList::Vertexv16(v16 x,v16 y ,v16 z)
{
	if (dl_attributes_filled > 2)
	{
		PackCommands();
	}
	dl_commands_buffer[dl_commands_filled] = FIFO_VERTEX16;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)((y << 16) | (x & 0xFFFF));
	dl_attributes_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)(z & 0xFFFF);
	dl_attributes_filled++;
	if (dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}

void DisplayList::Vertexv10(v10 x,v10 y,v10 z)
{
	if (dl_attributes_filled > 2)
	{
		PackCommands();
	}
	dl_commands_buffer[dl_commands_filled] = FIFO_VERTEX10;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32) ((z << 20) | (y << 10) | (x & 0xFFFF));
	dl_attributes_filled++;
	if (dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}



//Add Vertex to display list using floats converted into v16.
//Keep in mind v16 is from -7.0 to 7.0
void DisplayList::Vertexf_v16(float x,float y,float z)
{
	Vertexv16(floattov16(x),floattov16(y),floattov16(z));
}

//Add Vertex to display list using floats converted into v10.
//Keep in mind v16 is from -0.99 to 0.99
void DisplayList::Vertexf_v10(float x,float y,float z)
{
	Vertexv16(floattov10(x),floattov10(y),floattov10(z));
}

//Define texcoord using two t16 values
void DisplayList::TexCoordt16(t16 u,t16 v)
{
	u += 16;
	dl_commands_buffer[dl_commands_filled] = FIFO_TEX_COORD;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)(TEXTURE_PACK(u,v));
	dl_attributes_filled++;
	if(dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}

//Define textcoord using two floats (0.0 to 1.0)
void DisplayList::TexCoordf(float u,float v)
{
	//uvs for the nds have 0,0 at the top left corner so we will need
	//to move the uvs downwards
	int _width = textureWidth;
	int _height = textureHeight;
	TexCoordt16(floattot16(u*(8 << _width)),floattot16(v*(8<<_height)));
}

//Define Vertex Normal using a uint32
void DisplayList::Normaluint32(uint32 normal)
{
	dl_commands_buffer[dl_commands_filled] = FIFO_NORMAL;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32) normal;
	dl_attributes_filled++;
	if(dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}

//Define Vertex Normal using NORMALIZED float
void DisplayList::Normalf(float x,float y,float z)
{
	if(x >= 1 || x <= -1) x *= .95f;
	if(y >= 1 || y <= -1) y *= .95f;
	if(z >= 1 || z <= -1) z *= .95f;
	Normaluint32(NORMAL_PACK(floattov10(x), floattov10(y), floattov10(z)));
}

//Define Vertex Color using an RGB15
void DisplayList::ColorRgb15(rgb15 color)
{
	dl_commands_buffer[dl_commands_filled] = FIFO_COLOR;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)color;
	dl_attributes_filled++;
	if (dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}

//Define Vertex Color using Normalized float components
void DisplayList::Colorf(float r,float g,float b)
{
	ColorRgb15(RGB15(((uint8)(r*255))>>3, ((uint8)(g*255))>>3, ((uint8)(b*255))>>3));
}


//Scale mesh
void DisplayList::Scalef(float x,float y,float z)
{
	if (dl_attributes_filled > 2)
	{
		PackCommands();
	}
	dl_commands_buffer[dl_commands_filled] = FIFO_MTX_SCALE;
	dl_commands_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)floattof32(x);
	dl_attributes_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)floattof32(y);
	dl_attributes_filled++;
	dl_attributes_buffer[dl_attributes_filled] = (u32)floattof32(z);
	dl_attributes_filled++;
	if (dl_commands_filled == 4 || dl_attributes_filled == 4)
	{
		PackCommands();
	}
}


DisplayList::~DisplayList(void)
{
}
