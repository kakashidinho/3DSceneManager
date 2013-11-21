#include "Menu.h"
#include "OctreeSceneManager.h"
#include "Common.h"
#include "Mode2D.h"
#include <string>
#include <stdio.h>

#define PREVIEW_WIDTH 200
#define PREVIEW_HEIGHT 200
#define MAP_SELECTION_WIDTH 150
#define MAP_SELECTION_HEIGHT 50
#define LEVEL_SELECTION_WIDTH 100
#define LEVEL_SELECTION_HEIGHT 50
#define BUTTON_WIDTH 250//default button size
#define BUTTON_HEIGHT 75//default button size
#define BUTTON_WIDTH2 50//next/prev/add/sub button size
#define BUTTON_HEIGHT2 50//next/prev/add/sub button size
#define BUTTON_FONT_COLOR FONT_COLOR(255 , 255 , 0 , 255)

#define MAIN 0
#define HELP 1
#define SELECT 2

#define CONCAT_NAME_BASE(name1 , name2) name1##name2
#define CONCAT_NAME(name1 , name2) CONCAT_NAME_BASE(name1 , name2)
#define CREATE_BUTTON_CAPTION_BASE( button , captionText , offsetX , relativeY , fontSize , space , tempVarName) \
	wchar_t * tempVarName = UTF8(captionText);\
	button.caption = textWritter->CreateStaticText(\
										tempVarName , \
										(float)button.rect.X + (button .rect.W - textWritter->GetTextLength(tempVarName , fontSize , space) )/2.0f - offsetX,\
										button.rect.Y + relativeY , \
										BUTTON_FONT_COLOR ,\
										fontSize , space); \
	delete [] tempVarName;

#define CREATE_BUTTON_CAPTION(button , captionText ,offsetX ,relativeY ,  fontSize , space)\
	CREATE_BUTTON_CAPTION_BASE(button , captionText , offsetX , relativeY , fontSize , space , CONCAT_NAME(str , __COUNTER__))

//no unicode
#define RENDER_FRAME_CAPTION( rect , captionText , offsetX ,relativeY , fontSize , space ) \
					textWritter->Render(\
										captionText , \
										(float)rect.X + (rect.W - textWritter->GetTextLength(captionText , fontSize , space) )/2.0f - offsetX,\
										rect.Y + relativeY , \
										BUTTON_FONT_COLOR ,\
										fontSize , space); 

const HQRect previewRect = {(WINDOW_WIDTH - PREVIEW_WIDTH) / 2 , 10 , PREVIEW_WIDTH , PREVIEW_HEIGHT};
const HQRect previewRectIn = {previewRect.X + 5 , previewRect.Y + 5 ,previewRect.W - 15 , previewRect.H - 20 };
const HQRect mapSelectionRect = {(WINDOW_WIDTH - MAP_SELECTION_WIDTH) / 2 , 
									10 + previewRect.H + previewRect.Y, 
									MAP_SELECTION_WIDTH , 
									MAP_SELECTION_HEIGHT};
const HQRect quadtreeLevelsRect = {(WINDOW_WIDTH - LEVEL_SELECTION_WIDTH) / 2 , 
									10 + mapSelectionRect.H + mapSelectionRect.Y, 
									LEVEL_SELECTION_WIDTH , 
									LEVEL_SELECTION_HEIGHT};

const HQRect octreeLevelsRect = {(WINDOW_WIDTH - LEVEL_SELECTION_WIDTH) / 2 , 
									10 + quadtreeLevelsRect.H + quadtreeLevelsRect.Y, 
									LEVEL_SELECTION_WIDTH , 
									LEVEL_SELECTION_HEIGHT};

const HQRect numberMovSelectionRect = {WINDOW_WIDTH / 2 - LEVEL_SELECTION_WIDTH , 
									10 + octreeLevelsRect.H + octreeLevelsRect.Y , 
									LEVEL_SELECTION_WIDTH , 
									LEVEL_SELECTION_HEIGHT};

const HQRect numberStaticSelectionRect = {WINDOW_WIDTH / 2  , 
									10 + octreeLevelsRect.H + octreeLevelsRect.Y , 
									LEVEL_SELECTION_WIDTH , 
									LEVEL_SELECTION_HEIGHT};

extern unsigned int ge_numMovObjects;
extern unsigned int ge_numStaticObjects;

/*---------Menu::Button-------------*/
Menu::Button::Button() : caption(NULL) , highLighted(false)
{
}

