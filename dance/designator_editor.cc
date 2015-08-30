#include "../common/platform.h"
#include "dance_ui.h"

#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"

namespace dance {

class DesignatorEditCommand : public display::Undo {
public:
	DesignatorEditCommand(GrammarEditor* editor, Designator* designator) {
		_editor = editor;
		_modified = time(null);
		_designator = designator;
	}

	virtual bool applyEdit(DesignatorEditor* de) = 0;

	virtual bool revertEdit(DesignatorEditor* de) = 0;

	virtual void discardEdit() = 0;

	Designator* designator() const { return _designator; }

private:
	virtual void apply() {
		DesignatorEditor* de = _editor->frame()->edit(_designator);
		if (applyEdit(de))
			_editor->touchDesignator(_designator);
		_originalModified = _designator->modified();
		_designator->setModified(_modified);
		de->setModified(_modified);
		_designator->grammar()->touch();
	}

	virtual void revert() {
		DesignatorEditor* de = _editor->frame()->edit(_designator);
		if (revertEdit(de))
			_editor->touchDesignator(_designator);
		_designator->setModified(_originalModified);
		de->setModified(_originalModified);
		_designator->grammar()->touch();
	}

	virtual void discard() {
		discardEdit();
	}

	GrammarEditor*		_editor;
	Designator*			_designator;
	time_t				_modified;
	time_t				_originalModified;
};

class DesignatorLevelChangeCommand : public DesignatorEditCommand {
public:
	DesignatorLevelChangeCommand(GrammarEditor* editor, Designator* designator, int level) : DesignatorEditCommand(editor, designator) {
		_original = designator->level();
		_newLevel = level;
	}

	virtual bool applyEdit(DesignatorEditor* de) {
		designator()->setLevel(levels[_newLevel]);
		de->touchLevel();
		return false;
	}

	virtual bool revertEdit(DesignatorEditor* de) {
		designator()->setLevel(levels[_original]);
		de->touchLevel();
		return false;
	}

	virtual void discardEdit() {
	}

private:
	int					_original;
	int					_newLevel;
};

class DesignatorPhraseChangeCommand : public DesignatorEditCommand {
public:
	DesignatorPhraseChangeCommand(GrammarEditor* editor, Designator* designator, const string& text, int i) : DesignatorEditCommand(editor, designator) {
		_original = designator->phrases()[i];
		_newText = text;
		_phrase = i;
	}

	virtual bool applyEdit(DesignatorEditor* de) {
		designator()->setPhrase(_phrase, _newText);
		de->touchPhrase(_phrase);
		return false;
	}

	virtual bool revertEdit(DesignatorEditor* de) {
		designator()->setPhrase(_phrase, _original);
		de->touchPhrase(_phrase);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	string				_original;
	string				_newText;
	int					_phrase;
};

class AddDesignatorPhraseCommand : public DesignatorEditCommand {
public:
	AddDesignatorPhraseCommand(GrammarEditor* editor, Designator* designator, const string& text) : DesignatorEditCommand(editor, designator) {
		_newText = text;
		_phrase = designator->phrases().size();
	}

	virtual bool applyEdit(DesignatorEditor* de) {
		designator()->addPhrase();
		designator()->setPhrase(_phrase, _newText);
		if (_phrase == 0)
			de->tabModified();
		return true;
	}

	virtual bool revertEdit(DesignatorEditor* de) {
		designator()->setPhrase(_phrase, "");
		if (_phrase == 0)
			de->tabModified();
		return true;
	}

	virtual void discardEdit() {
	}

private:
	string				_newText;
	int					_phrase;
};

class ExpressionChangeCommand : public DesignatorEditCommand {
public:
	ExpressionChangeCommand(GrammarEditor* editor, Designator* designator, const string& expression) : DesignatorEditCommand(editor, designator) {
		_original = designator->expression();
		_newExpression = expression;
	}

	virtual bool applyEdit(DesignatorEditor* de) {
		designator()->setExpression(_newExpression);
		de->touchExpression();
		return true;
	}

	virtual bool revertEdit(DesignatorEditor* de) {
		designator()->setExpression(_original);
		de->touchExpression();
		return true;
	}

