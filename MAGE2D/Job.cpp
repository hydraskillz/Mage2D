#include "MageLib.h"

using namespace mage;

//---------------------------------------
Job::Job( JobPriority priority, JobType jobType )
	: mPriority( priority )
	, mJobType( jobType )
{}
//---------------------------------------
Job::~Job()
{}
//---------------------------------------