Menu::Button::~Button()
{
	delete caption;
}
/*-----------TerrainInfo------------*/
struct HeightMapInfo
{
	std::string info;
	std::string heightmap;
};

struct TerrainInfo{
	TerrainInfo() : heightMaps(NULL) , textures(NULL) , previewTexture(0) , simpleTextureIndex(0) {}
	~TerrainInfo() {
		if (previewTexture != 0)
			glDeleteTextures(1 , &previewTexture);
		delete[] heightMaps;
		for (unsigned int i = 0 ; i < maxLevel ; ++i)
			delete[] textures[i];
		delete[] textures;
	}
	std::string name;
	unsigned int numHeightMaps;
	HeightMapInfo *heightMaps;//<numHeightMaps> in size
	unsigned int maxLevel;
	unsigned int simpleTextureIndex;
	char **textures;//textures name , <maxLevel> in size
	float treeScale;
	float camVelocity;
	int heightMaploadMethod;

	GLuint previewTexture;
};

/*-----------Menu--------------*/
Menu::Menu(TextWritter *_textWritter , 
		   const char *buttonOnImage , 
		   const char *buttonOffImage ,
		   const char *frameImage , 
		   const char *backgroundImage,
		   const char *terrainInfoFile )
: textWritter(_textWritter) , section(MAIN) , terrainInfos(NULL) , numTerrains(0) ,
selectedTerrain(0) , selectedHeightMap(0) , selectedLevel(4) , selectedOctreeLevel(MAX_NUM_OCTREE_LEVELS) 
{

	this->InitButtons(buttonOnImage , buttonOffImage);
	this->InitHelpText();
	this->InitTerrainInfos(terrainInfoFile);

	this->frameTexture = LoadTexture ( frameImage);
	this->bgTexture = LoadTexture (backgroundImage); 

	//captions
	wchar_t *str  = UTF8("Terrain:");
	this->previewCaption = textWritter->CreateStaticText(str , 
		10, 
		prevButton[0].rect.Y + 33.f , 
		BUTTON_FONT_COLOR , 28.f , 1.0f);
	delete[] str;
		
		
	str  = UTF8("Grid size:");
	this->mapSelectionCaption = textWritter->CreateStaticText(str , 
		10, 
		prevButton[1].rect.Y + 33.f , 
		BUTTON_FONT_COLOR , 28.f , 1.0f);
	delete[] str;

	str  = UTF8("Quadtree levels:");
	this->quadtreeLevelsCaption = textWritter->CreateStaticText(str , 
		10, 
		subButton[0].rect.Y + 33.f , 
		BUTTON_FONT_COLOR , 28.f , 1.0f);
	delete[] str;

	str  = UTF8("Octree levels:");
	this->octreeLevelsCaption = textWritter->CreateStaticText(str , 
		10, 
		subButton[1].rect.Y + 33.f , 
		BUTTON_FONT_COLOR , 28.f , 1.0f);
	delete[] str;
}

Menu::~Menu()
{
	glDeleteTextures(1 , &this->textures[0]);
	glDeleteTextures(1 , &this->textures[1]);
	glDeleteTextures(1 , &this->frameTexture);
	glDeleteTextures(1 , &this->bgTexture);
	delete this->helpTexts[0];
	delete this->helpTexts[1];
	delete this->mapSelectionCaption;
	delete this->quadtreeLevelsCaption;
	delete this->octreeLevelsCaption;
	delete this->previewCaption;

	delete[] this->terrainInfos;
}

