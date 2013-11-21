#ifndef MENU_H
#define MENU_H

#include "TextWritter.h"
#ifndef CMATH
#	define CMATH
#endif
#include "../Libraries/HQUtil/include/math/HQUtilMath.h"

struct TerrainInfo;

struct SelectedTerrainOption
{
	const char *name;
	const char *heightmap;
	const char* const * textures;
	const char * simpleTexture;
	unsigned int level;
	float treeScale;
	float camVelocity;
	int heightMapLoadMethod;
};

class Menu
{
public:
	enum Event
	{
		NONE ,
		EXIT ,
		START ,
		NUMBER_MOVING_SELECT ,
		NUMBER_STATIC_SELECT
	};


	Menu(TextWritter *textWritter , 
		const char *buttonOnImage , 
		const char *buttonOffImage ,
		const char *frameImage ,
		const char *backgroundImage,
		const char *terrainInfoFile);
	~Menu();
	
	
	int Update(int mouseX , int mouseY , bool mouseClick);//return one of value in Event enumeration 
	void GetSelectedTerrainOption(SelectedTerrainOption &option) const;
	unsigned int GetSelectedOctreeLevels() const {return selectedOctreeLevel;} 

	void Render(bool inGame);
	void ResetToMain();

private:
	struct Button
	{
		Button();
		~Button();

		HQRect rect;//region
		Text * caption;
		bool highLighted;
	};

	TextWritter *textWritter;
	Button exitButton[2] , helpButton , startButton , selectButton ,
		nextButton[2] , prevButton[2] , addButton[2] , subButton[2] , 
		numberButton[2];
	Text * helpTexts[2] , *previewCaption , * mapSelectionCaption , * quadtreeLevelsCaption , * octreeLevelsCaption;
	GLuint textures[2];//on & off shape image of button
	GLuint frameTexture , bgTexture;
	
	TerrainInfo *terrainInfos;
	unsigned int numTerrains;
	unsigned int selectedTerrain;
	unsigned int selectedHeightMap;
	unsigned int selectedLevel;//quadtree 's number of level
	unsigned int selectedOctreeLevel;//octree's number of  level
	int section;

	void DrawButton(const Button & button) const;

	void InitButtons(const char *buttonOnImage , const char *buttonOffImage);
	void InitHelpText();
	void InitTerrainInfos(const char *terrainInfoFile);
};

#endif