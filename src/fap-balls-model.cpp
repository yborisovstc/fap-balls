
/* Copyright (c) 2009, Yuri Borisov
 * All rights reserved
 */

//*********************************************************
// FAP test 1. Objects 
// 01-Nov-2008 Yuri Borisov           Aligned with FAPWS FAPWS_REL_21SEP06
// Yuri Borisov  15-Dec-08             Ball states transition moved from the area to ball
// Yuri Borisov  25-Dec-08             Fixed error within the velocity update - now acceleration is calculated
//                                     added input iMoved for coord and velocity states, added modelling of collision
// Yuri Borisov  29-Dec-08             Added borders
// Yuri Borisov  20-Jan-08             Implemented not direct ball events
//*********************************************************

#include <fapplat.h>
#include <stdio.h>
#include "fap-balls-model.h"

const char* KAreaBeaconCName = "BeacC";
const char* KAreaBeaconXName = "BeacX";
const char* KAreaRectName = "AreaRect";

const char* KBallCoordName = "Coord";
const char* KBallMassName = "Mass";
const char* KBallRadiusName = "Radius";
const char* KBallVelName = "Velocity";
const char* KBallHookedPerm = "HookedPerm";
const char* KBallTranspName = "Transp";

const char* KObjBorderLeftName = "BorderLeft";
const char* KObjBorderRightName = "BorderRight";
const char* KObjBorderTopName = "BorderTop";
const char* KObjBorderBottomName = "BorderBottom";
const char* KBallName = "Ball%d";

// Gravitation constant
const TInt KInfl = 9;

// Area maximum dimension, points
const TInt KAreaMaxDim = 1200;

// Default hook stiffness coefficient, n/m
const TInt KHookSc = 10;
// Default border hook stiffness coefficient, n/m
const TInt KBorderHookSc = 1000;

const TInt KMassMax = 1000;

// The radius of borders
const TInt KBorderRadius = 50000;
// The mass of borders
const TInt KBorderMass = 30000;

// Constant of friction
const float KConstFriction = 0.50;

// Force of friction when manual moving, n
const float KFrictForceManMov = 200.0;

// The base of ind of external inputs of ball's velocity
const TInt KBallVelExtInputsBase = 6;
// The base of ind of external inputs of ball's coordinates
const TInt KBallCoordExtInputsBase = 5;

_TEMPLATE_ TBool CAE_TState<CF_Rect>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CF_Rect>::DoOperation()
{
}


_TEMPLATE_ TBool CAE_TState<CF_TdPoint>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CF_TdPoint>::DoOperation()
{
}


_TEMPLATE_ TBool CAE_TState<CF_TdPointF>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}


_TEMPLATE_ void CAE_TState<CF_TdPointF>::DoOperation()
{
}


_TEMPLATE_ TBool CAE_TState<CF_TdVectF>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}


_TEMPLATE_ void CAE_TState<CF_TdVectF>::DoOperation()
{
}


    CFT_Area::CFT_Area(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd)
: CAE_Object(aInstName, aMan), iBallAreaWnd(aBallAreaWnd)
{
}

CFT_Area* CFT_Area::NewL(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd)
{
    CFT_Area* self = new CFT_Area(aInstName, aMan, aBallAreaWnd);
    self->ConstructL();
    return self;
}

CFT_Area::~CFT_Area()
{
}

CFT_Ball* CFT_Area::CreateBall(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aInstName, TBool aBorder)
{
    char name[50];
    if (aInstName != NULL)
    {
	strcpy(name, aInstName);
    }
    else
    {
	sprintf(name, KBallName, CountCompWithType(KObUid_CAE_Object | KObUid_CAE_Var_Ball) + 1);
    }
    CFT_Ball* ball;
    if (aBorder)
    {
	ball = CFT_Ball::NewL(aInstName, this, iBallAreaWnd);
    }
    else
    {
	// TODO YB Why border is created on aBorder FALSE ??
	ball = CFT_Border::NewL(aInstName, this, iBallAreaWnd);
    }
    CF_TdPointF coord = {aCoordX, aCoordY};
    (*ball->iCoord) = coord;
    (*ball->iMass) = aMass;
    (*ball->iRad) = aRad;
    (*ball->iHookSc) = aBorder ? KBorderHookSc: KHookSc;
    AddBallL(ball, !aBorder);
    return ball;
}


