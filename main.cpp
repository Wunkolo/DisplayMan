
#include <stdlib.h>
#include <fbxsdk.h>
#include <iostream>
#include <stdio.h>
#include <fbxfilesdk/kfbxio/kfbxiosettings.h>
#include <fbxfilesdk/kfbxplugins/kfbxgeometryconverter.h>
#include <fbxfilesdk/kfbxplugins/kfbxnode.h>
#include <string>
#include <vector>
#include <fstream>

#include "DisplayList.h"


using namespace std;

struct options
{
	TextureNds_Size width, height;
	bool normals;
	bool texture;
	bool normalize;
	bool texinfo;
	bool colors;
} option;

KFbxSdkManager * lsdkManager = 0;
bool WriteList(KFbxNode * node , options _options);
void ScanNodes(KFbxNode * rootNode,int indent = 0);
unsigned int upperPow2(unsigned x);
void PrintHelp();
void PrintAbout();



int main(int argc, char** argv)
{

	lsdkManager = KFbxSdkManager::Create();

	KFbxIOSettings * ios = KFbxIOSettings::Create(lsdkManager,IOSROOT);

	lsdkManager->SetIOSettings(ios);

	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL,        true);
	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE,         true);
	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK,            false);
	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE,           false);
	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO,            false);
	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION,       false);
	(*(lsdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, false);


	KFbxImporter * Importer = KFbxImporter::Create(lsdkManager,"");


	if (argc < 1)
	{
		printf("File Name Required.\nUse -h for help.");
		return 0;
	}

	//default settings.
	option.height = Size_128;
	option.width = Size_128;
	option.normalize = false;
	option.normals = false;
	option.texture = false;
	option.colors = false;


	if (argc > 1 && !strcmp(argv[1],"-h"))
	{
		PrintHelp();
		return 0;
	}
	if (argc > 1 && !strcmp(argv[1],"-about"))
	{
		PrintAbout();
		return 0;
	}


	//Parse Arguments
	for (int i = 2 ; i < argc ; i++)
	{
		if (!strcmp(argv[i],"-n"))
		{
			//Include Normals
			option.normals = true;
		} 
		else if (!strcmp(argv[i],"-normalize"))
		{
			//normalize coordinates between -7 and 7
			option.normalize = true;

		}
		else if (!strcmp(argv[i],"-c"))
		{
			//Export with colors.
			option.colors = true;

		}
		else if (!strcmp(argv[i],"-ti"))
		{
			//normalize coordinates between -7 and 7
			option.texinfo = true;

		}
		else if (!strcmp(argv[i],"-t"))
		{
			option.texture = true;
			unsigned int width,height;
			sscanf(argv[i+1], "%ix%i", &width, &height);
			width = upperPow2(width);
			height = upperPow2(height);


			//convert from pixel size to relative texel size
			int j;
			for (j=0; j<8; j++) 
			{
				if (width  == (8 * pow((float)2,j)))
				{
					option.width = (TextureNds_Size)j;
					break;
				}
			}
			if (j >= 8)
			{
				printf("Invalid Texture Size\n");
				return 0;
			}
			for (j=0; j<8; j++) 
			{
				if (height == (8 * pow((float)2,j)))
				{
					option.height = (TextureNds_Size)j;
					break;
				}
			}
			if (j >= 8)
			{
				printf("Invalid Texture Size\n");
				return 0;
			}
			i += 1;
		};
	}








	if ( Importer->Initialize(argv[1],-1,lsdkManager->GetIOSettings()) == true )
	{
		KFbxScene * lScene = KFbxScene::Create(lsdkManager,"Scene");
		Importer->Import(lScene);
		Importer->Destroy();

		//scene is now loaded into lScene.
		KFbxNode * rootNode = lScene->GetRootNode();


		//What are we opening again?
		printf("%s\n",argv[1]);


		if (rootNode->GetChildCount() != 0)
		{
			ScanNodes(rootNode);
		}
	}
	else
	{
		printf("Error Importing file\nUse -h for help");
		lsdkManager->Destroy();
		return 0;
	}




	lsdkManager->Destroy();
	return 0;
}




