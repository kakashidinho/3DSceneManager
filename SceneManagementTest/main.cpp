#if defined _DEBUG || defined DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include <windows.h>
#include <process.h>
#include <Commctrl.h>
#pragma comment(lib,"Comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "resource.h"
#include "Common.h"
#include "Mode2D.h"
#include "TextWritter.h"
#include "Menu.h"
#include "Terrain.h"
#include "ChunkQuadTreeTerrain.h"
#include "Camera.h"
#include "Model.h"
#include "SkyDome.h"
#include "SphereObject.h"
#include "TreeObject.h"
#include "OctreeSceneManager.h"
#include "Timer.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>


	
/*-----constants definition----------------*/
#define WINDOW_STYLE WS_OVERLAPPEDWINDOW & (~(WS_MAXIMIZEBOX | WS_THICKFRAME))
#define desiredUPS 60 // expected updates per second
#define desiredUpdateTime 1.0f / desiredUPS
#define dAngle (HQPiFamily::_PI / 540.0f)
#define MinCamHeight 1.5f //main camera height relative to terrain
#define dHeight 0.5f
#define BIRD_EYE_CAM_WIDTH 200
#define BIRD_EYE_CAM_HEIGHT 200
#define MAX_FRAMES 0xffffffffffffffff
#define NUM_FPS_SAMPLES 32
#define TERRAIN_SCALE_FACTOR 1.f
#define ZFAR 3000.0f
#define CAMERA_FOVY HQPiFamily::_PIOVER4//camera field of view in y direction
const float vFNPOffsetY = (float)tan(CAMERA_FOVY / 2.0);
const float vFNPOffsetX = (float)(vFNPOffsetY * WINDOW_WIDTH / WINDOW_HEIGHT) ;
const float CAMERA_FOVX = (float)(2.0f * atan(vFNPOffsetX)); //camera field of view in x direction

#define NUM_SPHERE_MODELS 5
#define NUM_TREE_MODELS 3
#define MULTITHREAD_UPDATE 0

const char *sphereModelFiles[] = 
{
	"../DemoData/model/earth.obj",
	"../DemoData/model/sphere.obj",
	"../DemoData/model/sphereRed.obj",
	"../DemoData/model/sphereOrange.obj",
	"../DemoData/model/sphereBlue.obj"
};

const char *treeModelFiles[] = 
{
	"../DemoData/model/tree001.obj",
	"../DemoData/model/tree002.obj",
	"../DemoData/model/tree003.obj"
};

/*---------------------------------------*/
HDC hDC = NULL;//device context
HGLRC hRC = NULL;//main openGL rendering context
HGLRC hLRC = NULL;//openGL loading context

HINSTANCE hInst;
WNDCLASSEX wndclass;
HWND hwnd = NULL;
/*-------application state flags--------*/
bool wire ;//wireframe mode
bool disableMouse ;//mouse input disabled?
bool cull ;//backface culling enabled?
bool zbuffer ;//depth test enabled?
bool birdEye ;//display bird eye radar map?
bool drawTerrainSkirt ;//display terrain skirt?
bool geoMorpth ;//geomorphing enabled?
bool autoRotate ;//auto rotate around Oy
bool displayHud ;//display HUD? 
bool displayBBox ;//display bounding box of quadtree terrain
/*----------------camera---------------------*/
#define CAMERA_MOVE_FORWARD 0x1
#define CAMERA_MOVE_BACK 0x2
#define CAMERA_MOVE_LEFT 0x4
#define CAMERA_MOVE_RIGHT 0x8
#define CAMERA_MOVE_UP 0x10
#define CAMERA_MOVE_DOWN 0x20
#define CAMERA_TURN_LEFT 0x40
#define CAMERA_TURN_RIGHT 0x80
#define CAMERA_TURN_UP 0x100
#define CAMERA_TURN_DOWN 0x200

#define EnableFlag(state , flag) (state |= flag)
#define DisableFlag(state , flag) (state &= (~flag))

unsigned int cameraState = 0x0;//camera state flags

float camVelocity = 1.5f;//camera velocity
float camHeight = 20.0f;//camera height

/*-----------------mouse state-------------------------*/
struct MouseState{
	int dx;
	int dy;
};
MouseState mouseState;

unsigned char mouseInputBuffer[40];

/*----------------game state-----------------------*/
#define MENU 0x1//in menu
#define IN_GAME 0x2//in game
#define NUMBER_MOV_SELECT_DLG 0x4//number of moving objects selection dialog
#define NUMBER_STATIC_SELECT_DLG 0x8//number of static objects selection dialog

int gameState = MENU;

/*----------------render mode------------------------*/
#define BRUTE_FORCE 0//brute force rendering terrain
#define QUAD_TREE 0x1//quadtree terrain
#define OCTREE 0x2//octree for rendering objects
int renderMode = QUAD_TREE | OCTREE;

#if MULTITHREAD_UPDATE
/*----------------object updating thread state-----------------*/

#define START_UPDATE_OBJ 1//objects need to be updated
#define FINISH_UPDATE_OBJ 2//objects is updated
long g_objUpdateThreadState = 0;

#endif
/*---------------------------------------------------*/
Timer timer;//main timer
LARGE_INTEGER lastPerfCount;//last frame's performance count
LARGE_INTEGER curPerfCount;//current performance count
LARGE_INTEGER beginMenuPerfCount;//performance count when menu start appears in-game
ULONGLONG frames = 0;//number of frames rendered
float dTime;//period between last rendered frame and current frame
float dTimeInMenu = 0.0f;
float fpsSamples[NUM_FPS_SAMPLES];
float maxFps = -999999.0f;//max frames per second
float minFps = 999999.0f;//min frames per second
float fps = 0.0f;//frames per second
float avgFps = 0.0f;//average frames per second
unsigned int ge_numMovObjects = 100;//chosen number of moving objects in menu
unsigned int currentNumMovObjects = 0;//ingame number of moving objects
unsigned int ge_numStaticObjects = 100;//chosen number of static objects in menu
unsigned int currentNumStaticObjects = 0;//ingame number of static objects
unsigned int numTriangles;//number of rendered triangles
float ge_worldMaxHeight;//world's max height
Terrain *ge_terrain = NULL;//main terrain for rendering
SimpleTerrainTriStrip *simpleTerrain = NULL;//bruteforce terrain
ChunkQuadTreeTerrain *quadtreeTerrain = NULL;//quadtree terrain
Camera * camera = NULL;//camera
OctreeSceneManager* sceneManager = NULL;//octree scene manager for rendering objects
HQPlane viewFrustum[6];//6 planes of camera's view frustum
Model ** model = NULL;//models used for rendering objects
SphereObject *sphere = NULL;//spheres
TreeObject *tree = NULL;
TextWritter *textWritter = NULL;//text writter
Menu *menu = NULL;//menu manager
SkyDome *sky = NULL;//sky renderer
HQMatrix4 viewProjMatrix , projMatrix ;//main matrices
HQMatrix4 birdEyeViewMatrix , birdEyeProjMatrix[2];//matrices for rendering from bird eye point of view
HQVector4 birdEyeLookAt;
float errorThreshold = 0.0f;//user defined error metric for determining terrain LOD
bool g_loadingFinish = true;
bool g_loading = false;