void CFT_Area::LinkBall(CFT_Ball* aBallRec, CFT_Ball* aBallExt)
{
    aBallRec->iInpV->AddInputL(aBallExt->iCoord);
    aBallRec->iInpV->AddInputL(aBallExt->iMass);
    aBallRec->iInpV->AddInputL(aBallExt->iRad);
    aBallRec->iInpV->AddInputL(aBallExt->iInpV);
    aBallRec->iInpV->AddInputL(aBallExt->iTransp);

    aBallRec->iCoord->AddInputL(aBallExt->iCoord);
    aBallRec->iCoord->AddInputL(aBallExt->iRad);
    aBallRec->iCoord->AddInputL(aBallExt->iInpV);
    aBallRec->iCoord->AddInputL(aBallExt->iTransp);
}

void CFT_Area::AddBallL(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aName)
{
    CreateBall(aCoordX, aCoordY, aMass, aRad, aName);
}

void CFT_Area::AddBallL(CFT_Ball* aObBall, TBool aUseAreaHook)
{
    _FAP_ASSERT(aObBall != NULL);
    if ((aObBall->ObjectUid() & KObUid_ModifTypeMask) == KObUid_CAE_Var_Ball)
    {
	LinkL(aObBall->iLbDown, iLbDown); 
	if (aUseAreaHook)
	    LinkL(aObBall->iMcPos, iMcPos);
	int ctx = 0;
	CFT_Ball* ball = (CFT_Ball*) GetNextCompByType(KObUid_CAE_Object | KObUid_CAE_Var_Ball, &ctx);
	while (ball != NULL)
	{
	    if (ball != aObBall)
	    {
		LinkBall(ball, aObBall);
		LinkBall(aObBall, ball); 
	    }
	    ball = (CFT_Ball*) GetNextCompByType(KObUid_CAE_Object | KObUid_CAE_Var_Ball, &ctx);
	}
    }
}

void CFT_Area::ConstructL()
{
    CF_Rect rt = iBallAreaWnd->boundaryRect();
    //	TRACE ("WARNING !! Test\n");
    CAE_Object::ConstructL();
    iBeaconC = CAE_TState<TInt>::NewL(KAreaBeaconCName, this, TTransInfo());
    iBeaconX = CAE_TState<TInt>::NewL(KAreaBeaconXName, this, TTransInfo());
    iLbDown = CAE_TState<TInt>::NewL("LbDown", this, TTransInfo(), CAE_StateBase::EType_Input);
    iMcPos = CAE_TState<CF_TdPoint>::NewL("McPos", this, TTransInfo(), CAE_StateBase::EType_Input);
    iRect = CAE_TState<CF_Rect>::NewL(KAreaRectName, this, TTransInfo(), CAE_StateBase::EType_Input);

    // Create borders
    float midx, midy;
    midx = (rt.iRightLower.iX - rt.iLeftUpper.iX)/2.0;
    midy = (rt.iRightLower.iY - rt.iLeftUpper.iY)/2.0;
    printf("area_constr: lx= %d, ly= %d, rx= %d, ry= %d\n", rt.iLeftUpper.iX, rt.iLeftUpper.iY, rt.iRightLower.iX, rt.iRightLower.iY);
    CFT_Ball *bord_left, *bord_right, *bord_top, *bord_bottom = NULL;
    bord_left = CreateBall(rt.iLeftUpper.iX-KBorderRadius, midy, KBorderMass, KBorderRadius, KObjBorderLeftName, ETrue);	
    // Bind hook of the border with area rectangle
    LinkL(bord_left->iMcPos, iRect, CAE_TRANS(UpdateBordHookLeft));
    // Make the border hooked initially
    *(bord_left->iHookedPerm) = TRUE;
    *(bord_left->iTransp) = TRUE;
    bord_right = CreateBall(rt.iRightLower.iX + KBorderRadius, midy, KBorderMass, KBorderRadius, KObjBorderRightName, ETrue);	
    // Bind hook of the border with area rectangle
    LinkL(bord_right->iMcPos, iRect, CAE_TRANS(UpdateBordHookRight));
    // Make the border hooked initially
    *(bord_right->iHookedPerm) = TRUE;
    *(bord_right->iTransp) = TRUE;
    bord_top = CreateBall(midx, rt.iLeftUpper.iY - KBorderRadius, KBorderMass, KBorderRadius, KObjBorderTopName, ETrue);	
    LinkL(bord_top->iMcPos, iRect, CAE_TRANS(UpdateBordHookTop));
    *(bord_top->iHookedPerm) = TRUE;
    *(bord_top->iTransp) = TRUE;
    bord_bottom = CreateBall(midx, rt.iRightLower.iY + KBorderRadius, KBorderMass, KBorderRadius, KObjBorderBottomName, ETrue);	
    LinkL(bord_bottom->iMcPos, iRect, CAE_TRANS(UpdateBordHookBottom));
    *(bord_bottom->iHookedPerm) = TRUE;
    *(bord_bottom->iTransp) = TRUE;

    // Create balls
//    CreateBall(40, 40, 200, 20, "Ball1");
//    CreateBall(300, 300, 20, 30, "Ball2");
    CreateBall(200, 400, 3000, 40, "Ball3");
#if 0
    CreateBall(300, 300, 200, 40, "Ball2");
    CreateBall(400, 300, 300, 60, "Ball3");
#endif
}