void Menu::InitButtons(const char *buttonOnImage , const char *buttonOffImage)
{
	this->textures[0] = LoadTexture(buttonOnImage);
	this->textures[1] = LoadTexture(buttonOffImage);

	/*-------------buttons---------------------*/
	//exit

	exitButton[0].rect.X = (WINDOW_WIDTH - BUTTON_WIDTH) / 2  ; 
	exitButton[0].rect.Y = WINDOW_HEIGHT - 10 - BUTTON_HEIGHT ; 
	exitButton[0].rect.W = BUTTON_WIDTH ; 
	exitButton[0].rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(exitButton[0] , "Exit" , 5.f , 43.f , .0f , 1.0f);

	//exit button in terrain selection section
	exitButton[1].rect.X = (3 * WINDOW_WIDTH / 2 - BUTTON_WIDTH) / 2  ; 
	exitButton[1].rect.Y = WINDOW_HEIGHT - 10 - BUTTON_HEIGHT ; 
	exitButton[1].rect.W = BUTTON_WIDTH ; 
	exitButton[1].rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(exitButton[1] , "Exit" , 5.f , 43.f , .0f , 1.0f);

	//help
	helpButton.rect.X = (WINDOW_WIDTH - BUTTON_WIDTH) / 2  ; 
	helpButton.rect.Y = WINDOW_HEIGHT - 20 - 2 * BUTTON_HEIGHT ; 
	helpButton.rect.W = BUTTON_WIDTH ; 
	helpButton.rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(helpButton , "Shortcut keys" , 5.f ,43.f , .0f , 1.0f);

	//start
	startButton.rect.X = (WINDOW_WIDTH / 2 - BUTTON_WIDTH) / 2  ; 
	startButton.rect.Y = WINDOW_HEIGHT - 10 - BUTTON_HEIGHT ; 
	startButton.rect.W = BUTTON_WIDTH ; 
	startButton.rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(startButton , "Start" , 5.f ,43.f , .0f , 1.0f);

	//select button 
	selectButton.rect.X = (WINDOW_WIDTH - BUTTON_WIDTH) / 2  ; 
	selectButton.rect.Y = WINDOW_HEIGHT - 30 - 3 * BUTTON_HEIGHT ; 
	selectButton.rect.W = BUTTON_WIDTH ; 
	selectButton.rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(selectButton , "Choose terrain" , 5.f ,43.f , .0f , 1.0f);

	//next buttons
	nextButton[0].rect.X = previewRect.X + previewRect.W + 30;
	nextButton[0].rect.Y = previewRect.Y + (previewRect.H - BUTTON_HEIGHT2) / 2 ; 
	nextButton[1].rect.X = mapSelectionRect.X + mapSelectionRect.W + 30;
	nextButton[1].rect.Y = mapSelectionRect.Y + (mapSelectionRect.H - BUTTON_HEIGHT2) / 2 ; 

	nextButton[0].rect.W = nextButton[1].rect.W = BUTTON_WIDTH2 ; 
	nextButton[0].rect.H = nextButton[1].rect.H = BUTTON_HEIGHT2;
	
	CREATE_BUTTON_CAPTION(nextButton[0] , ">" , 0.f ,33.f , .0f , 1.0f);
	CREATE_BUTTON_CAPTION(nextButton[1] , ">" , 0.f ,33.f ,.0f , 1.0f);

	//prev buttons
	prevButton[0].rect.X = previewRect.X - BUTTON_WIDTH2 - 30;
	prevButton[0].rect.Y = previewRect.Y + (previewRect.H - BUTTON_HEIGHT2) / 2 ; 
	prevButton[1].rect.X = mapSelectionRect.X - BUTTON_WIDTH2 - 30;
	prevButton[1].rect.Y = mapSelectionRect.Y + (mapSelectionRect.H - BUTTON_HEIGHT2) / 2 ; 

	prevButton[0].rect.W = prevButton[1].rect.W = BUTTON_WIDTH2 ; 
	prevButton[0].rect.H = prevButton[1].rect.H = BUTTON_HEIGHT2;
	
	CREATE_BUTTON_CAPTION(prevButton[0] , "<" , 0.f ,33.f ,.0f , 1.0f);
	CREATE_BUTTON_CAPTION(prevButton[1] , "<" , 0.f ,33.f ,.0f , 1.0f);

	//add buttons
	addButton[0].rect.X = quadtreeLevelsRect.X + quadtreeLevelsRect.W + 30;
	addButton[0].rect.Y = quadtreeLevelsRect.Y + (quadtreeLevelsRect.H - BUTTON_HEIGHT2) / 2 ; 
	addButton[0].rect.W = BUTTON_WIDTH2 ; 
	addButton[0].rect.H = BUTTON_HEIGHT2;
	
	CREATE_BUTTON_CAPTION(addButton[0] , "+" , 0.f ,33.f ,.0f , 1.0f);

	addButton[1].rect.X = octreeLevelsRect.X + octreeLevelsRect.W + 30;
	addButton[1].rect.Y = octreeLevelsRect.Y + (octreeLevelsRect.H - BUTTON_HEIGHT2) / 2 ; 
	addButton[1].rect.W = BUTTON_WIDTH2 ; 
	addButton[1].rect.H = BUTTON_HEIGHT2;
	
	CREATE_BUTTON_CAPTION(addButton[1] , "+" , 0.f ,33.f ,.0f , 1.0f);

	//sub buttons
	subButton[0].rect.X = quadtreeLevelsRect.X - 30 - BUTTON_WIDTH2 ;  
	subButton[0].rect.Y = quadtreeLevelsRect.Y + (quadtreeLevelsRect.H - BUTTON_HEIGHT2) / 2 ; 
	subButton[0].rect.W = BUTTON_WIDTH2 ; 
	subButton[0].rect.H = BUTTON_HEIGHT2;
	
	CREATE_BUTTON_CAPTION(subButton[0] , "-" , 0.f ,33.f ,.0f , 1.0f);

	subButton[1].rect.X = octreeLevelsRect.X - 30 - BUTTON_WIDTH2 ;  
	subButton[1].rect.Y = octreeLevelsRect.Y + (octreeLevelsRect.H - BUTTON_HEIGHT2) / 2 ; 
	subButton[1].rect.W = BUTTON_WIDTH2 ; 
	subButton[1].rect.H = BUTTON_HEIGHT2;
	
	CREATE_BUTTON_CAPTION(subButton[1] , "-" , 0.f ,33.f ,.0f , 1.0f);
	
	//number selection buttons
	numberButton[0].rect.X = (numberMovSelectionRect.X - BUTTON_WIDTH) / 2  ;  
	numberButton[0].rect.Y = octreeLevelsRect.Y + octreeLevelsRect.H + 10 ; 
	numberButton[0].rect.W = BUTTON_WIDTH ; 
	numberButton[0].rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(numberButton[0] , "Num dynamic objects" , 5.f ,38.f ,24.f , 1.0f);

	numberButton[1].rect.X = numberStaticSelectionRect.X  + numberStaticSelectionRect.W +  numberButton[0].rect.X  ;  
	numberButton[1].rect.Y = octreeLevelsRect.Y + octreeLevelsRect.H + 10 ; 
	numberButton[1].rect.W = BUTTON_WIDTH ; 
	numberButton[1].rect.H = BUTTON_HEIGHT;
	
	CREATE_BUTTON_CAPTION(numberButton[1] , "Num static objects" , 5.f ,38.f ,24.f , 1.0f);
}

