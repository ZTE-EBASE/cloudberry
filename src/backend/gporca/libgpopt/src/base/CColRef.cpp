//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2008 Greenplum, Inc.
//
//	@filename:
//		CColRef.cpp
//
//	@doc:
//		Implementation of column reference class
//---------------------------------------------------------------------------

#include "gpopt/base/CColRef.h"

#include "gpos/base.h"

#ifdef GPOS_DEBUG
#include "gpos/error/CAutoTrace.h"

#include "gpopt/base/COptCtxt.h"
#endif	// GPOS_DEBUG

using namespace gpopt;

// invalid key
const ULONG CColRef::m_ulInvalid = gpos::ulong_max;

//---------------------------------------------------------------------------
//	@function:
//		CColRef::CColRef
//
//	@doc:
//		ctor
//		takes ownership of string; verify string is properly formatted
//
//---------------------------------------------------------------------------
CColRef::CColRef(const IMDType *pmdtype, const INT type_modifier, ULONG id,
				 const CName *pname)
	: m_pmdtype(pmdtype),
	  m_type_modifier(type_modifier),
	  m_pname(pname),
	  m_used(EUnknown),
	  m_mdid_table(nullptr),
	  m_id(id)
{
	GPOS_ASSERT(nullptr != pmdtype);
	GPOS_ASSERT(pmdtype->MDId()->IsValid());
	GPOS_ASSERT(nullptr != pname);
}


//---------------------------------------------------------------------------
//	@function:
//		CColRef::~CColRef
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CColRef::~CColRef()
{
	// we own the name
	GPOS_DELETE(m_pname);
}


//---------------------------------------------------------------------------
//	@function:
//		CColRef::HashValue
//
//	@doc:
//		static hash function
//
//---------------------------------------------------------------------------
ULONG
CColRef::HashValue(const ULONG &ulptr)
{
	return gpos::HashValue<ULONG>(&ulptr);
}

//---------------------------------------------------------------------------
//	@function:
//		CColRef::HashValue
//
//	@doc:
//		static hash function
//
//---------------------------------------------------------------------------
ULONG
CColRef::HashValue(const CColRef *colref)
{
	ULONG id = colref->Id();
	return gpos::HashValue<ULONG>(&id);
}


FORCE_GENERATE_DBGSTR(gpopt::CColRef);

//---------------------------------------------------------------------------
//	@function:
//		CColRef::OsPrint
//
//	@doc:
//		debug print
//
//---------------------------------------------------------------------------
IOstream &
CColRef::OsPrint(IOstream &os) const
{
	m_pname->OsPrint(os);
	os << " (" << Id() << "," << UsedStatusToString(m_used) << ")";

	return os;
}

const char* 
CColRef::UsedStatusToString(CColRef::EUsedStatus status) const {
	switch (status) {
		case EUsed: return "Used";
		case EUnused: return "Unused";
		case EUnknown: return "Unknown";
		case ESentinel: return "Invalid";
		default: 
			return "Invalid";
	}

	return "Invalid";
}

//---------------------------------------------------------------------------
//	@function:
//		CColRef::Pdrgpul
//
//	@doc:
//		Extract array of colids from array of colrefs
//
//---------------------------------------------------------------------------
ULongPtrArray *
CColRef::Pdrgpul(CMemoryPool *mp, CColRefArray *colref_array)
{
	ULongPtrArray *pdrgpul = GPOS_NEW(mp) ULongPtrArray(mp);
	const ULONG length = colref_array->Size();
	for (ULONG ul = 0; ul < length; ul++)
	{
		CColRef *colref = (*colref_array)[ul];
		pdrgpul->Append(GPOS_NEW(mp) ULONG(colref->Id()));
	}

	return pdrgpul;
}

//---------------------------------------------------------------------------
//	@function:
//		CColRef::Equals
//
//	@doc:
//		Are the two arrays of column references equivalent
//
//---------------------------------------------------------------------------
BOOL
CColRef::Equals(const CColRefArray *pdrgpcr1, const CColRefArray *pdrgpcr2)
{
	if (nullptr == pdrgpcr1 || nullptr == pdrgpcr2)
	{
		return (nullptr == pdrgpcr1 && nullptr == pdrgpcr2);
	}

	return pdrgpcr1->Equals(pdrgpcr2);
}

// check if the the array of column references are equal. Note that since we have unique
// copy of the column references, we can compare pointers.
BOOL
CColRef::Equals(const CColRef2dArray *pdrgdrgpcr1,
				const CColRef2dArray *pdrgdrgpcr2)
{
	ULONG ulLen1 = (pdrgdrgpcr1 == nullptr) ? 0 : pdrgdrgpcr1->Size();
	ULONG ulLen2 = (pdrgdrgpcr2 == nullptr) ? 0 : pdrgdrgpcr2->Size();

	if (ulLen1 != ulLen2)
	{
		return false;
	}

	for (ULONG ulLevel = 0; ulLevel < ulLen1; ulLevel++)
	{
		BOOL fEqual = (*pdrgdrgpcr1)[ulLevel]->Equals((*pdrgdrgpcr2)[ulLevel]);
		if (!fEqual)
		{
			return false;
		}
	}

	return true;
}
