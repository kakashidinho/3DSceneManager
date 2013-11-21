#ifndef TEXT_WRITER_H
#define TEXT_WRITER_H

#include "glHelper.h"

#define FONT_COLOR(red , green , blue , alpha) ((red & 0xff)| ((green & 0xff) << 8) |  ((blue & 0xff) << 16) | ((alpha & 0xff) << 24) )  

struct Glyph;
struct FontVertex;
struct Text
{
	virtual ~Text() {}
};

class TextWritter {
public:
	TextWritter(const char *fntName,unsigned int maxChar);//create text writter object from .fnt file , <maxChar> is max number of characters in one draw call
	~TextWritter();
	inline float GetFontSize() {return fontSize;};
	
	static void BeginRender();//remember to call Mode2D::Begin() before call this
	static void EndRender();
	
	//X,Y is starting postion of the line that the text will be drawn, <fontSize> = 0 => use default size
	//X,Y is in screen space (origin is top left)
	void Render(const wchar_t *text,float X,float Y,unsigned int fontColor,float fontSize=0,float spaceBetWeenChar = 0.0f);
	
	//X,Y is starting postion of the line that the text will be drawn, <fontSize> = 0 => use default size
	//X,Y is in screen space (origin is top left)
	Text* CreateStaticText(const wchar_t *text,float X,float Y,unsigned int fontColor,float fontSize=0,float spaceBetWeenChar = 0.0f);

	//X,Y is starting postion of the line that the text will be drawn, <fontSize> = 0 => use default size
	//X,Y is in screen space (origin is top left)
	void Render(const char *text,float X,float Y,unsigned int fontColor,float fontSize=0,float spaceBetWeenChar = 0.0f);
	
	//X,Y is starting postion of the line that the text will be drawn, <fontSize> = 0 => use default size
	//X,Y is in screen space (origin is top left)
	Text* CreateStaticText(const char *text,float X,float Y,unsigned int fontColor,float fontSize=0,float spaceBetWeenChar = 0.0f);

	void Render(const Text* staticText); 
	
	//get length in the screen of the text , text must not has newline character
	//<fontSize> = 0 => use default size
	float GetTextLength(const wchar_t *text,float fontSize = 0,float spaceBetweenChar = 0.0f);
	//get length in the screen of the text , text must not has newline character
	//<fontSize> = 0 => use default size
	float GetTextLength(const char *text,float fontSize = 0,float spaceBetweenChar = 0.0f);
private:
	Glyph *glyphs;//glyph table
	float lineHeight;
	float base;
	float fontSize;
	char *fntName;
	GLuint textureID;//font texture
	Text * defaultText;
	
	void SetupGlyphs();//worker function
	bool InitTextObject(Text* textObject ,const wchar_t *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetWeenChar  );
	bool InitTextObject(Text* textObject ,const char *text,float X,float Y,unsigned int fontColor,float fontSize,float spaceBetWeenChar  );
};

#endif