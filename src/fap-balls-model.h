
/* Copyright (c) 2009, Yuri Borisov
 * All rights reserved
 */

#ifndef __FAPTSOB_DEF_
#define __FAPTSOB_DEF_
//*********************************************************
// FAP test 1. Objects 
// Yuri Borisov  15-Dec-08 FAPW_CR_011 Aligned with FAPWS FAPWS_REL_21SEP06
// Yuri Borisov  15-Dec-08             Ball states transition moved from the area to ball
// Yuri Borisov  29-Dec-08             Added borders
// Yuri Borisov  20-Jan-08             Implemented not direct ball events
//*********************************************************

#include <math.h>
#include <fapplat.h>
#include <fapbase.h>
#include <gdk/gdk.h>


// Extended states UID
const TInt KObUid_CAE_Var_State_TPoint = KObUid_CAE_Var_State_Ext + 1 ;   // tagPOINT 0x00010001
const TInt KObUid_CAE_Var_State_Point = KObUid_CAE_Var_State_Ext + 2 ;   // CF_TdPoint 0x00010002
const TInt KObUid_CAE_Var_State_PointF = KObUid_CAE_Var_State_Ext + 3 ;   // CF_TdPointF 0x00010003
const TInt KObUid_CAE_Var_State_VectF = KObUid_CAE_Var_State_Ext + 4 ;   // CF_TdVectF 0x00010004
const TInt KObUid_CAE_Var_State_Rect = KObUid_CAE_Var_State_Ext + 5 ;   // CF_Rect 0x00010005


// Objects subtypes UIDs
const TInt KObUid_CAE_Var_Ball =	0x00000001;
const TInt KObUid_CAE_Var_Border =	0x00000002;

struct CF_TdPoint
{
	CF_TdPoint() {}
	CF_TdPoint(TInt aX, TInt aY): iX(aX), iY(aY) {}
	TInt iX;
	TInt iY;
};

struct CF_TdPointF
{
	float iX;
	float iY;
};

struct CF_TdVect
{
	TInt iX;
	TInt iY;
};

struct CF_TdVectF
{
	float iX;
	float iY;
};

struct CF_Rect
{
	CF_Rect(float aLeft, float aTop, float aRight, float aBottom) {
	    iLeftUpper.iX=aLeft; iLeftUpper.iY=aTop; iRightLower.iX=aRight; iRightLower.iY=aBottom;}
	CF_Rect() {}
	CF_TdPoint iLeftUpper;
	CF_TdPoint iRightLower;
};

//template<> inline TInt CAE_TState<tagPOINT>::DataTypeUid() {return KObUid_CAE_Var_State_TPoint;};

template<> inline TInt CAE_TState<CF_TdPoint>::DataTypeUid() {return KObUid_CAE_Var_State_Point;};

template<> inline TInt CAE_TState<CF_TdPointF>::DataTypeUid() {return KObUid_CAE_Var_State_PointF;};

template<> inline TInt CAE_TState<CF_TdVectF>::DataTypeUid() {return KObUid_CAE_Var_State_VectF;};

template<> inline TInt CAE_TState<CF_Rect>::DataTypeUid() {return KObUid_CAE_Var_State_Rect;};




struct CF_TdColor
{
	CF_TdColor(TUint8 aRed, TUint8 aGreen, TUint8 aBlue) : iRed(aRed), iGreen(aGreen), iBlue(aBlue) {}
	TUint8 iRed;
	TUint8 iGreen;
	TUint8 iBlue;
};

class CFT_Ball;

class MBallAreaWindow
{
public:
	virtual void redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase) = 0;
	virtual void drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor) = 0;
	virtual CF_Rect boundaryRect() = 0;
};

// Sumulator of 2d area. It contains the base points - beacons to 
// simulate the geometric placing of object inside the area
// Area includes the balls and the hook that can be used to manually move selected ball
class CFT_Area: public CAE_Object
{
public:
	static CFT_Area* NewL(const char* aInstName, CAE_Object* aReg, MBallAreaWindow* aBallAreaWnd);
	CFT_Area(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd);
	virtual ~CFT_Area();
	void Draw();
	void AddBallL(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aName = NULL);
	void AddBallL(CFT_Ball* aObBall, TBool aUseAreaHook);
protected:
	CAE_TRANS_DEF(UpdateInit, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookLeft, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookRight, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookTop, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookBottom, CFT_Area);
private:
	void ConstructL();
	static void LinkBall(CFT_Ball* aBallRec, CFT_Ball* aBallExt);
	CFT_Ball* CreateBall(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aInstName = NULL, TBool aBorder = EFalse);
public:
	MBallAreaWindow*	iBallAreaWnd;
	CAE_TState<TInt>*	iLbDown;
	// Position of the hook that is used in model of manual moving of ball 
	CAE_TState<CF_TdPoint>*	iMcPos; 
	// Area rectangle
	CAE_TState<CF_Rect>*	iRect; 
private:
	CAE_State*	iBeaconC;
	CAE_State*	iBeaconX;
//	CAE_TState<TInt>*	iInit;
};



