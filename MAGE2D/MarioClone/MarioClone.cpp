#include "MarioClone.h"

#include <FileSystem.h>
#include <JobManager.h>

using namespace mage;

MAGE_GAME_APP( MarioClone );

//---------------------------------------
MarioClone::MarioClone()
	: MageGame( "Super Mario Clone", 0, 0, 800, 600, false )
{
	mDataRoot = "../MarioClone/data/";
	AddPathToData( "../MarioClone/data" );
	//SetDataArchivePassword( "password", 1 );
}
//---------------------------------------
MarioClone::~MarioClone()
{}
//---------------------------------------
bool MarioClone::OnInitialize()
{
	// @TODO fix job manager from burning out the cpu
	JobManager::GetInstance()->SetMaxWorkerThreads( 0 );
	//mSpriteManager.LoadSpriteDefinition( "../MarioClone/data/sprites/mario.sprites" );
	//mSpriteManager.LoadSpriteAnimations( "../MarioClone/data/sprites/mario.anim" );

	// Do this in data
	mCamera->SetViewport( 320, 224 );
	//RectI worldBounds = mGameMap.GetWorldBounds();
	//mCamera->SetWorldBounds( 0, worldBounds.Right, 0, 224 );

	return true;
}
//---------------------------------------