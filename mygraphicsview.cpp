#include "mygraphicsview.h"

//Qt includes
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QTextStream>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <math.h>

MyDerivedQGraphicsItem::MyDerivedQGraphicsItem( QGraphicsItem * parent ) : QGraphicsRectItem( parent )
{

}

void MyDerivedQGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    QPen inverter(Qt::white);
    inverter.setWidth(0);
    painter->setPen(inverter);

    painter->drawRect( QGraphicsRectItem::rect() );
}

MyGraphicsView::MyGraphicsView(QWidget* parent) : QGraphicsView(parent) {



   // setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
#if 0
    //Set-up the scene
    QGraphicsScene* Scene = new QGraphicsScene(this);
    setScene(Scene);

    //Populate the scene
    for(int x = 0; x < 1000; x = x + 25) {
        for(int y = 0; y < 1000; y = y + 25) {

            if(x % 100 == 0 && y % 100 == 0) {
                Scene->addRect(x, y, 2, 2);

                QString pointString;
                QTextStream stream(&pointString);
                stream << "(" << x << "," << y << ")";
                QGraphicsTextItem* item = Scene->addText(pointString);
                item->setPos(x, y);
            } else {
                Scene->addRect(x, y, 1, 1);
            }
        }
    }

    //Set-up the view
    setSceneRect(0, 0, 1000, 1000);
#endif

    //setTransformationAnchor( QGraphicsView::AnchorUnderMouse );

    //Use ScrollHand Drag Mode to enable Panning
    setDragMode(ScrollHandDrag);
    setScene( new QGraphicsScene( this ) );

    QPen pen( Qt::magenta );
    pen.setJoinStyle( Qt::MiterJoin );
    pen.setWidth( 0 );

    rubberBand = new QGraphicsRectItem();
    rubberBand->setOpacity( 0.5 );
    rubberBand->setPen(pen);

    tl_ = new QGraphicsRectItem();
    tl_->setOpacity( 0.5 );
    tl_->setPen( pen );
    tr_ = new QGraphicsRectItem();
    tr_->setOpacity( 0.5 );
    tr_->setPen( pen );
    bl_ = new QGraphicsRectItem();
    bl_->setOpacity( 0.5 );
    bl_->setPen( pen );
    br_ = new QGraphicsRectItem();
    br_->setOpacity( 0.5 );
    br_->setPen( pen );

    cog_ = new QGraphicsRectItem();
    cog_->setOpacity( 0.5 );
    cog_->setPen( pen );
    cog_->setRect( 0,0,3,3 );

    rubberBand->setVisible( false );
    tl_->setVisible( false );
    tr_->setVisible( false );
    bl_->setVisible( false );
    br_->setVisible( false );
    cog_->setVisible( false );

    rubberBand->setZValue( 1 );
    tl_->setZValue( 1 );
    tr_->setZValue( 1 );
    bl_->setZValue( 1 );
    br_->setZValue( 1 );
    cog_->setZValue( 1 );

    scene()->addItem( rubberBand );
    scene()->addItem( tl_ );
    scene()->addItem( tr_ );
    scene()->addItem( bl_ );
    scene()->addItem( br_ );
    scene()->addItem( cog_ );

    tls = false;
    trs = false;
    bls = false;
    brs = false;
    cogs = false;

    sprite_ = NULL;
    rubberBandAdjusting = false;
}

MyGraphicsView::~MyGraphicsView(void)
{
    delete scene();
}

/**
  * Zoom the view in and out.
  */