// Sumulator of ball. It contain the states: coordinats, size, and mass
// Note: The inputs structure for velocity is {Coord, Mass, Radius, Velocity}. 
// The first 3 inputs are  {Coord, Mass, Radius} of the ball itself
// The external inputs structure for coord is {Coord,Radius, Velocity}
// The internal first are  {Coord, Mass, MsPos} of the ball itself
 
class CFT_Ball: public CAE_Object
{
public:
	enum TObSubtypeUid
	{
		EObStypeUid = KObUid_CAE_Var_Ball
	};
public:
	static inline TInt ObjectUid(); 
	static CFT_Ball* NewL(const char* aInstName, CAE_Object* aReg, MBallAreaWindow* aBallAreaWnd);
	CFT_Ball(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd): CAE_Object(aInstName, aMan), iBallAreaWnd(aBallAreaWnd) { iVariantUid = EObStypeUid;}
	void Draw();
//	void VectFLogFun(char* aBuf, CAE_StateBase* aState);
private:
	void ConstructL();
	static inline float GetDistance(CF_TdPointF aCoord1, CF_TdPointF aCoord2);
	// Get projections of ball's velocity
	static void GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang);
	template <class T> inline static T GetInp(CAE_StateBase* aState, TInt aInd, const char* aName, T& aVal);
protected:
	CAE_TRANS_DEF(UpdateCoord, CFT_Ball);
	CAE_TRANS_DEF(UpdateVelocity, CFT_Ball);
	CAE_TRANS_DEF(UpdateSelected, CFT_Ball);
public:
	CAE_TState<CF_TdVectF>*	iInpV;	// Velocity
	CAE_TState<CF_TdPointF>*	iCoord;
	CAE_TState<TInt>*	iMass;
	CAE_TState<TInt>*	iRad;
	CAE_TState<TBool>*	iMoved;
	CAE_TState<TInt>*	iLbDown;
	// Position of the hook that is used in model of manual moving of ball 
	CAE_TState<CF_TdPoint>*	iMcPos;
	// Hook stiffness coefficient, n/m
	CAE_TState<TInt>*	iHookSc;
	// Hooked permanently
	CAE_TState<TBool>*	iHookedPerm;
	// The ball is "transparent" when collising, i.e. to transparent balls dont have collision
	CAE_TState<TBool>*	iTransp;
private:
	MBallAreaWindow*	iBallAreaWnd;


private:
};

// Embedded to the class definition for VC 6.0 only to avoid C2893 error
template <class T> inline T CFT_Ball::GetInp(CAE_StateBase* aState, TInt aInd, const char* aName, T& aVal)
{
    CAE_StateBase* state = aState->Input(aInd);
    // Check if the structure of inputs is correct
    _FAP_ASSERT(strcmp(state->InstName(), aName) == 0);
    CAE_TState<T> *tstate = state->GetObject(tstate);
    _FAP_ASSERT(tstate != NULL);
    aVal = tstate->Value();
    return aVal;
}



inline TInt CFT_Ball::ObjectUid() { return EObUid | EObStypeUid;} 

inline float CFT_Ball:: GetDistance(CF_TdPointF aCoord1, CF_TdPointF aCoord2)
{
    float r2 = (aCoord1.iX - aCoord2.iX)*(aCoord1.iX - aCoord2.iX) + (aCoord1.iY - aCoord2.iY)*(aCoord1.iY - aCoord2.iY);
    return sqrt (r2);
}

class CFT_Border: public CFT_Ball
{
public:
	enum TObSubtypeUid
	{
		EObStypeUid = KObUid_CAE_Var_Border
	};
public:
	static inline TInt ObjectUid(); 
};

inline TInt CFT_Border::ObjectUid() { return EObUid | EObStypeUid;} 

#endif // __FAPTSOB_DEF_