void Menu::InitHelpText()
{
	/*-------help text-----*/
	wchar_t *text = UTF8("\
1\n\
2\n\
C\n\
F\n\
G\n\
R\n\
V\n\
Tab\n\
~ or `\n\
W\n\
S\n\
A\n\
D\n\
Q\n\
E\n\
");
	this->helpTexts[0] = textWritter->CreateStaticText(text	, 10 , 50 , BUTTON_FONT_COLOR , 28 , 1);

	delete[] text;

	text = UTF8("~ hoặc `");
	float len = textWritter->GetTextLength(text , 0 , 1) + 20;
	delete[]text;

	text = UTF8("\
- on/off Terrain's Quadtree\n\
- on/off Octree Scene Manager\n\
- on/off wireframe mode\n\
- increase Quadtree's error threshold\n\
- decrease Quadtree's error threshold\n\
- on/off Quadtree's crack filling\n\
- on/off Geomorph mode\n\
- on/off bird-eye view\n\
- on/off information display\n\
- move camera forward\n\
- move camera backward\n\
- move camera left\n\
- move camera right\n\
- increase camera's height\n\
- decrease camera's height\n\
");
	this->helpTexts[1] = textWritter->CreateStaticText(text	, 
		len , 
		50 , BUTTON_FONT_COLOR , 28 , 1);

	delete[] text;
}

