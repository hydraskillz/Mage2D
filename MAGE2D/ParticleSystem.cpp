#include "MageLib.h"

using namespace mage;

//---------------------------------------
ParticleSystem::ParticleSystem()
{
}
//---------------------------------------
ParticleSystem::~ParticleSystem()
{
}
//---------------------------------------


//---------------------------------------
Particle::Particle( Sprite* sprite )
	: mLife( 0.0f )
	, mAlive( true )
	, mSprite( sprite )
{}
//---------------------------------------
Particle::~Particle()
{}
//---------------------------------------
void Particle::SetPosition( float x, float y )
{
	mSprite->Position.Set( x, y );
}
//---------------------------------------
void Particle::OnUpdate( float dt )
{
	mLife -= dt;

	if ( life <= 0.0f )
	{
		mAlive = false;
	}

	if ( mAlive )
	{
		mVelocity += mAcceleration * dt;

		mSprite->Position += mVelocity;
	}
}
//---------------------------------------
void Particle::OnRender( SDL_Surface* surf_Display )
{	
}
//---------------------------------------