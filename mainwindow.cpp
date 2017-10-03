#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QXmlStreamWriter>
#include <QCloseEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "RectangleBinPack/GuillotineBinPack.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButtonAddSprite->setEnabled( false );
    ui->pushButtonDeleteSprite->setEnabled( false );
    pixmapItem = NULL;
    lastImageIndex_ = -1;
    spriteImageNextIndex_ = 0;
    spriteNextIndex_ = 0;
    ui->graphicsView->setCogButton( ui->pushButtonCog );
    fileOpen_ = false;
    isModified_= false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addSourceImage()
{
    QPixmap pixmap;
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open File"), QDir::currentPath());
    SpriteImage *spriteImage = NULL;

    if (!fileName.isEmpty()) {
        QImage image(fileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Sprite Packer"),
                                     tr("Cannot load %1.").arg(fileName));
            return;
        }

        // Unload the previous image if there is one.

        if( pixmapItem != NULL )
        {
            ui->graphicsView->scene()->removeItem( pixmapItem );
            delete pixmapItem;
            pixmapItem = NULL;
        }


        // Load the new image.

        pixmap = QPixmap::fromImage(image);
        pixmapItem = ui->graphicsView->scene()->addPixmap( pixmap );
        pixmapItem->setZValue( 0 );
        //ui->graphicsView->setScene( graphicsScene );
        ui->graphicsView->show();


        // Add a image item to the tree list.

        QTreeWidgetItem *item = new QTreeWidgetItem(kTreeItemImage);
        QStringList parts = fileName.split(tr("/"));
        QString lastBit = parts.at(parts.size()-1);

        spriteImage = new SpriteImage();
        spriteImage->fileName_ = fileName;
        spriteImage->index_ = spriteImageNextIndex_++;
        imageFileList_.append( spriteImage );

        item->setData( 0, Qt::UserRole, spriteImage->index_ );
        item->setData( 0, Qt::DisplayRole, lastBit );
        ui->treeWidget->addTopLevelItem( item );
        ui->treeWidget->clearSelection();
        item->setSelected( true );

        ui->pushButtonAddSprite->setEnabled( true );
        ui->pushButtonDeleteSprite->setEnabled( true );
        lastImageIndex_ = spriteImage->index_;

        dataChanged();
    }
}

void MainWindow::itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QTreeWidgetItem *parentItem = NULL;
    QPixmap pixmap;
    QImage image;
    int i = 0;

    if( item->type() == kTreeItemImage )
    {
        parentItem = item;
        ui->graphicsView->setSprite( NULL );
    }
    else
    {
        // Get the sprite data.
        i = item->data( 0, Qt::UserRole ).toInt();
        i = arrayIndexFromSpriteIndex( i );
        ui->graphicsView->setSprite( spriteList_.at( i ) );

        parentItem = item->parent();
    }

    ui->pushButtonAddSprite->setEnabled( true );
    ui->pushButtonDeleteSprite->setEnabled( true );


    // Check if we need to change the displayed image.

    if( lastImageIndex_ == parentItem->data(0,Qt::UserRole).toInt() )
        return;

    lastImageIndex_ = parentItem->data(0,Qt::UserRole).toInt();
    i = arrayIndexFromImageIndex( lastImageIndex_ );
    image = QImage(imageFileList_.at(i)->fileName());

    // Unload the previous image if there is one.

    if( pixmapItem != NULL )
    {
       ui->graphicsView->scene()->removeItem( pixmapItem );
       delete pixmapItem;
       pixmapItem = NULL;
    }


    // Load the new image.
    pixmap = QPixmap::fromImage(image);
    pixmapItem = ui->graphicsView->scene()->addPixmap( pixmap );
    pixmapItem->setZValue( 0 );
    //ui->graphicsView->setScene( graphicsScene );
    ui->graphicsView->show();
}

void MainWindow::addSprite()
{
    QList<QTreeWidgetItem *> selectedItems;;

    selectedItems = ui->treeWidget->selectedItems();
    QTreeWidgetItem *parentItem = NULL;
    Sprite *sprite = NULL;

    if( selectedItems.size() > 1 )
        return;

    if( selectedItems.size() == 0 )
        return;

    // Get the parent node.
    if( selectedItems.at( 0 )->type() == kTreeItemSprite )
        parentItem = selectedItems.at( 0 )->parent();
    else
        parentItem = selectedItems.at( 0 );


    sprite = new Sprite();
    sprite->index_ = spriteNextIndex_++;
    sprite->imageIndex_ = parentItem->data( 0, Qt::UserRole ).toInt();
    spriteList_.append( sprite );

    QTreeWidgetItem *item = new QTreeWidgetItem(kTreeItemSprite);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setData( 0, Qt::EditRole, tr( "New Sprite" ) );
    item->setData( 0, Qt::UserRole, sprite->index_ );
    parentItem->setExpanded( true );
    parentItem->addChild( item );
    ui->treeWidget->clearSelection();
    item->setSelected( true );
    ui->graphicsView->setSprite( sprite );

    dataChanged();
}