void CFT_Area::UpdateInit(CAE_State* /*aState*/)
{
}

void CFT_Area::UpdateBordHookLeft(CAE_State* aState)
{
    //printf("UpdateBordHookLeft: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *arectst = (CAE_TState<CF_Rect> *) self->Input(KAreaRectName);
    g_assert(arectst != NULL);
    const CF_Rect arect = arectst->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(arect.iLeftUpper.iX-KBorderRadius, midy); 
}

void CFT_Area::UpdateBordHookRight(CAE_State* aState)
{
    //printf("UpdateBordHookRight: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *arectst = (CAE_TState<CF_Rect> *) self->Input(KAreaRectName);
    g_assert(arectst != NULL);
    const CF_Rect arect = arectst->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(arect.iRightLower.iX + KBorderRadius, midy); 
}

void CFT_Area::UpdateBordHookTop(CAE_State* aState)
{
    //printf("UpdateBordHookRight: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *arectst = (CAE_TState<CF_Rect> *) self->Input(KAreaRectName);
    g_assert(arectst != NULL);
    const CF_Rect arect = arectst->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(midx, arect.iLeftUpper.iY - KBorderRadius); 
}

void CFT_Area::UpdateBordHookBottom(CAE_State* aState)
{
    //printf("UpdateBordHookRight: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *arectst = (CAE_TState<CF_Rect> *) self->Input(KAreaRectName);
    g_assert(arectst != NULL);
    const CF_Rect arect = arectst->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(midx, arect.iRightLower.iY + KBorderRadius); 
}

void CFT_Area::Draw()
{
    int ctx = 0;
    CFT_Ball* ball = (CFT_Ball*) GetNextCompByType(KObUid_CAE_Object | KObUid_CAE_Var_Ball, &ctx);

    while (ball != NULL)
    {
	ball->Draw();
	ball = (CFT_Ball*) GetNextCompByType(KObUid_CAE_Object | KObUid_CAE_Var_Ball, &ctx);
    }
}


// CFT_Ball

void VectFLogFun(char* aBuf, CAE_StateBase* aState)
{
    if (aState->Len() != sizeof(CF_TdVectF))
    {
	strcat(aBuf, "Error!");
    }
    else
    {
	CF_TdVectF* data = (CF_TdVectF*) aState->iCurr;
	sprintf(aBuf, "<%f,%f>", data->iX, data->iY);
    }
}

CFT_Ball* CFT_Ball::NewL(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd)
{
    CFT_Ball* self = new CFT_Ball(aInstName, aMan, aBallAreaWnd);
    self->ConstructL();
    return self;
};