void Menu::InitTerrainInfos(const char *terrainInfoFile)
{
	FILE *f = fopen(terrainInfoFile , "r");
	if (f == NULL)
		return;
	fscanf(f , " terrains %u" , &this->numTerrains);
	this->terrainInfos = new TerrainInfo[this->numTerrains];
	
	//get current directory
	DWORD size = GetCurrentDirectory(0 , 0);
	wchar_t *currentDir = new wchar_t[size];
	GetCurrentDirectory(size , currentDir);

	//find directory path
	const char *last = strrchr(terrainInfoFile,'/') ;
	if(last == NULL)
		last = strrchr(terrainInfoFile,'\\');
	int lastIndex = last - terrainInfoFile;
	char *dir = new char[lastIndex + 2];
	strncpy(dir , terrainInfoFile , lastIndex + 1);
	dir[lastIndex + 1] = '\0';
	
	SetCurrentDirectoryA(dir);

	delete[] dir;

	char str[256];

	for (unsigned int i = 0 ; i < this->numTerrains ; ++i)
	{
		fscanf(f,"%s" , str);//name
		this->terrainInfos[i].name = str;
		
		//load method
		fscanf(f," load method %d" , &this->terrainInfos[i].heightMaploadMethod);
		//tree scale factor
		fscanf(f," tree scale %f" , &this->terrainInfos[i].treeScale);
		//camera velocity
		fscanf(f," camera velocity %f" , &this->terrainInfos[i].camVelocity);
		//height maps
		fscanf(f," heightmaps %u" , &this->terrainInfos[i].numHeightMaps);
		this->terrainInfos[i].heightMaps = new HeightMapInfo [this->terrainInfos[i].numHeightMaps];
		for(unsigned int j = 0 ; j < this->terrainInfos[i].numHeightMaps ; ++j)
		{
			fscanf(f, " %s" , str);
			this->terrainInfos[i].heightMaps[j].info = str;
			fscanf(f, " %s" , str);
			this->terrainInfos[i].heightMaps[j].heightmap = str;
		}

		//textures

		fscanf(f," textures %u" , &this->terrainInfos[i].maxLevel);
		this->terrainInfos[i].textures = new char* [this->terrainInfos[i].maxLevel];
		fscanf(f," simpleTexture %u" , &this->terrainInfos[i].simpleTextureIndex);//texture for simple terrain
		this->terrainInfos[i].simpleTextureIndex--;

		for(unsigned int j = 0 ; j < this->terrainInfos[i].maxLevel ; ++j)
		{
			fscanf(f, " %s" , str);
			this->terrainInfos[i].textures[j] = new char[strlen(str) + 1];
			strcpy(this->terrainInfos[i].textures[j] , str );
		}

		//load preview texture
		SetCurrentDirectoryA(this->terrainInfos[i].name.c_str());
		this->terrainInfos[i].previewTexture = LoadTexture("preview.bmp");
		SetCurrentDirectoryA("..");
	}

	fclose(f);
	//reset current directory
	SetCurrentDirectory(currentDir);
	delete[] currentDir;
}

