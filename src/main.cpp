/* Copyright (c) 2009, Yuri Borisov
 * All rights reserved
 */

#include <gtk/gtk.h>
#include <fapext.h>
#include "fap-balls-model.h"




class CFT_BArrea_Painter: public MBallAreaWindow
{
public:
	CFT_BArrea_Painter(GtkWidget* aWidget) : iWidget(aWidget) {}
	virtual ~CFT_BArrea_Painter() {}
private:
	//from MBallAreaWindow
	virtual void redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase);
	virtual void drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor);
private:
	GtkWidget* iWidget;
};

void CFT_BArrea_Painter::redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase)
{
    GdkRectangle rect;
    rect.x = aCenter.iX-aRadius; if (rect.x < 0) rect.x = 0;
    rect.y = aCenter.iY-aRadius; if (rect.y < 0) rect.y = 0;
    rect.width = aRadius * 2.0;
    rect.height = aRadius * 2.0;
    if (aErase)
	gdk_window_clear_area(iWidget->window, rect.x, rect.y, rect.width, rect.height);
    gdk_window_invalidate_rect(iWidget->window, &rect, TRUE);
}

void CFT_BArrea_Painter::drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor)
{
    long x, y, size;
    x = aCenter.iX - aRadius;
    y = aCenter.iY - aRadius;
    size = aRadius * 2.0;

    GdkGC *gc;
    //gc = iWidget->style->fg_gc[GTK_WIDGET_STATE (iWidget)];
    //gdk_gc_copy(gc, iWidget->style->fg_gc[GTK_WIDGET_STATE (iWidget)]);
    gc = gdk_gc_new(iWidget->window); 
    GdkColor color;
    gboolean res = FALSE;
    //res = gdk_colormap_alloc_color(gtk_widget_get_colormap(iWidget), &color, FALSE, TRUE); 
    //res = gdk_colormap_alloc_color(gdk_rgb_get_colormap(), &color, FALSE, FALSE); 
    /* Needs to shift RGB codes because GdkColor uses 16-bit coding */ 
    /* Attribute "pixel" needs to be 0 value, because we use RGB color but not allocated from colormap */
    color.pixel = 0;
    color.red = aColor.iRed << 8;
    color.green = aColor.iGreen << 8;
    color.blue = aColor.iBlue << 8;
    /* Needs to use gdk_gc_set_rgb_fg_color because we use RGB colour but not allocated from colormap */
    //gdk_gc_set_foreground(gc, &color);
     gdk_gc_set_rgb_fg_color(gc, &color);
    gdk_draw_arc (iWidget->window, gc, TRUE, x, y, size, size, 0, 64 * 360);
}



const char* KLogSpecFileName = "/var/log/faplogspec.txt";
const char* KFAreaName = "Area";
    
/* Time slice of FAP environment, in milliseconds */
const gint KFapeTimeSlice = 30;

gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data);

gboolean idle_event_handler(gpointer data);

static gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer data);

static void destroy_event_handler(GtkWidget *widget, gpointer data);

static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
    
static gboolean handle_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data);

static gboolean handle_frame_event( GtkWidget *widget, GdkEvent *event, gpointer data);

static gboolean handle_area_size_allocate_event( GtkWidget *widget, GtkAllocation *allocation, gpointer data);


/* Finite automata programming environment */
CAE_Env* fape = NULL;
/* 2d area */ 
CFT_Area* farea = NULL;
/* Area painter */
CFT_BArrea_Painter* fapainter = NULL;


static GtkWidget *main_window;
static GtkWidget *drawing_area;

