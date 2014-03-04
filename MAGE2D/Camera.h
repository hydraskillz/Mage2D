/*
 * Author      : Matthew Johnson
 * Date        : 21/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class Camera
	{
	public:
		Camera( int viewWidth, int viewHeight );
		~Camera();

		float GetX() const;
		float GetY() const;

		void SetPosition( float x, float y );
		void SetViewport( int width, int height );
		void SetWorldBounds( const RectI& worldBounds );
		void SetWorldBounds( int xMin, int xMax, int yMin, int yMax );
		void LookAt( const Vec2f& position );
		void LookAt( float x, float y );

		bool RectInViewport( RectI rect ) const;

		inline int GetViewWidth() const;
		inline int GetViewHeight() const;

	private:
		float mPositionX, mPositionY;
		int mViewWidth, mViewHeight;
		RectI mViewportRect;
		RectI mWorldBounds;
		
	};

#include "Camera.hpp"

}