// Balls and area model 

#include <fapplat.h>
#include <fapext.h>
#include <fapstext.h>
#include <stdlib.h>
#include <stdio.h>
#include "fap-balls-model.h"

// Gravitation constant
const TInt KInfl = 9;
// Default hook stiffness coefficient, n/m
const TInt KHookSc = 2;
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
    long size = aRadius * 2.0;
    GdkGC *gc = gr_cont;
    // Needs shifting RGB because GdkColor uses 16-bit code. Att pixel = 0, because we use RGB color but not allocated from colormap
    GdkColor color = {0, aColor.iRed << 8, aColor.iGreen << 8, aColor.iBlue << 8};
    // Needs to use gdk_gc_set_rgb_fg_color because we use RGB colour but not allocated from colormap
    gdk_gc_set_rgb_fg_color(gc, &color);
    gdk_draw_arc (iWidget->window, gc, TRUE, aCenter.iX - aRadius, aCenter.iY - aRadius, size, size, 0, 64 * 360);
}

// Area

void CreateBall(TInt aCoordX, TInt aCoordY, TInt aMass, TInt aRad, TBool aHookedPerm, TBool aTransp, const char* aName, const char* aHookName)
{
    CAE_Object *area = fape->Root(); 
    CAE_Object* ball = (CAE_Object *) area->FindByName("ball");
    CAE_Object* newball = ball->CreateNewL(NULL, aName, area);
    CF_TdPointF coord(aCoordX, aCoordY);
    CAE_TState<TUint32>& srad =  *(newball->GetInpState("Rad"));
    CAE_TState<CF_TdPointF>& scoord = *(newball->GetInpState("Coord"));
    CAE_TState<TInt>& smass = *(newball->GetInpState("Mass"));
    CAE_TState<TBool>& shookedperm = *(newball->GetInpState("HookedPerm"));
    CAE_TState<TBool>& stransp = *(newball->GetInpState("Transp"));
    // Confirm states because newly created ball can be updated this tick  TODO [YB] not obvious fact
    srad = aRad; srad.Confirm();
    scoord= coord; scoord.Confirm();
    smass = aMass; smass.Confirm();
    shookedperm = aHookedPerm; shookedperm.Confirm();
    stransp = aTransp; stransp.Confirm();
    AddBallL(newball, aHookName);
}

void LinkBall(CAE_Object* aBallRec, CAE_Object* aBallExt)
{
    CAE_TState<CF_TdVectF>& s_vel = (CAE_TState<CF_TdVectF>&) aBallRec->GetInp("InpV");
    s_vel.AddExtInputL("Coord", aBallExt->GetOutput("Coord"));
    s_vel.AddExtInputL("Mass", aBallExt->GetInput("Mass"));
    s_vel.AddExtInputL("Rad", aBallExt->GetInput("Rad"));
    s_vel.AddExtInputL("InpV", aBallExt->GetInput("InpV"));
    s_vel.AddExtInputL("Transp", aBallExt->GetInput("Transp"));
}

