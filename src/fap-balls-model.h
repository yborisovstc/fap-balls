// Balls and area model 

#ifndef __FAPTSOB_DEF_
#define __FAPTSOB_DEF_

#include <math.h>
#include <fapplat.h>
#include <fapbase.h>
#include <gtk/gtk.h>

// The radius of borders
const TInt KBorderRadius = 50000;
// The mass of borders
const TInt KBorderMass = 30000;
// Maximum mass of ball
const TInt KBallMassMax = 1000;

struct CF_TdColor {
    CF_TdColor(TUint8 aRed, TUint8 aGreen, TUint8 aBlue) : iRed(aRed), iGreen(aGreen), iBlue(aBlue) {}
    TUint8 iRed, iGreen, iBlue;
};

class CFT_BArrea_Painter {
public:
	CFT_BArrea_Painter(GtkWidget* aWidget) : iWidget(aWidget) {}
	void redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase);
	void drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor);
	GtkWidget* iWidget;
};

// Sumulator of 2d area
void AddBallL(CAE_Object* aObBall, const char* aHookName);
void LinkBall(CAE_Object* aBallRec, CAE_Object* aBallExt);
void CreateBall(TInt aCoordX, TInt aCoordY, TInt aMass, TInt aRad, TBool aHookedPerm, TBool aTransp, const char* aName, const char* aHookName);
void UpdateBordHookLeft(CAE_Object* aObject, CAE_State* aState);
void UpdateBordHookRight(CAE_Object* aObject, CAE_State* aState);
void UpdateBordHookTop(CAE_Object* aObject, CAE_State* aState);
void UpdateBordHookBottom(CAE_Object* aObject, CAE_State* aState);
void UpdateBordersCount(CAE_Object* aObject, CAE_State* aState);
void UpdateBallCreationStart(CAE_Object* aObject, CAE_State* aState);
void UpdateBallCreationReady(CAE_Object* aObject, CAE_State* aState);

// Sumulator of ball 
void UpdateCoord(CAE_Object* aObject, CAE_State* aState);
void UpdateVelocity(CAE_Object* aObject, CAE_State* aState);
void UpdateSelected(CAE_Object* aObject, CAE_State* aState);
TBool GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang);
inline float GetDistance(CF_TdPointF aCoord1, CF_TdPointF aCoord2) { return (aCoord2 - aCoord1).Mod(); }

#endif // __FAPTSOB_DEF_

