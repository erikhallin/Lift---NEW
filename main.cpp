#define _WIN32_WINNT 0x0500 //Needed for GetConsoleWindow()
#include <iostream>
#include <windows.h>
#include <gl/gl.h>
#include <fcntl.h> //for console
#include <stdio.h>
#include "definitions.h"
#include "game.h"
#include "resource.h"

bool g_keys[256],g_mouse_but[4];
int  g_mouse_pos[2];
int  g_window_width=1200;//1920;
int  g_window_height=1080;//379;//1080;

using namespace std;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    string command_line=lpCmdLine;
    bool debug_mode=false;
    bool test_level=false;
    if(command_line=="debug") debug_mode=true;
    if(command_line=="test_level") test_level=true;

    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    //register window class
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_LIF));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "PlainSpaceJourney";
    wcex.hIconSm = LoadIcon(NULL, NULL);

    if (!RegisterClassEx(&wcex))
        return 0;

    if(!debug_mode)
    {
        //Detect screen resolution
        RECT desktop;
        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        // (horizontal, vertical)
        g_window_width = desktop.right;
        g_window_height = desktop.bottom;
    }

    //if debug mode start console
    if(debug_mode)
    {
        //Open a console window
        AllocConsole();
        //Connect console output
        HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        int hCrt          = _open_osfhandle((long) handle_out, _O_TEXT);
        FILE* hf_out      = _fdopen(hCrt, "w");
        setvbuf(hf_out, NULL, _IONBF, 1);
        *stdout = *hf_out;
        //Connect console input
        HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
        hCrt             = _open_osfhandle((long) handle_in, _O_TEXT);
        FILE* hf_in      = _fdopen(hCrt, "r");
        setvbuf(hf_in, NULL, _IONBF, 128);
        *stdin = *hf_in;
        //Set console title
        SetConsoleTitle("Debug Console");
        HWND hwnd_console=GetConsoleWindow();
        MoveWindow(hwnd_console,g_window_width,0,680,510,TRUE);

        cout<<"Software started\n";
        cout<<"Version: "<<_version<<endl;
    }
    else
    {
        ShowCursor(FALSE);//hide cursor
        //ShowWindow(GetConsoleWindow(),SW_HIDE);//hide console
    }

    //create main window
    hwnd = CreateWindowEx(0,
                          "PlainSpaceJourney",
                          "PlainSpaceJourney",
                          WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, //WS_OVERLAPPEDWINDOW for window
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          g_window_width,
                          g_window_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    //enable OpenGL for the window
    EnableOpenGL(hwnd, &hDC, &hRC);

    //main loop
    for(int i=0;i<256;i++) g_keys[i]=false; //reset keys
    game Game;
    int screen_size[2]={g_window_width,g_window_height};
    if( !Game.pre_init(screen_size) )//to load and draw loading screen
    {
        //close window
        DisableOpenGL(hwnd, hDC, hRC);
        DestroyWindow(hwnd);

        cout<<"ERROR: Game could not initialize\n";
        if(debug_mode) system("PAUSE");
        return 1;
    }
    //show loading screen
    SwapBuffers(hDC);
    if( !Game.init(g_keys,screen_size,test_level) )
    {
        //close window
        DisableOpenGL(hwnd, hDC, hRC);
        DestroyWindow(hwnd);

        cout<<"ERROR: Game could not initialize\n";
        if(debug_mode) system("PAUSE");
        return 1;
    }
    clock_t time_now=clock();
    clock_t time_last=time_now;
    //clock_t time_sum=0;
    clock_t time_step=_world_step_time*1000.0;//10.0;//0.010 ms -> 100 updates per sec
    bool update_screen=true;
    //int counter_fps=0;
    while (!bQuit)
    {
        //check for messages
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            //handle or dispatch messages
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            //quit test, temp
            //if(g_keys[VK_ESCAPE]) bQuit=TRUE;

            //update only when 20ms have passed, and until game have catched up with the current time
            time_now=clock();
            while( time_last+time_step <= time_now )
            {
                time_last+=time_step;

                bool quit_game=false;
                if( !Game.update(quit_game) )//static update
                {
                    //require reset of game
                    if( !Game.init(g_keys,screen_size,test_level,false) )
                    {
                        cout<<"ERROR: Game could not reinitialize\n";
                        if(debug_mode) system("PAUSE");
                        return 1;
                    }
                }
                if(quit_game) bQuit=TRUE;

                update_screen=true;
            }
            //draw, if anything updated
            if(update_screen)
            {
                update_screen=false;

                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                glLoadIdentity();
                Game.draw();
                //glFlush();
                SwapBuffers(hDC);
            }

            /*//update only in 50 ps
            time_now=clock();
            if( (float)time_last+time_step <= (float)time_now )
            {
                time_last=time_now;
                Game.update();
            }
            //draw
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glLoadIdentity();
            Game.draw();
            glFlush();
            SwapBuffers(hDC);*/

            //Game.update();
            //cout<<"Update done\n";
            //Game.draw();
            //cout<<"Draw done\n";

            //SwapBuffers(hDC);

            /*//FPS lock
            //int counter=0;
            time_now=clock();
            while( time_last+time_step > time_now )
            //while( (float)time_last+0.01666667*(float)CLOCKS_PER_SEC >= (float)time_now )//to 60  (1/60 = 0.0166667)
            //while( time_last+0.0100000*(float)CLOCKS_PER_SEC >= time_now )//to 100 (1/100 = 0.01)
            //while( time_last+0.0083333*(float)CLOCKS_PER_SEC > time_now )//to 120 (1/100 = 0.0083333)
            {
                //draw
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glLoadIdentity();
                Game.draw();
                glFlush();
                SwapBuffers(hDC);

                time_now=clock();
                //cout<<time_now<<endl;
                //counter++;
            }
            time_last=time_now;
            //cout<<counter<<endl;*/

            //SwapBuffers(hDC);

            /*//calc FPS
            if(counter_fps>=60)
            {
                float fps_avg=1.0/(time_sum/(float)CLOCKS_PER_SEC/(float)counter_fps);
                cout<<"FPS: "<<fps_avg<<endl;
                counter_fps=0;
                time_sum=0;
            }
            else
            {
                time_sum+=time_now-time_last;
                //cout<<"diff: "<<time_now<<" - "<<time_last<<" = "<<(time_now-time_last)<<endl;
                counter_fps++;
            }*/
        }
    }

    //shutdown OpenGL
    DisableOpenGL(hwnd, hDC, hRC);

    //destroy the window explicitly
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        }
        break;

        case WM_DESTROY:
        {
            return 0;
        }
        break;

        case WM_MOUSEMOVE:
        {
             g_mouse_pos[0]=LOWORD(lParam);
             g_mouse_pos[1]=HIWORD(lParam)+28;//-22 pixel shift to get same coord as drawing
        }
        break;

        case WM_LBUTTONDOWN:
        {
             g_mouse_but[0]=true;
        }
        break;

        case WM_LBUTTONUP:
        {
             g_mouse_but[0]=false;
        }
        break;

        case WM_RBUTTONDOWN:
        {
             g_mouse_but[1]=true;
             //cout<<"x: "<<g_mouse_pos[0]<<", y: "<<g_mouse_pos[1]<<endl; //temp
        }
        break;

        case WM_RBUTTONUP:
        {
             g_mouse_but[1]=false;
        }
        break;

        case WM_MOUSEWHEEL:
        {
            if(HIWORD(wParam)>5000) {g_mouse_but[2]=true;}
            if(HIWORD(wParam)>100&&HIWORD(wParam)<5000) {g_mouse_but[3]=true;}
        }
        break;

		case WM_KEYDOWN:
		{
			g_keys[wParam]=true;

			//cout<<wParam<<endl;
		}
		break;

		case WM_KEYUP:
		{
			g_keys[wParam]=false;
		}
		break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode

    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,g_window_width,g_window_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,g_window_width,g_window_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil( 0 );
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

