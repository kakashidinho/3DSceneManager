#include "glHelper.h"
#include "TextWritter.h"
#include <fstream>

struct Glyph
{
	Glyph() {width = xAdvance = 0;}
	/*--------------the following values are used in normalize texture coordinate space--*/ 
	float leftU;//leftmost texture coordinate
	float rightU;//rightmost texture coordinate
	float bottomV;//bottommost texture coordinate
	float topV;//topmost texture coordinate
	
	/*------------the following values are used in image space------*/
	/*------offsets of glypth quad's origin (bottom left)---------------------------------*/ 
	float xOffset;//signed distance from location that will insert this glyph
	float yOffset;//signed distance from main line of the text that contains this glyph
	//width of character
	int width;
	//height of character
	int height;
	int xAdvance;
};

struct FontVertex
{
	float x , y , z;//position
	float u , v ;//texcoord
	unsigned int color;//color
};

struct TextImpl : public Text
{
	TextImpl(unsigned int maxChar) : maxVertices(maxChar << 2) , numActiveVertices(0)
	{
		
		unsigned int numIndices = (this->maxVertices / 4) * 6;
		/*-------create index buffer----------*/
		this->indices = new unsigned short [numIndices];
		if (this->indices == NULL)
			return;
		//fill index buffer
		/*--------------------------------------

			i------i+3
			|\    |
			| \   |
			|  \  |		=>this quad has indices {i , i+1 , i+2 , //first triangle
			|   \ |								 i+2 , i+3 , i} //second triangle
		 i+1|____\|i+2

		---------------------------------------*/
		unsigned int i=0;unsigned int j=0;
		for(;j<numIndices;i+=4)
		{
			//first triangle
			this->indices[j] = i;
			this->indices[++j] = i+1;
			this->indices[++j] = i+2;
			//second triangle
			this->indices[++j] = i+2;
			this->indices[++j] = i+3;
			this->indices[++j] = i;
			j++;
		}
		/*------create vertex buffer-----------*/
		this->vertices = new FontVertex[maxVertices];

	}
	~TextImpl()
	{
		delete[] this->vertices;
		delete[] this->indices;
	}

	
	unsigned int maxVertices , numActiveVertices;
	FontVertex *vertices;
	unsigned short *indices;
};

/*-------------TextWritter----------------*/


//constructor
TextWritter::TextWritter(const char *fntName,unsigned int maxChar)
{
	this->glyphs = new Glyph[65536];

	this->fntName = new char[strlen(fntName) + 1];
	strcpy(this->fntName,fntName);

	this->textureID = 0;

	this->defaultText = NULL;
	TextImpl * newDefaultText = new TextImpl(maxChar);
	if (newDefaultText->vertices != NULL && newDefaultText->indices != NULL)
		this->defaultText = newDefaultText;

	this->SetupGlyphs();
}
TextWritter::~TextWritter()
{
	delete[] this->glyphs;
	delete[] this->fntName;
	delete this->defaultText;

	glDeleteTextures(1 , &this->textureID);
}



void TextWritter::SetupGlyphs()
{
	UINT numChar,width,height ;
	std::ifstream stream(fntName);
	if(!stream.good())
		return;
	stream.ignore(1000,'=');
	stream.ignore(1000,'=');
	stream >> this->fontSize;//font's size
	stream.ignore(1000,'\n');//to next line
	stream.ignore(1000,'=');
	stream >> this->lineHeight;//line's height
	stream.ignore(1000,'=');
	stream >> this->base;
	stream.ignore(1000,'=');
	stream >> width;//get width of bitmap image
	stream.ignore(1000,'=');
	stream >> height ;// get height of bitmap image
	stream.ignore(1000,'\n');//to next line

	char fileName[256];//get name of bitmap file that stores glyph symbols
	int i = 0;
	stream.ignore(1000,'\"');
	do{
		stream >> fileName[i++];
	}while(fileName[i-1] != '\"' && i < 256);
	fileName[i-1] = '\0';
	
	//get number of characters
	stream.ignore(1000,'=');
	stream >> numChar;

	for(UINT i = 0; i < numChar ; ++i)
	{
		int charID,X,Y;
		stream.ignore(1000,'=');
		stream >> charID;
		Glyph &newGlyph = this->glyphs[charID];
		stream.ignore(1000,'=');
		stream >> X ;
		stream.ignore(1000,'=');
		stream >> Y ;
		stream.ignore(1000,'=');
		stream >> newGlyph.width ;
		stream.ignore(1000,'=');
		stream >> newGlyph.height ;
		stream.ignore(1000,'=');
		stream >> newGlyph.xOffset ;
		stream.ignore(1000,'=');
		stream >> newGlyph.yOffset ;
		stream.ignore(1000,'=');
		stream >> newGlyph.xAdvance;
		//calculate texcoords for glyph symbol in texture
		newGlyph.leftU = (float) X / width;
		newGlyph.rightU = (float)(X + newGlyph.width) / width;
		newGlyph.topV = (float) Y / height;
		newGlyph.bottomV = (float)(Y + newGlyph.height) / height;
		stream.ignore(1000,'\n');//to next line
	}
	

	stream.close();
	
	if (this->glyphs['	'].xAdvance == 0)
		this->glyphs['	'].xAdvance = 2 * this->glyphs[' '].xAdvance;

	//find directory path
	char *last = strrchr(fntName,'/') ;
	if(last == NULL)
		last = strrchr(fntName,'\\');
	int lastIndex = last - fntName;
	char *filePath = new char[lastIndex + 2 + strlen(fileName)];
	strncpy(filePath,fntName,lastIndex + 1);
	strncpy(filePath + lastIndex + 1,fileName,strlen(fileName));
	filePath[lastIndex + 1 + strlen(fileName)] = '\0';
	
	/*-------create texture---------*/
	this->textureID = LoadTexture(filePath);
	
	delete [] filePath;
}

