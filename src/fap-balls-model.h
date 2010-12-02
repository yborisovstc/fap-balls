// Balls and area model 

#ifndef __FAPTSOB_DEF_
#define __FAPTSOB_DEF_

#include <math.h>
#include <fapplat.h>
#include <fapbase.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

struct CF_TdColor
{
    CF_TdColor(TUint8 aRed, TUint8 aGreen, TUint8 aBlue) : iRed(aRed), iGreen(aGreen), iBlue(aBlue) {}
    TUint8 iRed;
    TUint8 iGreen;
    TUint8 iBlue;
};

class CFT_BArrea_Painter
{
public:
	CFT_BArrea_Painter(GtkWidget* aWidget) : iWidget(aWidget) {}
	void redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase);
	void drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor);
private:
	GtkWidget* iWidget;
};

// Sumulator of 2d area
void AddBallL(CAE_Object* aObBall, TBool aUseAreaHook);
void LinkBall(CAE_Object* aBallRec, CAE_Object* aBallExt);
CAE_Object* CreateBall(float aCoordX,  float aCoordY, TInt aMass, TInt aRad, const char* aInstName = NULL, TBool aBorder = EFalse);
void CreateBorder(float aCoordX,  float aCoordY, const char* aInstName, TTransFun aHookUpdate);
void UpdateBordHookLeft(CAE_Object* aObject, CAE_State* aState);
void UpdateBordHookRight(CAE_Object* aObject, CAE_State* aState);
void UpdateBordHookTop(CAE_Object* aObject, CAE_State* aState);
void UpdateBordHookBottom(CAE_Object* aObject, CAE_State* aState);

// Sumulator of ball 
void UpdateCoord(CAE_Object* aObject, CAE_State* aState);
void UpdateVelocity(CAE_Object* aObject, CAE_State* aState);
void UpdateSelected(CAE_Object* aObject, CAE_State* aState);
void GetProjOfVel(CF_TdPointF aAngleBeg, CF_TdPointF aAngleEnd, CF_TdVectF aVel, CF_TdVectF& aVelNorm, CF_TdVectF& aVelTang);

inline float GetDistance(CF_TdPointF aCoord1, CF_TdPointF aCoord2) {
    float r2 = (aCoord1.iX - aCoord2.iX)*(aCoord1.iX - aCoord2.iX) + (aCoord1.iY - aCoord2.iY)*(aCoord1.iY - aCoord2.iY);
    return sqrt (r2);
}

#endif // __FAPTSOB_DEF_