void MainWindow::deleteSprite()
{
    QList<QTreeWidgetItem *> selectedItems;
    int i = 0, j = 0;
    selectedItems = ui->treeWidget->selectedItems();
    QTreeWidgetItem *parentItem = NULL;

    if( selectedItems.size() > 1 )
        return;

    if( selectedItems.size() == 0 )
        return;

    if( selectedItems.at(0)->type() == kTreeItemSprite )
    {
        i = arrayIndexFromSpriteIndex( selectedItems.at(0)->data(0,Qt::UserRole).toInt() );
        parentItem = selectedItems.at(0)->parent();
        parentItem->removeChild( selectedItems.at(0) );
        spriteList_.removeAt( i );
    }
    else
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Delete", "Are you sure you want to delete the entire image tree?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;

        // Unload the previous image if there is one.

        if( lastImageIndex_ == selectedItems.at(0)->data(0,Qt::UserRole).toInt() )
        {
            if( pixmapItem != NULL )
            {
               ui->graphicsView->scene()->removeItem( pixmapItem );
               delete pixmapItem;
               pixmapItem = NULL;
            }

            lastImageIndex_ = -1;
        }

        ui->treeWidget->takeTopLevelItem( ui->treeWidget->indexOfTopLevelItem( selectedItems.at(0) ) );
        for( i=0 ; i<selectedItems.at(0)->childCount() ; i++ )
        {
            j = arrayIndexFromSpriteIndex( selectedItems.at(0)->child(i)->data(0,Qt::UserRole).toInt() );
            spriteList_.removeAt(j);
        }

        i = arrayIndexFromImageIndex( selectedItems.at(0)->data(0,Qt::UserRole).toInt() );
        imageFileList_.removeAt( i );

        delete selectedItems.at(0);

        if( ui->treeWidget->topLevelItemCount() == 0 )
        {
            ui->pushButtonAddSprite->setEnabled( false );
            ui->pushButtonDeleteSprite->setEnabled( false );
        }
    }

    dataChanged();
}

void MainWindow::itemSelectionChanged()
{
    QList<QTreeWidgetItem *> selectedItems;
    selectedItems = ui->treeWidget->selectedItems();

    if( selectedItems.size() == 0 )
    {
        ui->graphicsView->setSprite( NULL );
        return;
    }

    itemClicked( selectedItems.at(0), 0);
}

void MainWindow::pushButtonCogClicked(bool checked)
{
    ui->graphicsView->pushButtonCogClicked( checked );
}

void MainWindow::fileNew()
{
    QFileDialog dlg(this, tr("New project name"), QDir::currentPath());
    fileName_= dlg.getSaveFileName();

    if( fileName_.isEmpty() )
        return;

    fileOpen_ = true;
    ui->pushButtonAddSourceImage->setEnabled( true );
    ui->actionClose->setEnabled( true );
    ui->actionNew->setEnabled( false );
    ui->actionOpen->setEnabled( false );
}

void MainWindow::parseXmlImage( QXmlStreamReader &reader )
{
    QXmlStreamReader::TokenType token;
    QString fileName;
    SpriteImage *spriteImage;
    QTreeWidgetItem *item;

    if( parseXmlReadTag( reader, "filename", fileName ) != true )
        return;

    item = new QTreeWidgetItem(kTreeItemImage);
    QStringList parts = fileName.split(tr("/"));
    QString lastBit = parts.at(parts.size()-1);

    spriteImage = new SpriteImage();
    spriteImage->fileName_ = fileName;
    spriteImage->index_ = spriteImageNextIndex_++;
    imageFileList_.append( spriteImage );

    item->setData( 0, Qt::UserRole, spriteImage->index_ );
    item->setData( 0, Qt::DisplayRole, lastBit );
    ui->treeWidget->addTopLevelItem( item );

    do
    {
        token = reader.readNext();

        if( token == QXmlStreamReader::StartElement )
            if( reader.name() == "sprite" )
                parseXmlSprite( item, reader );

        if( token == QXmlStreamReader::EndElement )
            if( reader.name() == "image" )
                break;
    }
    while( !reader.atEnd() && !reader.hasError() );
}

