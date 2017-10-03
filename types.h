#ifndef TYPES_H
#define TYPES_H

enum TreeItem
{
    kTreeItemImage,
    kTreeItemSprite
};

class SpriteImage
{
    public:

        SpriteImage()
        {
            index_ = 0;
        }

        ~SpriteImage()
        {

        }

        QString fileName(){ return fileName_; }

    private:

        QString fileName_;
        int index_;
        friend class MainWindow;
};

class Sprite
{
    public:

        Sprite()
        {
            index_ = 0;
            imageIndex_ = 0;
            isDefined_ = false;
            isCogDefined_ = false;
        }

        ~Sprite()
        {

        }

        void setRect( const QRect &rect ){ rect_ = rect; }
        QRect getRect( void ){ return( rect_ ); }
        void setCog( const QPoint cog ){ cog_ = cog; }
        QPoint getCog( void ){ return( cog_ ); }
        void setImageIndex( int index ){ imageIndex_ = index; }
        int getImageIndex( void ){ return( imageIndex_ ); }
        bool defined( void ){ return( isDefined_ ); }
        void setDefined( bool defined ){ isDefined_ = defined; }
        bool cogDefined( void ){ return( isCogDefined_ ); }
        void setCogDefined( bool defined ){ isCogDefined_ = defined; }

    private:

        int imageIndex_;
        QRect rect_;
        QPoint cog_;
        int index_;
        friend class MainWindow;
        bool isDefined_;
        bool isCogDefined_;
};

#endif // TYPES_H
