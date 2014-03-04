/*
 * Author      : Matthew Johnson
 * Date        : 29/Jun/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{
	class GameObject;

	class GameLogicNode
	{
	public:
		void AttachTo( GameObject* newOwner, int execPriority );
		const GameObject* GetOwner() const { return mOwner; }
		const std::string& GetName() const { return mName;  }
		const int GetExecPriority() const  { return mExecPriority; }
		std::string EvaluateVariablePropertyString( const std::string& var );

		virtual void Reset() {}
	protected:
		GameLogicNode( const std::string& name );
		virtual ~GameLogicNode();

		int mExecPriority;
		GameObject* mOwner;
		const std::string mName;
	};

}