int Menu::Update(int mouseX , int mouseY , bool mouseClick)
{
	//exit button pointer
	Button *pExitButton;

	switch (section)
	{
	case MAIN:
		pExitButton = &exitButton[0];
		//help button
		if (helpButton.rect.IsPointInside(mouseX , mouseY))
		{
			helpButton.highLighted = true;
			if (mouseClick)
			{
				helpButton.highLighted = false;
				section = HELP;
			}
		}
		else
			helpButton.highLighted = false;
		
		//select button
		if (selectButton.rect.IsPointInside(mouseX , mouseY))
		{
			selectButton.highLighted = true;
			if (mouseClick)
			{
				selectButton.highLighted = false;
				section = SELECT;
			}
		}
		else
			selectButton.highLighted = false;

		break;
	case SELECT:
		pExitButton = &exitButton[1];
		//start button
		if (startButton.rect.IsPointInside(mouseX , mouseY))
		{
			startButton.highLighted = true;
			if (mouseClick)
			{
				startButton.highLighted = false;
				section = MAIN;
				return START;
			}
		}
		else
			startButton.highLighted = false;
		
		//next/prev/add/sub button
		for (int i = 0 ; i < 2 ; ++i)
		{	
			//next
			if (nextButton[i].rect.IsPointInside(mouseX , mouseY))
			{
				nextButton[i].highLighted = true;
				if (mouseClick)
				{
					if (i == 0)//next terrain
					{
						this->selectedTerrain++;
						if (this->selectedTerrain == this->numTerrains)
							this->selectedTerrain = this->numTerrains - 1;
					}
					else//next heightmap
					{
						this->selectedHeightMap++;
					}
				}
			}
			else
				nextButton[i].highLighted = false;	
			
			//prev
			if (prevButton[i].rect.IsPointInside(mouseX , mouseY))
			{
				prevButton[i].highLighted = true;
				if (mouseClick)
				{
					if (i == 0)//prev terrain
					{
						if (this->selectedTerrain > 0)
							this->selectedTerrain --;
					}
					else//prev heightmap
					{
						if (this->selectedHeightMap > 0)
							this->selectedHeightMap --;
					}
				}
			}
			else
				prevButton[i].highLighted = false;	

			//add
			if (addButton[i].rect.IsPointInside(mouseX , mouseY))
			{
				addButton[i].highLighted = true;
				if (mouseClick)
				{
					if (i == 0)
						this->selectedLevel++;
					else if (this->selectedOctreeLevel < MAX_NUM_OCTREE_LEVELS)
						this->selectedOctreeLevel ++;
				}
			}
			else
				addButton[i].highLighted = false;	

			//sub
			if (subButton[i].rect.IsPointInside(mouseX , mouseY))
			{
				subButton[i].highLighted = true;
				if (mouseClick)
				{
					if (i == 0)
					{
						if (this->selectedLevel > 1)
							this->selectedLevel--;
					}
					else if (this->selectedOctreeLevel > 1)
						this->selectedOctreeLevel --;

				}
			}
			else
				subButton[i].highLighted = false;	

			//number selection button
			if (numberButton[i].rect.IsPointInside(mouseX , mouseY))
			{
				numberButton[i].highLighted = true;
				if (mouseClick)
				{
					numberButton[i].highLighted = false;
					if (i == 0)
						return NUMBER_MOVING_SELECT;
					else
						return NUMBER_STATIC_SELECT;
				}
			}
			else
				numberButton[i].highLighted = false;
		}
		


		//make sure selected heightmap & level are valid
		if (this->selectedHeightMap >= this->terrainInfos[this->selectedTerrain].numHeightMaps)
			this->selectedHeightMap = this->terrainInfos[this->selectedTerrain].numHeightMaps - 1;

		if (this->selectedLevel > this->terrainInfos[this->selectedTerrain].maxLevel)
			this->selectedLevel = this->terrainInfos[this->selectedTerrain].maxLevel;

		break;
	default:
		pExitButton = &exitButton[0];

	}
	//exit button
	if (pExitButton->rect.IsPointInside(mouseX , mouseY))
	{
		pExitButton->highLighted = true;
		if (mouseClick)
		{
			pExitButton->highLighted = false;
			switch (section)
			{
			case MAIN:  return EXIT;//exit
			default: 
				section = MAIN;//back to main
				return NONE;
			}
		}
	}
	else
		pExitButton->highLighted = false;

	return NONE;

}

