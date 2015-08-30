#include "../common/platform.h"
#include "dance_ui.h"

#include "../common/file_system.h"
#include "../display/background.h"
#include "../display/context_menu.h"
#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/outline.h"
#include "../display/root_canvas.h"
#include "../display/scrollbar.h"

namespace dance {

vector<string> geometryNames;

class FormationEditCommand : public display::Undo {
public:
	FormationEditCommand(GrammarEditor* editor, Formation* formation) {
		_editor = editor;
		_originalModified = formation->modified();
		_modified = time(null);
		_formation = formation;
	}

	virtual bool applyEdit(FormationEditor* fe) = 0;

	virtual bool revertEdit(FormationEditor* fe) = 0;

	virtual void discardEdit() = 0;

	Formation* formation() const { return _formation; }

private:
	virtual void apply() {
		FormationEditor* fe = _editor->frame()->edit(_formation);
		if (applyEdit(fe))
			_editor->touchFormation(_formation);
		_originalModified = _formation->modified();
		_formation->setModified(_modified);
		fe->setModified(_modified);
		_formation->grammar()->touch();
	}

	virtual void revert() {
		FormationEditor* fe = _editor->frame()->edit(_formation);
		if (revertEdit(fe))
			_editor->touchFormation(_formation);
		_formation->setModified(_originalModified);
		fe->setModified(_originalModified);
		_formation->grammar()->touch();
	}

	virtual void discard() {
		discardEdit();
	}

	GrammarEditor*		_editor;
	Formation*			_formation;
	time_t				_modified;
	time_t				_originalModified;
};

class FormationNameChangeCommand : public FormationEditCommand {
public:
	FormationNameChangeCommand(GrammarEditor* editor, Formation* formation, const string& name) : FormationEditCommand(editor, formation) {
		_original = formation->name();
		_newName = name;
	}

	virtual bool applyEdit(FormationEditor* fe) {
		formation()->setName(_newName);
		fe->touchName();
		return true;
	}

	virtual bool revertEdit(FormationEditor* fe) {
		formation()->setName(_original);
		fe->touchName();
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	string				_original;
	string				_newName;
};

class GeometryChangeCommand : public FormationEditCommand {
public:
	GeometryChangeCommand(GrammarEditor* editor, Formation* formation, Geometry value) : FormationEditCommand(editor, formation) {
		_original = formation->geometry();
		_newGeometry = value;
	}

	virtual bool applyEdit(FormationEditor* fe) {
		formation()->setGeometry(_newGeometry);
		fe->touchGeometry();
		return false;
	}

	virtual bool revertEdit(FormationEditor* fe) {
		formation()->setGeometry(_original);
		fe->touchGeometry();
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Geometry			_original;
	Geometry			_newGeometry;
};

class AddDancerCommand : public FormationEditCommand {
public:
	AddDancerCommand(GrammarEditor* editor, Formation* formation, int x, int y) : FormationEditCommand(editor, formation) {
		_x = x;
		_y = y;
	}

	virtual bool applyEdit(FormationEditor* fe) {
		Spot s(ACTIVE, ANY_FACING);
		formation()->setSpot(_x, _y, s);
		fe->touchDiagram();
		return false;
	}

	virtual bool revertEdit(FormationEditor* fe) {
		formation()->setSpot(_x, _y, Spot::empty);
		fe->touchDiagram();
		return false;
	}

	virtual void discardEdit() {
	}

private:
	int					_x;
	int					_y;
};

class ChangeDancerCommand : public FormationEditCommand {
public:
	ChangeDancerCommand(GrammarEditor* editor, Formation* formation, int x, int y, const Spot& spot) : FormationEditCommand(editor, formation) {
		_x = x;
		_y = y;
		_original = formation->spot(x, y);
		_spot = spot;
	}

	virtual bool applyEdit(FormationEditor* fe) {
		formation()->setSpot(_x, _y, _spot);
		fe->touchDiagram();
		return false;
	}

	virtual bool revertEdit(FormationEditor* fe) {
		formation()->setSpot(_x, _y, _original);
		fe->touchDiagram();
		return false;
	}

