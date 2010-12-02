// Balls and area model 

#include <fapplat.h>
#include <fapext.h>
#include <fapstext.h>
#include <stdlib.h>
#include <stdio.h>
#include "fap-balls-model.h"

const char* KBallName = "Ball%d";
// Gravitation constant
const TInt KInfl = 9;
// Default hook stiffness coefficient, n/m
const TInt KHookSc = 2;
// Maximum value of mass
const TInt KMassMax = 1000;
// The radius of borders
const TInt KBorderRadius = 50000;
// The mass of borders
const TInt KBorderMass = 30000;
// Constant of friction
const float KConstFriction = 19.80;
// Static Force of friction when manual moving, n
const float KFrictForceManMov = 20.0;
// Force of extrusion while superposition
const TInt KForceOfExtrs = 20;

extern CAE_Env* fape;
extern CFT_BArrea_Painter* fapainter;
extern GdkGC *gr_cont;

void CFT_BArrea_Painter::redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase)
{
    GdkRectangle rect;
    rect.x = aCenter.iX-aRadius; if (rect.x < 0) rect.x = 0;
    rect.y = aCenter.iY-aRadius; if (rect.y < 0) rect.y = 0;
    rect.width = aRadius * 2.0; rect.height = aRadius * 2.0;
    gdk_window_invalidate_rect(iWidget->window, &rect, TRUE);
}

void CFT_BArrea_Painter::drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor)
{
    long x = aCenter.iX - aRadius;
    long y = aCenter.iY - aRadius;
    long size = aRadius * 2.0;
    GdkGC *gc = gr_cont;
//    gc = gdk_gc_new(iWidget->window); 
    GdkColor color;
    // Needs shifting RGB because GdkColor uses 16-bit code. Att pixel = 0, because we use RGB color but not allocated from colormap
    color.pixel = 0;
    color.red = aColor.iRed << 8;
    color.green = aColor.iGreen << 8;
    color.blue = aColor.iBlue << 8;
    /* Needs to use gdk_gc_set_rgb_fg_color because we use RGB colour but not allocated from colormap */
    gdk_gc_set_rgb_fg_color(gc, &color);
    gdk_draw_arc (iWidget->window, gc, TRUE, x, y, size, size, 0, 64 * 360);
}

// Area

CAE_Object* CreateBall(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aInstName, TBool aBorder)
{
    CAE_Object *area = fape->Root(); 
    char name[50];
    if (aInstName != NULL) 
	strcpy(name, aInstName);
    else 
	sprintf(name, KBallName, area->CountCompWithType("Ball") + 1);

    CAE_Object* ball = (CAE_Object *) area->FindByName("ball");
    CAE_Object* newball = ball->CreateNewL(NULL, name, area);
    CF_TdPointF coord(aCoordX, aCoordY);
    CAE_TState<TInt> *srad = (CAE_TState<TInt>*) newball->GetInput("Rad");
    CAE_TState<CF_TdPointF> *scoord = (CAE_TState<CF_TdPointF>*) newball->GetOutput("Coord");
    CAE_TState<TInt> *smass = (CAE_TState<TInt>*) newball->GetInput("Mass");
    // Confirm states new values because newly created ball can be updated this tick and new values will be lost
    // TODO [YB] Consider this situation. It is not obvious for developers
    *srad = aRad; srad->Confirm();
    *scoord= coord; scoord->Confirm();
    *smass = aMass; smass->Confirm();

    AddBallL(newball, !aBorder);
    return newball;
}

void CreateBorder(float aCoordX,  float aCoordY, const char* aInstName, TTransFun aHookUpdate)
{
    CAE_Object *bord = CreateBall(aCoordX, aCoordY, KBorderMass, KBorderRadius, aInstName, ETrue);	
    CAE_TState<TBool> *shookedperm = (CAE_TState<TBool>*) bord->GetInput("HookedPerm");
    CAE_TState<TBool> *stransp = (CAE_TState<TBool>*) bord->GetInput("Transp");
    CAE_Object* farea = fape->Root();
    CAE_TState<CF_Rect>* srect = (CAE_TState<CF_Rect>*) farea->GetInput("Rect");
    _FAP_ASSERT(srect != NULL);
    CAE_TState<CF_TdPoint> *smcpos = (CAE_TState<CF_TdPoint> *) bord->GetInput("McPos");
    smcpos->SetInputL("McPos", srect);
    smcpos->SetTrans(TTransInfo(aHookUpdate));
    *shookedperm = TRUE; shookedperm->Confirm();
    *stransp = TRUE; stransp->Confirm();
}

