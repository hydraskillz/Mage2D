/*
 * Author      : Matthew Johnson
 * Date        : 26/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	//---------------------------------------
	// GameController
	//---------------------------------------
	class GameController
		: public GameLogicNode
	{
	public:
		GameController( const std::string& name, uint32 state );
		virtual ~GameController();

		void OnUpdate();
		virtual bool Triggered() const = 0;
		void AddTrigger( GameTrigger* trigger );
		void AddAction( GameAction* action );

		inline uint32 GetState() const { return mState; }
		static int GetTypeFromString( const std::string& contTypeString );
		static const char* GameControllerTypeToCString( int type );

		enum GameControllerType
		{
			GC_AND,
			GC_OR,
			GC_NAND,
			GC_NOR,
			GC_XOR,
			GC_XNOR,
			GC_EXPR,
			GC_SCRIPT,
			GC_COUNT
		};

	protected:
		uint32 mState;
		GameControllerType mType;
		std::vector< GameTrigger* > mTriggers;
		std::vector< GameAction* > mActions;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerAnd
	//---------------------------------------
	class GameControllerAnd
		: public GameController
	{
	public:
		GameControllerAnd( const std::string& name, uint32 state );
		virtual ~GameControllerAnd();

		virtual bool Triggered() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerNAnd
	//---------------------------------------
	class GameControllerNAnd
		: public GameControllerAnd
	{
	public:
		GameControllerNAnd( const std::string& name, uint32 state );
		virtual ~GameControllerNAnd();

		virtual bool Triggered() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerOr
	//---------------------------------------
	class GameControllerOr
		: public GameController
	{
	public:
		GameControllerOr( const std::string& name, uint32 state );
		virtual ~GameControllerOr();

		virtual bool Triggered() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerNOr
	//---------------------------------------
	class GameControllerNOr
		: public GameControllerOr
	{
	public:
		GameControllerNOr( const std::string& name, uint32 state );
		virtual ~GameControllerNOr();

		virtual bool Triggered() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerXOr
	//---------------------------------------
	class GameControllerXOr
		: public GameController
	{
	public:
		GameControllerXOr( const std::string& name, uint32 state );
		virtual ~GameControllerXOr();

		virtual bool Triggered() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerXNOr
	//---------------------------------------
	class GameControllerXNOr
		: public GameControllerXOr
	{
	public:
		GameControllerXNOr( const std::string& name, uint32 state );
		virtual ~GameControllerXNOr();

		virtual bool Triggered() const;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerExpr
	//---------------------------------------
	class GameControllerExpr
		: public GameController
	{
	public:
		GameControllerExpr( const std::string& name, uint32 state,
			const std::string& expression );
		virtual ~GameControllerExpr();

		virtual bool Triggered() const;

	protected:
		GameExpression mExpr;
	};
	//---------------------------------------


	//---------------------------------------
	// GameControllerScript
	//---------------------------------------
	class GameControllerScript
		: public GameController
	{
	public:
		GameControllerScript( const std::string& name, uint32 state,
			const std::string& script );
		virtual ~GameControllerScript();

		virtual bool Triggered() const;

	protected:
		std::string mScriptName;
	};
	//---------------------------------------

}