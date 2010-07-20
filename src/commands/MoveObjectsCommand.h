/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef MOVE_OBJECTS_COMMAND_H
#define MOVE_OBJECTS_COMMAND_H

#include "Command.h"
#include "Selection.h"

class Layer;
class Object;

class MoveObjectsCommand : public Command, public Selection::Controller {
public:
								MoveObjectsCommand(Object** objects,
									int32 objectCount, Layer* insertionLayer,
									int32 insertionIndex,
									Selection* selection);
	virtual						~MoveObjectsCommand();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform();
	virtual status_t			Undo();

	virtual void				GetName(BString& name);

private:
			bool				_ObjectIsDistantChildOf(const Object* object,
									const Layer* layer) const;

private:
			Object**			fObjects;
			int32				fObjectCount;
		
			struct PositionInfo {
				Layer*			parent;
				int32			index;
			};
			PositionInfo*		fOldPositions;
		
			Layer*				fInsertionLayer;
			int32				fInsertionIndex;
		
			Selection*			fSelection;
};

#endif // MOVE_OBJECTS_COMMAND_H
