/*
  * Simple Xlib application drawing a box in a window.
  * gcc input.c -o output -lX11
  */
 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Window findkmines(Display * display, Window parent){
    Window root_return;
    Window parent_return;
    Window * children_return;
    unsigned int nchildren_return;
    XQueryTree(display, parent, &root_return, &parent_return, &children_return, &nchildren_return);
    
    for (int i = 0; i < nchildren_return; i++){
      char * name = NULL;
      XFetchName(display, children_return[i], &name);
      if (name != NULL){
        if (name[0] == '\0'){
          XWindowAttributes attr;
          XGetWindowAttributes(display, children_return[i], &attr);
          printf("Found it! %i %i", attr.width, attr.height);
          return children_return[i];
        }
        printf(name);
        printf("\n");
      }
      Window result = findkmines(display, children_return[i]);
      if (result != NULL)
        return result;
    }
    return NULL;
}


int main(void)
{
    Display *display;
    Display *clientdisplay;
    Window window, root;
    XEvent event;
    char *msg = "Hello, World!";
    int s;
 
    /* open connection with the server */
    display = XOpenDisplay(NULL);
    clientdisplay = XOpenDisplay(":1");
    if (display == NULL)
    {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
 
    s = DefaultScreen(display);
  
    root = XDefaultRootWindow(clientdisplay);
    XWindowAttributes attr;
    XGetWindowAttributes(clientdisplay, root, &attr);
 
    /* create window */
    window = XCreateSimpleWindow(display, RootWindow(display, s), 10, 10, 200, 200, 1,
                           BlackPixel(display, s), WhitePixel(display, s));
 
    /* select kind of events we are interested in */
    XSelectInput(display, window, ExposureMask | KeyPressMask);
 
    /* map (show) the window */
    XMapWindow(display, window);

    Window game = root; // = findkmines(clientdisplay, root);
    
    
    
    if (game == NULL){
      game = root;
    }
    
    /* Get image from display */
    XImage *image;
    
 
    /* event loop */
    for (;;)
    {        

      image = XGetImage(clientdisplay, game, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
      XPutImage(display, window, DefaultGC(display, s), image, 0, 0, 0, 0, attr.width, attr.height);
      XDestroyImage(image);
      usleep(10000);
    }
 
    /* close connection to server */
    XCloseDisplay(display);
    XCloseDisplay(clientdisplay);
 
    return 0;
 }