	virtual void discardEdit() {
	}

private:
	string					_original;
	string					_newExpression;
};

void DesignatorEditor::touchLevel() {
	_levelName->set_value(_designator->level());
	if (_designator->level() == 0)
		_levelName->set_textColor(&display::red);
	else
		_levelName->set_textColor(&display::black);
}

void DesignatorEditor::touchExpression() {
	_expression->set_value(_designator->expression());
}

void DesignatorEditor::touchPhrase(int index) {
	if (index >= _designator->phrases().size())
		_phraseMap[index]->set_value("");
	else
		_phraseMap[index]->set_value(_designator->phrases()[index]);
	if (index == 0)
		tabModified();
}

const char* DesignatorEditor::tabLabel() {
	if (_designator->phrases().size() > 0)
		return _designator->phrases()[0].c_str();
	else
		return "<no phrases>";
}

display::Canvas* DesignatorEditor::createView(bool editable) {
	_levelName = new display::DropDown(_designator->level(), levels);
	if (_designator->level() == 0)
		_levelName->set_textColor(&display::red);
	display::Grid* infoArea = new display::Grid();
		infoArea->cell(new display::Label("Created:"));
		string date;
		if (_designator->created())
			date.localTime(_designator->created(), "%m/%d/%Y %H:%M:%S");
		infoArea->cell(new display::Label(date));
		infoArea->row();
		infoArea->cell(new display::Label("Modified:"));
		setModified(_designator->modified());
		infoArea->cell(modifiedLabel());
		infoArea->row();
		infoArea->cell(new display::Label("Level:"));
		display::Canvas* spacer = new display::Spacer(2, _levelName);
		infoArea->cell(new display::Filler(new display::Border(1, spacer)));
		if (editable) {
			field(_levelName);
			spacer->setBackground(&display::editableBackground);
			_levelName->valueChanged.addHandler(this, &DesignatorEditor::onLevelChanged, _levelName);
		}
		infoArea->row();
		infoArea->cell(new display::Label("Phrases:"));
		display::Grid* productions = new display::Grid();
			for (int i = 0; i < _designator->phrases().size(); i++) {
				if (i != 0)
					productions->row();
				display::Label* prod = new display::Label(_designator->phrases()[i]);
				if (editable)
					makeProductionField(prod, i, &DesignatorEditor::onProductionCommitted);
				productions->cell(new display::Spacer(2, new display::Bevel(2, true, prod)));
			}
			if (editable) {
				productions->row();
				display::Label* newProduction = new display::Label("");
				makeProductionField(newProduction, _designator->phrases().size(), &DesignatorEditor::onNewProductionCommitted);
				productions->cell(new display::Spacer(2, new display::Bevel(2, true, newProduction)));
			}
		productions->complete();
		infoArea->cell(productions);
		infoArea->row();
		infoArea->cell(new display::Label("Expression:"));
		_expression = new display::Label(_designator->expression());
		if (editable) {
			_expression->setBackground(&display::editableBackground);
			field(_expression);
			_expression->valueCommitted.addHandler(this, &DesignatorEditor::onExpressionCommitted);
		}
		infoArea->cell(new display::Spacer(2, new display::Bevel(2, true, _expression)));
		infoArea->row();
		infoArea->cell();
	infoArea->complete();
	return infoArea;
}

void DesignatorEditor::makeProductionField(display::Label* production, int i, void (DesignatorEditor::* func)(display::Label* label, int i)) {
	production->setBackground(&display::editableBackground);
	field(production);
	production->valueCommitted.addHandler(this, func, production, i);
	_phraseMap.push_back(production);
}

void DesignatorEditor::onLevelChanged(display::DropDown* label) {
	int old = _designator->level();
	if (label->value() != old)
		parent()->undoStack().addUndo(new DesignatorLevelChangeCommand(parent(), _designator, label->value()));
}

void DesignatorEditor::onProductionCommitted(display::Label* label, int index) {
	string old = _designator->phrases()[index];
	if (old != label->value())
		parent()->undoStack().addUndo(new DesignatorPhraseChangeCommand(parent(), _designator, label->value(), index));
}

void DesignatorEditor::onNewProductionCommitted(display::Label* label, int index) {
	if (label->value().size() && _designator->phrases().size() == index)
		parent()->undoStack().addUndo(new AddDesignatorPhraseCommand(parent(), _designator, label->value()));
}

void DesignatorEditor::onExpressionCommitted() {
	if (_expression->value() != _designator->expression()) 
		parent()->undoStack().addUndo(new ExpressionChangeCommand(parent(), _designator, _expression->value()));
}


}  // namespace dance