bool TextWritter::InitTextObject(Text* textObject ,const wchar_t *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetweenChar  )
{
	if(text == NULL ||  textObject == NULL)
		return false;
	float scale = 1.0f;//scaling ratio between <fontSize> & default font's size
	if(fontSize != 0.0f)
		scale = fontSize / this->fontSize;
	
	((TextImpl*)textObject)->numActiveVertices = 0;

	Y = Y - this->base * scale;
	float cX,cY;//insert location of character
	cX = X;
	cY = Y; 
	const float z = 1.0f;

	FontVertex *v = ((TextImpl*)textObject)->vertices;
	wchar_t c;
	for(unsigned int i=0;text[i]!=L'\0';++i)
	{
		c=text[i];
		Glyph& glyph = glyphs[c];
		if(c == L'\n')//new line
		{
			cY = cY + this->lineHeight * scale;
			cX = X;
			continue;
		}
		
		
		if (c != L' ' && c != L'	'  && glyph.width != 0)
		{
			//fill vertex buffer with 4 vertices that form glyph 's quad
			float xOffset = glyph.xOffset * scale;
			float yOffset = glyph.yOffset * scale;
			float gWidth = glyph.width * scale;
			float gHeight = glyph.height * scale;

			v[0].color = fontColor;
			v[0].z = z;
			v[0].x = cX + xOffset;
			v[0].y = cY + yOffset;
			v[0].u = glyph.leftU;
			v[0].v = glyph.topV;

			v[1].color = fontColor;
			v[1].z = z;
			v[1].x = v[0].x;
			v[1].y = v[0].y + gHeight;
			v[1].u = glyph.leftU;
			v[1].v = glyph.bottomV;

			v[2].color = fontColor;
			v[2].z = z;
			v[2].x = v[0].x + gWidth;
			v[2].y = v[1].y;
			v[2].u = glyph.rightU;
			v[2].v = glyph.bottomV;

			v[3].color = fontColor;
			v[3].z = z;
			v[3].x = v[2].x;
			v[3].y = v[0].y;
			v[3].u = glyph.rightU;
			v[3].v = glyph.topV;

			v += 4;
			((TextImpl*)textObject)->numActiveVertices += 4;
			if(((TextImpl*)textObject)->numActiveVertices == ((TextImpl*)textObject)->maxVertices)
				break;
		}
		cX += glyphs[c].xAdvance * scale + spaceBetweenChar;
	}

	return true;
}

