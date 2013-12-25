/*
 * Copyright 2007-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NATIVE_SAVER_H
#define NATIVE_SAVER_H


#include "AttributeSaver.h"
#include "SimpleFileSaver.h"


class NativeSaver : public SimpleFileSaver {
public:
								NativeSaver(const entry_ref& ref);
	virtual						~NativeSaver();

	virtual	status_t			Save(const DocumentRef& document);

protected:
			AttributeSaver		fAttrSaver;
};


#endif // NATIVE_SAVER_H
