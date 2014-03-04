//---------------------------------------
inline bool GameVar::IsNullVar() const
{
	return mType == GV_NULL;
}
//---------------------------------------
inline const char* GameVar::GetName() const
{
	return mName;
}
//---------------------------------------
inline GameVarType GameVar::GetType() const
{
	return mType;
}
//---------------------------------------
template< typename T >
inline void GameVar::AssignData( void* data, const void* target )
{
	*(T*)data = *(T*)target;
}
//---------------------------------------
template< typename T >
inline void GameVar::AddData( void* data, const void* target )
{
	*(T*)data += *(T*)target;
}
//---------------------------------------
template< typename T >
bool GameVar::GetDataAs( T* type ) const
{
	// @TODO rtti
	*type = *(T*)mData;
	return true;
}
//---------------------------------------
inline void GameVar::SetDisplayColor( const Color& color )
{
	mDisplayColor = color;
}
//---------------------------------------
inline const Color& GameVar::GetDisplayColor() const
{
	return mDisplayColor;
}
//---------------------------------------