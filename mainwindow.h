#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollBar>
#include <QGraphicsScene>
#include <QPixmap>
#include <QTreeWidgetItem>
#include <QXmlStreamReader>
#include "types.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QGraphicsPixmapItem *pixmapItem;
    QList<SpriteImage *> imageFileList_;
    QList<Sprite *> spriteList_;
    int lastImageIndex_;
    int spriteImageNextIndex_;
    int spriteNextIndex_;
    bool fileOpen_;
    bool isModified_;
    QString fileName_;

    int arrayIndexFromSpriteIndex( int index )
    {
        int i = 0;
        for( i=0 ; i<spriteList_.size() ; i++ )
            if( spriteList_.at(i)->index_ == index )
                break;
        return( i );
    }

    int arrayIndexFromImageIndex( int index )
    {
        int i = 0;
        for( i=0 ; i<imageFileList_.size() ; i++ )
            if( imageFileList_.at(i)->index_ == index )
                break;
        return( i );
    }

    void parseXmlImage( QXmlStreamReader &reader );
    void parseXmlSprite( QTreeWidgetItem *item, QXmlStreamReader &reader );
    bool parseXmlReadTag( QXmlStreamReader &reader, QString name, QString &result );

protected:
     void closeEvent(QCloseEvent *event);

private slots:
    void addSourceImage();
    void itemClicked(QTreeWidgetItem *item, int column);
    void addSprite();
    void deleteSprite();
    void itemSelectionChanged();
    void pushButtonCogClicked(bool checked);
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void fileClose();
    void dataChanged();
    void build();

};

#endif // MAINWINDOW_H
