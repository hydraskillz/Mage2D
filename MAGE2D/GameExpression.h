/*
 * Author      : Matthew Johnson
 * Date        : 10/Jul/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class GameExpression
	{
	public:
		// An expression in infix notation
		GameExpression( const std::string& expr );
		~GameExpression();

		// Owner is used to look up property values
		// If a syntax error is encountered, the result is undefined
		int Eval( GameObject* owner=NULL ) const;

		const char* ToString() const { return mExpr.c_str(); }

	protected:
		// Evaluates expression as much as possible without knowing variable values
		void Simplify();
		// Convert an infix expression into a postfix expression
		void InfixToPostfix();
		// True if char is a supported operator
		bool IsOperator( const char c ) const;
		// True if char is a number
		bool IsNumeric( const char c ) const;
		// True if char is whitespace
		bool IsWhitespace( const char c ) const;
		// True if char is the begining of a variable
		bool IsVariable( const char c ) const;
		// Syntax error reporting
		void SyntaxError( uint32 index, const char* msg, ... ) const;
		// Extract the variable name from the expression, assuming the variable starts at position i
		// The indexer i is moved to the end of the variable substring
		std::string GameExpression::ExtractVariable( uint32& i, const std::string& mExpr ) const;

	private:
		std::string mExpr;
	};

}