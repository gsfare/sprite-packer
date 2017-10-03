#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QRubberBand>
#include <QPushButton>

#include "types.h"

class MyDerivedQGraphicsItem : public QGraphicsRectItem
{
public:

    MyDerivedQGraphicsItem( QGraphicsItem * parent = 0 );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class MyGraphicsView : public QGraphicsView
{
     Q_OBJECT

public:
    MyGraphicsView(QWidget* parent = NULL);
    ~MyGraphicsView(void);

    void setSprite( Sprite *sprite );
    void setCogButton( QPushButton *button ){ cogButton_ = button; }
    void pushButtonCogClicked(bool checked);
protected:

    //Take over the interaction
    virtual void wheelEvent(QWheelEvent* event);
    virtual void mouseMoveEvent ( QMouseEvent * event );
    virtual void mousePressEvent ( QMouseEvent * event );
    virtual void mouseReleaseEvent ( QMouseEvent * event );

private:

    bool rubberBandAdjusting;
    QGraphicsRectItem *rubberBand;
    QGraphicsRectItem *tl_,*tm_,*tr_,*lm_,*rm_,*bl_,*bm_,*br_, *cog_;
    bool tls,trs,bls,brs, cogs;
    Sprite *sprite_;
    QGraphicsScene graphicsScene_;
    QPushButton *cogButton_;
    void setPads( QRectF &rect );

signals:

    void dataChanged();

};

#endif // MYGRAPHICSVIEW_H