bool TextWritter::InitTextObject(Text* textObject ,const char *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetweenChar  )
{
	if(text == NULL ||  textObject == NULL)
		return false;
	float scale = 1.0f;//scaling ratio between <fontSize> & default font's size
	if(fontSize != 0.0f)
		scale = fontSize / this->fontSize;
	
	((TextImpl*)textObject)->numActiveVertices = 0;

	Y = Y - this->base * scale;
	float cX,cY;//insert location of character
	cX = X;
	cY = Y; 
	const float z = 1.0f;

	FontVertex *v = ((TextImpl*)textObject)->vertices;
	char c;
	for(unsigned int i=0;text[i]!='\0';++i)
	{
		c=text[i];
		Glyph& glyph = glyphs[c];
		if(c == '\n')//new line
		{
			cY = cY + this->lineHeight * scale;
			cX = X;
			continue;
		}
		if (c != ' ' && c != '	' && glyph.width != 0)
		{
			//fill vertex buffer with 4 vertices that form glyph 's quad
			float xOffset = glyph.xOffset * scale;
			float yOffset = glyph.yOffset * scale;
			float gWidth = glyph.width * scale;
			float gHeight = glyph.height * scale;

			v[0].color = fontColor;
			v[0].z = z;
			v[0].x = cX + xOffset;
			v[0].y = cY + yOffset;
			v[0].u = glyph.leftU;
			v[0].v = glyph.topV;

			v[1].color = fontColor;
			v[1].z = z;
			v[1].x = v[0].x;
			v[1].y = v[0].y + gHeight;
			v[1].u = glyph.leftU;
			v[1].v = glyph.bottomV;

			v[2].color = fontColor;
			v[2].z = z;
			v[2].x = v[0].x + gWidth;
			v[2].y = v[1].y;
			v[2].u = glyph.rightU;
			v[2].v = glyph.bottomV;

			v[3].color = fontColor;
			v[3].z = z;
			v[3].x = v[2].x;
			v[3].y = v[0].y;
			v[3].u = glyph.rightU;
			v[3].v = glyph.topV;

			v += 4;
			((TextImpl*)textObject)->numActiveVertices += 4;
			if(((TextImpl*)textObject)->numActiveVertices == ((TextImpl*)textObject)->maxVertices)
				break;
		}
		cX += glyphs[c].xAdvance * scale + spaceBetweenChar;
	}

	return true;
}

void TextWritter::Render(const wchar_t *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetweenChar)
{
	TextImpl * textObject = (TextImpl *)this->defaultText;
	if (!this->InitTextObject(textObject , text , X , Y , fontColor , fontSize , spaceBetweenChar))
		return;
	
	this->Render(textObject);

}

void TextWritter::Render(const char *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetweenChar)
{
	TextImpl * textObject = (TextImpl *)this->defaultText;
	if (!this->InitTextObject(textObject , text , X , Y , fontColor , fontSize , spaceBetweenChar))
		return;
	
	this->Render(textObject);

}

Text* TextWritter::CreateStaticText(const wchar_t *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetweenChar )
{
	TextImpl *newTextObject = new TextImpl(wcslen(text));
	if (!this->InitTextObject(newTextObject , text , X , Y , fontColor , fontSize , spaceBetweenChar))
		return NULL;
	return newTextObject;
}

Text* TextWritter::CreateStaticText(const char *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetweenChar )
{
	TextImpl *newTextObject = new TextImpl(strlen(text));
	if (!this->InitTextObject(newTextObject , text , X , Y , fontColor , fontSize , spaceBetweenChar))
		return NULL;
	return newTextObject;
}

void TextWritter::Render(const Text* text)
{
	if (text == NULL)
		return;
	TextImpl * textObject = (TextImpl *)text;
	
	glBindTexture(GL_TEXTURE_2D , this->textureID);

	glVertexPointer(3 , GL_FLOAT  , sizeof(FontVertex) , textObject->vertices);
	glTexCoordPointer(2 , GL_FLOAT , sizeof(FontVertex) , (char*)textObject->vertices + 3 * sizeof(float));
	glColorPointer(4 , GL_UNSIGNED_BYTE , sizeof(FontVertex) , (char*)textObject->vertices + 5 * sizeof(float));

	glDrawElements(GL_TRIANGLES , (textObject->numActiveVertices >> 2) * 6,GL_UNSIGNED_SHORT, textObject->indices);
}

void TextWritter::BeginRender()
{	
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glEnable(GL_BLEND);

}
void TextWritter::EndRender()
{
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glDisable(GL_BLEND);
}

float TextWritter::GetTextLength(const wchar_t *text,float fontSize,float spaceBetweenChar)
{
	if(text == NULL)
		return 0.0f;
	float scale = 1.0f;//scaling ratio between <fontSize> & default font's size
	if(fontSize != 0.0f)
		scale = fontSize / this->fontSize;
	float length = 0.0f;
	wchar_t c;
	for(unsigned int i=0;text[i]!=L'\0';++i)
	{
		c=text[i];

		length += glyphs[c].xAdvance * scale + spaceBetweenChar;
	}
	return length;
}

float TextWritter::GetTextLength(const char *text,float fontSize,float spaceBetweenChar)
{
	if(text == NULL)
		return 0.0f;
	float scale = 1.0f;//scaling ratio between <fontSize> & default font's size
	if(fontSize != 0.0f)
		scale = fontSize / this->fontSize;
	float length = 0.0f;
	char c;
	for(unsigned int i=0;text[i]!='\0';++i)
	{
		c=text[i];

		length += glyphs[c].xAdvance * scale + spaceBetweenChar;
	}
	return length;
}