void LinkBall(CAE_Object* aBallRec, CAE_Object* aBallExt)
{
    CAE_TState<CF_TdVectF> *s_vel = (CAE_TState<CF_TdVectF> *) aBallRec->GetInput("InpV");
    CAE_TState<CF_TdPointF> *s_coord = (CAE_TState<CF_TdPointF> *) aBallRec->GetOutput("Coord");
    CAE_TState<CF_TdPointF> *se_coord = (CAE_TState<CF_TdPointF> *) aBallExt->GetOutput("Coord");
    CAE_TState<TInt> *se_mass = (CAE_TState<TInt> *) aBallExt->GetInput("Mass");
    CAE_TState<TInt> *se_rad = (CAE_TState<TInt>*) aBallExt->GetInput("Rad");
    CAE_TState<CF_TdVectF> *se_vel = (CAE_TState<CF_TdVectF> *) aBallExt->GetInput("InpV");
    CAE_TState<TBool> *se_transp = (CAE_TState<TBool>*) aBallExt->GetInput("Transp");

    s_vel->AddExtInputL("Coord", se_coord);
    s_vel->AddExtInputL("Mass", se_mass);
    s_vel->AddExtInputL("Rad", se_rad);
    s_vel->AddExtInputL("InpV", se_vel);
    s_vel->AddExtInputL("Transp", se_transp);
    s_coord->AddExtInputL("Coord", se_coord);
    s_coord->AddExtInputL("Rad", se_rad);
    s_coord->AddExtInputL("InpV", se_vel);
    s_coord->AddExtInputL("Transp", se_transp);
}

void AddBallL(CAE_Object* aObBall, TBool aUseAreaHook)
{
    CAE_Object *area = fape->Root(); 
    CAE_TState<TInt> *sball_lbdown = (CAE_TState<TInt>*) aObBall->GetInput("LbDown");
    CAE_TState<TInt> *sarea_lbdown = (CAE_TState<TInt>*) area->GetInput("LbDown");
    CAE_TState<CF_TdPoint> *sball_mcpos = (CAE_TState<CF_TdPoint>*) aObBall->GetInput("McPos");
    CAE_TState<CF_TdPoint> *sarea_mcpos = (CAE_TState<CF_TdPoint>*) area->GetInput("McPos");

    sball_lbdown->SetInputL("LbDown", sarea_lbdown); 
    if (aUseAreaHook)
	sball_mcpos->SetInputL("McPos", sarea_mcpos); 
    int ctx = 0;
    CAE_Object* ball = (CAE_Object*) area->GetNextCompByType("ball", &ctx);
    while (ball != NULL) {
	if (ball != aObBall) {
	    LinkBall(ball, aObBall);
	    LinkBall(aObBall, ball); 
	}
	ball = (CAE_Object*) area->GetNextCompByType("ball", &ctx);
    }
}

void UpdateBordHookLeft(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CF_Rect arect = ~*(CAE_TState<CF_Rect> *) self->Input("McPos");
    long midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(arect.iLeftUpper.iX-KBorderRadius, midy); 
}

void UpdateBordHookRight(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CF_Rect arect = ~*(CAE_TState<CF_Rect> *) self->Input("McPos");
    long midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(arect.iRightLower.iX + KBorderRadius, midy); 
}

void UpdateBordHookTop(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CF_Rect arect = ~*(CAE_TState<CF_Rect> *) self->Input("McPos");
    long midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    *self = CF_TdPoint(midx, arect.iLeftUpper.iY - KBorderRadius); 
}

void UpdateBordHookBottom(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CF_Rect arect = ~*(CAE_TState<CF_Rect> *) self->Input("McPos");
    long midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    *self = CF_TdPoint(midx, arect.iRightLower.iY + KBorderRadius); 
}


// CFT_Ball