void CFT_Ball::ConstructL()
{
    CAE_Object::ConstructL();
    iCoord = CAE_TState<CF_TdPointF>::NewL(KBallCoordName, this, CAE_TRANS(UpdateCoord), CAE_StateBase::EType_Reg, VectFLogFun);
    iMass = CAE_TState<TInt>::NewL(KBallMassName, this, TTransInfo(), CAE_StateBase::EType_Input);
    iInpV = CAE_TState<CF_TdVectF>::NewL(KBallVelName, this, CAE_TRANS(UpdateVelocity), CAE_StateBase::EType_Input);
    iRad = CAE_TState<TInt>::NewL(KBallRadiusName, this, TTransInfo(), CAE_StateBase::EType_Input);
    iMoved = CAE_TState<TBool>::NewL("Moved", this, CAE_TRANS(UpdateSelected));
    iLbDown = CAE_TState<TInt>::NewL("LbDown", this, TTransInfo(), CAE_StateBase::EType_Input);
    iMcPos = CAE_TState<CF_TdPoint>::NewL("McPos", this, TTransInfo(), CAE_StateBase::EType_Input);
    iHookSc = CAE_TState<TInt>::NewL("HookSc", this, TTransInfo(), CAE_StateBase::EType_Input);
    iHookedPerm = CAE_TState<TBool>::NewL(KBallHookedPerm, this, TTransInfo(), CAE_StateBase::EType_Input);
    iTransp = CAE_TState<TBool>::NewL(KBallTranspName, this, TTransInfo(), CAE_StateBase::EType_Input);
    // Specify inputs for states transition
    // Coordinate depends:
    // on velocity - clear dependency 
    // on itself -even if velocity is unchanged the coordinate have to be changed - free movement
    iCoord->AddInputL(iCoord);
    iCoord->AddInputL(iInpV);
    iCoord->AddInputL(iMcPos);
    iCoord->AddInputL(iMoved);
    iCoord->AddInputL(iTransp);
    // The sign of movement depends on the event of mouse left button action and sign of permanantly hooked
    iMoved->AddInputL(iLbDown);
    iMoved->AddInputL(iHookedPerm);
    // Velocity is an input. It depends internally:
    // On coordinate - the change of coordinate of itself changes the gravitation force, thus velocity
    // On mass - the change of mass influence on acceleration, thus on velocity
    // On radius - the collision (e.g. velocity) depends on radius
    // On the hook position (iMcPos)
    // Morover the velocity externally denend on coordinates and masses of other solids in the environment
    iInpV->AddInputL(iCoord);
    iInpV->AddInputL(iMass);
    iInpV->AddInputL(iRad);
    iInpV->AddInputL(iMoved);
    iInpV->AddInputL(iMcPos);
    iInpV->AddInputL(iTransp);
}


void CFT_Ball::UpdateCoord(CAE_State* aState)
{
    TInt mass = ~*iMass;

    if (mass != 0)
    {
	CF_TdPointF coord = ~*iCoord;
	CF_TdVectF vel = ~*iInpV;
	TBool selected = ~*iMoved;
	TBool transp = ~*iTransp;
	TInt currad = iRad->Value();
	CF_TdPointF newcoord;

	newcoord.iX = coord.iX + vel.iX;
	newcoord.iY = coord.iY + vel.iY;

	if (selected)
	{
	    // Selected. Verify if the ball trapped by the hook
	    CF_TdPoint mcpos = ~*iMcPos;
	    float disx = (mcpos.iX - coord.iX);
	    float disy = (mcpos.iY - coord.iY);
	    float dis = sqrt(disx*disx + disy*disy);
#if 0
	    if (dis < currad)
	    {
		// Yes. The ball in the trap. Stop the movement
		CF_TdVectF newvel = {0.0, 0.0};
		(*iInpV) = newvel;
		newcoord.iX = mcpos.iX;
		newcoord.iY = mcpos.iY;
	    }
#endif
	}
	//
	// Check if there will be some collision at the next update
	for (TInt i = KBallCoordExtInputsBase; aState->Input(i) != NULL; ) 
	{
	    CF_TdPointF excoord = GetInp(aState, i++, KBallCoordName, excoord);
	    TInt exrad = GetInp(aState, i++, KBallRadiusName, exrad);
	    CF_TdVectF exvel = GetInp(aState, i++, KBallVelName, exvel);
	    TBool extransp = GetInp(aState, i++, KBallTranspName, extransp);

	    CF_TdPointF newexcoord;
	    newexcoord.iX = excoord.iX + exvel.iX;
	    newexcoord.iY = excoord.iY + exvel.iY;
	    float r = GetDistance(newcoord, newexcoord);
	    if ((r <= (currad + exrad)) && !(transp && extransp))
	    {
		// Collision will happen next tick
		newcoord.iX = coord.iX;
		newcoord.iY = coord.iY;
	    }
	}

	(*iCoord) = newcoord;
	int rad = ~*iRad;
	//TODO YB With this line there is some flickering in the area.
	// It works fine without this line, i.e. without cleaning the old image of the ball
	// Not understand how can it be.
//	iBallAreaWnd->redraw(CF_TdPoint(coord.iX, coord.iY), rad, ETrue);
	iBallAreaWnd->redraw(CF_TdPoint(newcoord.iX, newcoord.iY), rad, EFalse);
    }
}

