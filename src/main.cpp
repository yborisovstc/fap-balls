
#include <gtk/gtk.h>
#include <stdlib.h>
#include <fapext.h>
#include <fapstext.h>
#include "fap-balls-model.h"

const TTransInfo KTinfo_Update_coord = TTransInfo(UpdateCoord, "trans_coord");
const TTransInfo KTinfo_Update_velocity = TTransInfo(UpdateVelocity, "trans_inpv");
const TTransInfo KTinfo_Update_moved = TTransInfo(UpdateSelected, "trans_moved");
const TTransInfo KTinfo_Update_borders_count = TTransInfo(UpdateBordersCount, "trans_borders_count");
const TTransInfo KTinfo_Update_ball_creation_ready = TTransInfo(UpdateBallCreationReady, "trans_ball_creation_ready");
const TTransInfo KTinfo_Update_ball_creation_start = TTransInfo(UpdateBallCreationStart, "trans_ball_creation_start");
const TTransInfo KTinfo_Update_bord_hook_left = TTransInfo(UpdateBordHookLeft, "trans_bord_hook_left");
const TTransInfo KTinfo_Update_bord_hook_right = TTransInfo(UpdateBordHookRight, "trans_bord_hook_right");
const TTransInfo KTinfo_Update_bord_hook_top = TTransInfo(UpdateBordHookTop, "trans_bord_hook_top");
const TTransInfo KTinfo_Update_bord_hook_bottom = TTransInfo(UpdateBordHookBottom, "trans_bord_hook_bottom");

// Transition functions register
const TTransInfo* tinfos[] = { &KTinfo_Update_coord, &KTinfo_Update_velocity, &KTinfo_Update_moved, 
    &KTinfo_Update_borders_count, &KTinfo_Update_ball_creation_ready, &KTinfo_Update_ball_creation_start,
    &KTinfo_Update_bord_hook_left, &KTinfo_Update_bord_hook_right, &KTinfo_Update_bord_hook_top, &KTinfo_Update_bord_hook_bottom,
    NULL};

const char* KLogSpecFileName = "/var/log/faplogspec.txt";
const char* KLogFileName = "fap-balls.log";
const char* KSpecFileName = "fap-balls-spec.xml";
/* Time slice of FAP environment, in milliseconds */
const gint KFapeTimeSlice = 50;

gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean idle_event_handler(gpointer data);
static gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer data);
static void destroy_event_handler(GtkWidget *widget, gpointer data);
static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean handle_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data);
static gboolean handle_frame_event( GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean handle_area_size_allocate_event( GtkWidget *widget, GtkAllocation *allocation, gpointer data);
static void draw_area();
static void draw_ball(CAE_Object *aBall);

GdkGC *gr_cont;
/* Finite automata programming environment */
CAE_Env* fape = NULL;
/* Area painter */
CFT_BArrea_Painter* fapainter = NULL;

static GtkWidget *main_window;
static GtkWidget *drawing_area;

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    drawing_area = gtk_drawing_area_new();
    // gtk_widget_set_size_request(drawing_area, 100, 100);
    gtk_container_add(GTK_CONTAINER(main_window), drawing_area);
//    gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 500);

    // Header for "delete_event" signal (this is given by the window manager, usually by the "close" option, or on the titlebar)
    g_signal_connect (G_OBJECT (main_window), "delete_event", G_CALLBACK (delete_event_handler), NULL);
    // Handler for "destroy" event. It occurs on call gtk_widget_destroy(), or on return FALSE in the "delete_event" callback
    g_signal_connect (G_OBJECT(main_window), "destroy", G_CALLBACK (destroy_event_handler), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(expose_event_callback), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "button_press_event", G_CALLBACK (handle_button_press_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "button_release_event", G_CALLBACK (handle_button_release_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event", G_CALLBACK (handle_motion_notify_event), NULL);
    g_signal_connect (G_OBJECT (drawing_area), "size_allocate", G_CALLBACK (handle_area_size_allocate_event), NULL);
    /* Sets the event mask (see GdkEventMask) for a widget. The event mask determines which events a widget will receive */
    gtk_widget_set_events (drawing_area, GDK_ALL_EVENTS_MASK);

    // Create environment, and painter
    fape = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    fapainter = new CFT_BArrea_Painter(drawing_area);

    gint x, y, width, height, depth;
    gdk_window_get_geometry(drawing_area->window, &x, &y, &width, &height, &depth);
    CAE_Object* farea = fape->Root();
    CAE_TState<CF_Rect>& srect =  *(farea->GetInpState("Rect"));
    srect = CF_Rect(x, y, x+width, y+height); srect.Confirm();

    gtk_widget_show(main_window);
    gtk_widget_show(drawing_area);
    gr_cont = gdk_gc_new(drawing_area->window); 
    /* Use idle of main loop to drive FAP environment */
    g_timeout_add(KFapeTimeSlice, idle_event_handler, NULL);

    gtk_main(); 
    return 0;
}

gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    draw_area();
    return TRUE;
}

gboolean idle_event_handler(gpointer data)
{
    if (fape != NULL) {
	fape->Step();
    }
    return ETrue;
}

static gboolean delete_event_handler(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    // If you return FALSE, GTK will emit the "destroy" signal. Returning TRUE means you don't want the window to be destroyed. 
    return FALSE;
}

static void destroy_event_handler(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}

static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAE_Object *area = fape->Root(); 
    CAE_TState<TInt>& lbdown = *(area->GetInpState("LbDown"));
    CAE_TState<CF_TdPoint>& mcpos = *(area->GetInpState("McPos"));
    if (event->button == 1) {
	lbdown = (event->type == GDK_BUTTON_PRESS) ? 1: 0;
	mcpos = CF_TdPoint(event->x, event->y);
    }
    return TRUE;
}

static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAE_Object *area = fape->Root(); 
    CAE_TState<TInt>& lbdown = *(area->GetInpState("LbDown"));
    CAE_TState<CF_TdPoint>& mcpos = *(area->GetInpState("McPos"));
    if (event->button == 1) {
	lbdown = (event->type == GDK_BUTTON_RELEASE) ? 0: 1;
	mcpos = CF_TdPoint(event->x, event->y);
    }
    return TRUE;
}


static gboolean handle_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    int x, y;
    GdkModifierType state;

    if (event->is_hint)
	gdk_window_get_pointer (event->window, &x, &y, &state);
    else {
	x = event->x; y = event->y;
	state = (GdkModifierType) event->state;
    }
    if (state) {
	CAE_Object *area = fape->Root(); 
	CAE_TState<CF_TdPoint>& mcpos = *(area->GetInpState("McPos"));
	mcpos = CF_TdPoint(x, y);
    }
    return TRUE;
}

static gboolean handle_area_size_allocate_event( GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
    gint x, y, width, height, depth;
    gdk_window_get_geometry(drawing_area->window, &x, &y, &width, &height, &depth);
    printf("handle_area_size_allocate: x= %d, y= %d, w= %d, h= %d\n", x, y, width, height);
    CAE_Object* farea = fape->Root();
    CAE_TState<CF_Rect>& srect =  *(farea->GetInpState("Rect"));
    srect = CF_Rect(x, y, x+width, y+height); srect.Confirm();
}

static void draw_area()
{
    int ctx = 0;
    CAE_Object* farea = fape->Root();
    CAE_Object* ball = (CAE_Object*) farea->GetNextCompByType("ball", &ctx);
    while (ball != NULL) {
	draw_ball(ball);
	ball = (CAE_Object*) farea->GetNextCompByType("ball", &ctx);
    }
}

static void draw_ball(CAE_Object *aBall)
{
    const TUint32& rad = *(aBall->GetOutpState("Rad"));
    const TInt& mass = *(aBall->GetOutpState("Mass"));
    const TBool& selected = *(aBall->GetOutpState("Moved"));
    const CF_TdPointF& coord = *(aBall->GetOutpState("Coord"));

    TUint8 cblue = 0xff - (mass*0xff)/KBallMassMax;
    if (cblue < 0) cblue = 0x00;
    TUint8 cgreen = selected ? 0xff : 0x00;
    fapainter->drawBall(coord, rad, CF_TdColor(0x00, cgreen, cblue));
}

