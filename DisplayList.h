#pragma once
#include "nds.h"
#include <stdio.h>
#include <vector>

#define Temp_size (4*1024*1024)
class DisplayList
{
public:
	DisplayList(void);
	void SetTextureSize(TextureNds_Size width,TextureNds_Size height);
	void PackCommands();

	void BeginVertexList(GeoType geotype);
	void EndVertexList();

	u32* BeginDisplayList();
	unsigned int EndList();



	void Vertexv16(v16 x,v16 y ,v16 z);
	void Vertexf_v16(float x,float y,float z);
	void Vertexv10(v10 x,v10  y,v10 z);
	void Vertexf_v10(float x,float y,float z);

	void TexCoordt16(t16 u,t16 v);
	void TexCoordf(float u,float v);

	void Normaluint32(uint32 normal);
	void Normalf(float x,float y,float z);

	void ColorRgb15(rgb15 color);
	void Colorf(float r,float g,float b);

	void Scalef(float x,float y,float z);


	~DisplayList(void);
private:
	
	
	std::vector<u32> dl_vector;

	u32 dl_displayList[Temp_size];

	u32 dl_displayList_filled;
	u32 dl_displayList_filled_start;

	u8 dl_commands_buffer[4];
	u32 dl_commands_filled;

	u32 dl_attributes_buffer[8];
	u32 dl_attributes_filled;
	

	TextureNds_Size textureWidth;
	TextureNds_Size textureHeight;
};