void CFT_Ball::UpdateSelected(CAE_State* aState)
{
    TInt down = ~*iLbDown;
    CAE_TState<TBool> *hooked_perm = (CAE_TState<TBool> *) aState->Input(KBallHookedPerm);
    g_assert(hooked_perm != NULL);
    TBool sel = EFalse;
    if (hooked_perm->Value())
    {
	sel = TRUE;
    }
    else
    {
	if (down)
	{
	    CF_TdPoint mcpos = ~*iMcPos;
	    CF_TdPointF coord = ~*iCoord;
	    float disx = (mcpos.iX - coord.iX);
	    float disy = (mcpos.iY - coord.iY);
	    float dis = sqrt(disx*disx + disy*disy);
	    sel = dis < ~*iRad;
	}
	else
	{
	    sel = EFalse;
	}
    }
    printf("CFT_Ball::UpdateSelected, sel=%d\n", sel);

    *iMoved = sel;
}

void CFT_Ball::UpdateVelocity(CAE_State* aState)
{
    TInt curmass = iMass->Value();
    TInt currad = iRad->Value();
    CF_TdPointF curcoord = iCoord->Value();
    CF_TdPoint hookcoord = iMcPos->Value();
    CF_TdVectF curvel = iInpV->Value();
    TBool selected = ~*iMoved;
    TBool transp = ~*iTransp;
    CF_TdVectF vforce = {0.0, 0.0};
    CF_TdVectF newvel = {0.0, 0.0};
    TInt i = 0;
    //printf("CFT_Ball::UpdateVelocity: %s, mass: %d, currad, %d, coord: %f^%f\n", InstName(), curmass, currad, curcoord.iX, curcoord.iY);
    //
    // Calculate the force from the hook
    if (selected)
    { 

	float disx = (hookcoord.iX - curcoord.iX);
	float disy = (hookcoord.iY - curcoord.iY);
	float dis = sqrt(disx*disx + disy*disy);
	//if (dis < currad)
	if (dis < 10)
	{
	    // Yes. The ball in the trap. Stop any movement
	    (*iInpV) = newvel;
	    return;
	}

	vforce.iX += disx * KHookSc;
	vforce.iY += disy * KHookSc;

	// Dempfing
	float cvelmod = (sqrt(curvel.iX*curvel.iX + curvel.iY*curvel.iY)); 
	float force = (sqrt(vforce.iX*vforce.iX + vforce.iY*vforce.iY)); 
#if 0
	if (cvelmod > 10.0)
	{
	    vforce.iX -= (curvel.iX/cvelmod) * KFrictForceManMov; 
	    vforce.iY -= (curvel.iY/cvelmod) * KFrictForceManMov; 
	}
#endif
	if (force > KFrictForceManMov)
	{
	    vforce.iX -= (curvel.iX) * KFrictForceManMov; 
	    vforce.iY -= (curvel.iY) * KFrictForceManMov; 
	}
	else
	{
	    vforce.iX = 0.0; 
	    vforce.iY = 0.0; 
	}
	printf("upd_vel %s: hcoord= %d, %d  ccoord= %f, %f  hforce= %f, %f\n", InstName(), hookcoord.iX, hookcoord.iY, 
		curcoord.iX, curcoord.iY, vforce.iX, vforce.iY);
    }

    // Calculate the resultant force from the balls 
    // TODO YB Using the  method GetInp below is very mess 
    for (i = KBallVelExtInputsBase; aState->Input(i) != NULL; ) // 1..2 - ball's internal inputs
    {
	CF_TdPointF excoord = GetInp(aState, i++, KBallCoordName, excoord);
	TInt exmass = GetInp(aState, i++, KBallMassName, exmass);
	TInt exrad = GetInp(aState, i++, KBallRadiusName, exrad);
	i++;
	i++;
	if (exmass != 0)
	{
	    float r = GetDistance(curcoord, excoord);
	    float force = (KInfl * curmass * exmass)/(r*r);
	    if (r > (currad + exrad))
	    {
		vforce.iX += ((excoord.iX - curcoord.iX)*force)/r;
		vforce.iY += ((excoord.iY - curcoord.iY)*force)/r;
	    }
	}
    }

    // Correct the velocity
    if (curmass > 0.0) // Exclude the case when the mass is not correct yet
    {
	newvel.iX = curvel.iX + vforce.iX/curmass;
	newvel.iY = curvel.iY + vforce.iY/curmass;
	//printf("CFT_Ball::UpdateVelocity: force: %f^%f\n", vforce.iX, vforce.iY);
#if 0
	if (selected)
	{
	    // Dempfing 
	    // TODO YB - to use more correct dempfing model
	    newvel.iX -= newvel.iX * KConstFriction;
	    newvel.iY -= newvel.iY * KConstFriction;
	}
#endif
    }

    // Predict the updated coordinate
    CF_TdPointF newcoord;
    newcoord.iX = curcoord.iX + curvel.iX;
    newcoord.iY = curcoord.iY + curvel.iY;

    // Check if there will be some collision at the next update
    for (i = KBallVelExtInputsBase; aState->Input(i) != NULL; ) // 1..2 - ball's internal inputs
    {
	CF_TdPointF excoord = GetInp(aState, i++, KBallCoordName, excoord);
	TInt exmass = GetInp(aState, i++, KBallMassName, exmass);
	TInt exrad = GetInp(aState, i++, KBallRadiusName, exrad);
	CF_TdVectF exvel = GetInp(aState, i++, KBallVelName, exvel);
	TBool extransp = GetInp(aState, i++, KBallTranspName, extransp);

	if (exmass != 0 && !(transp && extransp))
	{
	    CF_TdPointF newexcoord;
	    newexcoord.iX = excoord.iX + exvel.iX;
	    newexcoord.iY = excoord.iY + exvel.iY;
	    float r = GetDistance(newcoord, newexcoord);
	    if (r <= (currad + exrad))
	    {
		printf("CFT_Ball::UpdateVelocity: r: %f, currad: %d, exrad: %d\n", r, currad, exrad);
		// Get the projections of interacting balls
		CF_TdVectF curvel_norm, curvel_tang, exvel_norm, exvel_tang;
		GetProjOfVel(newexcoord, newcoord, curvel, curvel_norm, curvel_tang);
		GetProjOfVel(newexcoord, newcoord, exvel, exvel_norm, exvel_tang);
		// Calculate the new velocity. Only normal component is affected
		float mc1 = ((float) curmass - (float) exmass)/((float) curmass + (float) exmass);
		float mc2 = (2.0 * exmass)/((float) curmass + (float) exmass);
		newvel.iX = curvel_norm.iX * mc1 + exvel_norm.iX * mc2 + curvel_tang.iX;
		newvel.iY = curvel_norm.iY * mc1 + exvel_norm.iY * mc2 + curvel_tang.iY;
	    }
	}
    }
    (*iInpV) = newvel;
    //printf("CFT_Ball::UpdateVelocity: %s, old: %f, %f, new: %f, %f\n", InstName(), curvel.iX, curvel.iY, newvel.iX, newvel.iY);
}