#if MULTITHREAD_UPDATE
bool g_running = true;//is application still running?
#endif

GLuint loadingTextTexture = 0; GLuint loadingIconTexture = 0;//loading text and icon texture
HANDLE objectUpdateThread;//object updating thread id
HANDLE mutex[3];//2 mutexes , one for loading thread and one for object updating thread

/*---------------function prototype---------------------*/
bool InitOpenGLContext();//create window and openGL context
bool MyInit();//init window , opengl ..v..v
void RegisterRawMouseInput(bool _register);//register or unregister raw mouse input
void InitGame();//init ingame stuff
DWORD WINAPI InitGameThreadFunc(void *arg);//loading thread's routine
void UpdateCamera();//update current status of camera
void MyUpdate();//main update function
#if MULTITHREAD_UPDATE
DWORD WINAPI ObjectUpdateThreadFunc(void *arg);//object updating thread's routine
#endif
void MyDisplay();//main render function
void IngameDisplay();//ingame render function
void BirdEyeDisplay();//render from bird eye point of view
void DisplayHUD();//display info (fps , depth , ....v.v.v)
void LoadingScreenDisplay();//display loading screen
void MyLoop();//main application loop
void MyKeyboard(unsigned char key);
void MyCleanUp();//application 's cleanup function
void IngameCleanUp();//delete ingame data

INT_PTR CALLBACK NumberSettingDlg(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam);//dialog procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);//window procedure
/*---------------------------------------------*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	hInst = hInstance;
	InitCommonControls();
#if defined (_DEBUG) || defined(DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_crtBreakAlloc = 856;

#endif

	/*------------------------*/
	
	if (!MyInit())
		return 0;

	MSG msg = {0};


	/*---------main loop--------------*/
	
	bool l_loadingFinish;
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	while( msg.message!=WM_QUIT )
	{
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else {
			
			WaitForSingleObject(mutex[2] , INFINITE);
			l_loadingFinish = g_loadingFinish;
			ReleaseMutex(mutex[2]);

			if (l_loadingFinish)
			{
				if (g_loading)
				{
					//just finished loading ingame data
					g_loading = false;
					wglMakeCurrent(hDC , hRC);//make main context this thread's main rendering context

					/*---check for error----*/
					if (GL_OUT_OF_MEMORY == glGetError())
					{
						MessageBoxA(hwnd , "OpenGL Error : Out of memory" , "Error" , MB_OK);
						PostMessage(hwnd, WM_CLOSE, 0, 0);
					}
				}

				MyLoop();//main loop
			}
			else//still loading
			{
				LoadingScreenDisplay();//display loading screen
			}
		}
	}
	

	return 0;
}

/*--------------------function definition----------------------------*/

//dialog procedure
INT_PTR CALLBACK NumberSettingDlg(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam){
	
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			char str[5];
			if (gameState & NUMBER_MOV_SELECT_DLG)
				sprintf(str , "%u" , ge_numMovObjects);
			else if (gameState & NUMBER_STATIC_SELECT_DLG)
				sprintf(str , "%u" , ge_numStaticObjects);
			SetDlgItemTextA(hWndDlg,IDC_TEXTBOX,str);

			RECT rect , prect;
			GetWindowRect(hWndDlg , &rect);
			GetWindowRect(hwnd , &prect);

			unsigned int width = rect.right - rect.left;
			unsigned int height = rect.bottom - rect.top;
			unsigned int pwidth = prect.right - prect.left;
			unsigned int pheight = prect.bottom - prect.top;

			MoveWindow(hWndDlg , prect.left + (pwidth - width) / 2 , prect.top + (pheight - height)/2 , width , height , 0);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				char str[5];
				GetDlgItemTextA(hWndDlg , IDC_TEXTBOX , str , 5);
				int d = atoi(str);
				if (d < 0)
					d = 0;
				if (gameState & NUMBER_MOV_SELECT_DLG)
				{
					ge_numMovObjects = d;
				}
				else if (gameState & NUMBER_STATIC_SELECT_DLG)
				{
					ge_numStaticObjects = d;
				}

				EndDialog(hWndDlg,1);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg,0);
			return TRUE;
		}
		break;
	}
	return 0; 
}
//window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_INPUT:
		if (GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT)
		{
			unsigned int bufferSize;
			//get buffer size
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof (RAWINPUTHEADER));

			if (bufferSize > 40)//invalid
				break;
			else
			{
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (void*)mouseInputBuffer, &bufferSize, sizeof (RAWINPUTHEADER));
				RAWINPUT *rawInput = (RAWINPUT*) mouseInputBuffer;
				if (rawInput->header.dwType == RIM_TYPEMOUSE)
				{
					if (disableMouse)
					{
						mouseState.dx = 0;
						mouseState.dy = 0;
					}
					else
					{
						mouseState.dx = rawInput->data.mouse.lLastX;
						mouseState.dy = rawInput->data.mouse.lLastY;
					}
				}//if (rawInput->header.dwType == RIM_TYPEMOUSE)
			}//else
		}//if (GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT)
		
		break;
	case WM_MOUSEMOVE:
		{
			int x = lParam & 0xffff;
			int y = (lParam & 0xffff0000) >> 16;

			menu->Update(x , y , false);
		}
		break;
	case WM_LBUTTONUP://mouse released
		{
			int x = lParam & 0xffff;
			int y = (lParam & 0xffff0000) >> 16;
			
			/*--------check if mouse clicked any button------*/
			int re  = menu->Update(x , y , true);
			switch(re) 
			{
			case Menu::EXIT://exit button
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return 0;
				
			case Menu::START://start game button
				gameState |= IN_GAME;
				gameState &= ~MENU;
				menu->ResetToMain();
				IngameCleanUp();
				RegisterRawMouseInput(true);
				InitGame();
				break;
			case Menu::NUMBER_MOVING_SELECT://number of moving objects selection button
				gameState |= NUMBER_MOV_SELECT_DLG;
				DialogBoxA(hInst , "NumberSetting" , hwnd , NumberSettingDlg);
				gameState &= ~NUMBER_MOV_SELECT_DLG;
				break;
			case Menu::NUMBER_STATIC_SELECT://number of static objects selection button
				gameState |= NUMBER_STATIC_SELECT_DLG;
				DialogBoxA(hInst , "NumberSetting" , hwnd , NumberSettingDlg);
				gameState &= ~NUMBER_STATIC_SELECT_DLG;
				break;
			}
			
		}
		break;
	case WM_MOVE:
		{
			RECT rect;
			GetWindowRect(hwnd , &rect);
			ClipCursor(&rect);
		}
		break;
	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			RECT rect;
			GetWindowRect(hwnd , &rect);
			ClipCursor(&rect);
		}
		break;
	case WM_KEYDOWN: 
		if (g_loading)
			break;
		switch (wParam) 
		{
		case VK_ESCAPE: 
			if (gameState & IN_GAME)
			{
				if (gameState & MENU)
				{
					//end render menu
					LARGE_INTEGER endMenuPerfCount;
					timer.GetPerformanceCount(endMenuPerfCount);
					dTimeInMenu += timer.GetElapsedTimeF(beginMenuPerfCount,endMenuPerfCount);//how long does menu appears in total

					gameState &= ~MENU;
					RegisterRawMouseInput(true);
				}
				else
				{
					//begin render menu
					timer.GetPerformanceCount(beginMenuPerfCount);

					gameState |= MENU;
					menu->ResetToMain();
					RegisterRawMouseInput(false);
				}
			}
			else
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				return 0;
			} 
			break;
		default:
			MyKeyboard((unsigned char)wParam);
		}

		break;
	case WM_CLOSE:
		/*---------clean up---------------*/
		MyCleanUp();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	}


	return DefWindowProc(hwnd, message, wParam, lParam);
}

