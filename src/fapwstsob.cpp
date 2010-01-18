
//*********************************************************
// FAP test 1. Objects 
// 01-Nov-2008 Yuri Borisov           Aligned with FAPWS FAPWS_REL_21SEP06
// Yuri Borisov  15-Dec-08             Ball states transition moved from the area to ball
// Yuri Borisov  25-Dec-08             Fixed error within the velocity update - now acceleration is calculated
//                                     added input iMoved for coord and velocity states, added modelling of collision
// Yuri Borisov  29-Dec-08             Added borders
// Yuri Borisov  20-Jan-08             Implemented not direct ball events
//*********************************************************

#include "fapplat.h"
#include "fapwstsob.h"

const char* KAreaBeaconCName = "BeacC";
const char* KAreaBeaconXName = "BeacX";
const char* KBallCoordName = "Coord";
const char* KBallMassName = "Mass";
const char* KBallRadiusName = "Radius";
const char* KBallVelName = "Velocity";
const char* KObjBorderLeftName = "BorderLeft";
const char* KObjBorderRightName = "BorderRight";
const char* KObjBorderTopName = "BorderTop";
const char* KObjBorderBottomName = "BorderBottom";
const char* KBallName = "Ball%d";

// Gravitation constant
const TInt KInfl = 4;
 
const TInt KMassMax = 1000;

// The radius of borders
const TInt KBorderRadius = 50000;
// The mass of borders
const TInt KBorderMass = 30000;

// The base of ind of external inputs of ball's velocity
const TInt KBallVelExtInputsBase = 4;
// The base of ind of external inputs of ball's coordinates
const TInt KBallCoordExtInputsBase = 4;

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
		ball = CFT_Border::NewL(aInstName, this, iBallAreaWnd);
	}
	CF_TdPointF coord = {aCoordX, aCoordY};
	(*ball->iCoord) = coord;
	(*ball->iMass) = aMass;
	(*ball->iRad) = aRad;
	AddBallL(ball);
	return ball;
}


void CFT_Area::LinkBall(CFT_Ball* aBallRec, CFT_Ball* aBallExt)
{
	aBallRec->iInpV->AddInputL(aBallExt->iCoord);
	aBallRec->iInpV->AddInputL(aBallExt->iMass);
	aBallRec->iInpV->AddInputL(aBallExt->iRad);
	aBallRec->iInpV->AddInputL(aBallExt->iInpV);
	aBallRec->iCoord->AddInputL(aBallExt->iCoord);
	aBallRec->iCoord->AddInputL(aBallExt->iRad);
	aBallRec->iCoord->AddInputL(aBallExt->iInpV);
}

void CFT_Area::AddBallL(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aName)
{
	CreateBall(aCoordX, aCoordY, aMass, aRad, aName);
}

