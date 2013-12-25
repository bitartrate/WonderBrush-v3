/*
 * Copyright 2007-2013, Haiku. Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef FILTER_H
#define FILTER_H

#include "Object.h"

class Filter : public Object {
public:
								Filter();
								Filter(const Filter& other);
								Filter(float radius);
	virtual						~Filter();

	// BaseObject interface
	virtual	BaseObject*			Clone(ResourceResolver& resolver) const;
	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				IsRegularTransformable() const;

	virtual	void				ExtendDirtyArea(BRect& area) const;

	// Filter
			void				SetFilterRadius(float filterRadius);
	inline	float				FilterRadius() const
									{ return fFilterRadius; }

private:
 			float				fFilterRadius;
};

#endif // FILTER_H
