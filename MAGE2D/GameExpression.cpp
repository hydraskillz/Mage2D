#include "Magelib.h"

using namespace mage;

//---------------------------------------
GameExpression::GameExpression( const std::string& expr )
	: mExpr( expr )
{
	InfixToPostfix();
}
//---------------------------------------
GameExpression::~GameExpression()
{}
//---------------------------------------
int GameExpression::Eval( GameObject* owner ) const
{
	uint32 len = mExpr.length();
	std::stack< int > stack;
	//ConsolePrintf( "Evaluting %s\n", mExpr.c_str() );
	for ( uint32 i = 0; i < len; ++i )
	{
		const char c = mExpr[i];

		// Addition
		if ( c == '+' )
		{
			int a = stack.top(); stack.pop();
			int b = stack.top(); stack.pop();
			stack.push( a + b );
		}

		// Subtraction
		if ( c == '-' )
		{
			int a = stack.top(); stack.pop();
			int b = stack.top(); stack.pop();
			stack.push( a - b );
		}

		// Multiplication
		else if ( c == '*' )
		{
			int a = stack.top(); stack.pop();
			int b = stack.top(); stack.pop();
			stack.push( a * b );
		}

		// And
		else if ( c == '&' )
		{
			int a = stack.top(); stack.pop();
			int b = stack.top(); stack.pop();

			// Logic, &&
			if ( (i+1) < len && mExpr[i+1] == '&' )
			{
				++i;
				stack.push( a && b );
			}
			// Bitwise, &
			else
			{
				stack.push( a & b );
			}
		}

		// Or
		else if ( c == '|' )
		{
			int a = stack.top(); stack.pop();
			int b = stack.top(); stack.pop();

			// Logic, ||
			if ( (i+1) < len && mExpr[i+1] == '|' )
			{
				++i;
				stack.push( a || b );
			}
			// Bitwise, |
			else
			{
				stack.push( a | b );
			}
		}

		// Equal
		else if ( c == '=' )
		{
			int a = stack.top(); stack.pop();
			int b = stack.top(); stack.pop();

			// Logic, ==
			if ( (i+1) < len && mExpr[i+1] == '=' )
			{
				++i;
				stack.push( a == b );
			}
			// Syntax error (assignment not supported yet)
			else
			{
				SyntaxError( i, "Unexpected symbol" );
				break;
			}
		}

		// Begin variable
		else if ( IsVariable( c ) )
		{
			std::string varName = ExtractVariable( i, mExpr );
			if ( owner )
			{
				GameVar& gvar = owner->GetProperty( varName.substr( 1 ) );

				if ( gvar == GameVar::NULL_VAR )
				{
					SyntaxError( i, "Variable property not found" );
					break;
				}

				int value;
				gvar.GetDataAs( &value );

				stack.push( value );
			}
			else
			{
				SyntaxError( i, "Variable property without owner" );
				break;
			}
		}

		// Begin number
		else if ( IsNumeric( c ) )
		{
			stack.push( 0 );

			// Multi digit number
			for ( ; ; )
			{
				int a = 10 * stack.top(); stack.pop();
				int b = mExpr[i] - '0';
				stack.push( a + b );

				if ( (i+1) < len && IsNumeric( mExpr[i+1] ) )
					++i;
				else
					break;
			}
		}
		else if ( !IsWhitespace( c ) )
		{
			SyntaxError( i, "Unexpected symbol" );
			break;
		}
		// else whitespace separator
	}

	return stack.empty() ? 0 : stack.top();
}
//---------------------------------------
void GameExpression::Simplify()
{
}
//---------------------------------------
void GameExpression::InfixToPostfix()
{
	uint32 len = mExpr.length();
	std::string postfixExpr = "";
	std::stack< char > stack;

	for ( uint32 i = 0; i < len; ++i )
	{
		const char c = mExpr[i];

		// End expression
		if ( c == ')' )
		{
			postfixExpr += stack.top();
			stack.pop();
		}

		// Operator
		else if ( IsOperator( c ) )
		{
			stack.push( c );
		}

		// Number
		else if ( IsNumeric( c ) )
		{
			postfixExpr += c;
			// Add a space after numbers to separate them, i.e. 5 9 8 + instead of 598+
			if ( (i+1) < len && !IsNumeric(mExpr[i+1]))
				postfixExpr += ' ';
		}

		// Variable
		else if ( IsVariable( c ) )
		{
			postfixExpr += ExtractVariable( i, mExpr );
			postfixExpr += ' ';
		}
	}

	// Global expression
	while ( !stack.empty() )
	{
		postfixExpr += stack.top();
		stack.pop();
	}

	mExpr = postfixExpr;
}
//---------------------------------------
bool GameExpression::IsOperator( const char c ) const
{
	return ( c == '+' ) || ( c == '-' ) || ( c == '*' ) || ( c == '\\' ) || ( c == '&' ) || ( c == '|' ) || ( c == '=' );
}
//---------------------------------------
bool GameExpression::IsNumeric( const char c ) const
{
	return ( c >= '0' ) && ( c <= '9' );
}
//---------------------------------------
bool GameExpression::IsWhitespace( const char c ) const
{
	return ( c == ' ') || ( c == '\t' ) || ( c == '\n' ) || ( c == '\r' );
}
//---------------------------------------
bool GameExpression::IsVariable( const char c ) const
{
	return ( c == '@' );
}
//---------------------------------------
void GameExpression::SyntaxError( uint32 index, const char* msg, ... ) const
{
	ConsolePrintf( CONSOLE_ERROR, "SYNTAX ERROR:\n\"%s\"\n%*c\n%s\n",
		mExpr.c_str(), index + 2, '^', msg );
}
//---------------------------------------
std::string GameExpression::ExtractVariable( uint32& i, const std::string& mExpr ) const
{
	assertion( IsVariable( mExpr[i] ), "ExtractVariable call on non-variable index\n" );

	std::string var;
	int terminatingToken = std::string::npos;

	for ( uint32 c = i; c < mExpr.length(); ++c )
	{
		const char ch = mExpr[c];

		if ( IsOperator( ch ) || IsWhitespace( ch ) || ch == ')' )
		{
			terminatingToken = c;
			break;
		}
	}

	if ( terminatingToken == std::string::npos )
	{
		var = mExpr.substr( i );
		i = mExpr.length();
	}
	else
	{
		var = mExpr.substr( i, terminatingToken - i );
		i = terminatingToken - 1;
	}

	return var;
}
//---------------------------------------