void Menu::Render(bool inGame)
{
	Mode2D::Begin();
	glEnable(GL_BLEND);
	
	/*-------draw screen quad----------*/
	if (inGame)
	{
		glDisable(GL_TEXTURE_2D);

		glColor4f(0 , 0 ,0 , 0.5f);
		glBegin(GL_TRIANGLE_STRIP);

		glVertex3f(0 , 0  , 1.0f);
		glVertex3f(0 , WINDOW_HEIGHT , 1.0f);
		glVertex3f(WINDOW_WIDTH , 0 , 1.0f);
		glVertex3f(WINDOW_WIDTH , WINDOW_HEIGHT , 1.0f);

		glEnd();
		
		glEnable(GL_TEXTURE_2D);
		glColor4f(1 , 1 , 1 , 0.5f);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D , this->bgTexture);

		glBegin(GL_TRIANGLE_STRIP);

		glTexCoord2f(0 , 0 ); glVertex3f(0 , 0  , 1.0f);
		glTexCoord2f(0 , 1 ); glVertex3f(0 , WINDOW_HEIGHT , 1.0f);
		glTexCoord2f(1 , 0 ); glVertex3f(WINDOW_WIDTH , 0 , 1.0f);
		glTexCoord2f(1 , 1 ); glVertex3f(WINDOW_WIDTH , WINDOW_HEIGHT , 1.0f);

		glEnd();
		glColor4f(1 , 1 , 1 , 0.5f);
	}
	/*-------draw shape--------*/
	Button *pExitButton;
	switch(section)
	{
	case MAIN:
		pExitButton = &exitButton[0];
		this->DrawButton(this->helpButton);
		this->DrawButton(this->selectButton);
		break;
	case SELECT:
		pExitButton = &exitButton[1];
		this->DrawButton(this->startButton);
		for (int i = 0 ; i < 2; ++i)
		{
			this->DrawButton(this->nextButton[i]);
			this->DrawButton(this->prevButton[i]);
			this->DrawButton(this->addButton[i]);
			this->DrawButton(this->subButton[i]);
			this->DrawButton(this->numberButton[i]);
		}
		
		Mode2D::DrawRect(previewRect , this->frameTexture);
		Mode2D::DrawRect(mapSelectionRect , this->frameTexture);
		Mode2D::DrawRect(quadtreeLevelsRect , this->frameTexture);
		Mode2D::DrawRect(octreeLevelsRect , this->frameTexture);
		Mode2D::DrawRect(numberMovSelectionRect , this->frameTexture);
		Mode2D::DrawRect(numberStaticSelectionRect , this->frameTexture);

		break;
	default:
		pExitButton = &exitButton[0];
	}
	this->DrawButton(*pExitButton);

	glDisable(GL_BLEND);

	if (section == SELECT)
		Mode2D::DrawRect(previewRectIn , this->terrainInfos[this->selectedTerrain].previewTexture);

	/*--------draw texts & buttons caption ------*/
	TextWritter::BeginRender();
	
	this->textWritter->Render(pExitButton->caption);
	switch(section)
	{
	case MAIN:
		this->textWritter->Render(helpButton.caption);
		this->textWritter->Render(selectButton.caption);
		break;
	case HELP:
		this->textWritter->Render(this->helpTexts[0]);
		this->textWritter->Render(this->helpTexts[1]);
		break;
	case SELECT:
		{
			this->textWritter->Render(this->previewCaption);
			this->textWritter->Render(this->mapSelectionCaption);
			this->textWritter->Render(this->quadtreeLevelsCaption);
			this->textWritter->Render(this->octreeLevelsCaption);
			this->textWritter->Render(startButton.caption);
			for (int i = 0 ; i < 2; ++i)
			{
				this->textWritter->Render(this->nextButton[i].caption);
				this->textWritter->Render(this->prevButton[i].caption);
				this->textWritter->Render(this->addButton[i].caption);
				this->textWritter->Render(this->subButton[i].caption);
				this->textWritter->Render(this->numberButton[i].caption);
			}
			
			//draw heightmap info
			RENDER_FRAME_CAPTION(mapSelectionRect , 
								this->terrainInfos[this->selectedTerrain].heightMaps[this->selectedHeightMap].info.c_str() , 
								0.f,
								33.f , 
								28.f,
								1.f);
			//draw selected number of quadtree levels
			char str[32];
			sprintf(str , "%u" , this->selectedLevel);
			RENDER_FRAME_CAPTION(quadtreeLevelsRect , 
								str , 
								0.f,
								33.f , 
								28.f,
								1.f);

			//draw selected number of octree levels
			sprintf(str , "%u" , this->selectedOctreeLevel);
			RENDER_FRAME_CAPTION(octreeLevelsRect , 
								str , 
								0.f,
								33.f , 
								28.f,
								1.f);

			//draw selected number of moving objects 
			sprintf(str , "%u" , ge_numMovObjects);
			RENDER_FRAME_CAPTION(numberMovSelectionRect , 
								str , 
								0.f,
								33.f , 
								28.f,
								1.f);

			//draw selected number of static objects 
			sprintf(str , "%u" , ge_numStaticObjects);
			RENDER_FRAME_CAPTION(numberStaticSelectionRect , 
								str , 
								0.f,
								33.f , 
								28.f,
								1.f);

		}

		break;
	}

	TextWritter::EndRender();

	Mode2D::End();

	glColor4f(1 , 1 , 1 , 1);
}

void Menu::DrawButton(const Button & button) const
{
	if (button.highLighted)
		Mode2D::DrawRect(button.rect , this->textures[0]);
	else
		Mode2D::DrawRect(button.rect , this->textures[1]);
}

void Menu::GetSelectedTerrainOption(SelectedTerrainOption &option) const
{
	option.name = this->terrainInfos[this->selectedTerrain].name.c_str();
	option.heightmap = this->terrainInfos[this->selectedTerrain].heightMaps [ this->selectedHeightMap ].heightmap.c_str();
	option.level = this->selectedLevel;
	option.textures = this->terrainInfos[this->selectedTerrain].textures;
	option.simpleTexture = option.textures[ this->terrainInfos[this->selectedTerrain].simpleTextureIndex];
	option.heightMapLoadMethod = this->terrainInfos[this->selectedTerrain].heightMaploadMethod;
	option.treeScale = this->terrainInfos[this->selectedTerrain].treeScale;
	option.camVelocity = this->terrainInfos[this->selectedTerrain].camVelocity;
}

void Menu::ResetToMain()
{
	section = MAIN;
}
