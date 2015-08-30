#include "../common/platform.h"
#include "dance_ui.h"

#include "../display/background.h"
#include "../display/context_menu.h"
#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/scrollbar.h"
#include "../display/window.h"
#include "call.h"
#include "motion.h"

namespace dance {

static display::Color homeColor(0xc0c0c0);

class SequenceChangeCommand : public display::Undo {
public:
	SequenceChangeCommand(DanceEditor* editor, Sequence* sequence) {
		_editor = editor;
		_sequence = sequence;
		_originalModified = sequence->modified;
		_modified = time(null);
	}

	virtual void apply() {
		_sequence->modified = _modified;
		SequenceEditor* se = _editor->frame()->edit(_sequence);
		applyThis(se);
		_editor->touch(_sequence);
	}

	virtual void revert() {
		_sequence->modified = _originalModified;
		SequenceEditor* se = _editor->frame()->edit(_sequence);
		revertThis(se);
		_editor->touch(_sequence);
	}

	virtual void discard() {
	}

	virtual void applyThis(SequenceEditor* se) = 0;
	virtual void revertThis(SequenceEditor* se) = 0;

	Sequence* sequence() const { return _sequence; }

private:
	DanceEditor*		_editor;
	Sequence*			_sequence;
	string				_original;
	string				_newComment;
	time_t				_originalModified;
	time_t				_modified;
};

class CommentChangeCommand : public SequenceChangeCommand {
public:
	CommentChangeCommand(DanceEditor* editor, Sequence* sequence, const string& comment) : SequenceChangeCommand(editor, sequence) {
		_original = sequence->comment;
		_newComment = comment;
	}

	virtual void applyThis(SequenceEditor* se) {
		sequence()->comment = _newComment;
		se->touchComment();
	}

	virtual void revertThis(SequenceEditor* se) {
		sequence()->comment = _original;
		se->touchComment();
	}

private:
	string				_original;
	string				_newComment;
};

class SequenceLevelChangeCommand : public SequenceChangeCommand {
public:
	SequenceLevelChangeCommand(DanceEditor* editor, Sequence* sequence, int level) : SequenceChangeCommand(editor, sequence) {
		_original = sequence->level();
		_newLevel = level;
	}

	virtual void applyThis(SequenceEditor* se) {
		sequence()->setLevel(_newLevel);
		se->touchLevel();
	}

	virtual void revertThis(SequenceEditor* se) {
		sequence()->setLevel(_original);
		se->touchLevel();
	}

private:
	int					_original;
	int					_newLevel;
};

class CallChangeCommand : public SequenceChangeCommand {
public:
	CallChangeCommand(DanceEditor* editor, Sequence* sequence, int index, const string& call) : SequenceChangeCommand(editor, sequence) {
		_index = index;
		if (index < sequence->text().size())
			_original = sequence->text()[index];
		else
			_original = "";
		_newCall = call;
	}

	virtual void applyThis(SequenceEditor* se) {
		sequence()->setCall(_index, _newCall);
		se->touchCall(_index);
	}

	virtual void revertThis(SequenceEditor* se) {
		sequence()->setCall(_index, _original);
		se->touchCall(_index);
	}

private:
	int					_index;
	string				_original;
	string				_newCall;
};

class InsertCallCommand : public SequenceChangeCommand {
public:
	InsertCallCommand(DanceEditor* editor, Sequence* sequence, int index) : SequenceChangeCommand(editor, sequence) {
		_index = index;
	}

	virtual void applyThis(SequenceEditor* se) {
		for (int i = sequence()->text().size() - 1; i >= _index; i--)
			sequence()->setCall(i + 1, sequence()->text()[i]);
		sequence()->setCall(_index, "");
		se->touchInsertDeleteCall(_index);
	}

	virtual void revertThis(SequenceEditor* se) {
		for (int i = _index; i < sequence()->text().size() - 1; i++)
			sequence()->setCall(i, sequence()->text()[i + 1]);
		sequence()->setCall(sequence()->text().size() - 1, "");
		se->touchInsertDeleteCall(_index);
	}

private:
	int					_index;
};

class DeleteCallCommand : public SequenceChangeCommand {
public:
	DeleteCallCommand(DanceEditor* editor, Sequence* sequence, int index) : SequenceChangeCommand(editor, sequence) {
		_index = index;
	}

	virtual void applyThis(SequenceEditor* se) {
		for (int i = _index; i < sequence()->text().size() - 1; i++)
			sequence()->setCall(i, sequence()->text()[i + 1]);
		sequence()->setCall(sequence()->text().size() - 1, "");
		se->touchInsertDeleteCall(_index);
	}