void MainWindow::parseXmlSprite( QTreeWidgetItem *item, QXmlStreamReader &reader )
{
    QXmlStreamReader::TokenType token;
    Sprite *sprite;
    QTreeWidgetItem *childItem;
    QString spriteName, x, y, w, h, defined, cogdefined, cogx, cogy;
    QRect rect;

    if( parseXmlReadTag( reader, "name", spriteName ) != true )
        return;

    if( parseXmlReadTag( reader, "defined", defined ) != true )
        return;

    if( parseXmlReadTag( reader, "cogdefined", cogdefined ) != true )
        return;

    if( parseXmlReadTag( reader, "x", x ) != true )
        return;

    if( parseXmlReadTag( reader, "y", y ) != true )
        return;

    if( parseXmlReadTag( reader, "w", w ) != true )
        return;

    if( parseXmlReadTag( reader, "h", h ) != true )
        return;

    if( parseXmlReadTag( reader, "cogx", cogx ) != true )
        return;

    if( parseXmlReadTag( reader, "cogy", cogy ) != true )
        return;

    rect.setRect( x.toInt(), y.toInt(), w.toInt(), h.toInt() );

    sprite = new Sprite();
    sprite->index_ = spriteNextIndex_++;
    sprite->imageIndex_ = item->data( 0, Qt::UserRole ).toInt();
    sprite->setRect( rect );
    sprite->setDefined( defined.toInt() );
    sprite->setCogDefined( cogdefined.toInt() );
    sprite->setCog( QPoint( cogx.toInt(), cogy.toInt() ) );
    spriteList_.append( sprite );

    childItem = new QTreeWidgetItem(kTreeItemSprite);
    childItem->setFlags(item->flags() | Qt::ItemIsEditable);
    childItem->setData( 0, Qt::EditRole, spriteName );
    childItem->setData( 0, Qt::UserRole, sprite->index_ );
    item->setExpanded( true );
    item->addChild( childItem );

    token = reader.readNext();
    if( token != QXmlStreamReader::EndElement )
        return;

    if( reader.name() != "sprite" )
        return;
}

bool MainWindow::parseXmlReadTag( QXmlStreamReader &reader, QString name, QString &result )
{
    QXmlStreamReader::TokenType token;

    token = reader.readNext();
    if( token != QXmlStreamReader::StartElement )
        return( false );

    if( reader.name() != name )
        return( false );

    reader.readNext();
    result = reader.text().toString();
    token = reader.readNext();

    if( token != QXmlStreamReader::EndElement )
        return( false );

    if( reader.name() != name )
        return( false );

    return( true );
}

void MainWindow::fileOpen()
{
    QFileDialog dlg(this, tr("Open project"), QDir::currentPath());
    QFile file;
    QXmlStreamReader reader;
    QXmlStreamReader::TokenType token;

    fileName_= dlg.getOpenFileName();

    if( fileName_.isEmpty() )
        return;

    file.setFileName( fileName_ );
    if (!file.open(QIODevice::ReadOnly))
    {
        /* show wrror message if not able to open file */
        QMessageBox::warning(0, "File open failed", "Could not open file for loading.");
        return;
    }

    reader.setDevice( &file );

    while( !reader.atEnd() && !reader.hasError() )
    {
        token = reader.readNext();

        if( token == reader.isStartDocument() )
            continue;

        if( token == QXmlStreamReader::StartElement )
        {
            if(reader.name() == "spritepackerdocument")
                continue;

            if(reader.name() == "image")
            {
                parseXmlImage( reader );
            }
        }
    }


    if(reader.hasError())
    {
        QMessageBox::critical(this,
                              "Could not parse XML file",
                              reader.errorString(),
                              QMessageBox::Ok);
    }

    reader.clear();



    ui->actionOpen->setEnabled( false );
    ui->actionNew->setEnabled( false );
    ui->actionSave_As->setEnabled( true );
    ui->actionClose->setEnabled( true );
    ui->pushButtonAddSourceImage->setEnabled( true );
    ui->actionBuild->setEnabled( true );
}