void UpdateCoord(CAE_Object* aObject, CAE_State* aState)
{
    TInt mass = ~*(CAE_TState<TInt> *) aState->Input("MassS");
    if (mass != 0)
    {
	CAE_TState<CF_TdPointF> *sthis = (CAE_TState<CF_TdPointF> *) aState;
	CF_TdPointF coord = ~*(CAE_TState<CF_TdPointF> *) aState->Input("CoordS");
	CF_TdVectF vel = ~*(CAE_TState<CF_TdVectF> *) aState->Input("InpVS");
	TBool transp = ~*(CAE_TState<TBool>*) aState->Input("TranspS");
	TInt rad = ~*(CAE_TState<TInt>*) aState->Input("RadS");

	CF_TdPointF newcoord(coord.iX + vel.iX, coord.iY + vel.iY);
	// Check if there will be some collision at the next update
	for (TInt i = 0; ; i++) 
	{
	    CAE_TState<CF_TdPointF> *sexcoord = (CAE_TState<CF_TdPointF> *) aState->Input("Coord", i);
	    if (sexcoord == NULL) break;
	    CF_TdPointF excoord = ~*sexcoord;
	    TInt exrad = ~*(CAE_TState<TInt>*) aState->Input("Rad", i);
	    CF_TdVectF exvel = ~*(CAE_TState<CF_TdVectF> *) aState->Input("InpV", i);
	    TBool extransp = ~*(CAE_TState<TBool>*) aState->Input("Transp", i);

	    CF_TdPointF newexcoord(excoord.iX + exvel.iX, excoord.iY + exvel.iY);
	    float r = GetDistance(newcoord, newexcoord);
	    float curdis = GetDistance(coord, excoord);
	    TBool superpos = ((curdis < rad + exrad) && !transp);
	    // If there is no superposition, and collistion will happen, then fix coord
	    if (!superpos && ((r <= (rad + exrad)) && !(transp && extransp))) 
		newcoord = coord;
	}
	(*sthis) = newcoord;
	// Redraw the ball
	fapainter->redraw(CF_TdPoint(coord.iX, coord.iY), rad, ETrue);
	fapainter->redraw(CF_TdPoint(newcoord.iX, newcoord.iY), rad, EFalse);
    }
}

void UpdateSelected(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TBool> *sthis = (CAE_TState<TBool> *) aState;
    TInt down = ~*(CAE_TState<TInt>*) aState->Input("LbDownS");
    TBool hookedperm = ~*(CAE_TState<TBool> *) aState->Input("HookedPermS");
    TBool sel = ~*sthis;

    if (hookedperm) 
	sel = TRUE;
    else if (sel) {
	if (!down) 
	    sel = EFalse;
    }
    else {
	if (down) {
	    CF_TdPoint mcpos = ~*(CAE_TState<CF_TdPoint>*) aState->Input("McPosS");
	    CF_TdPointF coord = ~*(CAE_TState<CF_TdPointF> *) aState->Input("CoordS");
	    TInt rad = ~*(CAE_TState<TInt>*) aState->Input("RadS");
	    float disx = (mcpos.iX - coord.iX);
	    float disy = (mcpos.iY - coord.iY);
	    float dis = sqrt(disx*disx + disy*disy);
	    sel = dis < rad;
	}
	else 
	    sel = EFalse;
    }
    *sthis = sel;
}

