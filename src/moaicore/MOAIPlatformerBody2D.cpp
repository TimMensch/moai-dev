// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#include "pch.h"
#include <moaicore/MOAIDraw.h>
#include <moaicore/MOAIGfxDevice.h>
#include <moaicore/MOAIPartition.h>
#include <moaicore/MOAIPartitionResultMgr.h>
#include <moaicore/MOAIPlatformerBody2D.h>
#include <moaicore/MOAIPlatformerFsm2D.h>
#include <moaicore/MOAISurfaceSampler2D.h>

//================================================================//
// local
//================================================================//

//----------------------------------------------------------------//
// TODO: doxygen
int MOAIPlatformerBody2D::_getStatus ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAIPlatformerBody2D, "U" );

	state.Push ( self->mSteps );
	state.Push ( self->mCompleted );
	return 2;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAIPlatformerBody2D::_setCeilingAngle ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAIPlatformerBody2D, "U" );

	float angle = state.GetValue < float >( 2, 0.0f );
	self->SetCeilingAngle ( angle );
	self->ScheduleUpdate ();
	return 0;
}
//----------------------------------------------------------------//
// TODO: doxygen
int MOAIPlatformerBody2D::_setEllipse ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAIPlatformerBody2D, "U" );

	self->mHRad = state.GetValue < float >( 2, 32.0f );
	self->mVRad = state.GetValue < float >( 3, self->mHRad );
	self->ScheduleUpdate ();
	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAIPlatformerBody2D::_setFloorAngle ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAIPlatformerBody2D, "U" );

	float angle = state.GetValue < float >( 2, 0.0f );
	self->SetFloorAngle ( angle );
	self->ScheduleUpdate ();
	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAIPlatformerBody2D::_setMove ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAIPlatformerBody2D, "U" );
	
	self->mMove.mX = state.GetValue < float >( 2, self->mMove.mX );
	self->mMove.mY = state.GetValue < float >( 3, self->mMove.mY );
	
	if (( self->mMove.mX != 0.0f ) || ( self->mMove.mY != 0.0f )) {
		self->ScheduleUpdate ();
	}
	return 0;
}