	virtual void discardEdit() {
	}

private:
	int					_x;
	int					_y;
	Spot				_spot;
	Spot				_original;
};

const char* FormationEditor::tabLabel() {
	if (_formation->name().size() > 0)
		return _formation->name().c_str();
	else
		return "<unnamed>";
}

display::Canvas* FormationEditor::createView(bool editable) {
	name()->set_value(_formation->name());
	display::Grid* infoArea = new display::Grid();
		infoArea->cell(new display::Label("Name:"));
		infoArea->cell(new display::Spacer(2, new display::Bevel(2, true, name())));
		if (editable) {
			name()->setBackground(&display::editableBackground);
			field(name());
			name()->valueCommitted.addHandler(this, &FormationEditor::onNameCommitted, name());
		}
		infoArea->row();
		infoArea->cell(new display::Label("Created:"));
		string date;
		if (_formation->created())
			date.localTime(_formation->created(), "%m/%d/%Y %H:%M:%S");
		infoArea->cell(new display::Label(date));
		infoArea->row();
		infoArea->cell(new display::Label("Modified:"));
		setModified(_formation->modified());
		infoArea->cell(modifiedLabel());
		infoArea->row();
		infoArea->cell(new display::Label("Geometry:"));
		if (geometryNames.size() == 0) {
			geometryNames.push_back("");
			geometryNames.push_back("grid");
			geometryNames.push_back("hexagonal");
			geometryNames.push_back("ring");
		}
		_geometryName = new display::DropDown(_formation->geometry(), geometryNames);
		display::Canvas* spacer = new display::Spacer(2, _geometryName);
		infoArea->cell(new display::Filler(new display::Border(1, spacer)));
		if (editable) {
			spacer->setBackground(&display::editableBackground);
			field(_geometryName);
			_geometryName->valueChanged.addHandler(this, &FormationEditor::onGeometryChanged);
		}
	infoArea->complete();
	display::Grid* g = new display::Grid();
		g->cell(infoArea);
		g->row();
		_diagram = new Picture(_formation, 0);
		if (editable) {
			_diagram->mouseMove.addHandler(this, &FormationEditor::onMouseMove);
			_diagram->click.addHandler(this, &FormationEditor::onClick);
			_diagram->openContextMenu.addHandler(this, &FormationEditor::onOpenContextMenu);
		}
		g->cell(_diagram);
	g->complete();
	return g;
}

void FormationEditor::touchName() {
	name()->set_value(_formation->name());
	tabModified();
}

void FormationEditor::touchGeometry() {
	_geometryName->set_value(_formation->geometry());
}

void FormationEditor::touchDiagram() {
	_diagram->invalidate();
}

void FormationEditor::onNameCommitted(display::Label* label) {
	string old = _formation->name();
	if (old != label->value())
		parent()->undoStack().addUndo(new FormationNameChangeCommand(parent(), _formation, label->value()));
}

void FormationEditor::onGeometryChanged() {
	if (_formation->geometry() != _geometryName->value())
		parent()->undoStack().addUndo(new GeometryChangeCommand(parent(), _formation, (Geometry)_geometryName->value()));
}

void FormationEditor::onMouseMove(display::point p, display::Canvas* target) {
	PictureHit hit;

	if (_diagram->hitTest(p, &hit)) {
		const Spot& spot = _formation->spot(hit.x, hit.y);

		switch (spot.position) {
		case	EMPTY:
			if (_formation->blocked(hit.x, hit.y))
				_diagram->mouseCursor = display::standardCursor(display::NO);
			else
				_diagram->mouseCursor = null;
			break;

		case	WRAP:
			_diagram->mouseCursor = display::standardCursor(display::HAND);
			break;

		default:
			switch (hit.zone) {
			case	FZ_CENTER:
				_diagram->mouseCursor = display::standardCursor(display::HAND);
				break;

			case	FZ_LEFT:
				switch (spot.facing) {
				case	LEFT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZEALL);
					break;

				case	RIGHT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZEWE);
					break;

				case	HEAD_FACING:
				case	BACK_FACING:
				case	FRONT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::NO);
					break;

				case	SIDE_FACING:
					_diagram->mouseCursor = display::standardCursor(display::IBEAM);
					break;

				case	ANY_FACING:
					_diagram->mouseCursor = display::standardCursor(display::CROSS);
					break;

				default:
					_diagram->mouseCursor = null;
				}
				break;

			case	FZ_RIGHT:
				switch (spot.facing) {
				case	LEFT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZEWE);
					break;

				case	RIGHT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZEALL);
					break;

				case	HEAD_FACING:
				case	BACK_FACING:
				case	FRONT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::NO);
					break;

				case	SIDE_FACING:
					_diagram->mouseCursor = display::standardCursor(display::IBEAM);
					break;

				case	ANY_FACING:
					_diagram->mouseCursor = display::standardCursor(display::CROSS);
					break;

				default:
					_diagram->mouseCursor = null;
				}
				break;

			case	FZ_BACK:
				switch (spot.facing) {
				case	BACK_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZEALL);
					break;

				case	FRONT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZENS);
					break;

				case	SIDE_FACING:
				case	LEFT_FACING:
				case	RIGHT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::NO);
					break;

				case	HEAD_FACING:
					_diagram->mouseCursor = display::standardCursor(display::IBEAM);
					break;

				case	ANY_FACING:
					_diagram->mouseCursor = display::standardCursor(display::CROSS);
					break;

				default:
					_diagram->mouseCursor = null;
				}
				break;

			case	FZ_FRONT:
				switch (spot.facing) {
				case	BACK_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZENS);
					break;

				case	FRONT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::SIZEALL);
					break;

				case	SIDE_FACING:
				case	LEFT_FACING:
				case	RIGHT_FACING:
					_diagram->mouseCursor = display::standardCursor(display::NO);
					break;

				case	HEAD_FACING:
					_diagram->mouseCursor = display::standardCursor(display::IBEAM);
					break;

				case	ANY_FACING:
					_diagram->mouseCursor = display::standardCursor(display::CROSS);
					break;

				default:
					_diagram->mouseCursor = null;
				}
				break;

			default:
				_diagram->mouseCursor = null;
			}
			break;
		}
	}
}