bool InitOpenGLContext()
{
	/*---------create window-----------------*/
	
	wndclass.hIconSm       = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIcon         = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInst;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = L"Scene Management";
	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	
	if (RegisterClassEx(&wndclass) == 0)
		return false;
	RECT winRect = {0 , 0 , WINDOW_WIDTH , WINDOW_HEIGHT};
	AdjustWindowRect(&winRect,WINDOW_STYLE,FALSE);

	if (!(hwnd = CreateWindowEx( NULL, L"Scene Management",
		L"Scene Management",
		WINDOW_STYLE,
		0,
		0,
		winRect.right - winRect.left, winRect.bottom - winRect.top, 
		NULL, NULL, hInst, NULL)))
		return false;

	/*---------choose pixel format--------------*/
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd , 0 , sizeof (pfd));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.cDepthBits = 24;//need 24 bit depth buffer
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |	PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.cColorBits = 24;
	
	hDC = GetDC(hwnd);//device context
	if (hDC == NULL)
		return false;
	
	int pixelFormat = ChoosePixelFormat(hDC,&pfd);
	if (pixelFormat == 0)//failed
	{
		pfd.cDepthBits = 16;//try 16 bit depth buffer
		pixelFormat = ChoosePixelFormat(hDC,&pfd);
		if (pixelFormat == 0)//still failed
		{
			ReleaseDC(hwnd , hDC);
			hDC = NULL;
			MessageBoxA(NULL , "Can't init OpenGL" , "Error" , MB_OK);
			return false;
		}
	}

	/*---------create openGL context------------*/
	if(!SetPixelFormat(hDC,pixelFormat,&pfd))
	{
		ReleaseDC(hwnd , hDC);
		hDC = NULL;
		MessageBoxA(NULL , "Can't init OpenGL" , "Error" , MB_OK);
		return false;
	}
	
	hRC = wglCreateContext(hDC);
	hLRC = wglCreateContext(hDC);
	if (hRC == NULL || hLRC == NULL)
	{
		ReleaseDC(hwnd , hDC);
		hDC = NULL;
		MessageBoxA(NULL , "Can't init OpenGL" , "Error" , MB_OK);
		return false;
	}
	

	return true;
}

bool MyInit()
{	
	//create mutexes
	mutex[0] = CreateMutex(0 , 0 , 0);
	mutex[1] = CreateMutex(0 , 0 , 0);
	mutex[2] = CreateMutex(0 , 0 , 0);

#if MULTITHREAD_UPDATE
	//create object updating thread
	objectUpdateThread = CreateThread(0 , 0 , ObjectUpdateThreadFunc , NULL , 0 , 0);
#endif
	
	//init fpsSamples array
	for (unsigned int i = 0; i < NUM_FPS_SAMPLES ; ++i)
		fpsSamples[i] = 0.0f;
	
	/*-------------create window and openGL context----*/
	if (!InitOpenGLContext())
		return false;

	/*------------------------init openGL-----------------------------------------*/
	
	/*--------init main context----------*/
	wglMakeCurrent(hDC , hRC);

	InitExtensions();//check openGL capabilities

	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f , 0.0f , 0.0f ,1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	glFrontFace(GL_CCW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_NORMALIZE);

	//init ambient light
	float ambient[] = {0.25f , 0.25f , 0.25f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT , ambient);

	//main 3d projection matrix
	HQMatrix4rPerspectiveProjRH(CAMERA_FOVY ,(float) WINDOW_WIDTH / WINDOW_HEIGHT , 1.0f , ZFAR , &projMatrix , HQ_RA_OGL);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projMatrix);
	

	//disable vsync
	typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT != NULL)
		wglSwapIntervalEXT(0);

	
	/*-----text writter-----------*/
	textWritter = new TextWritter("../DemoData/font/fontVN.fnt" , 256);
	/*---------menu--------*/

	menu = new Menu(textWritter , 
		"../DemoData/buttonOn.tga" , 
		"../DemoData/buttonOff.tga" , 
		"../DemoData/frame.tga",
		"../DemoData/background.dds",
		"../DemoData/terrain/terrainList.txt");

	/*-------init loading context-------*/
	wglMakeCurrent(hDC , hLRC);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);

	loadingTextTexture = LoadTexture("../DemoData/loadingText.bmp");
	loadingIconTexture = LoadTexture("../DemoData/loadingIcon.tga");
	
	//switch back to main context
	wglMakeCurrent(hDC , hRC);

	return true;
}

void RegisterRawMouseInput(bool _register)
{
	/*------------register raw mouse input------------*/

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 1; 
	rid.usUsage = 2;//mouse 

	if (_register)
	{
		ShowCursor(FALSE);
	        
		rid.dwFlags = RIDEV_NOLEGACY;   //ignore legacy mouse messages
		rid.hwndTarget = hwnd;
	}
	else//unregister 
	{
		ShowCursor(TRUE);
		
		rid.dwFlags = RIDEV_REMOVE; 
		rid.hwndTarget = NULL;
	}

	RECT rect;
	GetWindowRect(hwnd , &rect);
	ClipCursor(&rect);//prevent cursor from moving outside window

	if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) 
	{
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	}
}