void MyGraphicsView::wheelEvent(QWheelEvent* event) {

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Scale the view / do the zoom
    double scaleFactor = 1.15;
    if(event->delta() > 0) {
        // Zoom in
        scale(scaleFactor, scaleFactor);
    } else {
        // Zooming out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    // Don't call superclass handler here
    // as wheel is normally used for moving scrollbars

    //QGraphicsView::wheelEvent( event );
}

void MyGraphicsView::mouseMoveEvent ( QMouseEvent * event )
{
    if( sprite_ != NULL )
    {
        QPointF point = mapToScene( event->pos() );
        QRectF rect;

        point.setX( round( (double)point.x() )  );
        point.setY( round( (double)point.y() )  );

        rect = rubberBand->rect();

        if( ( sprite_->defined() == true ) && ( rubberBandAdjusting == true ) )
        {
            if( ( cogs == true ) && ( cogButton_->isChecked() == true ) )
            {
                qDebug() << "cog->isSelected()";
                QRectF rect2  = cog_->rect();
                rect2.setX( point.x() );
                rect2.setY( point.y() );
                rect2.setWidth( 3 );
                rect2.setHeight( 3 );
                cog_->setRect( rect2 );
            }
            else if( tls == true )
            {
                qDebug() << "tl.isSelected()";
                rect.setTopLeft( point );

                if( rect.left() >= rubberBand->rect().right() - 1 )
                    rect.setLeft( rubberBand->rect().right() - 1 );

                if( rect.top() >= rubberBand->rect().bottom() - 1)
                    rect.setTop( rubberBand->rect().bottom() - 1 );
            }
            else if( trs == true )
            {
                qDebug() << "tr.isSelected()";
                rect.setTopRight( point );

                if( rect.right() <= rubberBand->rect().left() + 1 )
                    rect.setRight( rubberBand->rect().left() + 1 );

                if( rect.top() >= rubberBand->rect().bottom() - 1)
                    rect.setTop( rubberBand->rect().bottom() - 1 );
            }
            else if( bls == true )
            {
                qDebug() << "bl.isSelected()";
                rect.setBottomLeft( point );

                if( rect.left() >= rubberBand->rect().right() - 1 )
                    rect.setLeft( rubberBand->rect().right() - 1 );

                if( rect.bottom() <= rubberBand->rect().top() + 1)
                    rect.setBottom( rubberBand->rect().top() + 1 );
            }
            else if( brs == true )
            {
                qDebug() << "br.isSelected()";
                rect.setBottomRight( point );

                if( rect.right() <= rubberBand->rect().left() + 1 )
                    rect.setRight( rubberBand->rect().left() + 1 );

                if( rect.bottom() <= rubberBand->rect().top() + 1)
                    rect.setBottom( rubberBand->rect().top() + 1 );
            }
            else
            {
                qDebug() << "default";
                rect.setBottomRight( point );
            }
#if 0
            if( rect.width() < 0 )
                rect.setWidth( 0 );

            if( rect.height() < 0 )
                rect.setHeight( 0 );

            qDebug() << rect.x() << ( rubberBand->rect().x() + rubberBand->rect().width() - 1 );
            if( rect.x() >= ( rubberBand->rect().x() + rubberBand->rect().width() ) - 1 )
            {
                rect.setX( rubberBand->rect().x() + rubberBand->rect().width() - 1 );
                rect.setWidth( 1 );
            }

            if( rect.y() >= ( rubberBand->rect().y() + rubberBand->rect().height() ) - 1 )
            {
                rect.setY( rubberBand->rect().y() + rubberBand->rect().height() - 1 );
                rect.setHeight( 1 );
            }
#endif
            rubberBand->setRect( rect );
            setPads( rect );
        }
        else if( ( sprite_->defined() == true ) && ( rubberBandAdjusting == false ) )
        {
            if( tl_->isUnderMouse() || tr_->isUnderMouse() || bl_->isUnderMouse() || br_->isUnderMouse() || cog_->isUnderMouse() )
                setDragMode(NoDrag);
            else
                setDragMode(ScrollHandDrag);
        }
    }

    QGraphicsView::mouseMoveEvent ( event );
}

void MyGraphicsView::mousePressEvent ( QMouseEvent * event )
{
    qDebug() << event->pos().x() << event->pos().y();

    if( sprite_ != NULL )
    {
        if( sprite_->defined() == false )
        {
            rubberBandAdjusting = true;
            rubberBand->setVisible( true );

            QPointF startPoint = mapToScene( event->pos() );
            startPoint.setX( round( (double)startPoint.x() )  );
            startPoint.setY( round( (double)startPoint.y() )  );
            QRectF rect;
            rect.setTopLeft( startPoint );
            rect.setWidth( 1 );
            rect.setHeight( 1 );

            rubberBand->setEnabled( true );
            rubberBand->setRect( rect );
            rubberBand->setVisible( true );

            tl_->setVisible( true );
            tr_->setVisible( true );
            bl_->setVisible( true );
            br_->setVisible( true );
            cog_->setVisible( false );
            setPads( rect );

            sprite_->setDefined( true );
            cogButton_->setEnabled( true );

            brs = true;
        }
        else
        {
            if( ( cog_->isUnderMouse() ) && ( cogButton_->isChecked() == true ) )
            {
                qDebug() << "cog->isUnderMouse()";
                cogs = true;
                rubberBandAdjusting = true;
            }
            else if( tl_->isUnderMouse() )
            {
                qDebug() << "tl->isUnderMouse()";
                tls = true;
                rubberBandAdjusting = true;
            }
            else if( tr_->isUnderMouse() )
            {
                qDebug() << "tr->isUnderMouse()";
                trs = true;
                rubberBandAdjusting = true;
            }
            else if( bl_->isUnderMouse() )
            {
                qDebug() << "bl->isUnderMouse()";
                bls = true;
                rubberBandAdjusting = true;
            }
            else if( br_->isUnderMouse() )
            {
                qDebug() << "br->isUnderMouse()";
                brs = true;
                rubberBandAdjusting = true;
            }
        }

    }

    QGraphicsView::mousePressEvent ( event );
}


void MyGraphicsView::mouseReleaseEvent ( QMouseEvent * event )
{
    rubberBandAdjusting = false;
    tls = false;
    trs = false;
    bls = false;
    brs = false;
    cogs = false;

    if( sprite_ != NULL )
    {
        QRectF rectF = rubberBand->rect();
        QRect rect( rectF.x(), rectF.y(), rectF.width(), rectF.height() );

        sprite_->setRect( rect );
        if( cogButton_->isChecked() == true  )
            sprite_->setCog( QPoint( cog_->rect().x(), cog_->rect().y() ) );

        emit dataChanged();
    }
    QGraphicsView::mouseReleaseEvent ( event );
}

void MyGraphicsView::setPads( QRectF &rect )
{
    tl_->setRect( rect.x(), rect.y(), 2, 2 );
    //tm.setRect( startPoint.X(), startPoint.Y(), 2, 2 );
    tr_->setRect( rect.right()-2, rect.y(), 2, 2 );
    //lm.setRect( startPoint.X(), startPoint.Y(), 2, 2 );
    //lr.setRect( startPoint.X(), startPoint.Y(), 2, 2 );
    bl_->setRect( rect.x(), rect.bottom()-2, 2, 2 );
    //bm.setRect( startPoint.X(), startPoint.Y(), 2, 2 );
    br_->setRect( rect.right()-2, rect.bottom()-2, 2, 2 );
}

void MyGraphicsView::setSprite( Sprite *sprite )
{
    sprite_ = sprite;

    if( sprite_ == NULL )
    {
        setDragMode(ScrollHandDrag);
        rubberBand->setVisible( false );
        tl_->setVisible( false );
        tr_->setVisible( false );
        bl_->setVisible( false );
        br_->setVisible( false );
        cog_->setVisible( false );
        cogButton_->setEnabled( false );
        cogButton_->setChecked( false );
        return;
    }

    if( sprite_->defined() == false )
    {
        setDragMode(NoDrag);
        rubberBand->setVisible( false );
        tl_->setVisible( false );
        tr_->setVisible( false );
        bl_->setVisible( false );
        br_->setVisible( false );
        cog_->setVisible( false );
        cogButton_->setEnabled( false );
        cogButton_->setChecked( false );
    }
    else
    {
        setDragMode(ScrollHandDrag);
        QRect rect = sprite_->getRect();
        QRectF rectF( rect.x(), rect.y(), rect.width(), rect.height() );
        rubberBand->setRect( rectF );
        setPads( rectF );

        cogButton_->setEnabled( true );
        if( sprite_->cogDefined() == true )
        {
            rectF = QRectF( sprite_->getCog().x(), sprite_->getCog().y(), 3, 3 );
            cog_->setRect( rectF );
            cog_->setVisible( true );
            cogButton_->setChecked( true );            
        }
        else
            cogButton_->setChecked( false );



        rubberBand->setVisible( true );
        tl_->setVisible( true );
        tr_->setVisible( true );
        bl_->setVisible( true );
        br_->setVisible( true );
    }
}

void MyGraphicsView::pushButtonCogClicked(bool checked)
{
    if( sprite_ != NULL )
    {
        if( ( sprite_->cogDefined() == false ) && ( checked == true ) )
        {
            QRectF rect = rubberBand->rect();
            rect.setX( rect.x() + ( rect.width() / 2 ) );
            rect.setY( rect.y() + ( rect.height() / 2 ) );
            rect.setWidth( 3 );
            rect.setHeight( 3 );
            sprite_->setCog( QPoint( rect.x(), rect.y() ) );
            cog_->setRect( rect );
            cog_->setVisible( true );
        }
        else
        {
            cog_->setVisible( false );
        }
        sprite_->setCogDefined( checked );
    }

    emit dataChanged();
}