void MainWindow::fileSave()
{
    QXmlStreamWriter writer;
    QTreeWidgetItem *topLevelItem = NULL;
    QTreeWidgetItem *childItem = NULL;
    QFile file( fileName_ );
    Sprite *sprite;
    QRect rect;

    int i = 0, j = 0, index = 0;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        /* show wrror message if not able to open file */
        QMessageBox::warning(0, "File creation failed", "Could not create save file.");
        return;
    }

    writer.setDevice( &file );
    writer.writeStartDocument();
    writer.writeStartElement( "spritepackerdocument" );

    for( i=0 ; i<ui->treeWidget->topLevelItemCount() ; i++ )
    {
        topLevelItem = ui->treeWidget->topLevelItem( i );

        writer.writeStartElement( "image" );

        writer.writeTextElement( "filename", imageFileList_.at( arrayIndexFromImageIndex( topLevelItem->data(0,Qt::UserRole).toInt() ) )->fileName() );

        for( j=0 ; j<topLevelItem->childCount() ; j++ )
        {
            childItem = topLevelItem->child( j );
            index = arrayIndexFromSpriteIndex( childItem->data(0,Qt::UserRole).toInt() );
            sprite = spriteList_.at( index );
            rect = sprite->getRect();

            writer.writeStartElement( "sprite" );

            writer.writeTextElement( "name", childItem->text(0) );
            writer.writeTextElement( "defined", QString::number( sprite->defined() ) );
            writer.writeTextElement( "cogdefined", QString::number( sprite->cogDefined() ) );
            writer.writeTextElement( "x", QString::number( rect.x() ) );
            writer.writeTextElement( "y", QString::number( rect.y() ) );
            writer.writeTextElement( "w", QString::number( rect.width() ) );
            writer.writeTextElement( "h", QString::number( rect.height() ) );
            writer.writeTextElement( "cogx", QString::number( sprite->getCog().x() ) );
            writer.writeTextElement( "cogy", QString::number( sprite->getCog().y() ) );
            writer.writeEndElement();
        }

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
    file.close();

    isModified_ = false;
    ui->actionSave->setEnabled( false );
}

void MainWindow::fileSaveAs()
{
    QFileDialog dlg(this, tr("New project name"), QDir::currentPath());
    QString fileName = dlg.getSaveFileName();

    if( fileName.isEmpty() )
        return;

    fileName_ = fileName;

    fileSave();
}

void MainWindow::fileClose()
{
    QList<QTreeWidgetItem *> selectedItems;
    int i = 0, j = 0, index;
    selectedItems = ui->treeWidget->selectedItems();
    QTreeWidgetItem *parentItem = NULL;

    if( isModified_ == true )
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "File close", "Project has not been saved. Are you sure you want to close?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }

    if( pixmapItem != NULL )
    {
       ui->graphicsView->scene()->removeItem( pixmapItem );
       delete pixmapItem;
       pixmapItem = NULL;
    }

    lastImageIndex_ = -1;

    for( i=ui->treeWidget->topLevelItemCount() - 1 ; i>=0 ; i-- )
    {
        parentItem = ui->treeWidget->takeTopLevelItem( i );
        for( j=0 ; j<parentItem->childCount() ; j++ )
        {
            index = arrayIndexFromSpriteIndex( parentItem->child(i)->data(0,Qt::UserRole).toInt() );
            spriteList_.removeAt(j);
        }

        index = arrayIndexFromImageIndex( parentItem->data(0,Qt::UserRole).toInt() );
        imageFileList_.removeAt( index );

        delete parentItem;
    }

    spriteImageNextIndex_ = 0;
    spriteNextIndex_ = 0;

    isModified_ = false;
    ui->actionSave->setEnabled( false );
    ui->actionSave_As->setEnabled( false );
    ui->actionClose->setEnabled( false );
    ui->actionOpen->setEnabled( true );
    ui->actionNew->setEnabled( true );
    ui->pushButtonAddSourceImage->setEnabled( false );
    ui->pushButtonAddSprite->setEnabled( false );
    ui->pushButtonCog->setEnabled( false );
    ui->pushButtonDeleteSprite->setEnabled( false );
    ui->actionBuild->setEnabled( false );
}

void MainWindow::dataChanged()
{
    isModified_ = true;
    ui->actionSave->setEnabled( true );
    ui->actionSave_As->setEnabled( true );
    ui->actionBuild->setEnabled( true );
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if( isModified_ == true )
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Quit", "Project has not been saved. Are you sure you want to quit?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            event->ignore();
        else
            close();
    }
}