bool WriteList(KFbxNode * node , options option)
{
	KFbxGeometryConverter geoconvert(lsdkManager);




	DisplayList displist;



	//Move mesh to 0,0,0
	node->GeometricTranslation.Set(fbxDouble3(0,0,0));


	//triangulate mesh
	if (!geoconvert.TriangulateInPlace(node))
	{
		cout << "Unable to triangulate mesh" << endl;
		return false;
	}

	KFbxMesh * mesh = node->GetMesh();
	const unsigned int vertCount = mesh->GetControlPointsCount();

	//vertex array
	KFbxVector4* VertexArray = new KFbxVector4[vertCount];
	//copy mesh data into Vertex array.
	memcpy(VertexArray,mesh->GetControlPoints(),vertCount * sizeof(KFbxVector4));
	
	//////////////////////////////////////////////////////////////////////////
	//get uv info
	const KFbxGeometryElementUV* UVElement = mesh->GetElementUV(0);
	bool UV_UseIndex;
	int UV_indexCount;
	if (option.texture == true && UVElement != NULL)
	{
		displist.SetTextureSize(option.width,option.height);
		 
		 if (UVElement->GetMappingMode() != KFbxGeometryElementUV::eBY_POLYGON_VERTEX &&
			 UVElement->GetMappingMode() != KFbxGeometryElementUV::eBY_CONTROL_POINT)
		 {
			 printf("Unable to extract UVs. UVs disabled.\n");
			 option.texture = false;
		 }
		 else
		 {
			 UV_UseIndex = UVElement->GetReferenceMode() != KFbxGeometryElementUV::eDIRECT;
			 UV_indexCount = (UV_UseIndex) ? UVElement->GetIndexArray().GetCount() : 0;
		 }
	}
	if (option.texture == true && UVElement == NULL)
	{
		printf("Unable to extract UVs. UVs disabled.\n");
		option.texture = false;
	}


	//////////////////////////////////////////////////////////////////////////
	//Get normals.
	const KFbxLayerElementNormal* NormalsElement = mesh->GetLayer(0)->GetNormals();
	bool Normals_UseIndex;
	int Normal_IndexCount;
	if (option.normals == true && NormalsElement != NULL)
	{
		
		if (NormalsElement->GetMappingMode() != KFbxLayerElement::EMappingMode::eBY_POLYGON_VERTEX &&
			NormalsElement->GetMappingMode() != KFbxLayerElement::EMappingMode::eBY_CONTROL_POINT)
		{
			printf("Unable to extract Normals. Normals Disabled.\n");
			option.normals = false;
		}
		else
		{
			Normals_UseIndex = NormalsElement->GetReferenceMode() != KFbxLayerElementNormal::eDIRECT;
			Normal_IndexCount = (Normals_UseIndex) ? NormalsElement->GetIndexArray().GetCount() : 0;
		}
	}
	if (option.normals == true && NormalsElement == NULL)
	{
		printf("Unable to extract Normals. Normals Disabled.\n");
		option.normals = false;
	}

	//////////////////////////////////////////////////////////////////////////
	//Get Vertex Colors
	bool Colors_UseIndex;
	int Color_IndexCount;

	const KFbxLayerElementVertexColor* ColorsElement = mesh->GetLayer(0)->GetVertexColors();
	if (option.colors == true && ColorsElement != NULL)
	{
		if (ColorsElement->GetMappingMode() != KFbxLayerElement::EMappingMode::eBY_POLYGON_VERTEX &&
			ColorsElement->GetMappingMode() != KFbxLayerElement::EMappingMode::eBY_CONTROL_POINT)
		{
			printf(" Unable to load Vertex Colors. Colors Disabled.\n");
			option.colors = false;
		}
		else
		{
			Colors_UseIndex = ColorsElement->GetReferenceMode() != KFbxLayerElementVertexColor::eDIRECT;
			Color_IndexCount = (Colors_UseIndex) ? ColorsElement->GetIndexArray().GetCount() : 0;
		}
	}
	if (ColorsElement == NULL && option.colors == true)
	{
		printf("Unable to load Vertex Colors. Colors Disabled.\n");
		option.colors = false;
	}


	//////////////////////////////////////////////////////////////////////////
	//calculate shrink factor
	float scale_factor = 7.0f;
	for (unsigned int i = 0; i < vertCount; i++)
	{
		KFbxVector4 v = VertexArray[i];

		if (abs(v[0]) > scale_factor)
		{
			scale_factor = (float)abs(v[0]);
		}
		if (abs(v[1]) > scale_factor)
		{
			scale_factor = (float)abs(v[1]);
		}
		if (abs(v[2]) > scale_factor)
		{
			scale_factor = (float)abs(v[2]);
		}

	}

	scale_factor /= 7.0f;




	const int Polycount = mesh->GetPolygonCount();//its actually tri-count at this point.
	


	//////////////////////////////////////////////////////////////////////////
	//Display list.
	u32* displaylist = displist.BeginDisplayList();

	displist.BeginVertexList(Triangles);

	if (!option.normalize)
	{
		displist.Scalef(scale_factor,scale_factor,scale_factor);
	}


	//"When texture mapping, the Geometry Engine works faster if you issue commands in the order:
	//TexCoord -> Normal -> Vertex


	int polyindexcounter = 0;

	//go through each polygon.
	for (int polyindex = 0; polyindex < Polycount; polyindex++)
	{


		//scroll through the polygon's vertices
		const int polyvertcount = mesh->GetPolygonSize(polyindex);//polygon vertices
		if(polyvertcount != -1)
		{
			//if the polygon is valid, go through each of its vertices
			for (int vertindex = 0; vertindex < polyvertcount ; vertindex++)
			{
				int polyvertindex = mesh->GetPolygonVertex(polyindex,vertindex);
				
				if (polyvertindex == -1)
				{
					continue;
				}

				//////////////////////////////////////////////////////////////////////////
				//Get UV
				KFbxVector2 curUV;
				int uvvertindex;

				if (option.texture == true && UVElement != NULL)
				{
					if (UVElement->GetMappingMode() == KFbxGeometryElementUV::eBY_POLYGON_VERTEX)
					{
						uvvertindex = UV_UseIndex ? UVElement->GetIndexArray().GetAt(polyindexcounter) : polyindexcounter;
						curUV = UVElement->GetDirectArray().GetAt(uvvertindex);
					}
					else if (UVElement->GetMappingMode() == KFbxGeometryElementUV::eBY_CONTROL_POINT)
					{
						uvvertindex = UV_UseIndex ? UVElement->GetIndexArray().GetAt(polyvertindex) : polyvertindex;
						curUV = UVElement->GetDirectArray().GetAt(uvvertindex);
					}
				}

				//////////////////////////////////////////////////////////////////////////
				//Get Normal
				KFbxVector4 lastNormal,curNormal;
				int normalvertindex;
				//curNormal.Normalize();
				if (option.normals == true && NormalsElement != NULL)
				{
					if (NormalsElement->GetMappingMode() == KFbxLayerElement::EMappingMode::eBY_POLYGON_VERTEX)
					{
						normalvertindex = Normals_UseIndex ? NormalsElement->GetIndexArray().GetAt(polyindexcounter) : polyindexcounter;
						curNormal = NormalsElement->GetDirectArray().GetAt(normalvertindex);
					}
					else if (NormalsElement->GetMappingMode() == KFbxLayerElement::EMappingMode::eBY_CONTROL_POINT)
					{
						normalvertindex = Normals_UseIndex ? NormalsElement->GetIndexArray().GetAt(polyvertindex) : polyvertindex;
						curNormal = NormalsElement->GetDirectArray().GetAt(normalvertindex);
					}
				}

				//////////////////////////////////////////////////////////////////////////
				//Get Vertex Color
				KFbxColor lastColor, curColor;
				int colorvertindex;
				//mesh->GetPolygonVertex(polyindex,polyvertindex);
				if (option.colors && ColorsElement != NULL)
				{
					if (ColorsElement->GetMappingMode() == KFbxLayerElement::EMappingMode::eBY_POLYGON_VERTEX)
					{
						colorvertindex = Colors_UseIndex ? ColorsElement->GetIndexArray().GetAt(polyindexcounter) : polyindexcounter;
						curColor = ColorsElement->GetDirectArray().GetAt(colorvertindex);
					}
					else if (ColorsElement->GetMappingMode() == KFbxLayerElement::EMappingMode::eBY_CONTROL_POINT)
					{
						colorvertindex = Colors_UseIndex ? ColorsElement->GetIndexArray().GetAt(polyvertindex) : polyvertindex;
						curColor = ColorsElement->GetDirectArray().GetAt(colorvertindex);
					}
				}






				polyindexcounter++;




				//Get vert
				KFbxVector4 curVert = VertexArray[polyvertindex];
				curVert /= scale_factor;
				curVert -= node->GetGeometricTranslation(node->eSOURCE_SET);





				float x,y,z;
				float nx,ny,nz;
				float r,g,b;
				float u,v;




				//Write texture coords to the display list
				if (option.texture == true && UVElement != NULL)
				{
					if (curUV[0] > 1.0f || curUV[1] > 1.0f)
					{
						//curUV.Normalize();
					}
					u = ((float)(curUV[0]));
					v = ((float)(curUV[1]));
					v *= -1;
					v +=1;
					displist.TexCoordf(u,v);
				}

				//write normals to the display list
				if (option.normals == true && NormalsElement != NULL)
				{
						nx = (float)curNormal[0];
						ny = (float)curNormal[1];
						nz = (float)curNormal[2];
						displist.Normalf(nx,ny,nz);
						lastNormal = curNormal;
				}

				//write vertex colors to the display list
				if (option.colors == true && ColorsElement != NULL)
				{
					r = (float)curColor[0];
					g = (float)curColor[1];
					b = (float)curColor[2];
					displist.Colorf(r,g,b);
					lastColor = curColor;
				}

				//write vertex to display list
				x = ((float)(curVert[0]));
				y = ((float)(curVert[1]));
				z = ((float)(curVert[2]));
				displist.Vertexf_v16(x,y,z);


			}


		}
		else if ((polyvertcount) == -1)
		{
			continue;
		}
		else
		{
			continue;
		}
	}


	if (!option.normalize)
	{
		displist.Scalef(1.0f/scale_factor,1.0f/scale_factor,1.0f/scale_factor);
	}


	displist.EndVertexList();
	displist.EndList();


	//make file and write commands
	FILE * outfile = fopen(node->GetName(),"wb");
	if (outfile == NULL)
	{
		printf("Could not open output file.\n");
		return false;
	}



	fwrite(displaylist,displaylist[0] + 1,sizeof(unsigned int),outfile);
	if (option.texinfo == true)
	{
		
	}




	fclose(outfile);

	return true;
}




