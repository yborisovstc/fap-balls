
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


template<> inline const char* CAE_TState<CF_TdPoint>::Type() {return "StPoint";};

template<> inline const char* CAE_TState<CF_TdPointF>::Type() {return "StPointF";};

template<> inline const char* CAE_TState<CF_TdVectF>::Type() {return "StVectF";};

template<> inline const char* CAE_TState<CF_Rect>::Type() {return "StVectF";};




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
};

// Sumulator of 2d area. It contains the base points - beacons to 
// simulate the geometric placing of object inside the area
// Area includes the balls and the hook that can be used to manually move selected ball
class CFT_Area: public CAE_Object
{
public:
	static CFT_Area* NewL(const char* aInstName, CAE_Object* aReg, MBallAreaWindow* aBallAreaWnd, CF_Rect *aRect);
	CFT_Area(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd, CF_Rect *aRect);
	virtual ~CFT_Area();
	void Draw();
	void AddBallL(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aName = NULL);
	static void AddBallL(CAE_Object* aObBall, TBool aUseAreaHook);
	static void LinkBall(CAE_Object* aBallRec, CAE_Object* aBallExt);
	static CAE_Object* CreateBall(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aInstName = NULL, TBool aBorder = EFalse);
	static CFT_Ball* CreateBorder(float aCoordX,  float aCoordY, const char* aInstName, TTransFun aHookUpdate);
protected:
	void ConstructL();
	CAE_TRANS_DEF(UpdateInit, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookLeft, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookRight, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookTop, CFT_Area);
	CAE_TRANS_DEF(UpdateBordHookBottom, CFT_Area);
public:
	MBallAreaWindow*	iBallAreaWnd;
	CAE_TState<TInt>*	iLbDown;
	// Position of the hook that is used in model of manual moving of ball 
	CAE_TState<CF_TdPoint>*	iMcPos; 
	// Area rectangle
	CAE_TState<CF_Rect>*	iRect; 
	// Initial area rectangle
	CF_Rect  iInitRect;
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
	static inline const char* Type(); 
	static CFT_Ball* NewL(const char* aInstName, CAE_Object* aReg, MBallAreaWindow* aBallAreaWnd);
	CFT_Ball(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd): CAE_Object(aInstName, aMan), iBallAreaWnd(aBallAreaWnd) {}
	void Draw();
//	void VectFLogFun(char* aBuf, CAE_StateBase* aState);

	CAE_TRANS_DEF(UpdateCoord, CFT_Ball);
	CAE_TRANS_DEF(UpdateVelocity, CFT_Ball);
	CAE_TRANS_DEF(UpdateSelected, CFT_Ball);
private:
	static inline float GetDistance(CF_TdPointF aCoord1, CF_TdPointF aCoord2);
	// Get projections of ball's velocity
	static void GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang);
//	template <class T> inline static T GetInp(CAE_StateBase* aState, TInt aInd, const char* aName, T& aVal);
protected:
	void ConstructL();
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
template <class T> inline T GetInp(CAE_StateBase* aState, TInt aInd, const char* aName, T& aVal)
{
    CAE_StateBase* state = aState->Input(aInd);
    // Check if the structure of inputs is correct
    _FAP_ASSERT(strcmp(state->InstName(), aName) == 0);
    CAE_TState<T> *tstate = state->GetFbObj(tstate);
    _FAP_ASSERT(tstate != NULL);
    aVal = tstate->Value();
    return aVal;
}



inline const char* CFT_Ball::Type() { return "Ball";} 

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
	static CFT_Border* NewL(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd);
	CFT_Border(const char* aInstName, CAE_Object* aMan, MBallAreaWindow* aBallAreaWnd):  CFT_Ball(aInstName, aMan, aBallAreaWnd) {} 
	static inline const char* Type(); 
};

inline const char* CFT_Border::Type() { return "Border";} 

#endif // __FAPTSOB_DEF_