void MainWindow::build()
{
    int i = 0, j = 0;
    int count = 0, index = 0;
    QTreeWidgetItem *topLevelItem = NULL;
    QTreeWidgetItem *childItem = NULL;
    QImage fullImage;
    QImage *spriteImages = NULL;
    Sprite *sprite = NULL;
    QRect rect;
    QImage destImage;
    QXmlStreamWriter writer;
    QFile file( "/home/garethf/Desktop/sprites.xml" );


    rbp::GuillotineBinPack binPack;
    rbp::Rect *packedRects = NULL;

    destImage = QImage( 512, 512, QImage::Format_ARGB32 );
    QPainter painter( &destImage );

    binPack.Init( 512, 512 );
    rbp::GuillotineBinPack::GuillotineSplitHeuristic splitHeuristic = rbp::GuillotineBinPack::SplitLongerAxis;
    rbp::GuillotineBinPack::FreeRectChoiceHeuristic choiceHeuristic = rbp::GuillotineBinPack::RectBestAreaFit;

    for( i=0 ; i<ui->treeWidget->topLevelItemCount() ; i++ )
    {
        topLevelItem = ui->treeWidget->topLevelItem( i );
        count += topLevelItem->childCount();
    }

    spriteImages = new QImage[ count ];
    packedRects = new rbp::Rect[ count ];
    count = 0;


    // Loop for image.
    for( i=0 ; i<ui->treeWidget->topLevelItemCount() ; i++ )
    {
        topLevelItem = ui->treeWidget->topLevelItem( i );
        fullImage.load( imageFileList_.at( arrayIndexFromImageIndex( topLevelItem->data(0,Qt::UserRole).toInt() ) )->fileName() );

        // Loop for sprite.
        for( j=0 ; j<topLevelItem->childCount() ; j++ )
        {
            childItem = topLevelItem->child( j );
            index = arrayIndexFromSpriteIndex( childItem->data(0,Qt::UserRole).toInt() );
            sprite = spriteList_.at( index );
            rect = sprite->getRect();

            spriteImages[ count ] = fullImage.copy( rect );
            packedRects[ count ] = binPack.Insert( rect.width(), rect.height(), true, choiceHeuristic, splitHeuristic );

            painter.drawImage( packedRects[count].x, packedRects[count].y, spriteImages[ count ] );
            qDebug() << "Packed x=" << packedRects[count].x << " y=" << packedRects[count].y << " width=" << packedRects[count].width << " height=" << packedRects[count].height;
            count++;
        }
    }


    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        /* show wrror message if not able to open file */
        QMessageBox::warning(0, "File creation failed", "Could not create save file.");
        return;
    }

    writer.setDevice( &file );
    writer.writeStartDocument();
    writer.writeStartElement( "spritepackerdocument" );

    count = 0;

    for( i=0 ; i<ui->treeWidget->topLevelItemCount() ; i++ )
    {
        topLevelItem = ui->treeWidget->topLevelItem( i );

        //writer.writeStartElement( "image" );

        //writer.writeTextElement( "filename", imageFileList_.at( arrayIndexFromImageIndex( topLevelItem->data(0,Qt::UserRole).toInt() ) )->fileName() );

        for( j=0 ; j<topLevelItem->childCount() ; j++ )
        {
            childItem = topLevelItem->child( j );
            index = arrayIndexFromSpriteIndex( childItem->data(0,Qt::UserRole).toInt() );
            sprite = spriteList_.at( index );
            rect = sprite->getRect();

            writer.writeStartElement( "sprite" );

            writer.writeTextElement( "name", childItem->text(0) );
            writer.writeTextElement( "defined", QString::number( sprite->defined() ) );
            writer.writeTextElement( "cogdefined", QString::number( sprite->cogDefined() ) );
            writer.writeTextElement( "x", QString::number( packedRects[ count ].x ) );
            writer.writeTextElement( "y", QString::number( packedRects[ count ].y ) );
            writer.writeTextElement( "w", QString::number( packedRects[ count ].width ) );
            writer.writeTextElement( "h", QString::number( packedRects[ count ].height ) );
            writer.writeTextElement( "cogx", QString::number( sprite->getCog().x() - sprite->getRect().x() ) );
            writer.writeTextElement( "cogy", QString::number( sprite->getCog().y() - sprite->getRect().y() ) );
            writer.writeEndElement();
            count++;
        }

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
    file.close();

    delete [] spriteImages;
    delete [] packedRects;

    destImage.save( "/home/garethf/Desktop/sprites.jpeg");
}
