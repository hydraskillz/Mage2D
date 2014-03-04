#include "BulletHell.h"

#include <FileSystem.h>
#include <JobManager.h>

using namespace mage;

MAGE_GAME_APP( BulletHell );

//---------------------------------------
BulletHell::BulletHell()
	: MageGame( "Bullet Hell", 0, 0, 800, 600, false )
{
	mDataRoot = "../BulletHell/data/";
	AddPathToData( "../BulletHell/data" );
}
//---------------------------------------
BulletHell::~BulletHell()
{}
//---------------------------------------
bool BulletHell::OnInitialize()
{
	// @TODO fix job manager from burning out the cpu
	JobManager::GetInstance()->SetMaxWorkerThreads( 0 );
	//mSpriteManager.LoadSpriteDefinition( "../MarioClone/data/sprites/mario.sprites" );
	//mSpriteManager.LoadSpriteAnimations( "../MarioClone/data/sprites/mario.anim" );

	// Do this in data
	//mCamera->SetViewport( 320, 224 );
	//RectI worldBounds = mGameMap.GetWorldBounds();
	//mCamera->SetWorldBounds( 0, worldBounds.Right, 0, 224 );

	return true;
}
//---------------------------------------