void AddBallL(CAE_Object* aObBall, const char* aHookName)
{
    CAE_Object *area = fape->Root(); 
    CAE_TState<TInt> *sball_lbdown = (CAE_TState<TInt>*) aObBall->GetInput("LbDown");
    CAE_TState<TInt> *sarea_lbdown = (CAE_TState<TInt>*) area->GetInput("LbDown");
    CAE_TState<CF_TdPoint> *sball_mcpos = (CAE_TState<CF_TdPoint>*) aObBall->GetInput("McPos");
    CAE_TState<CF_TdPoint> *sarea_mcpos = (CAE_TState<CF_TdPoint>*) area->GetInput(aHookName);

    sball_lbdown->SetInputL("LbDown", sarea_lbdown); 
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
    CAE_TState<CF_TdPoint>& self = (CAE_TState<CF_TdPoint>&) *aState;
    const CF_Rect& arect = self.Inp("McPos");
    self = CF_TdPoint(arect.iLeftUpper.iX-KBorderRadius, (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0); 
}

void UpdateBordHookRight(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint>& self = (CAE_TState<CF_TdPoint>&) *aState;
    const CF_Rect& arect = self.Inp("McPos");
    self = CF_TdPoint(arect.iRightLower.iX + KBorderRadius, (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0); 
}

void UpdateBordHookTop(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint>& self = (CAE_TState<CF_TdPoint>&) *aState;
    const CF_Rect& arect = self.Inp("McPos");
    self = CF_TdPoint((arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0, arect.iLeftUpper.iY - KBorderRadius); 
}

void UpdateBordHookBottom(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPoint>& self = (CAE_TState<CF_TdPoint>&) *aState;
    const CF_Rect& arect = self.Inp("McPos");
    self = CF_TdPoint((arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0, arect.iRightLower.iY + KBorderRadius); 
}

// TODO [YB] To migrate to using object proxy instead of direct access to area
void UpdateBordersCount(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TInt>& sself = (CAE_TState<TInt>&) *aState;
    // TODO [YB] Implement one shot by detaching inputs
    if (~sself == 0 ) {
	// Create borders
	const CF_Rect& rt = sself.Inp("Rect");
	TInt midx = (rt.iRightLower.iX - rt.iLeftUpper.iX)/2;
	TInt midy = (rt.iRightLower.iY - rt.iLeftUpper.iY)/2;
	CreateBall(rt.iLeftUpper.iX-KBorderRadius, midy, KBorderMass, KBorderRadius, ETrue, ETrue, "Border_Left", "BorderHook_Left");	
	CreateBall(rt.iRightLower.iX + KBorderRadius, midy, KBorderMass, KBorderRadius, ETrue, ETrue, "Border_Right", "BorderHook_Right");	
	CreateBall(midx, rt.iLeftUpper.iY - KBorderRadius, KBorderMass, KBorderRadius, ETrue, ETrue, "Border_Top", "BorderHook_Top");	
	CreateBall(midx, rt.iRightLower.iY + KBorderRadius, KBorderMass, KBorderRadius, ETrue, ETrue, "Border_Bottom", "BorderHook_Bottom");	
	sself = ~sself + 1;
    }
}

void UpdateBallCreationReady(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TBool> &sself = (CAE_TState<TBool>&) *aState;
    const TBool &start= sself.Inp("Start");
    if (start && ~sself) {
	const CF_TdPoint& coord = sself.Inp("Coord");
	const TUint32& rad= sself.Inp("Rad");
	char *name = (char*) malloc(100);
	sprintf(name, "ball_%d", rand());
	CreateBall(coord.iX,  coord.iY, (const TInt&) sself.Inp("Mass"), rad, EFalse, EFalse, name, "McPos");
	free(name);
	sself = EFalse;
    }	
    else if (!start) 
	sself = ETrue;
}

void UpdateBallCreationStart(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TBool> &sself = (CAE_TState<TBool>&) *aState;
    const TBool& ready= sself.Inp("Ready");
    if (ready) sself = EFalse;
}


// CFT_Ball

void UpdateCoord(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdPointF>& sthis = (CAE_TState<CF_TdPointF>&) *aState;
    const CF_TdVectF& vel = sthis.Inp("InpVS");
    const TUint32& rad = sthis.Inp("RadS");
    CF_TdPointF newcoord = ~sthis + vel;
    sthis = newcoord;
    // Redraw the ball
    fapainter->redraw(~sthis, rad, ETrue);
    fapainter->redraw(newcoord, rad, EFalse);
}

void UpdateSelected(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TBool>& selected = (CAE_TState<TBool>&) *aState;
    const TInt& down = selected.Inp("LbDownS");
    const TBool& hookedperm = selected.Inp("HookedPermS");
    if (hookedperm) 
	selected = ETrue;
    else if (~selected && !down) 
	selected = EFalse;
    else if (!~selected && down) {
	const CF_TdPoint& mcpos = selected.Inp("McPosS");
	const CF_TdPointF& coord = selected.Inp("CoordS");
	const TUint32& rad = selected.Inp("RadS");
	selected = (mcpos - coord).Mod() < rad;
    }
}

void UpdateVelocity(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<CF_TdVectF>& sthis = (CAE_TState<CF_TdVectF>&) *aState;
    const CF_TdPointF& curcoord = sthis.Inp("CoordS");
    const CF_TdVectF& curvel = sthis.Inp("InpVS");
    const TBool& selected = sthis.Inp("MovedS");
    const TBool& transp = sthis.Inp("TranspS");
    const TUint32& currad = sthis.Inp("RadS");
    const CF_TdPoint& hookcoord = sthis.Inp("McPosS");
    const TInt& curmass = sthis.Inp("MassS");
    
    CF_TdVectF vforce(0.0, 0.0), newvel(0.0, 0.0);
    TInt i = 0;
    // Calculate the force from the hook
    if (selected) { 
	CF_TdVectF vdis(curcoord, CF_TdPointF(hookcoord));
	// If the ball in the trap then stop any movement
	if (vdis.Mod() < 10) {
	    sthis = newvel; return;
	}
	vforce += vdis * KHookSc;
	// Dempfing
	if (vforce.Mod() > KFrictForceManMov) 
	    vforce -= (curvel * KConstFriction); 
	else 
	    vforce = CF_TdPointF(0.0, 0.0); 
	printf("upd_vel %s: hcoord= %d, %d  ccoord= %f, %f  hforce= %f, %f\n", aObject->InstName(), hookcoord.iX, hookcoord.iY, 
		curcoord.iX, curcoord.iY, vforce.iX, vforce.iY);
    }
    // Calculate the resultant force from the balls 
    for (i = 0; sthis.Input("Coord", i) != NULL; i++) 
    {
	const CF_TdPointF& excoord = sthis.Inp("Coord", i);
	const TInt& exmass = sthis.Inp("Mass", i);
	const TUint32& exrad = sthis.Inp("Rad", i);
	if (exmass != 0) {
	    float r = (curcoord - excoord).Mod();
	    float force = (KInfl * curmass * exmass)/(r*r);
	    if (r > (currad + exrad)) 
		vforce += CF_TdVectF(curcoord, excoord)*(force/r);
	    else if (!transp) // Superposition
		vforce = (r == 0.0) ? CF_TdVectF(-KForceOfExtrs, -KForceOfExtrs) : CF_TdVectF(excoord, curcoord)*(KForceOfExtrs/r);
	}
    }
    newvel = curvel + vforce/curmass;
    // Check if there will be some collision at the next update. Use coordinate prediction for that
    CF_TdPointF newcoord = curcoord + curvel;
    for (i = 0; sthis.Input("Coord", i) != NULL; i++) 
    {
	const CF_TdPointF& excoord = sthis.Inp("Coord", i);
	const TInt& exmass = sthis.Inp("Mass", i);
	const TUint32& exrad = sthis.Inp("Rad", i);
	const CF_TdVectF& exvel = sthis.Inp("InpV", i);
	const TBool& extransp = sthis.Inp("Transp", i);
	if (exmass != 0 && !(transp && extransp)) {
	    CF_TdPointF newexcoord = excoord + exvel;
	    float newdis = GetDistance(newcoord, newexcoord);
	    float curdis = GetDistance(curcoord, excoord);
	    // Handle collision. If the ball already moving away, then ignore collistion
	    if ((newdis < (currad + exrad))) {
		CF_TdVectF curvel_norm, curvel_tang, exvel_norm, exvel_tang;
		TBool div = GetProjOfVel(newcoord, newexcoord, curvel, curvel_norm, curvel_tang);
		GetProjOfVel(newcoord, newexcoord, exvel, exvel_norm, exvel_tang);
		float mc1 = ((float) curmass - (float) exmass)/((float) curmass + (float) exmass);
		float mc2 = (2.0 * exmass)/((float) curmass + (float) exmass);
		if (!div)
		    newvel = curvel_norm * mc1 + exvel_norm * mc2 + curvel_tang;
	    }
	}
    }
    sthis = newvel;
}

TBool GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang)
{
    CF_TdVectF axle(aAngleBeg, aAngleEnd);
    float axle_m2 = axle.M2(), vel_m2 = aVel.M2();
    float axle_mod = sqrt(axle_m2), vel_mod = sqrt(vel_m2);
    float diff_m2 = (aVel - axle).M2();
    float cos_angle = (vel_m2 > 0.0) && (axle_mod > 0.0) ? (vel_m2 + axle_m2 - diff_m2)/(2 * vel_mod * axle_mod) : 1.0; 
    aVelNorm = (axle_mod > 0.0) ? axle * (vel_mod * cos_angle / axle_mod) : aVelNorm;
    aVelTang = aVel - aVelNorm;
    return (cos_angle < 0.0);
}

