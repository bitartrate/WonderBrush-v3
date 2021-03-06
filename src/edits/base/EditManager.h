/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef EDIT_MANAGER_H
#define EDIT_MANAGER_H

#include "EditStack.h"
#include "Notifier.h"
#include "UndoableEdit.h"

class BString;
class EditContext;
class RWLocker;

class EditManager : public Notifier {
public:
								EditManager(RWLocker* locker);
	virtual						~EditManager();

			status_t			Perform(UndoableEdit* edit,
									EditContext& context);
			status_t			Perform(const UndoableEditRef& edit,
									EditContext& context);

			status_t			Undo(EditContext& context);
			status_t			Redo(EditContext& context);

			bool				GetUndoName(BString& name);
			bool				GetRedoName(BString& name);

			void				Clear();
			void				Save();
			bool				IsSaved();

private:
			status_t			_AddEdit(const UndoableEditRef& edit);

private:
			RWLocker*			fLocker;

			EditStack			fUndoHistory;
			EditStack			fRedoHistory;
			UndoableEditRef		fEditAtSave;
};

#endif // EDIT_MANAGER_H
