#include "MageLib.h"

using namespace mage;

//---------------------------------------
Camera::Camera( int viewWidth, int viewHeight )
	: mPositionX( 0 )
	, mPositionY( 0 )
{
	SetViewport( viewWidth, viewHeight );
}
//---------------------------------------
Camera::~Camera()
{}
//---------------------------------------
float Camera::GetX() const
{
	return mPositionX;
}
//---------------------------------------
float Camera::GetY() const
{
	return mPositionY;
}
//---------------------------------------
void Camera::SetPosition( float x, float y )
{
	mPositionX = x;
	mPositionY = y;
	mViewportRect.Left = (int) x;
	mViewportRect.Top = (int) y;
}
//---------------------------------------
void Camera::SetViewport( int width, int height )
{
	mViewWidth = width;
	mViewHeight = height;
	mViewportRect.Set( 0, 0, width, height );
}
//---------------------------------------
void Camera::SetWorldBounds( const RectI& worldBounds )
{
	mWorldBounds = worldBounds;
}
//---------------------------------------
void Camera::SetWorldBounds( int xMin, int xMax, int yMin, int yMax )
{
	mWorldBounds.Set( xMin, yMin, xMax, yMax );
}
//---------------------------------------
void Camera::LookAt( const Vec2f& position )
{
	LookAt( position.x, position.y );
}
//---------------------------------------
void Camera::LookAt( float x, float y )
{
	mPositionX = x - mViewWidth * 0.5f;
	mPositionY = y - mViewHeight * 0.5f;

	// Clamp in X
	if ( mWorldBounds.Width() <= mViewWidth )
	{
		mPositionX = (float) mWorldBounds.Left;
	}
	else
	{
		Mathf::ClampToRange( mPositionX,
			(float) mWorldBounds.Left,
			(float) mWorldBounds.Right - mViewWidth );
	}

	// Clamp in Y
	if ( mWorldBounds.Height() <= mViewHeight )
	{
		mPositionY = (float) mWorldBounds.Top;
	}
	else
	{
		Mathf::ClampToRange( mPositionY,
			(float) mWorldBounds.Top,
			(float) mWorldBounds.Bottom - mViewHeight );
	}
}
//---------------------------------------
bool Camera::RectInViewport( RectI rect ) const
{
	// Put rect into camera space
	rect.Left -= (int) GetX();
	rect.Top  -= (int) GetY();
	// Test intersection
	return RectI::TestIntersection( mViewportRect, rect );
}
//---------------------------------------z