//================================================================//
// MOAIPlatformerBody2D
//================================================================//

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::Draw ( int subPrimID ) {
	UNUSED ( subPrimID );

	MOAIGfxDevice& gfxDevice = MOAIGfxDevice::Get ();
	
	MOAIDraw& draw = MOAIDraw::Get ();
	UNUSED ( draw ); // mystery warning in vs2008
	
	draw.Bind ();
	
	gfxDevice.SetVertexTransform ( MOAIGfxDevice::VTX_WORLD_TRANSFORM, this->GetUnitToWorldMtx ());
	gfxDevice.SetVertexMtxMode ( MOAIGfxDevice::VTX_STAGE_MODEL, MOAIGfxDevice::VTX_STAGE_PROJ );
	
	gfxDevice.SetPenColor ( 0x7f7f7f7f );
	gfxDevice.SetPenWidth ( 1.0f );
	USAffine3D worldToUnit = this->GetWorldToUnitMtx ();
	USRect debugBoundsRect = this->mDebugBounds.GetRect ( USBox::PLANE_XY );
	worldToUnit.Transform ( debugBoundsRect );
	draw.DrawRectOutline ( debugBoundsRect );
	
	gfxDevice.SetPenColor ( 0x7f7f7f7f );
	gfxDevice.SetPenWidth ( 2.0f );
	draw.DrawEllipseOutline ( 0.0f, 0.0f, 1.0f, 1.0f, 32 );
	draw.DrawLine ( 0.0f, 0.0f, 0.0f, -( 1.0f + ( this->mSkirt / this->mVRad )));
	
	gfxDevice.SetPenColor ( 0xffffffff );
	gfxDevice.SetPenWidth ( 2.0f );
	draw.DrawEllipseArcOutline ( 0.0f, 0.0f, 1.0f, 1.0f, this->mCeilAngle, 180.0f - this->mFloorAngle, 8 );
	draw.DrawEllipseArcOutline ( 0.0f, 0.0f, 1.0f, 1.0f, 180.0f + this->mFloorAngle, 360.0f - this->mCeilAngle, 8 );
	
	MOAISurfaceBuffer2D buffer;
	this->GatherSurfacesForBounds ( buffer, this->mDebugBounds );
	u32 top = buffer.GetTop ();
	if ( top ) {
	
		gfxDevice.SetPenColor ( 0xff0000ff );
		gfxDevice.SetPenWidth ( 2.0f );
		
		for ( u32 i = 0; i < top; ++i ) {
			const MOAISurface2D& surface = buffer.GetSurface ( i );
			draw.DrawLine ( surface.mV0, surface.mV1 );
		}
	}
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::GatherSurfacesForBounds ( MOAISurfaceBuffer2D& buffer, const USBox& bounds ) {

	MOAIPartition* partition = this->GetPartition ();
	if ( !partition ) return;
	
	MOAIPartitionResultBuffer& props = MOAIPartitionResultMgr::Get ().GetBuffer ();
	u32 totalResults = partition->GatherProps ( props, 0, bounds, 0 );
	
	if ( totalResults ) {

		MOAISurfaceSampler2D sampler;
		sampler.Init ( buffer, this->GetUnitRectForWorldBounds ( bounds ), this->GetUnitToWorldMtx (), this->GetWorldToUnitMtx ());
		
		for ( u32 i = 0; i < totalResults; ++i ) {
			MOAIPartitionResult* result = props.GetResultUnsafe ( i );
			MOAIProp* prop = result->mProp;
			prop->SampleSurfaces ( sampler );
		}
	}
}

//----------------------------------------------------------------//
//u32 MOAIPlatformerBody2D::GetLocalFrame ( USRect& frame ) {
//	
//	frame.mXMin = -this->mHRad;
//	frame.mYMin = -( this->mVRad + this->mSkirt );
//	frame.mXMax = this->mHRad;
//	frame.mYMax = this->mVRad;
//	
//	return FRAME_OK;
//}

//----------------------------------------------------------------//
u32 MOAIPlatformerBody2D::GetPropBounds ( USBox& bounds ) {

	USRect rect;

	rect.mXMin = -this->mHRad;
	rect.mYMin = -( this->mVRad + this->mSkirt );
	rect.mXMax = this->mHRad;
	rect.mYMax = this->mVRad;
	
	bounds.Init ( rect.mXMin, rect.mYMax, rect.mXMax, rect.mYMin, 0.0f, 0.0f );
	return BOUNDS_OK;
}

//----------------------------------------------------------------//
USRect MOAIPlatformerBody2D::GetUnitRectForWorldBounds ( const USBox& bounds ) {

	USVec3D loc = this->GetWorldLoc ();
	USRect rect = bounds.GetRect ( USBox::PLANE_XY );
	rect.Offset ( -loc.mX, -loc.mY );
	rect.Scale ( 1.0f / this->mHRad, 1.0f / this->mVRad );
	return rect;
}

//----------------------------------------------------------------//
USAffine3D MOAIPlatformerBody2D::GetUnitToWorldMtx () {
	
	USAffine3D transform;
	
	transform.Scale (
		this->mHRad,
		this->mVRad,
		1.0f
	);
	
	transform.Append ( this->GetLocalToWorldMtx ());
	return transform;
}

//----------------------------------------------------------------//
USAffine3D MOAIPlatformerBody2D::GetWorldToUnitMtx () {
	
	USAffine3D transform;
	
	transform.Scale (
		1.0f / this->mHRad,
		1.0f / this->mVRad,
		1.0f
	);
	
	transform.Prepend ( this->GetWorldToLocalMtx ());
	return transform;
}

//----------------------------------------------------------------//
MOAIPlatformerBody2D::MOAIPlatformerBody2D () :
	mMove ( 0.0f, 0.0f ),
	mHRad ( 32.0f ),
	mVRad ( 32.0f ),
	mSkirt ( 0.0f ),
	mSteps ( 0 ),
	mCompleted ( true ),
	mDetachMode ( DETACH_ON_UP ),
	mIsStanding ( false ) {
	
	RTTI_BEGIN
		RTTI_EXTEND ( MOAIProp )
	RTTI_END
	
	//this->SetQueryMask ( MOAIContentLibrary2D::CAN_DRAW_DEBUG );
	
	this->SetFloorAngle ( 60.0f );
	this->SetCeilingAngle ( 120.0f );
}

//----------------------------------------------------------------//
MOAIPlatformerBody2D::~MOAIPlatformerBody2D () {
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::OnDepNodeUpdate () {
	
	MOAIPlatformerFsm2D fsm;
	this->BuildTransforms (); // not sure about this here
	fsm.Move ( *this );
	MOAIProp::OnDepNodeUpdate ();
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::RegisterLuaClass ( MOAILuaState& state ) {
	
	MOAIProp::RegisterLuaClass ( state );
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::RegisterLuaFuncs ( MOAILuaState& state ) {
	
	MOAIProp::RegisterLuaFuncs ( state );
	
	luaL_Reg regTable [] = {
		{ "getStatus",				_getStatus },
		{ "setCeilingAngle",		_setCeilingAngle },
		{ "setEllipse",				_setEllipse },
		{ "setFloorAngle",			_setFloorAngle },
		{ "setMove",				_setMove },
		{ NULL, NULL }
	};
	
	luaL_register ( state, 0, regTable );
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::SetCeilingAngle ( float angle ) {

	this->mCeilAngle = angle;
	this->mCeilCos = -Cos ( angle * ( float )D2R );
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::SetFloorAngle ( float angle ) {

	this->mFloorAngle = angle;
	this->mFloorCos = Cos ( angle * ( float )D2R );
}

//----------------------------------------------------------------//
void MOAIPlatformerBody2D::SetMove ( float x, float y ) {

	this->mMove.Init ( x, y );
	this->ScheduleUpdate ();
}