void ScanNodes(KFbxNode * rootNode,int indent)
{
	for (int i = 0; i < rootNode->GetChildCount();i++)
	{
		for (int p = 0; p < indent; p++)
		{
			printf("-");
		}

		KFbxNode * curNode = rootNode->GetChild(i);


		//node attributes
		if (curNode->GetNodeAttribute() != NULL)
		{
			KFbxNodeAttribute::EAttributeType nodeType = curNode->GetNodeAttribute()->GetAttributeType();
			switch (nodeType)
			{
			case KFbxNodeAttribute::eMESH:
				{
					printf("%s : Mesh",curNode->GetName());
					if (WriteList(curNode,option))
					{
						printf("-Done.\n");
					}
					else
					{
						printf("-Failed to create display list.\n");
					}

					break;
				}
			default:
				{
					printf("%s\n",curNode->GetName());
					break;
				}
			}

		}
		else
		{
			printf("%s\n",curNode->GetName());
		}




		//recursive
		ScanNodes(curNode,indent+1);
	}
}


unsigned int upperPow2(unsigned x)
{
	--x;
	x |= x >>1;
	x |= x >>2;
	x |= x >>4;
	x |= x >>8;
	x |= x >>16;
	return ++x;
}

void PrintHelp()
{
	printf("Display Man by DEElekgolo : %s %s\n",__DATE__,__TIME__);
	printf("Format: DisplayMan.exe [filename] [arguments]\n");
	printf("Arguments:\n");
	printf("     -n : export with normals\n");
	printf("     -normalize : Scale down mesh to -7 to 7 space. (Good for if you want to avoid using scale matrices)\n");
	printf("     -t ?x? : Export with texture UVs. (ex. -t 128x182) Resolutions are rounded to the nearest power of 2.\n");
	printf("	 -about : Display information about this software.\n");
	printf("     -c : export with Vertex Colors\n");
	printf("By default the file will be exported with only vertex coordinates and scale to its original size\n");
	printf("This tool supports .dxf, .dae, .3ds, .obj, and .fbx. I recommend using .fbx as it supports the most features.\n");

	printf("Send all bugs or feature requests to DEElekgolo@aim.com\n");
}

void PrintAbout()
{
	printf(
		"This software contains Autodesk® FBX® code developed by Autodesk,\n Inc. Copyright 2011 Autodesk, Inc. All rights,\n"
		"reserved. Such code is provided “as is” and Autodesk, Inc. disclaims any and all warranties, whether express or\n"
		"implied, including without limitation the implied warranties of merchantability, fitness for a particular purpose or non-\n"
		"infringement of third party rights. In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special,\n"
		"exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss\n"
		"of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract,\n"
		"strict liability, or tort (including negligence or otherwise) arising in any way out of such code.\n");
}