void InitGame()
{
	/*-------------reset values---------------------*/
	frames = 0;
	maxFps = -999999.0f;//max frames per second
	minFps = 999999.0f;//min frames per second
	dTimeInMenu = 0.0f;
	errorThreshold = 8.0f;//quadtree error threshold
	camHeight = 20.0f;
	/*-------------init values--------------------------*/
	currentNumMovObjects = ge_numMovObjects;
	currentNumStaticObjects = ge_numStaticObjects;

	/*--------------init state flags----------------*/
	wire = false;//wireframe mode
	disableMouse = false;//mouse input disabled?
	cull = true;//backface culling enabled?
	zbuffer = true;//depth test enabled?
	birdEye = false;//display bird eye radar map?
	drawTerrainSkirt = true;//display terrain skirt?
	geoMorpth = true;//geomorphing enabled?
	autoRotate = false;//auto rotate around Oy
	displayHud = true;//display HUD? 
	displayBBox = false;//display bounding box of quadtree node
	
	renderMode = QUAD_TREE;
	if (currentNumMovObjects + currentNumStaticObjects > 0)//only enable octree if there're objects in scene
		renderMode |= OCTREE;
#if MULTITHREAD_UPDATE
	g_objUpdateThreadState = 0;
#endif


	
	//change opengl state
	if (wire)
	{
		glDisable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
	}
	if (cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	if (zbuffer)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	

	/*--start loading thread--------*/
	g_loadingFinish = false;
	g_loading = true;

	wglMakeCurrent(hDC , hLRC);//make loading context this thread's main rendering context so that it can render loading icon

	CreateThread(0 , 0 , InitGameThreadFunc , NULL , 0 , 0);//start thread
	
}

DWORD WINAPI InitGameThreadFunc(void *arg)
{
	srand((unsigned int )time(NULL));

	wglMakeCurrent(hDC , hRC);//make main context the loading thread's main rendering context so that textures can be loaded in this thread.

	SelectedTerrainOption option;
	menu->GetSelectedTerrainOption(option);

	/*---------------------------*/
	/*---------init terrain & camera------------*/
	//get current directory
	DWORD size = GetCurrentDirectory(0 , 0);
	wchar_t *currentDir = new wchar_t[size];
	GetCurrentDirectory(size , currentDir);

	
	SetCurrentDirectoryA("../DemoData/terrain");
	SetCurrentDirectoryA(option.name);

	//simple terrain

	simpleTerrain = new SimpleTerrainTriStrip(option.heightmap , option.simpleTexture , TERRAIN_SCALE_FACTOR , option.heightMapLoadMethod);
	
	//quadtree terrain
	ge_terrain = quadtreeTerrain = new ChunkQuadTreeTerrain(
		option.heightmap , option.textures , option.level,
		errorThreshold , CAMERA_FOVX , WINDOW_HEIGHT,
		TERRAIN_SCALE_FACTOR  , option.heightMapLoadMethod); 
	((ChunkQuadTreeTerrain*)ge_terrain)->EnableGeoMorph(geoMorpth);
		
	
	//reset current directory
	SetCurrentDirectory(currentDir);
	delete[] currentDir;

	ge_worldMaxHeight = ge_terrain->GetMaxHeight() + 1000.0f;

	//camera
	camera = new Camera(15.0f , 15.0f, camHeight);
	camVelocity = option.camVelocity;
	
	//birdEye camera view and ortho projection matrices
	HQVector4 birdEyePos((ge_terrain->GetStartX() + ge_terrain->GetMaxX()) / 2.0f , 
				ge_worldMaxHeight, 
				(ge_terrain->GetStartZ() + ge_terrain->GetMaxZ()) / 2.0f );
	birdEyeLookAt.Set(birdEyePos.x , 
						ge_terrain->GetMinHeight() ,
						birdEyePos.z);
	HQMatrix4rLookAtRH(&birdEyePos , &birdEyeLookAt , &HQVector4(0,0,1) , &birdEyeViewMatrix);

	HQMatrix4rOrthoProjRH(ge_terrain->GetMaxX() - ge_terrain->GetStartX() , 
						ge_terrain->GetMaxZ() - ge_terrain->GetStartZ(),
						1.0f , ge_worldMaxHeight - ge_terrain->GetMinHeight() + 10.0f, &birdEyeProjMatrix[0] , HQ_RA_OGL);

	HQMatrix4rOrthoProjRH(ge_terrain->GetMaxX() - ge_terrain->GetStartX() , 
						ge_terrain->GetMaxZ() - ge_terrain->GetStartZ(),
						1.0f , 2 * ZFAR + 10.0f, &birdEyeProjMatrix[1] , HQ_RA_OGL);
	
	//scene manager
	if (renderMode & OCTREE)
		sceneManager = new OctreeSceneManager(
			ge_terrain->GetStartX() , ge_terrain->GetMaxX() ,
			ge_terrain->GetMinHeight() , ge_worldMaxHeight ,
			ge_terrain->GetStartZ() , ge_terrain->GetMaxZ() ,
			menu->GetSelectedOctreeLevels());

	//models
	model = new Model *[NUM_SPHERE_MODELS + NUM_TREE_MODELS];
	for (unsigned int i = 0 ; i < NUM_SPHERE_MODELS ; ++i)
		model[i] = new Model(sphereModelFiles[i]);

	for (unsigned int i = 0 ; i < NUM_TREE_MODELS ; ++i)
		model[i + NUM_SPHERE_MODELS] = new Model(treeModelFiles[i]);

	//spheres
	sphere = (SphereObject *) malloc(currentNumMovObjects * sizeof(SphereObject));
	for (unsigned int i = 0 ; i < currentNumMovObjects ; ++i)
	{
		int index = rand() % NUM_SPHERE_MODELS;//random model
		float scale = rand() % 4 + 1.0f;//scale factor
		//random position
		float sphereRadius = model[index]->GetBoundingSphere().radius * scale;
		HQVector4 position;
		unsigned int row = rand() % (ge_terrain->GetNumVertZ() - 1) + 1;
		unsigned int col = rand() % (ge_terrain->GetNumVertX() - 1) + 1;
		position.x = ge_terrain->GetStartX() + ge_terrain->GetCellSizeX() * col;
		position.z = ge_terrain->GetStartZ() + ge_terrain->GetCellSizeZ() * row;
		
		/*-----------check if sphere is not inside world----------------*/
		if ((position.x - sphereRadius) < ge_terrain->GetStartX() )
			position.x = ge_terrain->GetStartX() + sphereRadius + 0.0001f;
		else if ((position.x + sphereRadius) > ge_terrain->GetMaxX() )
			position.x = ge_terrain->GetMaxX() - sphereRadius - 0.0001f;
		
		if ((position.z - sphereRadius) < ge_terrain->GetStartZ() )
			position.z = ge_terrain->GetStartZ() + sphereRadius + 0.0001f;
		else if ((position.z + sphereRadius) > ge_terrain->GetMaxZ() )
			position.z = ge_terrain->GetMaxZ() - sphereRadius - 0.0001f;

		position.y = ge_terrain->GetHeight(position.x , position.z) + (rand() % 900 + 100.0f);

		if ((position.y + sphereRadius) > ge_worldMaxHeight)
			position.y = ge_worldMaxHeight - sphereRadius - 0.0001f;
		
		
		//random velocity
		HQVector4 velocity;
		velocity.x = (float)(rand() % 100);
		velocity.y = (float)(rand() % 100);
		velocity.z = (float)(rand() % 100);
		velocity.Normalize(); 
		
		//call constructor
		new (sphere + i) SphereObject(
			sceneManager , 
			model[index] , 
			position,
			velocity,
			scale);
	}
	
	//trees
	tree = (TreeObject *) malloc(currentNumStaticObjects * sizeof(TreeObject));
	for (unsigned int i = 0 ; i < currentNumStaticObjects ; ++i)
	{
		int index = rand() % NUM_TREE_MODELS + NUM_SPHERE_MODELS;//random model
		unsigned int row = rand() % (ge_terrain->GetNumVertZ() - 1) + 1;
		unsigned int col = rand() % (ge_terrain->GetNumVertX() - 1) + 1;
		
		//call constructor
		new (tree + i) TreeObject(
			sceneManager , 
			model[index] , 
			row , col ,
			option.treeScale);
	}
	
	//sky
	sky = new SkyDome(ZFAR , 50 , 50 , 
		"../DemoData/sky/waterPosX.bmp" ,
		"../DemoData/sky/waterNegX.bmp" ,
		"../DemoData/sky/waterPosY.bmp" ,
		"../DemoData/sky/waterNegY.bmp" ,
		"../DemoData/sky/waterPosZ.bmp" ,
		"../DemoData/sky/waterNegZ.bmp" );

	//done

	wglMakeCurrent(NULL , NULL);

	WaitForSingleObject(mutex[2] , INFINITE);
	g_loadingFinish = true;
	ReleaseMutex(mutex[2]);

	return 0;
}

void MyUpdate()
{
	//update camera
	UpdateCamera();
	viewProjMatrix = camera->GetViewMatrix() * projMatrix;
	HQMatrix4rGetFrustum(&viewProjMatrix , viewFrustum , HQ_RA_OGL);

#if MULTITHREAD_UPDATE
	WaitForSingleObject(mutex[0] , INFINITE);
	g_objUpdateThreadState = START_UPDATE_OBJ;//request worker thread for updating objects
	ReleaseMutex(mutex[0]);
#endif

	
	//update terrain
	ge_terrain->Update(camera->GetPosition() , viewFrustum);

#if !MULTITHREAD_UPDATE
	/*-------update objects----------*/
	if (renderMode & OCTREE)
	{
		for (unsigned int i = 0 ; i < currentNumMovObjects ; ++i)
			sphere[i].Update(dTime);
	}
	else
	{
		for (unsigned int i = 0 ; i < currentNumMovObjects ; ++i)
			sphere[i].UpdateWithoutSceneManager(dTime);
	}
#else
	
	int l_objUpdateState = 0;
	
	/*-----wait until object updating thread finishes its work---------*/
	do 
	{	
		Sleep(1);

		WaitForSingleObject(mutex[0] , INFINITE);
		l_objUpdateState = g_objUpdateThreadState;
		ReleaseMutex(mutex[0]);
	} while(l_objUpdateState != FINISH_UPDATE_OBJ);
#endif

}

#if MULTITHREAD_UPDATE
DWORD WINAPI ObjectUpdateThreadFunc(void *arg)
{
	int l_state;
	bool l_running;
	WaitForSingleObject(mutex[1] , INFINITE);
	l_running  = g_running;
	ReleaseMutex(mutex[1]);

	while(l_running)//is application still running?
	{
		WaitForSingleObject(mutex[0] , INFINITE);
		l_state = g_objUpdateThreadState;
		ReleaseMutex(mutex[0]);
		
		if (l_state == START_UPDATE_OBJ)//object updating request from main thread?
		{
			/*-------update objects----------*/
			if (renderMode & OCTREE)
			{
				for (unsigned int i = 0 ; i < currentNumMovObjects ; ++i)
					sphere[i].Update(dTime);
			}
			else
			{
				for (unsigned int i = 0 ; i < currentNumMovObjects ; ++i)
					sphere[i].UpdateWithoutSceneManager(dTime);
			}

			WaitForSingleObject(mutex[0] , INFINITE);
			g_objUpdateThreadState = FINISH_UPDATE_OBJ;//notify main thread that objects is updated
			ReleaseMutex(mutex[0]);
		}
		else
			Sleep(1);

		WaitForSingleObject(mutex[1] , INFINITE);
		l_running  = g_running;//update running state of app
		ReleaseMutex(mutex[1]);
	}

	return 0;
}
#endif

void MyDisplay()
{
	/*--------clear screen-----------*/
	glClearColor(0.0f , 0.0f , 0.0f ,1.0f);
	GLenum flags = GL_DEPTH_BUFFER_BIT;
	if (wire)
		flags |= GL_COLOR_BUFFER_BIT;
	glClear(flags);

	if (gameState & IN_GAME)//render terrain & objects
		IngameDisplay();
	if (gameState & MENU)//display menu
	{
		if (wire)//disable wireframe mode
			glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
		
		menu->Render((gameState & IN_GAME) != 0);

		if (wire)//re enable wireframe mode
		{
			glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
			glDisable(GL_TEXTURE_2D);
		}
	}

	SwapBuffers(hDC);
}

void IngameDisplay()
{	
	glViewport(0 , 0 , WINDOW_WIDTH ,WINDOW_HEIGHT);
	
	/*----draw terrain-----------*/
	ge_terrain->Render();
	
	/*-----------draw objects------------------*/
	//enable lighting
	
	const float pos[] = {1 , 1 , 1 , 0};
	glLightfv(GL_LIGHT0 , GL_POSITION , pos);
	glEnable(GL_LIGHT0);

	glEnable(GL_LIGHTING);
	
	//render objects
	if (renderMode & OCTREE)
	{
		sceneManager->Render(viewFrustum);
		numTriangles = sceneManager->GetNumTriangles();
	}
	else
	{
		numTriangles = 0;
		for (unsigned int i = 0 ; i <  currentNumMovObjects ; ++i)
		{
			sphere[i].Render();
			numTriangles += sphere[i].GetNumTriangles();
		}
		for (unsigned int i = 0 ; i <  currentNumStaticObjects ; ++i)
		{
			tree[i].Render();
			numTriangles += tree[i].GetNumTriangles();
		}
	}
	glDisable(GL_LIGHTING);
	
	//draw the sky
	if (!wire)
		sky->Render(camera->GetPosition());
	
	/*--------debug puspose rendering----------*/
	//display quadtree & octree bounding box
	if (displayBBox)
	{
		if (renderMode & QUAD_TREE)
			((ChunkQuadTreeTerrain*)ge_terrain)->RenderBoundingBox();
		if (renderMode & OCTREE)
			sceneManager->RenderAgain(true);
	}
	if (birdEye)//render from bird eye point of view
	{
		BirdEyeDisplay();
	}
	
	/*----------draw HUD------------*/
	DisplayHUD();

	
	

}

void BirdEyeDisplay()
{
	glClearColor(0.0f , 1.0f , 0.0f ,1.0f);

	glViewport(WINDOW_WIDTH - BIRD_EYE_CAM_WIDTH , 
		WINDOW_HEIGHT - BIRD_EYE_CAM_HEIGHT , 
		BIRD_EYE_CAM_WIDTH , 
		BIRD_EYE_CAM_HEIGHT);
	glScissor(WINDOW_WIDTH - BIRD_EYE_CAM_WIDTH , 
		WINDOW_HEIGHT - BIRD_EYE_CAM_HEIGHT , 
		BIRD_EYE_CAM_WIDTH , 
		BIRD_EYE_CAM_HEIGHT);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);


	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(birdEyeProjMatrix[0]);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(birdEyeViewMatrix);
	
	
	/*--------draw bounding boxes of terrain-----*/
	if (renderMode & QUAD_TREE)
		((ChunkQuadTreeTerrain*)ge_terrain)->RenderBoundingBox();
	/*-------draw objects-------------------*/
	if (renderMode & OCTREE)
	{
		sceneManager->RenderAgain(true);//draw octree node bounding box
		sceneManager->RenderAgain(false);//draw object
	}
	else
	{
		for (unsigned int i = 0 ; i <  currentNumMovObjects ; ++i)
		{
			sphere[i].Render();
		}
	}
	/*--------draw camera frustum---------------*/
	glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);

	HQVector4 pos;
	HQMatrix4 invCamView , subView;
	/*-------create matrices--------------*/
	
	pos.Set(birdEyeLookAt.x , 
		camera->GetPosition().y + ZFAR + 1.0001f, 
		birdEyeLookAt.z );

	HQMatrix4rLookAtRH(&pos , &birdEyeLookAt , &HQVector4(0,0,1) , &subView);

	HQMatrix4Inverse(&camera->GetViewMatrix() , &invCamView);//inverse of camera 's view matrix

	/*-------------------*/
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(subView);
	glMultMatrixf(invCamView);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(birdEyeProjMatrix[1]);

	
	/*
	2 -------------- 1
	  \			   /
	   \		  /
	  3 \--------/0
	*/

	glBegin(GL_QUADS);


	//upper side of view frusum
	glColor3f(0 , 0 , 1);

	glVertex3f(vFNPOffsetX , vFNPOffsetY , -1.0f) ; 

	glVertex3f(vFNPOffsetX * ZFAR , vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(-vFNPOffsetX * ZFAR , vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(-vFNPOffsetX , vFNPOffsetY , -1.0f) ; 
	
	

	//bottom side of view frustum
	glColor3f(0 , 0 , 0);

	glVertex3f(vFNPOffsetX , -vFNPOffsetY , -1.0f) ; 

	glVertex3f(vFNPOffsetX * ZFAR , -vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(-vFNPOffsetX * ZFAR , -vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(-vFNPOffsetX , -vFNPOffsetY , -1.0f) ; 
	
	//far side of view frustum
	glColor3f(1 , 0.5f , 0);

	glVertex3f(vFNPOffsetX * ZFAR , -vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(vFNPOffsetX * ZFAR , vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(-vFNPOffsetX * ZFAR , vFNPOffsetY * ZFAR , -ZFAR) ; 

	glVertex3f(-vFNPOffsetX * ZFAR , -vFNPOffsetY * ZFAR , -ZFAR) ; 

	//near side of view frustum

	glVertex3f(vFNPOffsetX  , -vFNPOffsetY  , -1.0f) ;

	glVertex3f(vFNPOffsetX , vFNPOffsetY  , -1.0f) ; 

	glVertex3f(-vFNPOffsetX  , vFNPOffsetY  , -1.0f) ; 

	glVertex3f(-vFNPOffsetX  , -vFNPOffsetY  , -1.0f) ; 



	glEnd();
	
	glPopAttrib();
	glColor3f(1 , 1 , 1);
	/*--------------------------------*/
	

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void DisplayHUD()
{
	if (wire)//disable wireframe mode
	{
		glEnable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
	}

	Mode2D::Begin();//start 2D mode
	TextWritter::BeginRender();//begin drawing text

	if (displayHud)//display HUD
	{
		unsigned int octreeLevel = (sceneManager != NULL) ? (sceneManager->GetMaxDepth() + 1) : 0;
		unsigned int numVisibleObjs = (renderMode & OCTREE)? sceneManager->GetNumVisbleObjects() : currentNumMovObjects + currentNumStaticObjects;//number of visible objects
		unsigned int numObjTriangles = numTriangles;
		numTriangles += ge_terrain->GetNumTriangles();
		char str[256];
		sprintf(str , "quadtree error threshold : %.0f\nquadtree levels : %u\noctree levels : %u\nfps : %.2f\naverage fps : %.2f\nmax fps : %.2f\nmin fps : %.2f\ntriangles : %u\nrendered objects : %u\nobjects' triangles : %u" , 
			errorThreshold , quadtreeTerrain->GetMaxLevel() + 1 ,  octreeLevel , fps , 
			avgFps , maxFps , minFps , 
			numTriangles , 
			numVisibleObjs,
			numObjTriangles);
		textWritter->Render(str ,10.0f , 120.0f , FONT_COLOR(255 , 255 , 0 , 255) , 30.0f , 1.0f);
		
		if (renderMode & QUAD_TREE)
		{
			textWritter->Render("quadtree is on" , 10 , 30 , FONT_COLOR(0 , 255 , 0 , 255) , 30.0f , 1.0f);
			if (geoMorpth)
				textWritter->Render("geomorph is on" , 10 , 90 , FONT_COLOR(0 , 255 , 0 , 255) , 30.0f , 1.0f);
			else
				textWritter->Render("geomorph is off" , 10 , 90 , FONT_COLOR(255 , 0 , 0 , 255) , 30.0f , 1.0f);
		}
		else
			textWritter->Render("quadtree is off" , 10 , 30 , FONT_COLOR(255 , 0 , 0 , 255) , 30.0f , 1.0f);

		if (renderMode & OCTREE)
		{
			textWritter->Render("octree is on" , 10 , 60 , FONT_COLOR(0 , 255 , 0 , 255) , 30.0f , 1.0f);
		}
		else
			textWritter->Render("octree is off" , 10 , 60 , FONT_COLOR(255 , 0 , 0 , 255) , 30.0f , 1.0f);
		
	
		
	}
	else //display only fps
	{
		char str[256];
		sprintf(str , "fps : %.2f\naverage fps : %.2f\nmax fps : %.2f\nmin fps : %.2f\n" , 
			fps , avgFps , maxFps , minFps );
		textWritter->Render(str , 10.0f , 30.0f , FONT_COLOR(255 , 255 , 0 , 255) , 30.0f , 1.0f);
		
	}
	TextWritter::EndRender();//stop drawing text
	Mode2D::End();//stop 2D mode

	if (wire)//re enable wireframe mode
	{
		glDisable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
	}
}

void LoadingScreenDisplay()
{
	glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
	glClearColor(1.0f , 1.0f , 1.0f ,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	const HQRect fullScreen = {0 , 0 , WINDOW_WIDTH , WINDOW_HEIGHT};
	const int iconHeight = WINDOW_HEIGHT / 10;
	const int iconWidth = iconHeight;//WINDOW_WIDTH / 10;
	const int iconCenterX = WINDOW_WIDTH /2;
	const int iconCenterY = WINDOW_HEIGHT / 2;
	const HQRect iconRect = {- iconWidth/2 , - iconHeight/2 , iconWidth , iconHeight};
	static unsigned int angle = 0;
	angle += 12 ;
	if (angle > 360)
		angle -= 360;

	float roundedAngle = (float)(angle / 30 * 30);
	
	Mode2D::Begin();

	Mode2D::DrawRect(fullScreen , loadingTextTexture);//loading text

	glMatrixMode(GL_MODELVIEW);
	glTranslatef((float)iconCenterX, (float)iconCenterY , 0);
	glRotatef(roundedAngle , 0 , 0, 1);
	Mode2D::DrawRect(iconRect , loadingIconTexture);//loading icon

	Mode2D::End();

	SwapBuffers(hDC);

	Sleep(13);
}

void MyLoop()
{
	bool isInMenu = (gameState & MENU) != 0;
	if (!isInMenu)//only calculate fps and update game when not in menu
	{
		if (frames == 0)
		{
			timer.StartTimer();//start point for calculating total time in-game
			lastPerfCount = timer.GetStartCount();
		}
		if (frames >= NUM_FPS_SAMPLES)
		{
			if (minFps > fps)
				minFps = fps;
			if (maxFps < fps)
				maxFps = fps;
		}

		timer.GetPerformanceCount(curPerfCount);//get current performance count
		dTime = timer.GetElapsedTimeF(lastPerfCount , curPerfCount);
		lastPerfCount = curPerfCount;
	
		MyUpdate();//only update when menu not appears
	}
	else
		timer.GetPerformanceCount(lastPerfCount);

	MyDisplay();
	
	if (!isInMenu)//only calculate fps when not in menu
	{
		//calculate frames per second
		timer.StopTimer();
		float totalTime = timer.GetElapsedTimeF() - dTimeInMenu;
		frames ++;
		avgFps = frames / totalTime;

		dTime = timer.GetElapsedTimeF(curPerfCount , timer.GetStopCount());
		if (dTime <= 0.0)
			fpsSamples[frames % NUM_FPS_SAMPLES] = 0.0f;
		else
		{
			unsigned int changeSample = frames % NUM_FPS_SAMPLES;
			float oldVal = fpsSamples[changeSample];
			fpsSamples[changeSample] = 1.0f / dTime;
			fps += (fpsSamples[changeSample] - oldVal) / NUM_FPS_SAMPLES;
		}

		if (frames == MAX_FRAMES)//maximum value
		{
			//restart timer
			frames = 0;
		}
	}
}

void UpdateCamera()
{
	if (cameraState & CAMERA_MOVE_UP)
	{
		DisableFlag(cameraState , CAMERA_MOVE_UP);
		
		camHeight += dHeight;
		if (camHeight > ge_worldMaxHeight)
			camHeight = ge_worldMaxHeight;
		
		camera->SetCameraHeight(camHeight);
	}
	else if (cameraState & CAMERA_MOVE_DOWN)
	{
		DisableFlag(cameraState , CAMERA_MOVE_DOWN);
		camHeight -= dHeight;
		if (camHeight < MinCamHeight)
			camHeight = MinCamHeight;

		camera->SetCameraHeight(camHeight);
	}
	if (cameraState & CAMERA_MOVE_FORWARD)
	{
		DisableFlag(cameraState , CAMERA_MOVE_FORWARD);
		camera->Move(camVelocity);
	}
	else if (cameraState & CAMERA_MOVE_BACK)
	{
		DisableFlag(cameraState , CAMERA_MOVE_BACK);
		camera->Move(-camVelocity);
	}
	
	if (cameraState & CAMERA_MOVE_LEFT)
	{
		DisableFlag(cameraState , CAMERA_MOVE_LEFT);
		camera->Strafe(-camVelocity);
	}
	else if (cameraState & CAMERA_MOVE_RIGHT)
	{
		DisableFlag(cameraState , CAMERA_MOVE_RIGHT);
		camera->Strafe(camVelocity);
	}

	if (cameraState & CAMERA_TURN_LEFT)
	{
		DisableFlag(cameraState , CAMERA_TURN_LEFT);
		camera->RotateX(dAngle);
	}
	else if (cameraState & CAMERA_TURN_RIGHT)
	{
		DisableFlag(cameraState , CAMERA_TURN_RIGHT);
		camera->RotateX(-dAngle);
	}

	if (cameraState & CAMERA_TURN_UP)
	{
		DisableFlag(cameraState , CAMERA_TURN_UP);
		camera->RotateY(dAngle);
	}
	else if (cameraState & CAMERA_TURN_DOWN)
	{
		DisableFlag(cameraState , CAMERA_TURN_DOWN);
		camera->RotateY(-dAngle);
	}
	
	if (mouseState.dx != 0)
	{
		camera->RotateX(-mouseState.dx * dAngle);
		mouseState.dx = 0;
	}
	if (mouseState.dy != 0)
	{
		camera->RotateY(-mouseState.dy * dAngle);
		mouseState.dy = 0;
	}
	
	if (autoRotate)
		camera->RotateX(dAngle * dTime * desiredUPS);

	camera->Update();
	camera->Move(0.0f);
	camera->Strafe(0.0f);
}


void MyKeyboard(unsigned char key)
{
	if (!(gameState & IN_GAME))
		return;
	switch(key)
	{
	case 'c' : case 'C':
		wire = !wire;
		if (wire)
		{
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
		}
		else
		{
			glEnable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
		}
		break;
	case 'w' : case 'W':
		EnableFlag(cameraState , CAMERA_MOVE_FORWARD);
		break;
	case 's' : case 'S':
		EnableFlag(cameraState , CAMERA_MOVE_BACK);
		break;
	case 'a' : case 'A':
		EnableFlag(cameraState , CAMERA_MOVE_LEFT);
		break;
	case 'd' : case 'D':
		EnableFlag(cameraState , CAMERA_MOVE_RIGHT);
		break;
	case 'q' : case 'Q':
		EnableFlag(cameraState , CAMERA_MOVE_UP);
		break;
	case 'e' : case 'E':
		EnableFlag(cameraState , CAMERA_MOVE_DOWN);
		break;
	case 'z' : case 'Z':
		cull = !cull;
		if (cull)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		break;
	/*
	case 'x' : case 'X' :
		zbuffer = !zbuffer;
		if (zbuffer)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		break;
	*/
	case 'f' : case 'F':
		if (!(renderMode & QUAD_TREE))//not using quadtree
			break;
		if (errorThreshold < 1000.0f)
			errorThreshold++;
		((ChunkQuadTreeTerrain *)ge_terrain)->SetT(errorThreshold);
		break;
	case 'G' : case 'g':
		if (!(renderMode & QUAD_TREE))//not using quadtree
			break;
		if (errorThreshold > 1.0f)
			errorThreshold--;
		((ChunkQuadTreeTerrain *)ge_terrain)->SetT(errorThreshold);
		break;
	case 'r' : case 'R':
		if (!(renderMode & QUAD_TREE))//not using quadtree
			break;
		drawTerrainSkirt = !drawTerrainSkirt;
		((ChunkQuadTreeTerrain *)ge_terrain)->EnableDrawSkirt(drawTerrainSkirt);
		break;
	case '1'://quadtree
		if (renderMode & QUAD_TREE)
		{
			//disable
			ge_terrain = simpleTerrain;
			renderMode &= ~QUAD_TREE;
		}
		else
		{
			ge_terrain = quadtreeTerrain;
			renderMode |= QUAD_TREE;
		}
		break;
	case '2'://octree
		if (currentNumMovObjects + currentNumStaticObjects == 0)//this key only usable if there're objects in scene
			break;
		if (renderMode & OCTREE)
		{
			//disable
			renderMode &= ~OCTREE;
		}
		else
		{
			renderMode |= OCTREE;
		}
		break;
	case '\t':
		birdEye = ! birdEye;
		break;
	case 'm' : case 'M':
		disableMouse = !disableMouse;
		break;
	case VK_OEM_3: //case '`': case '~':
		displayHud = !displayHud;
		break;
	case 'v' : case 'V':
		if (!(renderMode & QUAD_TREE))//not using quadtree
			break;
		geoMorpth = !geoMorpth;
		((ChunkQuadTreeTerrain*)ge_terrain)->EnableGeoMorph(geoMorpth);
		break;
	case 'b' : case 'B':
		displayBBox = !displayBBox;
		break;
	case 32 : //space
		autoRotate = !autoRotate;
		break;

	case VK_UP:
		EnableFlag(cameraState , CAMERA_TURN_UP);
		break;
	case VK_DOWN:
		EnableFlag(cameraState , CAMERA_TURN_DOWN);
		break;
	case VK_LEFT:
		EnableFlag(cameraState , CAMERA_TURN_LEFT);
		break;
	case VK_RIGHT:
		EnableFlag(cameraState , CAMERA_TURN_RIGHT);
		break;
	default:
		break;
	}
}


void MyCleanUp()
{ 

#if MULTITHREAD_UPDATE
	WaitForSingleObject(mutex[1] , INFINITE);
	g_running = false;
	ReleaseMutex(mutex[1]);

	WaitForSingleObject(objectUpdateThread , INFINITE);
#endif

	CloseHandle(mutex[0]);
	CloseHandle(mutex[1]);
	CloseHandle(mutex[2]);

	IngameCleanUp();
	
	SafeDelete(menu);
	SafeDelete(textWritter);
	
	/*-----delete loading context's resource-------*/
	if (hLRC)
	{
		BOOL re = wglMakeCurrent(hDC , hLRC);

		if(loadingTextTexture)
			glDeleteTextures(1 , &loadingTextTexture);
		if(loadingIconTexture)
			glDeleteTextures(1 , &loadingIconTexture);
	}

	wglMakeCurrent(NULL , NULL);
	if (hRC)
	{
		wglDeleteContext(hRC);
		wglDeleteContext(hLRC);
	}
	if (hDC)
		ReleaseDC(hwnd , hDC);

	ShowCursor(TRUE);
	ClipCursor(NULL);

}

void IngameCleanUp()
{
	SafeDelete(simpleTerrain);
	SafeDelete(quadtreeTerrain);
	SafeDelete(camera);
	SafeDelete(sceneManager);
	SafeDelete(sky);
	if (sphere)
	{
		for(unsigned int i = 0 ; i < currentNumMovObjects ; ++i)
			sphere[i].~SphereObject();
		free(sphere);
		sphere = NULL;
	}
	if (tree)
	{
		for(unsigned int i = 0 ; i < currentNumStaticObjects ; ++i)
			tree[i].~TreeObject();
		free(tree);
		tree = NULL;
	}

	if (model)
	{
		for (unsigned int i = 0 ; i < NUM_SPHERE_MODELS + NUM_TREE_MODELS ; ++i)
		{
			SafeDelete(model[i]);
		}
		delete[] model;
		model = NULL;
	}
}

