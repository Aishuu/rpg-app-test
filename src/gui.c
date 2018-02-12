#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <time.h>

#include "define.h"
#include "gui.h"

#define CELL_SIZE   75

void drawBoard () {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 1., -1., 1., 1., 20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

    glBegin(GL_QUADS);
      glColor3f(.5, .5, .5); glVertex3f(-.85, -.85, -2.);
      glColor3f(.5, .5, .5); glVertex3f( .85, -.85, -2.);
      glColor3f(.5, .5, .5); glVertex3f( .85,  .85, -2.);
      glColor3f(.5, .5, .5); glVertex3f(-.85,  .85, -2.);
    glEnd();
    glBegin(GL_QUADS);
      glColor3f(1., 1., 1.); glVertex3f(-.8, -.8, -1.5);
      glColor3f(1., 1., 1.); glVertex3f( .8, -.8, -1.5);
      glColor3f(1., 1., 1.); glVertex3f( .8,  .8, -1.5);
      glColor3f(1., 1., 1.); glVertex3f(-.8,  .8, -1.5);
    glEnd();

    int i;
    for (i=0; i<64; i+=2) {
        glBegin(GL_QUADS);
          glColor3f(0., 0., 0.); glVertex3f((i%8+(i/8)%2)*0.2-0.8, (i/8)*0.2-0.8, -1.);
          glColor3f(0., 0., 0.); glVertex3f((i%8+(i/8)%2)*0.2-0.6, (i/8)*0.2-0.8, -1.);
          glColor3f(0., 0., 0.); glVertex3f((i%8+(i/8)%2)*0.2-0.6, (i/8)*0.2-0.6, -1.);
          glColor3f(0., 0., 0.); glVertex3f((i%8+(i/8)%2)*0.2-0.8, (i/8)*0.2-0.6, -1.);
        glEnd();
    }
}

void drawGame (Game game) {
    int i,j;
    for (i=0; i<8; i++)
        for (j=0; j<8; j++) {
            if (game->pieces[i][j].p_type == RIEN)
                continue;
            float color = 0.;
            switch (game->pieces[i][j].p_type) {
                case PION:
                    color = 0.8;
                    break;
                case CAVALIER:
                    color = 0.6;
                    break;
                case FOU:
                    color = 0.4;
                    break;
                case TOUR:
                    color = 0.2;
                    break;
                case DAME:
                    color = 0.1;
                    break;
                default:
                    break;
            }
            if (game->pieces[i][j].p_color == NOIR)
                glColor3f(color, color, 1.); 
            else
                glColor3f(1., color, color); 
            glBegin(GL_QUADS);
              glVertex3f(i*0.2-0.75, j*0.2-0.75, 0.);
              glVertex3f(i*0.2-0.65, j*0.2-0.75, 0.);
              glVertex3f(i*0.2-0.65, j*0.2-0.65, 0.);
              glVertex3f(i*0.2-0.75, j*0.2-0.65, 0.);
            glEnd();
        }
}

void startGUI (Game game) {
    Display                 *dpy;
    Window                  root;
    GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo             *vi;
    Colormap                cmap;
    XSetWindowAttributes    swa;
    Window                  win;
    GLXContext              glc;
    XWindowAttributes       gwa;
    XEvent                  xev;
    dpy = XOpenDisplay(NULL);
     
    if(dpy == NULL) {
        fprintf(stderr, "cannot connect to X server\n\n");
        exit(0);
    }

    root = DefaultRootWindow(dpy);

    vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
        fprintf(stderr, "no appropriate visual found\n\n");
        exit(0);
    } 

    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;
    win = XCreateWindow(dpy, root, 0, 0, 10*CELL_SIZE, 10*CELL_SIZE, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Chess - ANN");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
    glEnable(GL_DEPTH_TEST);

    clock_t c = clock();

    while(1) {
        if (XPending(dpy)) {
            XNextEvent(dpy, &xev);

            if(xev.type == Expose) {
                XGetWindowAttributes(dpy, win, &gwa);
                glViewport(0, 0, gwa.width, gwa.height);
                drawBoard ();
                drawGame (game);
                glXSwapBuffers(dpy, win);
            } else if(xev.type == KeyPress) {
                glXMakeCurrent(dpy, None, NULL);
                glXDestroyContext(dpy, glc);
                XDestroyWindow(dpy, win);
                XCloseDisplay(dpy);
                exit(0);
            }
        }
        clock_t cc = clock();
        if (cc - c > PERIODE_COUP) {
            c = cc;
            int r = game_coup (game);
            if (r != 0) {
                glXMakeCurrent(dpy, None, NULL);
                glXDestroyContext(dpy, glc);
                XDestroyWindow(dpy, win);
                XCloseDisplay(dpy);
                exit(0);
            }
            drawBoard ();
            drawGame (game);
            glXSwapBuffers(dpy, win);
        }
    }
}