void FormationEditor::onClick(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	PictureHit hit;

	if (_diagram->hitTest(p, &hit)) {
		const Spot& spot = _formation->spot(hit.x, hit.y);
		Spot s = spot;

		switch (spot.position) {
		case	EMPTY:
			if (_formation->blocked(hit.x, hit.y)) {
				parent()->frame()->setStatus("Position is too close to another dancer");
				return;
			} else
				parent()->undoStack().addUndo(new AddDancerCommand(parent(), _formation, hit.x, hit.y));
			break;

		case	WRAP:
			// delete WRAP mark
			break;

		default:
			// Determine zone and act appropriately
			switch (hit.zone) {
			case	FZ_CENTER:
//				_diagram->mouseCursor = display::standardCursor(display::HAND);
				break;

			case	FZ_LEFT:
				switch (spot.facing) {
				case	LEFT_FACING:
					s.facing = ANY_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	RIGHT_FACING:
					s.facing = SIDE_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	SIDE_FACING:
					s.facing = RIGHT_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	ANY_FACING:
					s.facing = LEFT_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				}
				break;

			case	FZ_RIGHT:
				switch (spot.facing) {
				case	LEFT_FACING:
					s.facing = SIDE_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	RIGHT_FACING:
					s.facing = ANY_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	SIDE_FACING:
					s.facing = LEFT_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	ANY_FACING:
					s.facing = RIGHT_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				}
				break;

			case	FZ_BACK:
				switch (spot.facing) {
				case	BACK_FACING:
					s.facing = ANY_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	FRONT_FACING:
					s.facing = HEAD_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	HEAD_FACING:
					s.facing = FRONT_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	ANY_FACING:
					s.facing = BACK_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;
				}
				break;

			case	FZ_FRONT:
				switch (spot.facing) {
				case	BACK_FACING:
					s.facing = HEAD_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	FRONT_FACING:
					s.facing = ANY_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	HEAD_FACING:
					s.facing = BACK_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;

				case	ANY_FACING:
					s.facing = FRONT_FACING;
					parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));
					break;
				}
				break;
			}
			break;
		}
	}
}

void FormationEditor::onOpenContextMenu(display::point p, display::Canvas* target) {
	static const char* positionChoices[] = {
		null,											//	NO_POSITION,			// not found in any pattern
		"Make this spot an active dancer",				//	ACTIVE,					// a
		"Make this spot an active boy dancer",			//	ACTIVE_BOY,				// b
		"Make this spot an active girl dancer",			//	ACTIVE_GIRL,			// g
		"Make this spot a designated dancer",			//	ACTIVE_DESIGNATED,		// d
		"Make this spot a non-designated dancer",		//	ACTIVE_NONDESIGNATED,	// n
		"Make this spot an active center dancer",		//	CENTER,					// c
		"Make this spot an active end dancer",			//	END,					// e
		"Make this spot an active very center dancer",	//	VERY_CENTER,			// C
		"Make this spot an active very end dancer",		//	VERY_END,				// E

		"Make this spot an inactive dancer",			//	INACTIVE,				// i

		// Syntactic tokens

		null,											//	END_OF_ROW,				// end of string

		// Not dancers, but location and/or relationship markers

		"Make this spot empty",							//	EMPTY,					// .
		null,											//	SAME_ROW,				// -
		null,											//	SAME_COLUMN,			// |
		null,											//	TO_THE_LEFT,			// <
		null,											//	TO_THE_BACK,			// ^
		"Make this spot a wrap-around marker",			//	WRAP,					// wraps to beginning of ring
	};
	PictureHit hit;

	if (_diagram->hitTest(p, &hit)) {
		const Spot& spot = _formation->spot(hit.x, hit.y);
		display::ContextMenu* c = new display::ContextMenu(root(), p, target);
			for (int i = FIRST_POSITION; i <= LAST_POSITION; i++) {
				if (positionChoices[i] == null)
					continue;
				if (i == WRAP && _formation->geometry() != RING)
					continue;
				if (spot.position != i)
					c->choice(positionChoices[i])->click.addHandler(this, &FormationEditor::setPosition, i);
			}
		c->show();
	}
}

void FormationEditor::setPosition(display::point p, display::Canvas* target, int newValue) {
	PictureHit hit;

	if (_diagram->hitTest(p, &hit)) {
		const Spot& spot = _formation->spot(hit.x, hit.y);
		Spot s;
		s.position = (PositionType)newValue;
		if (!isDancer(s.position))
			s.facing = ANY_FACING;
		else
			s.facing = spot.facing;
		parent()->undoStack().addUndo(new ChangeDancerCommand(parent(), _formation, hit.x, hit.y, s));

	}
}

}  // namespace dance
