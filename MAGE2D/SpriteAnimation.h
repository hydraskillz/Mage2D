/*
 * Author      : Matthew Johnson
 * Date        : 22/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	struct SpriteAnimationComponent
	{
		// @TODO rotation/flip animation

		// The component to draw
		SpriteComponent* SprComponent;
		std::string SprComponentClipName;
		// Offsets
		int FrameOffsetX;
		int FrameOffsetY;
		int FrameZOrder;		// Unused
	};

	// A frame of animation
	// Contains a map of components that use this animation
	struct SpriteAnimationFrame
	{
		int Index;
		int Delay;
		// @TODO We only support one component per frame right now...
		SpriteAnimationComponent* SprAnimComponent;
	};

	// A named animation
	// Contains a vector of animation frames
	struct SpriteAnimation
	{
		enum SpriteAnimationLoopType
		{
			ANIM_LOOP,
			ANIM_ONCE
		};

		std::string Name;
		int LoopType;
		int FrameCount;
		std::vector< SpriteAnimationFrame > Frames;
	};

	// A collection of animations
	// Has the name of the SpriteDefinition this AnimationSet was created for
	// Contains a map of SpriteAnimations
	struct SpriteAnimationSet
	{
		std::string Name;
		SpriteDefinition* MyDefinition;
		std::map< std::string, SpriteAnimation > Animations;
	};

}