void CFT_Ball::GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang)
{
    CF_TdPointF axle;
    axle.iX = aAngleEnd.iX - aAngleBeg.iX;
    axle.iY = aAngleEnd.iY - aAngleBeg.iY;
    float axle_m2 = axle.iX * axle.iX + axle.iY * axle.iY;
    float axle_mod = sqrt(axle_m2);
    CF_TdPointF diff;
    float vel_m2 = aVel.iX * aVel.iX + aVel.iY * aVel.iY;
    diff.iX = aVel.iX - axle.iX;				
    diff.iY = aVel.iY - axle.iY;				
    float diff_m2 = diff.iX * diff.iX + diff.iY * diff.iY;
    float vel_mod = sqrt(vel_m2);
    float cos_angle;
    if (vel_m2 > 0.0)
	cos_angle = (vel_m2 + axle_m2 - diff_m2)/(2 * vel_mod * axle_mod);
    else
	cos_angle = 1.0;
    // Calculate the normal component of current velocity
    // Module of normal component of current velocity
    float vel_norm_mod = vel_mod * cos_angle;
    aVelNorm.iX = axle.iX * vel_norm_mod / axle_mod;
    aVelNorm.iY = axle.iY * vel_norm_mod / axle_mod;
    // Calculate the tangential component of current velocity
    aVelTang.iX = aVel.iX - aVelNorm.iX;
    aVelTang.iY = aVel.iY - aVelNorm.iY;
}



void CFT_Ball::Draw()
{
    int rad = ~*iRad;
    TBool selected = ~*iMoved;
    CF_TdPointF coord;
    long centx, centy;
    coord = ~*iCoord;
    centx = coord.iX;
    centy = coord.iY;
    int mass = ~*iMass;
    //TUint8 cblue = 0xff - (mass*0xff)/KMassMax;
    TUint8 cblue = 0xff;
    if (cblue < 0)
	cblue = 0x00;
    TUint8 cgreen = 0x00;
    if (selected)
	cgreen |= 0xff;
    iBallAreaWnd->drawBall(CF_TdPoint(centx, centy), rad, CF_TdColor(0x00, cgreen, cblue));
}

