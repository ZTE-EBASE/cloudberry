//---------------------------------------------------------------------------
//	Greenplum Database
//	Copyright (C) 2009 Greenplum, Inc.
//
//	@filename:
//		CColRefSet.cpp
//
//	@doc:
//		Implementation of column reference set based on bit sets
//---------------------------------------------------------------------------

#include "gpopt/base/CColRefSet.h"

#include "gpos/base.h"

#include "gpopt/base/CAutoOptCtxt.h"
#include "gpopt/base/CColRefSetIter.h"
#include "gpopt/base/CColumnFactory.h"

using namespace gpopt;

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::CColRefSet
//
//	@doc:
//		ctor
//
//---------------------------------------------------------------------------
CColRefSet::CColRefSet(CMemoryPool *mp, ULONG ulSizeBits)
	: CBitSet(mp, ulSizeBits)
{
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::CColRefSet
//
//	@doc:
//		copy ctor;
//
//---------------------------------------------------------------------------
CColRefSet::CColRefSet(CMemoryPool *mp, const CColRefSet &bs) : CBitSet(mp, bs)
{
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::CColRefSet
//
//	@doc:
//		ctor, copy from col refs array
//
//---------------------------------------------------------------------------
CColRefSet::CColRefSet(CMemoryPool *mp, const CColRefArray *colref_array,
					   ULONG size)
	: CBitSet(mp, size)
{
	Include(colref_array);
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::~CColRefSet
//
//	@doc:
//		dtor
//
//---------------------------------------------------------------------------
CColRefSet::~CColRefSet() = default;

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::FMember
//
//	@doc:
//		Check if given column ref is in the set
//
//---------------------------------------------------------------------------
BOOL
CColRefSet::FMember(const CColRef *colref) const
{
	return CBitSet::Get(colref->Id());
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::PcrAny
//
//	@doc:
//		Return random member
//
//---------------------------------------------------------------------------
CColRef *
CColRefSet::PcrAny() const
{
	// for now return the first column
	return PcrFirst();
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::PcrFirst
//
//	@doc:
//		Return first member
//
//---------------------------------------------------------------------------
CColRef *
CColRefSet::PcrFirst() const
{
	CColRefSetIter crsi(*this);
	if (crsi.Advance())
	{
		return crsi.Pcr();
	}

	GPOS_ASSERT(0 == Size());
	return nullptr;
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Include
//
//	@doc:
//		Include a constant column ref in set
//
//---------------------------------------------------------------------------
void
CColRefSet::Include(const CColRef *colref)
{
	CBitSet::ExchangeSet(colref->Id());
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Include
//
//	@doc:
//		Include column refs from an array
//
//---------------------------------------------------------------------------
void
CColRefSet::Include(const CColRefArray *colref_array)
{
	ULONG length = colref_array->Size();
	for (ULONG i = 0; i < length; i++)
	{
		Include((*colref_array)[i]);
	}
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Include
//
//	@doc:
//		Include a set of columns in bitset
//
//---------------------------------------------------------------------------
void
CColRefSet::Include(const CColRefSet *pcrs)
{
	CColRefSetIter crsi(*pcrs);
	while (crsi.Advance())
	{
		Include(crsi.Pcr());
	}
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Exclude
//
//	@doc:
//		Remove column from bitset
//
//---------------------------------------------------------------------------
void
CColRefSet::Exclude(const CColRef *colref)
{
	CBitSet::ExchangeClear(colref->Id());
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Exclude
//
//	@doc:
//		Remove a set of columns from bitset
//
//---------------------------------------------------------------------------
void
CColRefSet::Exclude(const CColRefSet *pcrs)
{
	CColRefSetIter crsi(*pcrs);
	while (crsi.Advance())
	{
		Exclude(crsi.Pcr());
	}
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Exclude
//
//	@doc:
//		Remove an array of columns from bitset
//
//---------------------------------------------------------------------------
void
CColRefSet::Exclude(const CColRefArray *colref_array)
{
	for (ULONG i = 0; i < colref_array->Size(); i++)
	{
		Exclude((*colref_array)[i]);
	}
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Replace
//
//	@doc:
//		Replace column with another column in bitset
//
//---------------------------------------------------------------------------
void
CColRefSet::Replace(const CColRef *pcrOut, const CColRef *pcrIn)
{
	if (FMember(pcrOut))
	{
		Exclude(pcrOut);
		Include(pcrIn);
	}
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Replace
//
//	@doc:
//		Replace an array of columns with another array of columns
//
//---------------------------------------------------------------------------
void
CColRefSet::Replace(const CColRefArray *pdrgpcrOut,
					const CColRefArray *pdrgpcrIn)
{
	const ULONG length = pdrgpcrOut->Size();
	GPOS_ASSERT(length == pdrgpcrIn->Size());

	for (ULONG ul = 0; ul < length; ul++)
	{
		Replace((*pdrgpcrOut)[ul], (*pdrgpcrIn)[ul]);
	}
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Pdrgpcr
//
//	@doc:
//		Convert set into array
//
//---------------------------------------------------------------------------
CColRefArray *
CColRefSet::Pdrgpcr(CMemoryPool *mp) const
{
	CColRefArray *colref_array = GPOS_NEW(mp) CColRefArray(mp);

	CColRefSetIter crsi(*this);
	while (crsi.Advance())
	{
		colref_array->Append(crsi.Pcr());
	}

	return colref_array;
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::Phmicr
//
//	@doc:
//		Convert set into map
//
//---------------------------------------------------------------------------
IntToColRefMap *
CColRefSet::Phmicr(CMemoryPool *mp) const
{
	IntToColRefMap *phmicr = GPOS_NEW(mp) IntToColRefMap(mp);

	CColRefSetIter crsi(*this);
	while (crsi.Advance())
	{
		phmicr->Insert(GPOS_NEW(mp) INT(crsi.Pcr()->Id()), crsi.Pcr());
	}

	return phmicr;
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::HashValue
//
//	@doc:
//		Compute hash value by combining hashes of components
//
//---------------------------------------------------------------------------
ULONG
CColRefSet::HashValue()
{
	ULONG size = this->Size();
	ULONG ulHash = gpos::HashValue<ULONG>(&size);

	// limit the number of columns used in hash computation
	ULONG length = std::min(size, (ULONG) 8);

	CColRefSetIter crsi(*this);
	for (ULONG i = 0; i < length; i++)
	{
		(void) crsi.Advance();
		ulHash =
			gpos::CombineHashes(ulHash, gpos::HashPtr<CColRef>(crsi.Pcr()));
	}

	return ulHash;
}


//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::OsPrint
//
//	@doc:
//		Helper function to print a colref set
//
//---------------------------------------------------------------------------
IOstream &
CColRefSet::OsPrint(IOstream &os) const
{
	return OsPrint(os, gpos::ulong_max);
}

IOstream &
CColRefSet::OsPrint(IOstream &os, ULONG ulLenMax) const
{
	ULONG length = Size();
	ULONG ul = 0;

	CColRefSetIter crsi(*this);
	while (crsi.Advance() && ul < std::min(length, ulLenMax))
	{
		CColRef *colref = crsi.Pcr();
		colref->OsPrint(os);
		if (ul < length - 1)
		{
			os << ", ";
		}
		ul++;
	}

	if (ulLenMax < length)
	{
		os << "...";
	}

	return os;
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::ExtractColIds
//
//	@doc:
//		Extract array of column ids from colrefset
//
//---------------------------------------------------------------------------
void
CColRefSet::ExtractColIds(CMemoryPool *mp, ULongPtrArray *colids) const
{
	CColRefSetIter crsi(*this);
	while (crsi.Advance())
	{
		CColRef *colref = crsi.Pcr();
		ULONG colid = colref->Id();
		colids->Append(GPOS_NEW(mp) ULONG(colid));
	}
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::ExtractColIds
//
//	@doc:
//		extract the index(relative position) in current CColRefSet
//		notice that: Whenever the current CColRefSet changes, the index will also change.
//
//---------------------------------------------------------------------------
void
CColRefSet::ExtractIndex(const CColRef *querycr, ULONG *colidx) const
{
	GPOS_ASSERT(nullptr != querycr);
	GPOS_ASSERT(FMember(querycr));
	*colidx = 0;
	CColRefSetIter crsi(*this);
	while (crsi.Advance())
	{
		CColRef *colref = crsi.Pcr();
		if (colref == querycr) {
			return;
		}
		*colidx += 1;
	}
	*colidx = gpos::ulong_max;
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::FContained
//
//	@doc:
//		Check if the current colrefset is a subset of any of the colrefsets in
//		the given array
//
//---------------------------------------------------------------------------
BOOL
CColRefSet::FContained(const CColRefSetArray *pdrgpcrs)
{
	GPOS_ASSERT(nullptr != pdrgpcrs);

	const ULONG length = pdrgpcrs->Size();
	for (ULONG ul = 0; ul < length; ul++)
	{
		if ((*pdrgpcrs)[ul]->ContainsAll(this))
		{
			return true;
		}
	}

	return false;
}

BOOL
CColRefSet::FIntersects(const CColRefSet *pcrs)
{
	GPOS_ASSERT(nullptr != pcrs);
	CColRefSet *intersecting_colrefset = GPOS_NEW(m_mp) CColRefSet(m_mp, *this);
	intersecting_colrefset->Intersection(pcrs);

	BOOL intersects = intersecting_colrefset->Size() > 0;
	intersecting_colrefset->Release();

	return intersects;
}

//---------------------------------------------------------------------------
//	@function:
//		CColRefSet::FCovered
//
//	@doc:
//		Are the columns in the column reference set covered by the array of
//		column ref sets
//---------------------------------------------------------------------------
BOOL
CColRefSet::FCovered(CColRefSetArray *pdrgpcrs, CColRefSet *pcrs)
{
	GPOS_ASSERT(nullptr != pdrgpcrs);
	GPOS_ASSERT(nullptr != pcrs);
	GPOS_ASSERT(0 < pdrgpcrs->Size());

	if (0 == pcrs->Size())
	{
		return false;
	}

	CColRefSetIter crsi(*pcrs);
	while (crsi.Advance())
	{
		CColRef *colref = crsi.Pcr();
		BOOL fFound = false;
		const ULONG length = pdrgpcrs->Size();
		for (ULONG ul = 0; ul < length && !fFound; ul++)
		{
			CColRefSet *pcrs = (*pdrgpcrs)[ul];
			if (pcrs->FMember(colref))
			{
				fFound = true;
			}
		}

		if (!fFound)
		{
			return false;
		}
	}

	return true;
}


// EOF
