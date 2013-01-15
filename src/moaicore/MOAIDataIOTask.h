// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#ifndef MOAIDATAIOTASK_H
#define MOAIDATAIOTASK_H

#include <moaicore/MOAITask.h>
#include <moaicore/MOAIData.h>

//================================================================//
// MOAIDataIOTask
//================================================================//
class MOAIDataIOTask : 
	public MOAITask < MOAIDataIOTask > {
private:

	enum {
		IDLE,
		LOADING,
		SAVING,
	};

	STLString		mFilename;
	MOAIData*		mData;
	u32				mState;

	//----------------------------------------------------------------//
	void		Execute				();

public:

	SET	( cc8*, Filename, mFilename )
	GET_SET ( MOAIData*, Data, mData )

	//----------------------------------------------------------------//
	void		LoadData			( cc8* filename, MOAIData& target );
	void		SaveData			( cc8* filename, MOAIData& target );
				MOAIDataIOTask		();
				~MOAIDataIOTask		();
};

#endif