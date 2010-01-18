#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qtimer.h>

#include <fapext.h>
#include "fapwstsob.h"

struct Ball
{
  int iX;
  int iY;
  int iRad;
};

class AreaWidget: public QWidget, public MBallAreaWindow
{
    Q_OBJECT
public:
	AreaWidget(QWidget *parent = 0, const char *name = 0);
	~AreaWidget();
protected:
	// from QWidget
	void paintEvent(QPaintEvent*);
	void mousePressEvent(QMouseEvent* aEvent);
	void mouseReleaseEvent(QMouseEvent* aEvent);
	void mouseMoveEvent(QMouseEvent* aEvent);
	// from MBallAreaWindow
	virtual void redraw(CF_TdPoint aCenter, TInt aRadius, TBool aErase);
	virtual void drawBall(CF_TdPoint aCenter, TInt aRadius, CF_TdColor aColor);
	virtual CF_Rect boundaryRect();
private:
private slots:
	void tick();
private:
	QTimer* iTimer;
	CAE_Env* iFapE;
	CFT_Area* iFapA;
};
