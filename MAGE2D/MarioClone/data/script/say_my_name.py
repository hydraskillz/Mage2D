import mage
mage.ConsolePrint( 'My name is ' + mage.GetOwnerName() )

# Get the value of 'OwnerName.Score'
score = mage.GetIntProperty( 'Score' )

# if property get was ok
if score != None:
    # add to score and set the value
    score = score + 1
    mage.SetIntProperty( 'Score', score )