void CFT_Area::AddBallL(CFT_Ball* aObBall)
{
	_FAP_ASSERT(aObBall != NULL);
	if ((aObBall->ObjectUid() & KObUid_ModifTypeMask) == KObUid_CAE_Var_Ball)
	{
		LinkL(aObBall->iLbDown, iLbDown); 
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

	// Create borders
	CreateBall(-KBorderRadius, rt.iRightLower.iY / 2, KBorderMass, KBorderRadius, KObjBorderLeftName, ETrue);	
	CreateBall(rt.iRightLower.iX + KBorderRadius, rt.iRightLower.iY / 2, KBorderMass, KBorderRadius, KObjBorderRightName, ETrue);	
	CreateBall(rt.iRightLower.iX / 2, rt.iLeftUpper.iY - KBorderRadius, KBorderMass, KBorderRadius, KObjBorderTopName, ETrue);	
	CreateBall(rt.iRightLower.iX / 2, rt.iRightLower.iY + KBorderRadius, KBorderMass, KBorderRadius, KObjBorderBottomName, ETrue);	

	// Create balls
	CreateBall(40, 40, 200, 20, "Ball1");
	CreateBall(300, 300, 100, 30, "Ball2");
	CreateBall(200, 400, 300, 40, "Ball3");
}

void CFT_Area::UpdateInit(CAE_State* /*aState*/)
{
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
	// Specify inputs for states transition
	// Coordinate depends:
	// on velocity - clear dependency 
	// on itself -even if velocity is unchanged the coordinate have to be changed - free movement
	iCoord->AddInputL(iCoord);
	iCoord->AddInputL(iInpV);
	iCoord->AddInputL(iMcPos);
	iCoord->AddInputL(iMoved);
	// The sign of movement depends on the event of mouse left button action
	iMoved->AddInputL(iLbDown);
	// Velocity is an input. It depends internally:
	// On coordinate - the change of coordinate of itself changes the gravitation force, thus velocity
	// On mass - the change of mass influence on acceleration, thus on velocity
	// On radius - the collision (e.g. velocity) depends on radius
	// Morover the velocity externally denend on coordinates and masses of other solids in the environment
	iInpV->AddInputL(iCoord);
	iInpV->AddInputL(iMass);
	iInpV->AddInputL(iRad);
	iInpV->AddInputL(iMoved);
}


void CFT_Ball::UpdateCoord(CAE_State* aState)
{
	TInt mass = ~*iMass;
	if (mass != 0)
	{
		CF_TdPointF coord = ~*iCoord;
		CF_TdVectF vel = ~*iInpV;
		TBool selected = ~*iMoved;
		TInt currad = iRad->Value();
		CF_TdPointF newcoord;
		if (!selected)
		{
			newcoord.iX = coord.iX + vel.iX;
			newcoord.iY = coord.iY + vel.iY;
		}
		else
		{
			CF_TdPoint mcpos = ~*iMcPos;
			newcoord.iX = mcpos.iX;
			newcoord.iY = mcpos.iY;
		}

		// Check if there will be some collision at the next update
		for (TInt i = KBallCoordExtInputsBase; aState->Input(i) != NULL; ) 
		{
			CF_TdPointF excoord = GetInp(aState, i++, KBallCoordName, excoord);
			TInt exrad = GetInp(aState, i++, KBallRadiusName, exrad);
			CF_TdVectF exvel = GetInp(aState, i++, KBallVelName, exvel);
			
			CF_TdPointF newexcoord;
			newexcoord.iX = excoord.iX + exvel.iX;
			newexcoord.iY = excoord.iY + exvel.iY;
			float r = GetDistance(newcoord, newexcoord);
			if (r <= (currad + exrad))
			{
				// Collision will happen next tick
				newcoord.iX = coord.iX;
				newcoord.iY = coord.iY;
			}
		}
		
		(*iCoord) = newcoord;
		int rad = ~*iRad;
		iBallAreaWnd->redraw(CF_TdPoint(coord.iX, coord.iY), rad, ETrue);
		iBallAreaWnd->redraw(CF_TdPoint(newcoord.iX, newcoord.iY), rad, EFalse);
	}
}

void CFT_Ball::UpdateSelected(CAE_State* /*aState*/)
{
	TInt down = ~*iLbDown;
	TBool sel = EFalse;
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
	*iMoved = sel;
}

void CFT_Ball::UpdateVelocity(CAE_State* aState)
{
	TInt curmass = iMass->Value();
	TInt currad = iRad->Value();
	CF_TdPointF curcoord = iCoord->Value();
	CF_TdVectF curvel = iInpV->Value();
	TBool selected = ~*iMoved;
	CF_TdVectF vforce = {0.0, 0.0};
	CF_TdVectF newvel = {0.0, 0.0};
	TInt i = 0;
	// Handle manual selection
	if (selected)
	{
		// Temporarily solution - just reset the velocity
		// In this case the user cannot "throw" the ball
		newvel.iX = 0.0;
		newvel.iY = 0.0;
	}
	else
	{ // No ball selection from the user. 
		// Calculate the resultant force  
		for (i = KBallVelExtInputsBase; aState->Input(i) != NULL; ) // 1..2 - ball's internal inputs
		{
			CF_TdPointF excoord = GetInp(aState, i++, KBallCoordName, excoord);
			TInt exmass = GetInp(aState, i++, KBallMassName, exmass);
			TInt exrad = GetInp(aState, i++, KBallRadiusName, exrad);
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
			
			if (exmass != 0)
			{
				CF_TdPointF newexcoord;
				newexcoord.iX = excoord.iX + exvel.iX;
				newexcoord.iY = excoord.iY + exvel.iY;
				float r = GetDistance(newcoord, newexcoord);
				if (r <= (currad + exrad))
				{
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
	}
	(*iInpV) = newvel;
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