void UpdateVelocity(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdVectF> *sthis = (CAE_TState<CF_TdVectF> *) aState;
    CF_TdPointF curcoord = ~*(CAE_TState<CF_TdPointF> *) aState->Input("CoordS");
    CF_TdVectF curvel = ~*(CAE_TState<CF_TdVectF> *) aState->Input("InpVS");
    TBool selected = ~*(CAE_TState<TBool>*) aState->Input("MovedS");
    TBool transp = ~*(CAE_TState<TBool>*) aState->Input("TranspS");
    TInt currad = ~*(CAE_TState<TInt>*) aState->Input("RadS");
    CF_TdPoint hookcoord = ~*(CAE_TState<CF_TdPoint>*) aState->Input("McPosS");
    TInt curmass = ~*(CAE_TState<TInt> *) aState->Input("MassS");
    
    CF_TdVectF vforce(0.0, 0.0), newvel(0.0, 0.0);
    TInt i = 0;
    // Calculate the force from the hook
    if (selected) { 
	CF_TdVectF vdis(curcoord, CF_TdPointF(hookcoord));
	float dis = sqrt(vdis.M2());
	// If the ball in the trap then stop any movement
	//if (dis < currad)
	if (dis < 10) {
	    *sthis = newvel; return;
	}
	vforce += vdis * KHookSc;
	// Dempfing
	float cvelmod = sqrt(curvel.M2()); 
	float force = sqrt(vforce.M2()); 
	if (force > KFrictForceManMov) 
	    vforce -= (curvel * KConstFriction); 
	else 
	    vforce = CF_TdPointF(0.0, 0.0); 
	printf("upd_vel %s: hcoord= %d, %d  ccoord= %f, %f  hforce= %f, %f\n", aObject->InstName(), hookcoord.iX, hookcoord.iY, 
		curcoord.iX, curcoord.iY, vforce.iX, vforce.iY);
    }
    // Calculate the resultant force from the balls 
    for (i = 0; ; i++) 
    {
	CAE_TState<CF_TdPointF> *sexcoord = (CAE_TState<CF_TdPointF> *) aState->Input("Coord", i);
	if (sexcoord == NULL) break;
	CF_TdPointF excoord = ~*sexcoord;
	TInt exmass = ~*(CAE_TState<TInt> *) aState->Input("Mass", i);
	TInt exrad = ~*(CAE_TState<TInt>*) aState->Input("Rad", i);
	TBool sextransp = ~*(CAE_TState<TBool>*) aState->Input("Transp", i);

	if (exmass != 0) {
	    float r = GetDistance(curcoord, excoord);
	    float force = (KInfl * curmass * exmass)/(r*r);
	    if (r > (currad + exrad)) 
		vforce += CF_TdVectF(curcoord, excoord)*(force/r);
	    else if (!transp) // Superposition
		vforce = (r == 0.0) ? CF_TdVectF(-KForceOfExtrs, -KForceOfExtrs) : CF_TdVectF(excoord, curcoord)*(KForceOfExtrs/r);
	}
    }
    if (curmass > 0.0) 
	newvel = curvel + vforce/curmass;

    // Predict the updated coordinate
    CF_TdPointF newcoord = curcoord + CF_TdPointF(curvel);
    // Check if there will be some collision at the next update
    for (i = 0; ; i++) 
    {
	CAE_TState<CF_TdPointF> *sexcoord = (CAE_TState<CF_TdPointF> *) aState->Input("Coord", i);
	if (sexcoord == NULL) break;
	CF_TdPointF excoord = ~*sexcoord;
	TInt exmass = ~*(CAE_TState<TInt> *) aState->Input("Mass", i);
	TInt exrad = ~*(CAE_TState<TInt>*) aState->Input("Rad", i);
	CF_TdVectF exvel = ~*(CAE_TState<CF_TdVectF> *) aState->Input("InpV", i);
	TBool extransp = ~*(CAE_TState<TBool>*) aState->Input("Transp", i);

	if (exmass != 0 && !(transp && extransp)) {
	    CF_TdPointF newexcoord = excoord + CF_TdPointF(exvel);
	    float newdis = GetDistance(newcoord, newexcoord);
	    float curdis = GetDistance(curcoord, excoord);
	    // Handle collision && !superposition
	    if ((newdis < (currad + exrad)) && !(curdis <= (currad + exrad))) {
		printf("CFT_Ball::UpdateVelocity: r: %f, currad: %d, exrad: %d\n", newdis, currad, exrad);
		// Get the projections of interacting balls
		CF_TdVectF curvel_norm, curvel_tang, exvel_norm, exvel_tang;
		GetProjOfVel(newexcoord, newcoord, curvel, curvel_norm, curvel_tang);
		GetProjOfVel(newexcoord, newcoord, exvel, exvel_norm, exvel_tang);
		// Calculate the new velocity. Only normal component is affected
		float mc1 = ((float) curmass - (float) exmass)/((float) curmass + (float) exmass);
		float mc2 = (2.0 * exmass)/((float) curmass + (float) exmass);
		newvel = curvel_norm * mc1 + exvel_norm * mc2 + curvel_tang;
	    }
	}
    }
    (*sthis) = newvel;
}

// TODO [YB] Migrate from point to vector
void GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang)
{
    CF_TdPointF axle(aAngleEnd - aAngleBeg);
    float axle_m2 = axle.M2();
    float axle_mod = sqrt(axle_m2);
    float vel_m2 = aVel.M2();
    CF_TdPointF diff(aVel.iX - axle.iX, aVel.iY - axle.iY);
    float diff_m2 = diff.M2();
    float vel_mod = sqrt(vel_m2);
    float cos_angle = (vel_m2 > 0.0) ? (vel_m2 + axle_m2 - diff_m2)/(2 * vel_mod * axle_mod) : 1.0; 
    // Calculate the normal and tang components of current velocity
    float vel_norm_mod = vel_mod * cos_angle;
    aVelNorm = CF_TdVectF(axle) * vel_norm_mod / axle_mod;
    aVelTang = aVel - aVelNorm;
}

