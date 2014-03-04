#include "MageLib.h"

using namespace mage;

//---------------------------------------
Sprite::Sprite( SpriteAnimationSet& animation, const std::string& initialAnimName )
	: mAnimationSet( animation )
	, RelativeToCamera( true )
	, Owner( NULL )
{
	auto anim = animation.Animations.find( initialAnimName );
	if ( anim != animation.Animations.end() )
	{
		mSprAnimInfo.CurrentAnimationName = anim->first;
	}
	mSprAnimInfo.CurrentAnimationFrame = 0;
	mSprAnimInfo.CurrentAnimationFrameTime = 0.0f;
}
//---------------------------------------
Sprite::~Sprite()
{}
//---------------------------------------
void Sprite::OnUpdate( float dt )
{
	mSprAnimInfo.CurrentAnimationFrameTime += dt;

	// Check if animation exists
	if ( mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName ) != mAnimationSet.Animations.end() )
	{
		const SpriteAnimation& sprAnim = mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName )->second;
		const SpriteAnimationFrame& sprFrame = sprAnim.Frames[ mSprAnimInfo.CurrentAnimationFrame ];

		// Check if frame needs to be increased
		if ( mSprAnimInfo.CurrentAnimationFrameTime >= sprFrame.Delay * dt )
		{
			++mSprAnimInfo.CurrentAnimationFrame;
			mSprAnimInfo.CurrentAnimationFrameTime = 0.0f;
		}

		// Loop frame
		if ( mSprAnimInfo.CurrentAnimationFrame >= sprAnim.FrameCount )
		{
			if ( sprAnim.LoopType == SpriteAnimation::ANIM_LOOP )
			{
				// Loop back to first frame
				mSprAnimInfo.CurrentAnimationFrame = 0;
			}
			else if ( sprAnim.LoopType == SpriteAnimation::ANIM_ONCE )
			{
				// Hold on last frame
				mSprAnimInfo.CurrentAnimationFrame = sprAnim.FrameCount - 1;
			}

			// Fire events for animation completion, if any
			auto completionEvent = mOnCompletionEvents.find( mSprAnimInfo.CurrentAnimationName );
			if ( completionEvent != mOnCompletionEvents.end() )
			{
				Dictionary params;
				params.Set( "Sprite", this );
				params.Set( "EventName", completionEvent->second );
				EventManager::FireEvent( completionEvent->second, params );
			}
		}
	}
}
//---------------------------------------
void Sprite::OnDraw( SDL_Surface* screen, const Camera& camera )
{
	if ( mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName ) != mAnimationSet.Animations.end() )
	{
		const SpriteAnimationComponent* sprComp = 
			mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName )->second
			.Frames[ mSprAnimInfo.CurrentAnimationFrame ].SprAnimComponent;
		const RectI& clip = sprComp->SprComponent->SpriteClips[ sprComp->SprComponentClipName ];
		SDL_Surface* surf = mAnimationSet.MyDefinition->SpriteSheet;

		int px = (int) Position.x + sprComp->FrameOffsetX * 2;
		int py = (int) Position.y + sprComp->FrameOffsetY * 2;

		if ( Owner && RelativeToCamera )
		{
			px += (int) Owner->Position.x;
			py += (int) Owner->Position.y;
		}

		RectI worldClip( px, py, px + clip.Width(), py + clip.Height() );

		if ( RelativeToCamera )
		{
			if ( camera.RectInViewport( worldClip ) )
			{
				//Surface::DrawRect( screen,
				//	px - (int) camera.GetX(),
				//	py - (int) camera.GetY(),
				//	clip.Width(), clip.Height(),
				//	0xffff00ff, false );

				Surface::DrawSurface( screen, surf,
					px - (int) camera.GetX(),
					py - (int) camera.GetY(),
					clip.Left, clip.Top, clip.Width(), clip.Height() );
			
			}
		}
		else
		{
			//Surface::DrawRect( screen,
			//	px,
			//	py,
			//	clip.Width(), clip.Height(),
			//	0xffff00ff, false );

			Surface::DrawSurface( screen, surf,
				px,
				py,
				clip.Left, clip.Top, clip.Width(), clip.Height() );
		}
	}
}
//---------------------------------------
void Sprite::SetAnimationState( const std::string& animName, int frame )
{
	// Don't do anything if already in given state
	if ( mSprAnimInfo.CurrentAnimationName != animName )
	{
		mSprAnimInfo.CurrentAnimationName = animName;
		mSprAnimInfo.CurrentAnimationFrame = frame;
		mSprAnimInfo.CurrentAnimationFrameTime = 0.0f;
	}
}
//---------------------------------------
void Sprite::RegisterOnCompletionEvent( const std::string& animName, const std::string& eventName )
{
	mOnCompletionEvents[ animName ] = eventName;
}
//---------------------------------------
RectI Sprite::GetClippingRectForCurrentAnimation() const
{
	RectI clip;
	if ( mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName ) != mAnimationSet.Animations.end() )
	{
		const SpriteAnimationComponent* sprComp = 
			mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName )->second
			.Frames[ mSprAnimInfo.CurrentAnimationFrame ].SprAnimComponent;
		const std::string& clipName = sprComp->SprComponentClipName;
		SpriteComponent* sprComp0 = sprComp->SprComponent;
		if ( sprComp0 )
		{
			clip = sprComp0->SpriteClips[ clipName ];
		}
	}
	return clip;
}
//---------------------------------------
const std::string& Sprite::GetCurrentAnimationName() const
{
	return mSprAnimInfo.CurrentAnimationName;
}
//---------------------------------------
bool Sprite::IsCurrentAnimationFinished() const
{
	// Check if animation exists
	if ( mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName ) != mAnimationSet.Animations.end() )
	{
		const SpriteAnimation& sprAnim = mAnimationSet.Animations.find( mSprAnimInfo.CurrentAnimationName )->second;

		return mSprAnimInfo.CurrentAnimationFrame == sprAnim.FrameCount - 1;
	}
	return false;
}
//---------------------------------------