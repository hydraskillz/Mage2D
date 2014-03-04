//---------------------------------------
inline bool Entity::IsAlive() const
{
	return mAlive;
}
//---------------------------------------
inline int Entity::GetHealth() const
{
	return mHealth;
}
//---------------------------------------
inline float Entity::GetAge() const
{
	return mAge;
}
//---------------------------------------
inline unsigned int Entity::GetEntityId() const
{
	return mEntityId;
}
//---------------------------------------
inline void Entity::SetMaxHealth( int maxHealth )
{
	mMaxHealth = maxHealth;
}
//---------------------------------------
inline void Entity::SetHealthToMax()
{
	mHealth = mMaxHealth;
}
//---------------------------------------
inline Vec2f Entity::GetPosition() const
{
	return mPosition;
}
//---------------------------------------
inline float Entity::GetMass() const
{
	return mMass;
}
//---------------------------------------
inline void Entity::SetMass( float mass )
{
	mMass = mass;
	mInvMass = mMass == 0.0f ? 0.0f : 1.0f / mass;
}
//---------------------------------------
inline void Entity::SetGameMap( Map* map )
{
	mGameMap = map;
}
//---------------------------------------
template< typename TFunction >
void Entity::ForEachEntity( TFunction fn )
{
	for ( auto itr = msEntities.begin(); itr != msEntities.end(); ++itr )
	{
		fn( *itr );
	}
}
//---------------------------------------
inline void Entity::SetUserFlags( const std::string& flags )
{
	mUserFlags = flags;
}
//---------------------------------------
inline const std::string& Entity::GetUserFlags() const
{
	return mUserFlags;
}
//---------------------------------------