
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
#include <fapext.h>
#include <stdlib.h>
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
const TInt KHookSc = 2;
// Default border hook stiffness coefficient, n/m
const TInt KBorderHookSc = 1000;

const TInt KMassMax = 1000;

// The radius of borders
const TInt KBorderRadius = 50000;
// The mass of borders
const TInt KBorderMass = 30000;

// Constant of friction
const float KConstFriction = 19.80;

// Static Force of friction when manual moving, n
const float KFrictForceManMov = 20.0;

// The base of ind of external inputs of ball's velocity
const TInt KBallVelExtInputsBase = 6;
// The base of ind of external inputs of ball's coordinates
const TInt KBallCoordExtInputsBase = 5;

// Force of extrusion while superposition
const TInt KForceOfExtrs = 20;

extern CAE_Env* fape;
/* Area painter */
extern MBallAreaWindow* fapainter;

_TEMPLATE_ TBool CAE_TState<CF_Rect>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CF_Rect>::DoOperation()
{
}

_TEMPLATE_ char* CAE_TState<CF_Rect>::DataToStr(TBool aCurr) const
{
    int buflen = Len();
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_Rect* data = (CF_Rect*) (aCurr? iCurr: iNew);
    sprintf(buf, "(%i,%i):(%i,%i)", data->iLeftUpper.iX, data->iLeftUpper.iY, data->iRightLower.iX, data->iRightLower.iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_Rect>::DataFromStr(const char* aStr, void *aData) const
{
}



_TEMPLATE_ TBool CAE_TState<CF_TdPoint>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ char* CAE_TState<CF_TdPoint>::DataToStr(TBool aCurr) const
{
    int buflen = 40;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_TdPoint* data = (CF_TdPoint*) (aCurr ? iCurr : iNew);
    sprintf(buf, "(%d,%d)", data->iX, data->iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_TdPoint>::DataFromStr(const char* aStr, void *aData) const
{
    CF_TdPoint* data = (CF_TdPoint*) aData;
    sscanf(aStr, "(%d,%d)", &(data->iX), &(data->iY));
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

_TEMPLATE_ char* CAE_TState<CF_TdPointF>::DataToStr(TBool aCurr) const
{
    int buflen = 40;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_TdPointF* data = (CF_TdPointF*) (aCurr ? iCurr : iNew);
    sprintf(buf, "(%f,%f)", data->iX, data->iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_TdPointF>::DataFromStr(const char* aStr, void *aData) const
{
    CF_TdPointF* data = (CF_TdPointF*) aData;
    sscanf(aStr, "(%f,%f)", &(data->iX), &(data->iY));
}

_TEMPLATE_ TBool CAE_TState<CF_TdVectF>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ char* CAE_TState<CF_TdVectF>::DataToStr(TBool aCurr) const
{
    int buflen = 40;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_TdVectF* data = (CF_TdVectF*) (aCurr ? iCurr : iNew);
    sprintf(buf, "(%f,%f)", data->iX, data->iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_TdVectF>::DataFromStr(const char* aStr, void *aData) const
{
    CF_TdVectF* data = (CF_TdVectF*) aData;
    sscanf(aStr, "(%f,%f)", &(data->iX), &(data->iY));
}



_TEMPLATE_ void CAE_TState<CF_TdVectF>::DoOperation()
{
}


    CFT_Area::CFT_Area(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd, CF_Rect *aRect)
: CAE_Object(aInstName, aMan), iBallAreaWnd(aBallAreaWnd), iInitRect(*aRect)
{
}

CFT_Area* CFT_Area::NewL(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd, CF_Rect *aRect)
{
    CFT_Area* self = new CFT_Area(aInstName, aMan, aBallAreaWnd, aRect);
    self->ConstructL();
    return self;
}

CFT_Area::~CFT_Area()
{
}

CAE_Object* CFT_Area::CreateBall(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aInstName, TBool aBorder)
{
    CAE_Object *area = fape->Root(); 
    char name[50];
    if (aInstName != NULL) {
	strcpy(name, aInstName);
    }
    else {
	sprintf(name, KBallName, area->CountCompWithType("Ball") + 1);
    }
    CAE_Object* ball = (CAE_Object *) area->FindByName("ball");
    // TODO [YB] To consider if any transf can be able to add element to area
    CAE_Object* newball = ball->CreateNewL(NULL, name, area);

    CF_TdPointF coord = {aCoordX, aCoordY};

    CAE_TState<TInt> *srad = (CAE_TState<TInt>*) newball->GetInput("Rad");
    CAE_TState<CF_TdPointF> *scoord = (CAE_TState<CF_TdPointF>*) newball->GetOutput("Coord");
    CAE_TState<TInt> *smass = (CAE_TState<TInt>*) newball->GetInput("Mass");

    *srad = aRad;
    *scoord= coord;
    *smass = aMass;
    // Confirm states new values because newly created ball can be updated this tick and new values will be lost
    // TODO [YB] Consider this situation. It is not obvious for developers
    srad->Confirm();
    scoord->Confirm();
    smass->Confirm();
    AddBallL(newball, !aBorder);
    return newball;
}

CFT_Ball* CFT_Area::CreateBorder(float aCoordX,  float aCoordY, const char* aInstName, TTransFun aHookUpdate)
{
    CAE_Object *bord = CreateBall(aCoordX, aCoordY, KBorderMass, KBorderRadius, aInstName, ETrue);	
    CAE_TState<TBool> *shookedperm = (CAE_TState<TBool>*) bord->GetInput("HookedPerm");
    CAE_TState<TBool> *stransp = (CAE_TState<TBool>*) bord->GetInput("Transp");

    CAE_Object* farea = fape->Root();
    CAE_TState<CF_Rect>* srect = (CAE_TState<CF_Rect>*) farea->GetInput("Rect");
    _FAP_ASSERT(srect != NULL);
    CAE_TState<CF_TdPoint> *smcpos = (CAE_TState<CF_TdPoint> *) bord->GetInput("McPos");
//    farea->LinkL(smcpos, srect, aHookUpdate);
    smcpos->SetInputL("McPos", srect);
    smcpos->SetTrans(TTransInfo(aHookUpdate));
    *shookedperm = TRUE;
    *stransp = TRUE;
    shookedperm->Confirm();
    stransp->Confirm();
#if 0
    // Bind hook of the border with area rectangle
    LinkL(bord->iMcPos, iRect, aHookUpdate);
    // Make the border hooked initially
    *(bord->iHookedPerm) = TRUE;
    *(bord->iTransp) = TRUE;
#endif
}

void CFT_Area::LinkBall(CAE_Object* aBallRec, CAE_Object* aBallExt)
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

void CFT_Area::AddBallL(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aName)
{
    CreateBall(aCoordX, aCoordY, aMass, aRad, aName);
}

// TODO [YB] Why any transf can have access to any env elements?
void CFT_Area::AddBallL(CAE_Object* aObBall, TBool aUseAreaHook)
{
    CAE_Object *area = fape->Root(); 
    _FAP_ASSERT(aObBall != NULL);

    CAE_TState<TInt> *sball_lbdown = (CAE_TState<TInt>*) aObBall->GetInput("LbDown");
    CAE_TState<TInt> *sarea_lbdown = (CAE_TState<TInt>*) area->GetInput("LbDown");
    CAE_TState<CF_TdPoint> *sball_mcpos = (CAE_TState<CF_TdPoint>*) aObBall->GetInput("McPos");
    CAE_TState<CF_TdPoint> *sarea_mcpos = (CAE_TState<CF_TdPoint>*) area->GetInput("McPos");

    sball_lbdown->SetInputL("LbDown", sarea_lbdown); 
    if (aUseAreaHook)
	sball_mcpos->SetInputL("McPos", sarea_mcpos); 
    int ctx = 0;
    CAE_Object* ball = (CAE_Object*) area->GetNextCompByType("ball", &ctx);
    while (ball != NULL)
    {
	if (ball != aObBall)
	{
	    LinkBall(ball, aObBall);
	    LinkBall(aObBall, ball); 
	}
	ball = (CAE_Object*) area->GetNextCompByType("ball", &ctx);
    }
}

void CFT_Area::ConstructL()
{
    //	TRACE ("WARNING !! Test\n");
    CAE_Object::ConstructL();
    iBeaconC = CAE_TState<TInt>::NewL(KAreaBeaconCName, this, TTransInfo());
    iBeaconX = CAE_TState<TInt>::NewL(KAreaBeaconXName, this, TTransInfo());
    iLbDown = CAE_TState<TInt>::NewL("LbDown", this, TTransInfo(), CAE_StateBase::EType_Input);
    iMcPos = CAE_TState<CF_TdPoint>::NewL("McPos", this, TTransInfo(), CAE_StateBase::EType_Input);
    iRect = CAE_TState<CF_Rect>::NewL(KAreaRectName, this, TTransInfo(), CAE_StateBase::EType_Input);
    *iRect = iInitRect;

    // Create borders
    CF_Rect rt = iInitRect;
    float midx, midy;
    midx = (rt.iRightLower.iX - rt.iLeftUpper.iX)/2.0;
    midy = (rt.iRightLower.iY - rt.iLeftUpper.iY)/2.0;
    printf("area_constr: lx= %d, ly= %d, rx= %d, ry= %d\n", rt.iLeftUpper.iX, rt.iLeftUpper.iY, rt.iRightLower.iX, rt.iRightLower.iY);
    CreateBorder(rt.iLeftUpper.iX-KBorderRadius, midy, KObjBorderLeftName, CAE_TRANS(UpdateBordHookLeft));	
    CreateBorder(rt.iRightLower.iX + KBorderRadius, midy, KObjBorderRightName, CAE_TRANS(UpdateBordHookRight));	
    CreateBorder(midx, rt.iLeftUpper.iY - KBorderRadius, KObjBorderTopName, CAE_TRANS(UpdateBordHookTop));	
    CreateBorder(midx, rt.iRightLower.iY + KBorderRadius, KObjBorderBottomName, CAE_TRANS(UpdateBordHookBottom));	

    // Create balls
    CreateBall(40, 40, 2000, 20, "Ball1");
    CreateBall(300, 300, 2000, 30, "Ball2");
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
    CAE_TState<CF_Rect> *sarect = (CAE_TState<CF_Rect> *) aState->Input("McPos");
    g_assert(sarect != NULL);
    const CF_Rect arect = sarect->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(arect.iLeftUpper.iX-KBorderRadius, midy); 
}

void CFT_Area::UpdateBordHookRight(CAE_State* aState)
{
    //printf("UpdateBordHookRight: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *sarect = (CAE_TState<CF_Rect> *) self->Input("McPos");
    g_assert(sarect != NULL);
    const CF_Rect arect = sarect->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(arect.iRightLower.iX + KBorderRadius, midy); 
}

void CFT_Area::UpdateBordHookTop(CAE_State* aState)
{
    //printf("UpdateBordHookRight: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *sarect = (CAE_TState<CF_Rect> *) self->Input("McPos");
    g_assert(sarect != NULL);
    const CF_Rect arect = sarect->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(midx, arect.iLeftUpper.iY - KBorderRadius); 
}

void CFT_Area::UpdateBordHookBottom(CAE_State* aState)
{
    //printf("UpdateBordHookRight: name = %s\n", aState->InstName());
    CAE_TState<CF_TdPoint> *self = (CAE_TState<CF_TdPoint> *) aState;
    CAE_TState<CF_Rect> *sarect = (CAE_TState<CF_Rect> *) self->Input("McPos");
    g_assert(sarect != NULL);
    const CF_Rect arect = sarect->Value();
    float midx, midy;
    midx = (arect.iRightLower.iX - arect.iLeftUpper.iX)/2.0;
    midy = (arect.iRightLower.iY - arect.iLeftUpper.iY)/2.0;
    *self = CF_TdPoint(midx, arect.iRightLower.iY + KBorderRadius); 
}

void CFT_Area::Draw()
{
    int ctx = 0;
    CFT_Ball* ball = (CFT_Ball*) GetNextCompByType("Ball", &ctx);

    while (ball != NULL)
    {
	ball->Draw();
	ball = (CFT_Ball*) GetNextCompByType("Ball", &ctx);
    }

    /*
    // Draw borders
    ctx = 0;
    CFT_Ball* bord = (CFT_Ball*) GetNextCompByType(KObUid_CAE_Object | KObUid_CAE_Var_Border, &ctx);
    

    while (bord != NULL)
    {
	bord->Draw();
	bord = (CFT_Ball*) GetNextCompByType(KObUid_CAE_Object | KObUid_CAE_Var_Border, &ctx);
    }
*/
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
    iCoord = CAE_TState<CF_TdPointF>::NewL(KBallCoordName, this, CAE_TRANS(UpdateCoord), CAE_StateBase::EType_Reg);
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
    iCoord->AddInputL("CoordS", iCoord);
    iCoord->AddInputL("MassS", iMass);
    iCoord->AddInputL("InpVS", iInpV);
    iCoord->AddInputL("McPosS", iMcPos);
    iCoord->AddInputL("MovedS", iMoved);
    iCoord->AddInputL("TranspS", iTransp);
    // The sign of movement depends on the event of mouse left button action and sign of permanantly hooked
    iMoved->AddInputL("LbDownS", iLbDown);
    iMoved->AddInputL("HookedPermS", iHookedPerm);
    // Velocity is an input. It depends internally:
    // On coordinate - the change of coordinate of itself changes the gravitation force, thus velocity
    // On mass - the change of mass influence on acceleration, thus on velocity
    // On radius - the collision (e.g. velocity) depends on radius
    // On the hook position (iMcPos)
    // Morover the velocity externally denend on coordinates and masses of other solids in the environment
    iInpV->AddInputL("CoordS", iCoord);
    iInpV->AddInputL("MassS", iMass);
    iInpV->AddInputL("RadS", iRad);
    iInpV->AddInputL("MovedS", iMoved);
    iInpV->AddInputL("McPosS", iMcPos);
    iInpV->AddInputL("TranspS", iTransp);
}


void CFT_Ball::UpdateCoord(CAE_State* aState)
{
    // TODO [YB] Consider Input() to return CAE_State (now casting is required)
    CAE_TState<TInt> *smass = (CAE_TState<TInt> *) aState->Input("MassS");
    TInt mass = ~*smass;

    if (mass != 0)
    {
	CAE_TState<CF_TdPointF> *sthis = (CAE_TState<CF_TdPointF> *) aState;
	CAE_TState<CF_TdPointF> *scoord = (CAE_TState<CF_TdPointF> *) aState->Input("CoordS");
	CAE_TState<CF_TdVectF> *sinpv = (CAE_TState<CF_TdVectF> *) aState->Input("InpVS");
	CAE_TState<TBool> *smoved = (CAE_TState<TBool>*) aState->Input("MovedS");
	CAE_TState<TBool> *stransp = (CAE_TState<TBool>*) aState->Input("TranspS");
	CAE_TState<TInt> *srad = (CAE_TState<TInt>*) aState->Input("RadS");
	CAE_TState<CF_TdPoint> *smcpos = (CAE_TState<CF_TdPoint>*) aState->Input("McPosS");

	CF_TdPointF coord = ~*scoord;
	CF_TdVectF vel = ~*sinpv;
	TBool selected = ~*smoved;
	TBool transp = ~*stransp;
	TInt rad = ~*srad;
	CF_TdPointF newcoord;

	newcoord.iX = coord.iX + vel.iX;
	newcoord.iY = coord.iY + vel.iY;

	// TODO [YB] "Moved" is not used here - to remove this input
	if (selected)
	{
	    // Selected. Verify if the ball trapped by the hook
	    CF_TdPoint mcpos = ~*smcpos;
	    float disx = (mcpos.iX - coord.iX);
	    float disy = (mcpos.iY - coord.iY);
	    float dis = sqrt(disx*disx + disy*disy);
#if 0
	    if (dis < rad)
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
	for (TInt i = 0; ; i++) 
	{
	    CAE_TState<CF_TdPointF> *sexcoord = (CAE_TState<CF_TdPointF> *) aState->Input("Coord", i);
	    if (sexcoord == NULL) break;
	    CAE_TState<TInt> *sexrad = (CAE_TState<TInt>*) aState->Input("Rad", i);
	    CAE_TState<CF_TdVectF> *sexvel = (CAE_TState<CF_TdVectF> *) aState->Input("InpV", i);
	    CAE_TState<TBool> *sextransp = (CAE_TState<TBool>*) aState->Input("Transp", i);

	    CF_TdPointF excoord = ~*sexcoord;
	    TInt exrad = ~*sexrad;
	    CF_TdVectF exvel = ~*sexvel;
	    TBool extransp = ~*sextransp;

	    CF_TdPointF newexcoord;
	    newexcoord.iX = excoord.iX + exvel.iX;
	    newexcoord.iY = excoord.iY + exvel.iY;
	    float r = GetDistance(newcoord, newexcoord);
	    float curdis = GetDistance(coord, excoord);
	    TBool superpos = ((curdis < rad + exrad) && !transp);
	    // If there is no superposition, and collistion happens, fix coord
	    if (!superpos && ((r <= (rad + exrad)) && !(transp && extransp)))
	    {
		// Collision will happen next tick
		newcoord.iX = coord.iX;
		newcoord.iY = coord.iY;
	    }
	}

	(*sthis) = newcoord;
	//TODO YB With this line there is some flickering in the area.
	// It works fine without this line, i.e. without cleaning the old image of the ball
	// Not understand how can it be.
	fapainter->redraw(CF_TdPoint(coord.iX, coord.iY), rad, ETrue);
	fapainter->redraw(CF_TdPoint(newcoord.iX, newcoord.iY), rad, EFalse);
    }
}

void CFT_Ball::UpdateSelected(CAE_State* aState)
{
    CAE_TState<TBool> *sthis = (CAE_TState<TBool> *) aState;
    CAE_TState<TInt> *sdown = (CAE_TState<TInt>*) aState->Input("LbDownS");
    CAE_TState<TBool> *shookedperm = (CAE_TState<TBool> *) aState->Input("HookedPermS");
    g_assert(shookedperm != NULL);

    TInt down = ~*sdown;
    TBool hookedperm = ~*shookedperm;

    TBool sel = ~*sthis;
    if (hookedperm) {
	sel = TRUE;
    }
    else if (sel) {
	if (!down) 
	    sel = EFalse;
    }
    else {
	if (down) {
	    CAE_TState<CF_TdPoint> *smcpos = (CAE_TState<CF_TdPoint>*) aState->Input("McPosS");
	    CAE_TState<CF_TdPointF> *scoord = (CAE_TState<CF_TdPointF> *) aState->Input("CoordS");
	    CAE_TState<TInt> *srad = (CAE_TState<TInt>*) aState->Input("RadS");
	    CF_TdPoint mcpos = ~*smcpos;
	    CF_TdPointF coord = ~*scoord;
	    TInt rad = ~*srad;

	    float disx = (mcpos.iX - coord.iX);
	    float disy = (mcpos.iY - coord.iY);
	    float dis = sqrt(disx*disx + disy*disy);
	    sel = dis < rad;
	    if (sel)
		printf("CFT_Ball::UpdateSelected, selected\n");
	}
	else {
	    sel = EFalse;
	}
    }

    *sthis = sel;
}

void CFT_Ball::UpdateVelocity(CAE_State* aState)
{
    CAE_TState<CF_TdVectF> *sthis = (CAE_TState<CF_TdVectF> *) aState;

    CAE_TState<TInt> *smass = (CAE_TState<TInt> *) aState->Input("MassS");
    CAE_TState<CF_TdPointF> *scoord = (CAE_TState<CF_TdPointF> *) aState->Input("CoordS");
    CAE_TState<CF_TdVectF> *sinpv = (CAE_TState<CF_TdVectF> *) aState->Input("InpVS");
    CAE_TState<TBool> *smoved = (CAE_TState<TBool>*) aState->Input("MovedS");
    CAE_TState<TBool> *stransp = (CAE_TState<TBool>*) aState->Input("TranspS");
    CAE_TState<TInt> *srad = (CAE_TState<TInt>*) aState->Input("RadS");
    CAE_TState<CF_TdPoint> *smcpos = (CAE_TState<CF_TdPoint>*) aState->Input("McPosS");

    TInt curmass = ~*smass;
    CF_TdPointF curcoord = ~*scoord;
    CF_TdVectF curvel = ~*sinpv;
    TBool selected = ~*smoved;
    TBool transp = ~*stransp;
    TInt currad = ~*srad;
    CF_TdPoint hookcoord = ~*smcpos;
    
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
	    *sthis = newvel;
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
	    vforce.iX -= (curvel.iX) * KConstFriction; 
	    vforce.iY -= (curvel.iY) * KConstFriction; 
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
    for (i = 0; ; i++) 
    {
	CAE_TState<CF_TdPointF> *sexcoord = (CAE_TState<CF_TdPointF> *) aState->Input("Coord", i);
	if (sexcoord == NULL) break;
	CAE_TState<TInt> *sexmass = (CAE_TState<TInt> *) aState->Input("Mass", i);
	CAE_TState<TInt> *sexrad = (CAE_TState<TInt>*) aState->Input("Rad", i);
	CAE_TState<TBool> *sextransp = (CAE_TState<TBool>*) aState->Input("Transp", i);

	CF_TdPointF excoord = ~*sexcoord;
	TInt exmass = ~*sexmass;
	TInt exrad = ~*sexrad;
	TBool extransp = ~*sextransp;

	if (exmass != 0)
	{
	    float r = GetDistance(curcoord, excoord);
	    float force = (KInfl * curmass * exmass)/(r*r);
	    if (r > (currad + exrad))
	    {
		vforce.iX += ((excoord.iX - curcoord.iX)*force)/r;
		vforce.iY += ((excoord.iY - curcoord.iY)*force)/r;
	    }
	    else if (!transp) {
		// Superposition
		vforce.iX = (r == 0.0) ? - KForceOfExtrs : -((excoord.iX - curcoord.iX)*KForceOfExtrs)/r;
		vforce.iY = (r == 0.0) ? - KForceOfExtrs : -((excoord.iY - curcoord.iY)*KForceOfExtrs)/r;
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
    for (i = 0; ; i++) 
    {
	CAE_TState<CF_TdPointF> *sexcoord = (CAE_TState<CF_TdPointF> *) aState->Input("Coord", i);
	if (sexcoord == NULL) break;
	CAE_TState<TInt> *sexmass = (CAE_TState<TInt> *) aState->Input("Mass", i);
	CAE_TState<TInt> *sexrad = (CAE_TState<TInt>*) aState->Input("Rad", i);
	CAE_TState<CF_TdVectF> *sexvel = (CAE_TState<CF_TdVectF> *) aState->Input("InpV", i);
	CAE_TState<TBool> *sextransp = (CAE_TState<TBool>*) aState->Input("Transp", i);

	CF_TdPointF excoord = ~*sexcoord;
	TInt exmass = ~*sexmass;
	TInt exrad = ~*sexrad;
	CF_TdVectF exvel = ~*sexvel;
	TBool extransp = ~*sextransp;

	if (exmass != 0 && !(transp && extransp))
	{
	    CF_TdPointF newexcoord;
	    newexcoord.iX = excoord.iX + exvel.iX;
	    newexcoord.iY = excoord.iY + exvel.iY;
	    float r = GetDistance(newcoord, newexcoord);
	    float curdis = GetDistance(curcoord, excoord);
	    if (curdis <= (currad + exrad)) {
		// Superposition
	    }
	    else if (r < (currad + exrad))
	    {
		// Collistion
		// TODO [YB] There can be that balls collisioned initially for instance when
		// ball is "inside" border. In this case balls cannot be moved outside even by the hook
		// if velocity is 0. It should be allowed to hook it out
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
    (*sthis) = newvel;
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
    TUint8 cblue = 0xff - (mass*0xff)/KMassMax;
    if (cblue < 0)
	cblue = 0x00;
    TUint8 cgreen = 0x00;
    if (selected)
	cgreen |= 0xff;
    iBallAreaWnd->drawBall(CF_TdPoint(centx, centy), rad, CF_TdColor(0x00, cgreen, cblue));
}


// Border 



CFT_Border* CFT_Border::NewL(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd)
{
    CFT_Border* self = new CFT_Border(aInstName, aMan, aBallAreaWnd);
    self->ConstructL();
    return self;
};


