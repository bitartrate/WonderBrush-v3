/*
 * Copyright 2009-2012, Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef LIST_H
#define LIST_H

#include <new>
#include <stdlib.h>

#include <SupportDefs.h>

template <typename ItemType, bool PlainOldData, uint32 BlockSize = 8>
class List {
	typedef List<ItemType, PlainOldData, BlockSize> SelfType;
public:
	List()
		:
		fItems(NULL),
		fCount(0),
		fAllocatedCount(0)
	{
	}

	List(const SelfType& other)
		:
		fItems(NULL),
		fCount(0),
		fAllocatedCount(0)
	{
		*this = other;
	}

	virtual ~List()
	{
		if (!PlainOldData) {
			// Make sure to call destructors of old objects.
			_Resize(0);
		}
		free(fItems);
	}

	SelfType& operator=(const SelfType& other)
	{
		if (this == &other)
			return *this;

		if (PlainOldData) {
			if (_Resize(other.fCount))
				memcpy(fItems, other.fItems, fCount * sizeof(ItemType));
		} else {
			// Make sure to call destructors of old objects.
			// NOTE: Another option would be to use
			// ItemType::operator=(const ItemType& other), but then
			// we would need to be carefull which objects are already
			// initialized. Also the ItemType requires to implement the
			// operator, while doing it this way requires only a copy
			// constructor.
			_Resize(0);
			for (uint32 i = 0; i < other.fCount; i++) {
				if (!Add(other.ItemAtFast(i)))
					break;
			}
		}
		return *this;
	}

	bool operator==(const SelfType& other) const
	{
		if (this == &other)
			return true;

		if (fCount != other.fCount)
			return false;
		if (fCount == 0)
			return true;

		if (PlainOldData) {
			return memcmp(fItems, other.fItems,
				fCount * sizeof(ItemType)) == 0;
		} else {
			for (uint32 i = 0; i < other.fCount; i++) {
				if (*ItemAtFast(i) != *other.ItemAtFast(i))
					return false;
			}
		}
		return true;
	}

	bool operator!=(const SelfType& other) const
	{
		return !(*this == other);
	}

	inline void Clear()
	{
		_Resize(0);
	}

	inline uint32 CountItems() const
	{
		return fCount;
	}

	inline bool Add(const ItemType& copyFrom)
	{
		if (_Resize(fCount + 1)) {
			ItemType* item = fItems + fCount - 1;
			// Initialize the new object from the original.
			if (!PlainOldData)
				new (item) ItemType(copyFrom);
			else
				*item = copyFrom;
			return true;
		}
		return false;
	}

	inline void Remove()
	{
		if (fCount > 0)
			_Resize(fCount - 1);
	}

	inline const ItemType& ItemAt(int32 index) const
	{
		if (index >= (int32)fCount)
			return fNullItem;
		return ItemAtFast(index);
	}

	inline const ItemType& ItemAtFast(int32 index) const
	{
		return *(fItems + index);
	}

	inline const ItemType& LastItem() const
	{
		return ItemAt((int32)fCount - 1);
	}

private:
	inline bool _Resize(uint32 count)
	{
		if (count > fAllocatedCount) {
			uint32 allocationCount = (count + BlockSize - 1)
				/ BlockSize * BlockSize;
			ItemType* items = reinterpret_cast<ItemType*>(
				realloc(fItems, allocationCount * sizeof(ItemType)));
			if (items == NULL)
				return false;
			fItems = items;

			fAllocatedCount = allocationCount;
		} else if (count < fCount) {
			if (!PlainOldData) {
				// Uninit old objects so that we can re-use them when
				// appending objects without the need to re-allocate.
				for (uint32 i = count; i < fCount; i++) {
					ItemType* object = fItems + i;
					object->~ItemType();
				}
			}
		}
		fCount = count;
		return true;
	}

	ItemType*		fItems;
	ItemType		fNullItem;
	uint32			fCount;
	uint32			fAllocatedCount;
};

#endif // LIST_H