int main(int argc, char *argv[])
{

    printf("main\n");
	    
    gtk_init(&argc, &argv);

    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    drawing_area = gtk_drawing_area_new();
    // gtk_widget_set_size_request(drawing_area, 100, 100);
    gtk_container_add(GTK_CONTAINER(main_window), drawing_area);
//    gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 500);

    /* Define the header for "delete_event" signal (this is given
       by the window manager, usually by the "close" option, or on the titlebar) */
    g_signal_connect (G_OBJECT (main_window), "delete_event", G_CALLBACK (delete_event_handler), NULL);
    /* Define the handler for "destroy" event.  This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT(main_window), "destroy", G_CALLBACK (destroy_event_handler), NULL);
	
    g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(expose_event_callback), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "button_press_event", G_CALLBACK (handle_button_press_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "button_release_event", G_CALLBACK (handle_button_release_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event", G_CALLBACK (handle_motion_notify_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "size_allocate", G_CALLBACK (handle_area_size_allocate_event), NULL);

    /* Sets the event mask (see GdkEventMask) for a widget.
     * The event mask determines which events a widget will receive */
//    gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK
//	    | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_set_events (drawing_area, GDK_ALL_EVENTS_MASK);

    /* Create finite automata environment */
    fape = CAE_Env::NewL(1, KLogSpecFileName);
    /* Create area painter */
    fapainter = new CFT_BArrea_Painter(drawing_area);
    gint x, y, width, height, depth;
    gdk_window_get_geometry(drawing_area->window, &x, &y, &width, &height, &depth);
    CF_Rect rect = CF_Rect(x, y, x+width, y+height);
    /* Create 2d area */
    farea = CFT_Area::NewL(KFAreaName, NULL, fapainter, &rect);
    fape->AddL(farea);

    gtk_widget_show(main_window);
    gtk_widget_show(drawing_area);

    /* Use idle of main loop to drive FAP environment */
    g_timeout_add(KFapeTimeSlice, idle_event_handler, NULL);

    /* Created main loop */
    gtk_main(); 
    return 0;
}

gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    /* Redraw balls area */
    farea->Draw();
    return TRUE;
}

gboolean idle_event_handler(gpointer data)
{
    if (fape != NULL)
    {
	fape->Step();
    }
}

static gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    /* If you return FALSE in the "delete_event" signal handler, GTK will emit the "destroy" signal. 
     * Returning TRUE means you don't want the window to be destroyed. 
     * This is useful for popping up 'are you sure you want to quit?' type dialogs. */
    return FALSE;
}

static void destroy_event_handler(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}

static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (event->button == 1)
    {
	*farea->iLbDown = (event->type == GDK_BUTTON_PRESS) ? 1: 0;
	*farea->iMcPos = CF_TdPoint(event->x, event->y);
    }
    return TRUE;
}

static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    if (event->button == 1)
    {
	*farea->iLbDown = (event->type == GDK_BUTTON_RELEASE) ? 0: 1;
	*farea->iMcPos = CF_TdPoint(event->x, event->y);
    }
    return TRUE;
}


static gboolean handle_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    int x, y;
    GdkModifierType state;

    if (event->is_hint)
	gdk_window_get_pointer (event->window, &x, &y, &state);
    else
    {
	x = event->x;
	y = event->y;
	state = (GdkModifierType) event->state;
    }

    if (state)
    {
	*farea->iMcPos = CF_TdPoint(x, y);
    }
    return TRUE;
}

static gboolean handle_area_size_allocate_event( GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
    gint x, y, width, height, depth;
//    gdk_window_get_position(iWidget->window, &x, &y);
//    gdk_drawable_get_size(iWidget->window, &width, &height);
    gdk_window_get_geometry(drawing_area->window, &x, &y, &width, &height, &depth);
    printf("handle_area_size_allocate: x= %d, y= %d, w= %d, h= %d\n", x, y, width, height);
    *(farea->iRect) = CF_Rect(x, y, x+width, y+height);
    CF_Rect* nr = (CF_Rect *) farea->iRect->iNew;
//    printf("new: tx= %d, ty= %d, bx= %d, by= %d\n", nr->iLeftUpper.iX, nr->iLeftUpper.iY, nr->iRightLower.iX, nr->iRightLower.iY);
}

