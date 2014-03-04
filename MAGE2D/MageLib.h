#pragma once

//#pragma comment( linker, "/nodefaultlib:libc.lib" )
//#pragma comment( linker, "/nodefaultlib:libcd.lib" )

#include <MageMath.h>

// Typedefs used throughout the engine
#include "MageTypes.h"
#include "BitHacks.h"

// Debugging output
#include "DebugIO.h"

// Assertion for low-level errors
#include "Assertion.h"

// Custom memory management
#include "MageMemory.h"

// C headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// C++ headers
#include "stl_headers.h"
#include <sstream>

#ifdef USE_MEMORY_MANAGER
#	define new new( __FILE__, __LINE__ )
#endif

// Best troll evar
//#define switch getchar(); switch

// Core headers
#include "InitTerm.h"
#include "Dictionary.h"
#include "CircularBuffer.h"
#include "Event.h"
#include "Console.h"
#include "Clock.h"
#include "ProfilingSystem.h"
#include "FrameRateControl.h"
#include "Color.h"
#include "HashUtil.h"

// File/IO manipulation
#include "StringUtil.h"
#include "FileSystem.h"
#include "XmlReader.h"
#include "XmlUtil.h"
#include "CommandArgs.h"

// Audio
#include "SoundManager.h"

// Threads
#include "Mutex.h"
#include "Thread.h"
#include "Job.h"
#include "JobManager.h"
#include "FileLoadJob.h"
#include "HashBufferJob.h"

// Graphics
//#include "Texture.h"
#include "Font.h"
#include "Surface.h"
#include "Camera.h"

#include "SpriteDefinition.h"
#include "SpriteAnimation.h"
#include "Sprite.h"
#include "SpriteManager.h"

#include "MapObject.h"
#include "Tile.h"
#include "Map.h"

#include "TriggeredEvent.h"

#include "Definitionbase.h"
#include "EntityDefinition.h"
#include "Entity.h"

//#define DEBUG_GAMEOBJECT_VERBOSE

#include "GameVar.h"
#include "GameExpression.h"
#include "GameString.h"
#include "GameLogicNode.h"
#include "GameTrigger.h"
#include "GameAction.h"
#include "GameController.h"
#include "GameObjectDefinition.h"
#include "GameObject.h"
#include "GameNodeEditor.h"

// Other
#include "InputManager.h"
#include "GameInfo.h"

// Script
//#include "PythonWrapper.h"

// App
#include "MageAppBase.h"
#include "MageGame.h"