	virtual void revertThis(SequenceEditor* se) {
		for (int i = sequence()->text().size() - 1; i >= _index; i--)
			sequence()->setCall(i + 1, sequence()->text()[i]);
		sequence()->setCall(_index, "");
		se->touchInsertDeleteCall(_index);
	}

private:
	int					_index;
};

SequenceEditor::SequenceEditor(DanceFrame* frame, DanceEditor* parent, Sequence* sequence) {
	selected.addHandler(this, &SequenceEditor::onSelected);
	_frame = frame;
	_parent = parent;
	_sequence = sequence;
	_body = null;
	_view = null;
	_selectedCall = -1;
	_keyFocus = 0;
	_selectedInProgress = false;
	_choicesHandler = null;
}

SequenceEditor::~SequenceEditor() {
	for (int i = 0; i < _callMap.size(); i++) {
		CallMapEntry& cme = _callMap[i];
		cme.notes->prune();
		delete cme.notes;
		delete cme.drillDownHandler;
	}
	delete _choicesHandler;
	delete _body;
}

void SequenceEditor::select(int index) {
	if (index == _selectedCall)
		return;
	CallMapEntry& cme = _callMap[_selectedCall];
	if (index >= 0 && index <= _sequence->stages().size())
		_selectedCall = index;
	else
		_selectedCall = 0;
	if (_body->isBindable())
		_body->rootCanvas()->setKeyboardFocus(_callMap[_selectedCall + 1].call);
}

void SequenceEditor::touchComment() {
	_comment->set_value(_sequence->comment);
	touchModified(false);
}

void SequenceEditor::touchLevel() {
	_levelName->set_value(_sequence->level());
	setPreference("defaultLevel", levels[_sequence->level()]);
	touchModified(true);
	updateCallList();
}

void SequenceEditor::touchCall(int index) {
	const string& text = _sequence->text()[index];
	display::Label* call = _callMap[index + 1].call;
	call->set_value(text);
	call->set_textColor(&display::black);
	touchModified(true);
	if (text.size() != 0 && _callMap.size() == index + 2) {
		// Expand grid
		_callArea->reopen(-1, -1);
			defineCall();
		_callArea->complete();
	}
	CallMapEntry* last = &_callMap[_callMap.size() - 1];
	if (_sequence->resolved()) {
		clearField(last->call);
		last->call->set_value("home");
		last->call->set_textColor(&homeColor);
	} else
		defineField(last->call, _sequence->text().size());
	select(index);
}

void SequenceEditor::touchInsertDeleteCall(int startingAt) {
	for (int i = startingAt; i < _sequence->text().size() - 1; i++) {
		display::Label* call = _callMap[i + 1].call;
		call->set_value(_sequence->text()[i]);
		call->set_textColor(&display::black);
	}
	if (_sequence->text().size() < _callMap.size() - 1) {
		// This is a delete, purge the final row of the call map.
		CallMapEntry* last = &_callMap[_callMap.size() - 1];
		clearField(last->call);
		display::Canvas* c = last->drillDown->canvas();
		if (c != null) {
			last->drillDown->setCanvas(null);
			delete c;
		}
		c = last->cause->canvas();
		if (c != null) {
			last->cause->setCanvas(null);
			delete c;
		}
		last->statusCell->setCanvas(null);
		last->callCell->setCanvas(null);
		delete last->status;
		delete last->call;
		delete last->notes;
		last->notesCell->setCanvas(null);
		_callMap.resize(_callMap.size() - 1);
	}
	touchCall(_sequence->text().size() - 1);		// If this was an insert, this will expand the grid
													// If not, it won't.
}

void SequenceEditor::touchModified(bool affectsOutcomes) {
	string mdate;
	if (_sequence->modified)
		mdate.localTime(_sequence->modified, "%m/%d/%Y %H:%M:%S");
	if (affectsOutcomes) {
		if (!_sequence->current(myDefinitions)) {
			_parent->touch(_sequence);
			_sequence->updateStages(myDefinitions);
			updateAllCalls();
		}
	}
	_modified->set_value(mdate);
}

void SequenceEditor::updateAllCalls() {
	for (int i = 0; i < _sequence->stages().size(); i++)
		updateCallData(i);
}

display::Color warning(0xff8000);
display::SolidBackground yellowBackground(&display::yellow);
display::SolidBackground redBackground(&display::red);

void SequenceEditor::updateCallData(int index) {
	if (index < 0 || index > _sequence->stages().size())
		return;
	CallMapEntry* cme = &_callMap[index + 1];
	if (index >= _sequence->stages().size()) {
		display::Canvas* c = cme->drillDown->canvas();
		if (c != null) {
			cme->drillDown->setCanvas(null);
			delete c;
		}
		c = cme->cause->canvas();
		if (c != null) {
			cme->cause->setCanvas(null);
			delete c;
		}
		cme->status->set_value("");
		return;
	}
	const Stage* stage = _sequence->stages()[index];
	if (stage->stepCount() > 0) {
		if (cme->drillDown->canvas() == null) {
			display::Bevel* b = new display::Bevel(2, new display::Label("?"));
			b->setBackground(&display::buttonFaceBackground);
			cme->drillDownHandler = new display::ButtonHandler(b, 0);
			cme->drillDownHandler->click.addHandler(this, &SequenceEditor::onDrillDown, index);
			display::Grid* g = new display::Grid();
				g->cell(b);
				g->cell();
			g->complete();
			cme->drillDown->setCanvas(g);
		}
	} else {
		display::Canvas* c = cme->drillDown->canvas();
		if (c != null) {
			cme->drillDown->setCanvas(null);
			delete c;
			delete cme->drillDownHandler;
			cme->drillDownHandler = null;
		}
	}
	if (stage->failed()) {
		cme->status->set_textColor(&display::red);
		cme->status->set_value("Failed");
		if (cme->cause->canvas() == null) {
			display::Label* cause = new display::Label(stage->cause()->text());
			switch (stage->cause()->explanationClass()) {
			default:
			case	GENERAL_ERROR:
				cause->setBackground(&yellowBackground);
				cause->set_textColor(&display::green);
				break;

			case	PROGRAM_BUG:
				cause->setBackground(&yellowBackground);
				cause->set_textColor(&display::red);
				break;

			case	DEFINITION_ERROR:
				cause->setBackground(&redBackground);
				cause->set_textColor(&display::black);
				break;

			case	USER_ERROR:
				cause->set_textColor(&display::red);
			}
			cme->cause->setCanvas(cause);
		} else {
			display::Label* cause = (display::Label*)cme->cause->canvas();
			cause->set_textColor(&display::red);
			cause->set_value(stage->cause()->text());
		}
	} else if (stage->cause()) {
		cme->status->set_value("");
		if (cme->cause->canvas() == null) {
			display::Label* cause = new display::Label(stage->cause()->text());
			cause->set_textColor(&warning);
			cme->cause->setCanvas(cause);
		} else {
			display::Label* cause = (display::Label*)cme->cause->canvas();
			cause->set_textColor(&warning);
			cause->set_value(stage->cause()->text());
		}
	} else {
		cme->status->set_value("");
		display::Canvas* c = cme->cause->canvas();
		if (c != null) {
			cme->cause->setCanvas(null);
			delete c;
		}
	}
}

void SequenceEditor::resetKeyFocus() {
	_body->rootCanvas()->setKeyboardFocus(null);
	restoreKeyFocus();
}

void SequenceEditor::restoreKeyFocus() {
	display::RootCanvas* root = _body->rootCanvas();
	if (root) {
		display::Field* f = root->nthUnder(_keyFocus, _body);
		if (f == null)
			f = root->nthUnder(0, _body);
		if (f)
			root->setKeyboardFocus(f->label());
	}
}

bool SequenceEditor::deleteTab() {
	return false;
}

void SequenceEditor::onSelected() {
	_selectedInProgress = true;
	setPreference("defaultLevel", levels[_sequence->level()]);
	if (_sequence->updateStages(myDefinitions))
		_parent->touch(_sequence);
	_root = _body->rootCanvas();
	if (_view == null) 	{
		display::Grid* infoArea = new display::Grid();
			infoArea->cell(new display::Label("Comment:"));
			_comment = new display::Label(_sequence->comment);
			_comment->setBackground(&display::editableBackground);
			defineField(_comment, -1);
			_comment->valueCommitted.addHandler(this, &SequenceEditor::onCommentChanged);
			infoArea->cell(new display::Spacer(2, new display::Bevel(2, true, _comment)));
			infoArea->row();
			infoArea->cell(new display::Label("Created:"));
			string date;
			date.localTime(_sequence->created, "%m/%d/%Y %H:%M:%S");
			infoArea->cell(new display::Label(date));
			infoArea->row();
			infoArea->cell(new display::Label("Modified:"));
			_modified = new display::Label("");
			touchModified(false);
			infoArea->cell(_modified);
			infoArea->row();
			infoArea->cell(new display::Label("Created With:", 10));
			infoArea->cell(new display::Label(_sequence->createdWith));
			infoArea->row();
			infoArea->cell(new display::Label("Level:"));
			_levelName = new display::DropDown(_sequence->level(), levels);
			display::Spacer* spacer = new display::Spacer(2, _levelName);
			_levelName->valueChanged.addHandler(this, &SequenceEditor::onLevelChanged);
			defineField(_levelName, -1);
			spacer->setBackground(&display::editableBackground);
			infoArea->cell(new display::Filler(new display::Border(1, spacer)));
		infoArea->complete();
		_callArea = new display::Grid();

			CallMapEntry* first = defineCall();
			first->call->set_value("home");
			first->call->set_textColor(&homeColor);
			first->drillDown->setCanvas(new display::Label("", 1));
			_selectedCall = -1;

			for (int i = 0; i < _sequence->text().size(); i++) {
				CallMapEntry* cme = defineCall();

				cme->call->set_value(_sequence->text()[i]);
				defineField(cme->call, i);
				if (i < _sequence->notes().size() && _sequence->notes()[i].size() > 0) {
					cme->notes->set_value(_sequence->notes()[i]);
					cme->notesCell->setCanvas(cme->notes);
				}
				updateCallData(i);
			}
			CallMapEntry* last = defineCall();
			if (_sequence->resolved()) {
				last->call->set_value("home");
				last->call->set_textColor(&homeColor);
			} else
				defineField(last->call, _sequence->text().size());
		_callArea->complete();
		_callArea->setBackground(&display::editableBackground);
		_callArea->mouseCursor = display::standardCursor(display::HAND);
		display::ScrollableCanvas* choicesArea = new display::ScrollableCanvas();
		_choicesHandler = new display::ScrollableCanvasHandler(choicesArea);
		_choicesList = new display::Spacer(0);
		choicesArea->append(_choicesList);
		choicesArea->setBackground(&display::buttonFaceBackground);
		display::Grid* g = new display::Grid();
			display::Spacer* s = new display::Spacer(0, 0, 0, 10, infoArea);
			s->setBackground(&display::buttonFaceBackground);
			g->cell(s);
			g->row();
			g->cell(_callArea);
			g->row();
			g->cell(choicesArea);
		g->complete();
		_view = g;
		_body->append(_view);
	} else
		updateAllCalls();
	restoreKeyFocus();
	_selectedInProgress = false;
}

const char* SequenceEditor::tabLabel() {
	if (_sequence->comment.size() > 0)
		return _sequence->comment.c_str();
	else
		return "<sequence>";
}

display::Canvas* SequenceEditor::tabBody() {
	if (_body == null)
		_body = new display::Spacer(0, null);
	return _body;
}

SequenceEditor::CallMapEntry* SequenceEditor::defineCall() {
	CallMapEntry cme;
	cme.row = _callArea->rows();
	_callArea->row();
	cme.drillDown = _callArea->cell();
	cme.drillDownHandler = null;
	cme.status = new display::Label("", 3);
	cme.statusCell = _callArea->cell(cme.status);
	cme.call = new display::Label("");
	cme.callCell = _callArea->cell(cme.call);
	_callArea->row();
	_callArea->cell();
	_callArea->cell();
	cme.cause = _callArea->cell();
	_callArea->row();
	_callArea->cell();
	_callArea->cell();
	cme.notes = new display::Label("");
	cme.notesCell = _callArea->cell();
	_callMap.push_back(cme);
	return &_callMap[_callMap.size() - 1];
}

void SequenceEditor::defineField(display::Label* c, int index) {
	for (display::Field* f = _root->fields(); f != null; f = f->next)
		if (f->label() == c)
			return;
	_root->field(c);
	c->functionKey.addHandler(_parent, &DanceEditor::onFunctionKey);
	c->functionKey.addHandler(this, &SequenceEditor::onFunctionKey, c);
	if (index >= 0) {
		c->valueCommitted.addHandler(this, &SequenceEditor::onCallChanged, index);
		c->openContextMenu.addHandler(this, &SequenceEditor::onOpenContextMenu);
		c->gotKeyboardFocus.addHandler(this, &SequenceEditor::onGotKeyboardFocus, index);
		c->appearanceChanged.addHandler(this, &SequenceEditor::onAppearanceChanged, index);
	}
}

void SequenceEditor::clearField(display::Label* c) {
	_root->removeFields(c);
	c->valueCommitted.removeHandlers();
	c->click.removeHandlers();
	c->openContextMenu.removeHandlers();
	c->functionKey.removeHandlers();
}

void SequenceEditor::updateCallList() {
	display::Field* callField = _root->nthUnder(_keyFocus, _body);
	if (callField == null)
		return;
	display::Label* call = callField->label();
	for (int i = 1; i < _callMap.size(); i++) {
		if (_callMap[i].call == call) {
			vector<string> productions;
			myDefinitions->parsePartial(ANYCALL, call->value().substr(0, call->cursor()), _sequence->level(), &productions);
			vector<string*> p;
			for (int j = 0; j < productions.size(); j++)
				p.push_back(&productions[j]);
			p.sort();
			for (int j = 0; j < p.size() - 1; ) {
				if (*p[j] == *p[j + 1])
					p.remove(j + 1);
				else
					j++;
			}
			display::Grid* g = new display::Grid();
				for (int j = 0; j < p.size(); j++) {
					g->cell(new display::Label(*p[j]));
					g->row();
				}
				g->cell();
			g->complete();
			if (_choicesList->child) {
				display::Canvas* c = _choicesList->child;
				c->prune();
				delete c;
			}
			_choicesList->append(g);
			break;
		}
	}
}

void SequenceEditor::onGotKeyboardFocus(int index) {
	if (!_selectedInProgress)
		_frame->animate(this, index, _sequence);
	if (index <= _sequence->stages().size()) {
		display::Label* call = _callMap[index + 1].call;
		_keyFocus = _root->indexUnder(call, _body);
		updateCallList();
	}
}

void SequenceEditor::onAppearanceChanged(int index) {
	if (index <= _sequence->stages().size())
		updateCallList();
}

int SequenceEditor::hitCall(display::point p) {
	display::Cell* cell = _callArea->hit(p);
	if (cell) {
		int firstRow = 1 + 3 * (cell->top() / 3);		// Each call gets three rows in the grid.  Integer divide rounds down.

		for (int i = 1; i < _callMap.size(); i++)
			if (firstRow == _callMap[i].row)
				return i - 1;
	}
	return -1;
}

void SequenceEditor::onOpenContextMenu(display::point p, display::Canvas* target) {
	int index = hitCall(p);
	if (index >= 0 && index < _sequence->text().size()) {
		display::ContextMenu* c = new display::ContextMenu(_root, p, target);
		c->choice("Insert a call")->click.addHandler(this, &SequenceEditor::insertCall, index);
		c->choice("Delete this call")->click.addHandler(this, &SequenceEditor::deleteCall, index);
		c->show();
	}
}

void SequenceEditor::onDrillDown(display::Bevel* target, int index) {
	_frame->drillDown(index, _sequence);
}

void SequenceEditor::onCommentChanged() {
	if (_comment->value() != _sequence->comment)
		_parent->undoStack().addUndo(new CommentChangeCommand(_parent, _sequence, _comment->value()));
}

void SequenceEditor::onLevelChanged() {
	if (_levelName->value() != _sequence->level())
		_parent->undoStack().addUndo(new SequenceLevelChangeCommand(_parent, _sequence, _levelName->value()));
}

void SequenceEditor::onCallChanged(int index) {
	if (index == _sequence->text().size()) {
		if (_callMap[index + 1].call->value().size() == 0)
			return;
	} else if (_sequence->text()[index] == _callMap[index + 1].call->value())
		return;
	_parent->undoStack().addUndo(new CallChangeCommand(_parent, _sequence, index, _callMap[index + 1].call->value()));
}

void SequenceEditor::onFunctionKey(display::FunctionKey fk, display::ShiftState ss, display::Label* label) {
	int i;
	switch (fk) {
	case	display::FK_RETURN:
		i = _root->indexUnder(label, _body);
		_keyFocus = i + 1;
		_root->run(this, &SequenceEditor::resetKeyFocus);
		break;

	case	display::FK_INSERT:
		i = hitCall(label->bounds.topLeft);
		if (i >= 0 && i < _sequence->text().size())
			insertCall(display::point(), null, i);
		break;
	}
}

void SequenceEditor::insertCall(display::point p, display::Canvas* target, int i) {
	_parent->undoStack().addUndo(new InsertCallCommand(_parent, _sequence, i));
}

void SequenceEditor::deleteCall(display::point p, display::Canvas* target, int i) {
	_parent->undoStack().addUndo(new DeleteCallCommand(_parent, _sequence, i));
}

}  // namespace dance
