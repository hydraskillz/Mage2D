/*
 * Author      : Matthew Johnson
 * Date        : 22/May/2013
 * Description :
 *   
 */
 
#pragma once

struct SDL_Surface;

namespace mage
{

	class GameObject;

	class Sprite
	{
	public:
		Sprite( SpriteAnimationSet& animation, const std::string& initialAnimName );
		~Sprite();

		void OnUpdate( float dt );
		void OnDraw( SDL_Surface* screen, const Camera& camera );

		void SetAnimationState( const std::string& animName, int frame=0 );
		void RegisterOnCompletionEvent( const std::string& animName, const std::string& eventName );
		RectI GetClippingRectForCurrentAnimation() const;

		const std::string& GetCurrentAnimationName() const;
		bool IsCurrentAnimationFinished() const;

		Vec2f Position;
		bool RelativeToCamera;
		GameObject* Owner;

	private:
		struct AnimationInfo
		{
			std::string CurrentAnimationName;
			int CurrentAnimationFrame;
			float CurrentAnimationFrameTime;
		};

		std::map< std::string, std::string > mOnCompletionEvents;

		AnimationInfo mSprAnimInfo;
		const SpriteAnimationSet& mAnimationSet;

		Sprite& operator= ( const Sprite& other );
	};

}