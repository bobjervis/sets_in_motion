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

class NewDesignatorCommand : public display::Undo {
public:
	NewDesignatorCommand(GrammarEditor* editor) {
		_editor = editor;
		_designator = new Designator(editor->grammar());
		_outlineItem = _editor->newDesignatorOI(_designator);
	}

	virtual void apply() {
		_editor->grammar()->addDesignator(_designator);
		_editor->addDesignatorOI(_outlineItem);
		_editor->editDesignator(_designator);
	}

	virtual void revert() {
		_editor->grammar()->removeDesignator(_designator);
		_outlineItem->extract();
	}

	virtual void discard() {
	}

private:
	GrammarEditor*			_editor;
	Designator*				_designator;
	display::OutlineItem*	_outlineItem;
};

class DeleteDesignatorCommand : public display::Undo {
public:
	DeleteDesignatorCommand(GrammarEditor* editor, Designator* designator, display::OutlineItem* oi) {
		_editor = editor;
		_designator = designator;
		_outlineItem = oi;
	}

	virtual void apply() {
		_editor->grammar()->removeDesignator(_designator);
		_outlineItem->extract();
	}

	virtual void revert() {
		_editor->grammar()->addDesignator(_designator);
		_editor->addDesignatorOI(_outlineItem);
	}

	virtual void discard() {
	}

private:
	GrammarEditor*			_editor;
	Designator*				_designator;
	display::OutlineItem*	_outlineItem;
};

class NewFormationCommand : public display::Undo {
public:
	NewFormationCommand(GrammarEditor* editor) {
		_editor = editor;
		_formation = new Formation(editor->grammar(), "", UNSPECIFIED_GEOMETRY);
		_outlineItem = _editor->newFormationOI(_formation);
	}

	virtual void apply() {
		_editor->grammar()->addFormation(_formation);
		_editor->addFormationOI(_outlineItem);
		_editor->editFormation(_formation);
	}

	virtual void revert() {
		_editor->grammar()->removeFormation(_formation);
		_outlineItem->extract();
	}

	virtual void discard() {
	}

private:
	GrammarEditor*			_editor;
	Formation*				_formation;
	display::OutlineItem*	_outlineItem;
};

class DeleteFormationCommand : public display::Undo {
public:
	DeleteFormationCommand(GrammarEditor* editor, Formation* formation, display::OutlineItem* oi) {
		_editor = editor;
		_formation = formation;
		_outlineItem = oi;
	}

	virtual void apply() {
		_editor->grammar()->removeFormation(_formation);
		_outlineItem->extract();
	}

	virtual void revert() {
		_editor->grammar()->addFormation(_formation);
		_editor->addFormationOI(_outlineItem);
	}

	virtual void discard() {
	}

private:
	GrammarEditor*			_editor;
	Formation*				_formation;
	display::OutlineItem*	_outlineItem;
};

class NewDefinitionCommand : public display::Undo {
public:
	NewDefinitionCommand(GrammarEditor* editor) {
		_editor = editor;
		_definition = new Definition(editor->grammar());
		_outlineItem = _editor->newDefinitionOI(_definition);
	}

	virtual void apply() {
		_editor->grammar()->addDefinition(_definition);
		_editor->addDefinitionOI(_outlineItem);
		_editor->editDefinition(_definition);
	}

	virtual void revert() {
		_editor->grammar()->removeDefinition(_definition);
		_outlineItem->extract();
	}

	virtual void discard() {
	}

private:
	GrammarEditor*			_editor;
	Definition*				_definition;
	display::OutlineItem*	_outlineItem;
};

class DeleteDefinitionCommand : public display::Undo {
public:
	DeleteDefinitionCommand(GrammarEditor* editor, Definition* definition, display::OutlineItem* oi) {
		_editor = editor;
		_definition = definition;
		_outlineItem = oi;
	}

	virtual void apply() {
		_editor->grammar()->removeDefinition(_definition);
		_outlineItem->extract();
	}

	virtual void revert() {
		_editor->grammar()->addDefinition(_definition);
		_editor->addDefinitionOI(_outlineItem);
	}

	virtual void discard() {
	}

private:
	GrammarEditor*			_editor;
	Definition*				_definition;
	display::OutlineItem*	_outlineItem;
};

class DefinitionEditCommand : public display::Undo {
public:
	DefinitionEditCommand(GrammarEditor* editor) {
		_editor = editor;
		_modified = time(null);
	}

	virtual Definition* definition() = 0;

	virtual bool applyEdit(DefinitionEditor* de) = 0;

	virtual bool revertEdit(DefinitionEditor* de) = 0;

	virtual void discardEdit() = 0;

private:
	virtual void apply() {
		DefinitionEditor* de = _editor->frame()->edit(definition());
		if (applyEdit(de))
			_editor->touchDefinition(definition());
		_originalModified = definition()->modified();
		definition()->setModified(_modified);
		de->setModified(_modified);
		definition()->grammar()->touch();
		_editor->tabModified();
	}

	virtual void revert() {
		DefinitionEditor* de = _editor->frame()->edit(definition());
		if (revertEdit(de))
			_editor->touchDefinition(definition());
		definition()->setModified(_originalModified);
		de->setModified(_originalModified);
		definition()->grammar()->touch();
		_editor->tabModified();
	}

	virtual void discard() {
		discardEdit();
	}

	GrammarEditor*		_editor;
	time_t				_modified;
	time_t				_originalModified;
};

class NameChangeCommand : public DefinitionEditCommand {
public:
	NameChangeCommand(GrammarEditor* editor, Definition* definition, const string& name) : DefinitionEditCommand(editor) {
		_definition = definition;
		_original = definition->name();
		_newName = name;
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		_definition->setName(_newName);
		de->touchName();
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_definition->setName(_original);
		de->touchName();
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	string				_original;
	string				_newName;
};

class LevelChangeCommand : public DefinitionEditCommand {
public:
	LevelChangeCommand(GrammarEditor* editor, Definition* definition, int level) : DefinitionEditCommand(editor) {
		_definition = definition;
		_original = definition->level();
		_newLevel = level;
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		_definition->setLevel(levels[_newLevel]);
		de->touchLevel();
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_definition->setLevel(levels[_original]);
		de->touchLevel();
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	int					_original;
	int					_newLevel;
};

class ProductionChangeCommand : public DefinitionEditCommand {
public:
	ProductionChangeCommand(GrammarEditor* editor, Definition* definition, const string& text, int i) : DefinitionEditCommand(editor) {
		_definition = definition;
		_original = definition->productions()[i];
		_newText = text;
		_production = i;
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		_definition->setProduction(_production, _newText);
		de->touchProduction(_production);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_definition->setProduction(_production, _original);
		de->touchProduction(_production);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	string				_original;
	string				_newText;
	int					_production;
};

class AddProductionCommand : public DefinitionEditCommand {
public:
	AddProductionCommand(GrammarEditor* editor, Definition* definition, const string& text) : DefinitionEditCommand(editor) {
		_definition = definition;
		_newText = text;
		_production = _definition->productions().size();
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		_definition->addProduction();
		_definition->setProduction(_production, _newText);
		if (_production == 0)
			de->tabModified();
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_definition->setProduction(_production, "");
		if (_production == 0)
			de->tabModified();
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	string				_newText;
	int					_production;
};

class VariantLevelChangeCommand : public DefinitionEditCommand {
public:
	VariantLevelChangeCommand(GrammarEditor* editor, Variant* variant, int level) : DefinitionEditCommand(editor) {
		_variant = variant;
		_original = variant->level();
		_newLevel = level;
	}

	virtual Definition* definition() { return _variant->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_variant->setLevel(levels[_newLevel]);
		de->touchVariantLevel(_variant);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_variant->setLevel(levels[_original]);
		de->touchVariantLevel(_variant);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Variant*			_variant;
	int					_original;
	int					_newLevel;
};

class VariantPrecedenceChangeCommand : public DefinitionEditCommand {
public:
	VariantPrecedenceChangeCommand(GrammarEditor* editor, Variant* variant, int precedence) : DefinitionEditCommand(editor) {
		_variant = variant;
		_original = variant->precedence();
		_newPrecedence = precedence;
	}

	virtual Definition* definition() { return _variant->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_variant->setPrecedence(precedences[_newPrecedence]);
		de->touchVariantPrecedence(_variant);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_variant->setPrecedence(precedences[_original]);
		de->touchVariantPrecedence(_variant);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Variant*			_variant;
	int					_original;
	int					_newPrecedence;
};

class FormationChangeCommand : public DefinitionEditCommand {
public:
	FormationChangeCommand(GrammarEditor* editor, const string& text, Variant* v, int i) : DefinitionEditCommand(editor) {
		_original = v->patterns()[i];
		_newText = text;
		_variant = v;
		_formation = i;
	}

	virtual Definition* definition() { return _variant->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_variant->setPattern(_formation, _newText);
		de->touchPattern(_variant, _formation);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_variant->setPattern(_formation, _original);
		de->touchPattern(_variant, _formation);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	string				_original;
	string				_newText;
	Variant*			_variant;
	int					_formation;
};

class AddFormationCommand : public DefinitionEditCommand {
public:
	AddFormationCommand(GrammarEditor* editor, const string& text, Variant* v) : DefinitionEditCommand(editor) {
		_newText = text;
		_variant = v;
		_formation = _variant->patterns().size();
	}

	virtual Definition* definition() { return _variant->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_variant->addPattern(_newText);
		_variant->setPattern(_formation, _newText);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_variant->setPattern(_formation, "");
		return true;
	}

	virtual void discardEdit() {
	}

private:
	string				_newText;
	Variant*			_variant;
	int					_formation;
};

class RepeatChangeCommand : public DefinitionEditCommand {
public:
	RepeatChangeCommand(GrammarEditor* editor, const string& text, Part* p) : DefinitionEditCommand(editor) {
		_original = p->repeat();
		_newText = text;
		_part = p;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_part->setRepeat(_newText);
		de->touchRepeat(_part);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_part->setRepeat(_original);
		de->touchRepeat(_part);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	string				_original;
	string				_newText;
	Part*				_part;
};

class ActionChangeCommand : public DefinitionEditCommand {
public:
	ActionChangeCommand(GrammarEditor* editor, const string& text, Part* p, int i) : DefinitionEditCommand(editor) {
		Action* a = p->action(i);
		if (a->noop())
			_original = "";
		else
			_original = ((SimpleAction*)a)->action();
		_newText = text;
		_part = p;
		_action = i;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_part->setAction(_action, _newText);
		de->touchAction(_part, _action);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_part->setAction(_action, _original);
		de->touchAction(_part, _action);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	string				_original;
	string				_newText;
	Part*				_part;
	int					_action;
};

class AddActionCommand : public DefinitionEditCommand {
public:
	AddActionCommand(GrammarEditor* editor, const string& text, Part* p) : DefinitionEditCommand(editor) {
		_newText = text;
		_part = p;
		_action = _part->actions();
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_part->addAction(_newText);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_part->setAction(_action, "");
		return true;
	}

	virtual void discardEdit() {
	}

private:
	string				_newText;
	Part*				_part;
	int					_action;
};

class SwitchActionCommand : public DefinitionEditCommand {
public:
	SwitchActionCommand(GrammarEditor* editor, Part* p, int action, Action* newAction) : DefinitionEditCommand(editor) {
		_part = p;
		_action = action;
		if (_action < _part->actions())
			_original = _part->action(_action);
		else
			_original = null;
		_newAction = newAction;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		if (_original)
			_part->setAction(_action, _newAction);
		else
			_part->addAction(_newAction);
		de->touchAction(_part, _action);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_part->setAction(_action, _original);
		de->touchAction(_part, _action);
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Part*		_part;
	int			_action;
	Action*		_original;
	Action*		_newAction;
};

class WhoChangeCommand : public DefinitionEditCommand {
public:
	WhoChangeCommand(GrammarEditor* editor, Part* p, int action, int track, const string& who) : DefinitionEditCommand(editor) {
		_part = p;
		_action = action;
		_track = track;
		_newWho = who;
		if (track < ((CompoundAction*)_part->action(_action))->tracks().size())
			_original = ((CompoundAction*)_part->action(_action))->tracks()[track]->who;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setWho(_track, _newWho);
		de->touchAction(_part, _action);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setWho(_track, _original);
		de->touchAction(_part, _action);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Part*		_part;
	int			_action;
	int			_track;
	string		_original;
	string		_newWho;
};

class WhatChangeCommand : public DefinitionEditCommand {
public:
	WhatChangeCommand(GrammarEditor* editor, Part* p, int action, int track, const string& what) : DefinitionEditCommand(editor) {
		_part = p;
		_action = action;
		_track = track;
		_newWhat = what;
		if (track < ((CompoundAction*)_part->action(_action))->tracks().size())
			_original = ((CompoundAction*)_part->action(_action))->tracks()[track]->what;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setWhat(_track, _newWhat);
		de->touchAction(_part, _action);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setWhat(_track, _original);
		de->touchAction(_part, _action);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Part*		_part;
	int			_action;
	int			_track;
	string		_original;
	string		_newWhat;
};

class AnyWhoCanChangeCommand : public DefinitionEditCommand {
public:
	AnyWhoCanChangeCommand(GrammarEditor* editor, Part* p, int action, int track, bool anyWhoCan) : DefinitionEditCommand(editor) {
		_part = p;
		_action = action;
		_track = track;
		_newAnyWhoCan = anyWhoCan;
		if (track < ((CompoundAction*)_part->action(_action))->tracks().size())
			_original = ((CompoundAction*)_part->action(_action))->tracks()[track]->anyWhoCan;
		else
			_original = false;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setAnyWhoCan(_track, _newAnyWhoCan);
		de->touchAction(_part, _action);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setAnyWhoCan(_track, _original);
		de->touchAction(_part, _action);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Part*		_part;
	int			_action;
	int			_track;
	bool		_original;
	bool		_newAnyWhoCan;
};

class FinishTogetherChangeCommand : public DefinitionEditCommand {
public:
	FinishTogetherChangeCommand(GrammarEditor* editor, Part* p, int action, int track, bool finishTogether) : DefinitionEditCommand(editor) {
		_part = p;
		_action = action;
		_track = track;
		_newFinishTogether = finishTogether;
		if (track < ((CompoundAction*)_part->action(_action))->tracks().size())
			_original = ((CompoundAction*)_part->action(_action))->tracks()[track]->finishTogether;
		else
			_original = false;
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setFinishTogether(_track, _newFinishTogether);
		de->touchAction(_part, _action);
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		((CompoundAction*)_part->action(_action))->setFinishTogether(_track, _original);
		de->touchAction(_part, _action);
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Part*		_part;
	int			_action;
	int			_track;
	bool		_original;
	bool		_newFinishTogether;
};

class AddVariantCommand : public DefinitionEditCommand {
public:
	AddVariantCommand(GrammarEditor* editor, Definition* definition) : DefinitionEditCommand(editor) {
		_definition = definition;
		_variant = new Variant(definition);
		Part* p = _variant->nextPart("");
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		_definition->insertVariant(INT_MAX, _variant);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_definition->removeLastVariant();
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	Variant*			_variant;
};

class DeleteVariantCommand : public DefinitionEditCommand {
public:
	DeleteVariantCommand(GrammarEditor* editor, Definition* definition, Variant* v) : DefinitionEditCommand(editor) {
		_definition = definition;
		_index = -1;
		for (int i = 0; i < definition->variants().size(); i++)
			if (definition->variant(i) == v) {
				_index = i;
				break;
			}
		_variant = v;
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		if (_index >= 0) {
			_definition->deleteVariant(_index);
			return true;
		}
		return false;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		if (_index >= 0) {
			_definition->insertVariant(_index, _variant);
			return true;
		}
		return false;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	Variant*			_variant;
	int					_index;
};

class SwapVariantsCommand : public DefinitionEditCommand {
public:
	SwapVariantsCommand(GrammarEditor* editor, Definition* definition, int firstIndex) : DefinitionEditCommand(editor) {
		_definition = definition;
		_firstIndex = firstIndex;
	}

	virtual Definition* definition() { return _definition; }

	virtual bool applyEdit(DefinitionEditor* de) {
		Variant* v = _definition->variants()[_firstIndex];
		_definition->deleteVariant(_firstIndex);
		_definition->insertVariant(_firstIndex + 1, v);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		Variant* v = _definition->variants()[_firstIndex];
		_definition->deleteVariant(_firstIndex);
		_definition->insertVariant(_firstIndex + 1, v);
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Definition*			_definition;
	int					_firstIndex;
};

class AddPartCommand : public DefinitionEditCommand {
public:
	AddPartCommand(GrammarEditor* editor, Variant* variant) : DefinitionEditCommand(editor) {
		_variant = variant;
		_part = new Part(variant, "");
	}

	virtual Definition* definition() { return _variant->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_variant->addPart(_part);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_variant->removeLastPart();
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Variant*			_variant;
	Part*				_part;
};

class DeletePartCommand : public DefinitionEditCommand {
public:
	DeletePartCommand(GrammarEditor* editor, Variant* variant, Part* part) : DefinitionEditCommand(editor) {
		_part = part;
		_variant = variant;
		for (int i = 0; i < variant->partCount(); i++)
			if (variant->part(i) == part) {
				_index = i;
				break;
			}
	}

	virtual Definition* definition() { return _part->variant()->definition(); }

	virtual bool applyEdit(DefinitionEditor* de) {
		_variant->deletePart(_index);
		return true;
	}

	virtual bool revertEdit(DefinitionEditor* de) {
		_variant->insertPart(_index, _part);
		return true;
	}

	virtual void discardEdit() {
	}

private:
	Variant*			_variant;
	Part*				_part;
	int					_index;
};

GrammarEditor::GrammarEditor(DanceFrame* frame, const string& lockPreference, Grammar* grammar) {
	_frame = frame;
	_grammar = grammar;
	_body = null;
	_lockPreference = lockPreference;
	_listening = false;
	_outlineHandler = null;
	_scrollerHandler = null;
}

GrammarEditor::~GrammarEditor() {
	delete _outlineHandler;
	delete _scrollerHandler;
	delete _body;
}

bool GrammarEditor::deleteTab() {
	return false;
}

const char* GrammarEditor::tabLabel() {
	if (_grammar == defaultDefinitions)
		return "Definitions";
	else
		return "My Definitions";
}

display::Canvas* GrammarEditor::tabBody() {
	if (_body == null) {
		_outline = new display::Outline();
		_outlineHandler = new display::OutlineHandler(_outline);
		display::Label* nameLabel = new display::Label(_grammar->filename());
		nameLabel->setBackground(&display::editableBackground);
		if (_grammar->error())
			nameLabel->set_textColor(&display::red);
		display::ScrollableCanvas* sc = new display::ScrollableCanvas();
		sc->append(_outline);
		_scrollerHandler = new display::ScrollableCanvasHandler(sc);
		sc->setBackground(&display::editableBackground);
		_body = sc;

		display::OutlineItem* oi = new display::OutlineItem(_outline, nameLabel);
		_outline->set_itemTree(oi);
		_synonyms = new display::OutlineItem(_outline, new display::Label("Synonyms"));
		display::Label* def = new display::Label("Definitions");
		def->openContextMenu.addHandler(this, &GrammarEditor::onDefinitionsOpenContextMenu);
		_definitions = new display::OutlineItem(_outline, def);

		display::Label* forms = new display::Label("Formations");
		forms->openContextMenu.addHandler(this, &GrammarEditor::onFormationsOpenContextMenu);
		_formations = new display::OutlineItem(_outline, forms);

		display::Label* designations = new display::Label("Dancer Designations");
		designations->openContextMenu.addHandler(this, &GrammarEditor::onDesignatorsOpenContextMenu);
		_designations = new display::OutlineItem(_outline, designations);

		oi->append(_synonyms);
		const vector<Synonym*>& synonyms = _grammar->synonyms();
		vector<Synonym*> sorted;
		for (int i = 0; i < synonyms.size(); i++)
			sorted.push_back(synonyms[i]);
		sorted.sort();
		for (int i = 0; i < sorted.size(); i++) {
			display::Label* key = new display::Label(sorted[i]->synonym(), 5);
			key->setBackground(&display::buttonFaceBackground);
			display::Label* value = new display::Label(sorted[i]->value());
			display::Grid* g = new display::Grid();
				g->cell(key);
				g->cell(value);
			g->complete();
			display::OutlineItem* synOi = new display::OutlineItem(_outline, g);
			_synonyms->append(synOi);
		}
		oi->append(_definitions);
		const vector<Definition*>& definitions = _grammar->definitions();
		oi->append(_formations);
		oi->append(_designations);
		oi->setExpanded();
		_definitions->setExpanded();
		vector<Definition*> sortedDefs;
		for (int i = 0; i < definitions.size(); i++)
			sortedDefs.push_back(definitions[i]);
		sortedDefs.sort();
		for (int i = 0; i < sortedDefs.size(); i++)
			addDefinitionOI(newDefinitionOI(sortedDefs[i]));
		const vector<Formation*>& formations = _grammar->formations();
		vector<Formation*> sortedForms;
		for (int i = 0; i < formations.size(); i++)
			sortedForms.push_back(formations[i]);
		sortedForms.sort();
		for (int i = 0; i < sortedForms.size(); i++)
			addFormationOI(newFormationOI(sortedForms[i]));
		const vector<Designator*>& designators = _grammar->designators();
		vector<Designator*> sortedDes;
		for (int i = 0; i < designators.size(); i++)
			sortedDes.push_back(designators[i]);
		sortedDes.sort();
		for (int i = 0; i < sortedDes.size(); i++)
			addDesignatorOI(newDesignatorOI(sortedDes[i]));
	}
	return _body;
}

void GrammarEditor::extendContextMenu(display::ContextMenu *menu) {
	string lockState = getPreference(_lockPreference);
	// Make the unlocked state require a specific value, so the default
	// is to be locked.
	if (lockState == "false")
		menu->choice("Lock to prevent editing")->click.addHandler(this, &GrammarEditor::onLock);
	else
		menu->choice("Unlock for editing")->click.addHandler(this, &GrammarEditor::onUnlock);
}

void GrammarEditor::onDefinitionClick(display::MouseKeys mKeys, display::point p, display::Canvas* target, Definition* definition) {
	editDefinition(definition);
}

void GrammarEditor::onFormationClick(display::MouseKeys mKeys, display::point p, display::Canvas* target, Formation* formation) {
	editFormation(formation);
}

void GrammarEditor::onDesignatorClick(display::MouseKeys mKeys, display::point p, display::Canvas* target, Designator* designator) {
	editDesignator(designator);
}

void GrammarEditor::onUnlock(display::point p, display::Canvas* target) {
	setPreference(_lockPreference, "false");
	_frame->refreshSubEditors(this);
}

void GrammarEditor::onLock(display::point p, display::Canvas* target) {
	setPreference(_lockPreference, "true");
	_frame->refreshSubEditors(this);
}

void GrammarEditor::onDefinitionsOpenContextMenu(display::point p, display::Canvas* target) {
	string lock = getPreference(_lockPreference);
	if (lock == "false") {
		display::ContextMenu* c = new display::ContextMenu(_body->rootCanvas(), p, target);
		c->choice("Create new definition")->click.addHandler(this, &GrammarEditor::newDefinition);
		c->show();
	}
}

void GrammarEditor::onDefinitionOpenContextMenu(display::point p, display::Canvas* target, Definition* definition, display::OutlineItem* oi) {
	string lock = getPreference(_lockPreference);
	if (lock == "false") {
		display::ContextMenu* c = new display::ContextMenu(_body->rootCanvas(), p, target);
		c->choice("Delete this definition")->click.addHandler(this, &GrammarEditor::deleteDefinition, definition, oi);
		c->show();
	}
}

void GrammarEditor::onFormationsOpenContextMenu(display::point p, display::Canvas* target) {
	string lock = getPreference(_lockPreference);
	if (lock == "false") {
		display::ContextMenu* c = new display::ContextMenu(_body->rootCanvas(), p, target);
		c->choice("Create new formation")->click.addHandler(this, &GrammarEditor::newFormation);
		c->show();
	}
}

void GrammarEditor::onFormationOpenContextMenu(display::point p, display::Canvas* target, Formation* formation, display::OutlineItem* oi) {
	string lock = getPreference(_lockPreference);
	if (lock == "false") {
		display::ContextMenu* c = new display::ContextMenu(_body->rootCanvas(), p, target);
		c->choice("Delete this formation")->click.addHandler(this, &GrammarEditor::deleteFormation, formation, oi);
		c->show();
	}
}

void GrammarEditor::onDesignatorsOpenContextMenu(display::point p, display::Canvas* target) {
	string lock = getPreference(_lockPreference);
	if (lock == "false") {
		display::ContextMenu* c = new display::ContextMenu(_body->rootCanvas(), p, target);
		c->choice("Create new dancer designation")->click.addHandler(this, &GrammarEditor::newDesignator);
		c->show();
	}
}

void GrammarEditor::onDesignatorOpenContextMenu(display::point p, display::Canvas* target, Designator* designator, display::OutlineItem* oi) {
	string lock = getPreference(_lockPreference);
	if (lock == "false") {
		display::ContextMenu* c = new display::ContextMenu(_body->rootCanvas(), p, target);
		c->choice("Delete this dancer designation")->click.addHandler(this, &GrammarEditor::deleteDesignator, designator, oi);
		c->show();
	}
}

void GrammarEditor::newDefinition(display::point p, display::Canvas* target) {
	_undoStack.addUndo(new NewDefinitionCommand(this));
}

void GrammarEditor::deleteDefinition(display::point p, display::Canvas* target, Definition* definition, display::OutlineItem* oi) {
	_undoStack.addUndo(new DeleteDefinitionCommand(this, definition, oi));
}

void GrammarEditor::newFormation(display::point p, display::Canvas* target) {
	_undoStack.addUndo(new NewFormationCommand(this));
}

void GrammarEditor::deleteFormation(display::point p, display::Canvas* target, Formation* formation, display::OutlineItem* oi) {
	_undoStack.addUndo(new DeleteFormationCommand(this, formation, oi));
}

void GrammarEditor::newDesignator(display::point p, display::Canvas* target) {
	_undoStack.addUndo(new NewDesignatorCommand(this));
}

void GrammarEditor::deleteDesignator(display::point p, display::Canvas* target, Designator* designator, display::OutlineItem* oi) {
	_undoStack.addUndo(new DeleteDesignatorCommand(this, designator, oi));
}

display::Color noVariants(0xc0c0c0);

display::OutlineItem* GrammarEditor::newDefinitionOI(Definition* def) {
	display::Label* key = new display::Label(def->label());
	key->mouseCursor = display::standardCursor(display::HAND);
	key->click.addHandler(this, &GrammarEditor::onDefinitionClick, def);
	if (def->variant(0) == null)
		key->set_textColor(&noVariants);
	display::OutlineItem* dOi = new display::OutlineItem(_outline, key);
	key->openContextMenu.addHandler(this, &GrammarEditor::onDefinitionOpenContextMenu, def, dOi);
	DefinitionMapEntry dme;
	dme.key = key;
	dme.definition = def;
	_definitionMap.push_back(dme);
	return dOi;
}

void GrammarEditor::addDefinitionOI(display::OutlineItem* oi) {
	_definitions->append(oi);
}

display::OutlineItem* GrammarEditor::newFormationOI(Formation* f) {
	display::Label* key = new display::Label(f->name());
	display::Grid* g = new display::Grid();
		g->cell(key);
		g->cell(new display::Filler(new Picture(f, 8)));
	g->complete();
	key->mouseCursor = display::standardCursor(display::HAND);
	key->click.addHandler(this, &GrammarEditor::onFormationClick, f);
	display::OutlineItem* dOi = new display::OutlineItem(_outline, g);
	key->openContextMenu.addHandler(this, &GrammarEditor::onFormationOpenContextMenu, f, dOi);
	FormationMapEntry fme;
	fme.key = key;
	fme.formation = f;
	_formationMap.push_back(fme);
	return dOi;
}

void GrammarEditor::addFormationOI(display::OutlineItem* oi) {
	_formations->append(oi);
}

display::OutlineItem* GrammarEditor::newDesignatorOI(Designator* d) {
	display::Label* key = new display::Label(d->label());
	key->mouseCursor = display::standardCursor(display::HAND);
	key->click.addHandler(this, &GrammarEditor::onDesignatorClick, d);
	display::OutlineItem* dOi = new display::OutlineItem(_outline, key);
	key->openContextMenu.addHandler(this, &GrammarEditor::onDesignatorOpenContextMenu, d, dOi);
	DesignatorMapEntry dme;
	dme.key = key;
	dme.designator = d;
	_designatorMap.push_back(dme);
	return dOi;
}

void GrammarEditor::addDesignatorOI(display::OutlineItem* oi) {
	_designations->append(oi);
}

void GrammarEditor::onFunctionKey(display::FunctionKey fk, display::ShiftState ss) {
	switch (fk) {
	case	display::FK_Y:
		if (ss == display::SS_CONTROL) {
			if (_undoStack.nextRedo())
				_undoStack.redo();
			else
				_frame->setStatus("No more redo");
		}
		break;

	case	display::FK_Z:
		if (ss == display::SS_CONTROL) {
			if (_undoStack.nextUndo())
				_undoStack.undo();
			else
				_frame->setStatus("No more undo");
		}
		break;

	}
}

bool GrammarEditor::needsSave() {
	return !_undoStack.atSavedState();
}

bool GrammarEditor::save() {
	if (!fileSystem::createBackupFile(_grammar->filename())) {
		_frame->setStatus("Could not create backup for " + _grammar->filename());
		return false;
	}
	_grammar->compact();
	if (_grammar->write(_grammar->filename())) {
		_frame->setStatus(tabLabel() + string(" saved to ") + _grammar->filename());
		_undoStack.markSavedState();
		tabModified();
		return true;
	} else {
		_frame->setStatus(string("FAILED: ") + tabLabel() + " save to " + _grammar->filename());
		return false;
	}
}

void GrammarEditor::touchDefinition(Definition* definition) {
	DefinitionEditor* de = _frame->edit(definition);
	de->rebuildView();
	for (int i = 0; i < _definitionMap.size(); i++) {
		if (_definitionMap[i].definition == definition) {
			_definitionMap[i].key->set_value(definition->label());
			if (definition->variant(0) == null)
				_definitionMap[i].key->set_textColor(&noVariants);
			else
				_definitionMap[i].key->set_textColor(&display::black);
			break;
		}
	}
}

void GrammarEditor::editDefinition(Definition* definition) {
	if (!_listening) {
		_body->rootCanvas()->afterInputEvent.addHandler(&_undoStack, &display::UndoStack::rememberCurrentUndo);
		_listening = true;
	}
	_frame->edit(definition);
}

void GrammarEditor::touchFormation(Formation* formation) {
	FormationEditor* fe = _frame->edit(formation);
	_frame->rootCanvas()->run((GrammarElementEditor*)fe, &GrammarElementEditor::refreshView);
	for (int i = 0; i < _formationMap.size(); i++) {
		if (_formationMap[i].formation == formation) {
			_formationMap[i].key->set_value(formation->name());
			break;
		}
	}
}

void GrammarEditor::editFormation(Formation* formation) {
	if (!_listening) {
		_body->rootCanvas()->afterInputEvent.addHandler(&_undoStack, &display::UndoStack::rememberCurrentUndo);
		_listening = true;
	}
	_frame->edit(formation);
}

void GrammarEditor::touchDesignator(Designator* designator) {
	DesignatorEditor* de = _frame->edit(designator);
	_frame->rootCanvas()->run((GrammarElementEditor*)de, &GrammarElementEditor::refreshView);
	for (int i = 0; i < _designatorMap.size(); i++) {
		if (_designatorMap[i].designator == designator) {
			_designatorMap[i].key->set_value(designator->label());
			break;
		}
	}
}

void GrammarEditor::editDesignator(Designator* designator) {
	if (!_listening) {
		_body->rootCanvas()->afterInputEvent.addHandler(&_undoStack, &display::UndoStack::rememberCurrentUndo);
		_listening = true;
	}
	_frame->edit(designator);
}

GrammarElementEditor::GrammarElementEditor(DanceFrame* frame, GrammarEditor* parent) {
	selected.addHandler(this, &GrammarElementEditor::onSelected);
	unselected.addHandler(this, &GrammarElementEditor::onUnselected);
	_frame = frame;
	_parent = parent;
	_body = null;
	_view = null;
	_name = null;
	_root = null;
	_scroller = null;
	_scrollerHandler = null;
	_scrollX = 0;
	_scrollY = 0;
	_keyFocus = 0;
}

GrammarElementEditor::~GrammarElementEditor() {
	delete _scrollerHandler;
	delete _body;
}

void GrammarElementEditor::rebuildView() {
	_scrollX = _scroller->horizontal()->value();
	_scrollY = _scroller->vertical()->value();
	_frame->rootCanvas()->setKeyboardFocus(null);
	_frame->rootCanvas()->run(this, &GrammarElementEditor::refreshView);
}

void GrammarElementEditor::refreshView() {
	if (_view) {
		if (_root) {
			display::Canvas* c = _root->keyFocus();
			if (c && c->under(_view))
				_root->setKeyboardFocus(null);
		}
		_view->prune();
		if (_root) {
			_root->removeFields(_view);
			_root = null;
		}
		delete _view;
		_view = null;
	}
	if (_body->isBindable()) {
		composeView();
		restoreKeyFocus();
	}
}

void GrammarElementEditor::setModified(time_t m) {
	string mdate;
	if (m)
		mdate.localTime(m, "%m/%d/%Y %H:%M:%S");
	_modifiedLabel->set_value(mdate);
}

bool GrammarElementEditor::deleteTab() {
	return false;
}

void GrammarElementEditor::onSelected() {
	composeView();
	restoreKeyFocus();
}

void GrammarElementEditor::onUnselected() {
	display::RootCanvas* root = _body->rootCanvas();
	if (root)
		root->setKeyboardFocus(null);
}

display::Canvas* GrammarElementEditor::tabBody() {
	if (_body == null) {
		_body = new display::Spacer(0, null);
		_scroller = new display::ScrollableCanvas();
		_scroller->setBackground(&display::buttonFaceBackground);
		_scrollerHandler = new display::ScrollableCanvasHandler(_scroller);
		_body->append(_scroller);
	}
	return _body;
}

void GrammarElementEditor::extendContextMenu(display::ContextMenu* menu) {
	_parent->extendContextMenu(menu);
}

void GrammarElementEditor::field(display::Label* label, display::Label* prior) {
	_root->fieldAfter(label, prior);
	label->functionKey.addHandler(_parent, &GrammarEditor::onFunctionKey);
	label->functionKey.addHandler(this, &GrammarElementEditor::onFunctionKey, label);
}

void GrammarElementEditor::removeField(display::Label* label) {
	_root->removeFields(label);
}

void GrammarElementEditor::composeView() {
	if (_view == null) {
		_name = new display::Label("");
		_modifiedLabel = new display::Label("");
		string lock = getPreference(_parent->lockPreference());
		bool editable = lock == "false";
		if (editable)
			_root = _body->rootCanvas();
		_view = createView(editable);
		_scroller->vertical()->set_value(0);
		_scroller->horizontal()->set_value(0);
		while (_scroller->child)
			_scroller->child->prune();
		_scroller->append(_view);
		_scrollerHandler->newClient(_view);
	}
}

void GrammarElementEditor::resetKeyFocus() {
	_body->rootCanvas()->setKeyboardFocus(null);
	restoreKeyFocus();
}

void GrammarElementEditor::restoreKeyFocus() {
	display::RootCanvas* root = _body->rootCanvas();
	if (root) {
		display::Field* f = root->nthUnder(_keyFocus, _body);
		if (f == null)
			f = root->nthUnder(0, _body);
		if (f)
			root->setKeyboardFocus(f->label());
		_scroller->arrange();
		_scroller->horizontal()->set_value(_scrollX);
		_scroller->vertical()->set_value(_scrollY);
	}
}

void GrammarElementEditor::onFunctionKey(display::FunctionKey fk, display::ShiftState ss, display::Label* label) {
	int i;
	switch (fk) {
	case	display::FK_RETURN:
		i = _root->indexUnder(label, _body);
		_keyFocus = i + 1;
		_scrollX = _scroller->horizontal()->value();
		_scrollY = _scroller->vertical()->value();
		_root->run(this, &GrammarElementEditor::resetKeyFocus);
		break;
	}
}

void DefinitionEditor::touchName() {
	name()->set_value(_definition->name());
	tabModified();
}

void DefinitionEditor::touchLevel() {
	_levelName->set_value(_definition->level());
	if (_definition->level() == 0)
		_levelName->set_textColor(&display::red);
	else
		_levelName->set_textColor(&display::black);
}

void DefinitionEditor::touchProduction(int production) {
	if (production == 0)
		tabModified();
	if (production >= _definition->productions().size())
		_productionMap[production]->set_value("");
	else
		_productionMap[production]->set_value(_definition->productions()[production]);
}

void DefinitionEditor::touchVariantLevel(Variant* v) {
	for (int i = 0; i < _levelMap.size(); i++) {
		if (_levelMap[i].variant == v) {
			_levelMap[i].label->set_value(v->level());
			return;
		}
	}
}

void DefinitionEditor::touchVariantPrecedence(Variant* v) {
	for (int i = 0; i < _precedenceMap.size(); i++) {
		if (_precedenceMap[i].variant == v) {
			_precedenceMap[i].label->set_value(v->precedence());
			return;
		}
	}
}

void DefinitionEditor::touchPattern(Variant* v, int formation) {
	for (int i = 0; i < _formationMap.size(); i++) {
		if (_formationMap[i].variant == v &&
			_formationMap[i].formation == formation) {
			if (formation >= v->patterns().size())
				_formationMap[i].label->set_value("");
			else
				_formationMap[i].label->set_value(v->patterns()[formation]);
			if (formation >= v->recognizers().size() || 
				v->recognizers()[formation])
				_formationMap[i].label->set_textColor(&display::black);
			else
				_formationMap[i].label->set_textColor(&display::red);
			return;
		}
	}
}

void DefinitionEditor::touchRepeat(Part* p) {
	for (int i = 0; i < _repeatMap.size(); i++) {
		if (_repeatMap[i].part == p) {
			_repeatMap[i].label->set_value(p->repeat());
			return;
		}
	}
}

void DefinitionEditor::touchAction(Part* p, int action) {
	for (int i = 0; i < _actionMap.size(); i++) {
		if (_actionMap[i]->part() == p &&
			_actionMap[i]->action() == action) {
			_actionMap[i]->update();
			return;
		}
	}
}

const char* DefinitionEditor::tabLabel() {
	if (_definition->productions().size() > 0)
		return _definition->productions()[0].c_str();
	else
		return "<no phrases>";
}

display::Canvas* DefinitionEditor::createView(bool editable) {
	_productionMap.clear();
	_formationMap.clear();
	_levelMap.clear();
	_precedenceMap.clear();
	_actionMap.clear();
	_repeatMap.clear();
	_levelName = new display::DropDown(_definition->level(), levels);
	if (_definition->level() == 0)
		_levelName->set_textColor(&display::red);
	name()->set_value(_definition->name());
	display::Grid* infoArea = new display::Grid();
		infoArea->cell(new display::Label("Name:"));
		infoArea->cell(new display::Spacer(2, new display::Bevel(2, true, name())));
		if (editable) {
			name()->setBackground(&display::editableBackground);
			field(name());
			name()->valueCommitted.addHandler(this, &DefinitionEditor::onNameCommitted, name());
		}
		infoArea->row();
		infoArea->cell(new display::Label("Created:"));
		string date;
		if (_definition->created())
			date.localTime(_definition->created(), "%m/%d/%Y %H:%M:%S");
		infoArea->cell(new display::Label(date));
		infoArea->row();
		infoArea->cell(new display::Label("Modified:"));
		setModified(_definition->modified());
		infoArea->cell(modifiedLabel());
		infoArea->row();
		infoArea->cell(new display::Label("Level:"));
		display::Canvas* spacer = new display::Spacer(2, _levelName);
		infoArea->cell(new display::Filler(new display::Border(1, spacer)));
		if (editable) {
			field(_levelName);
			spacer->setBackground(&display::editableBackground);
			_levelName->valueChanged.addHandler(this, &DefinitionEditor::onLevelChanged, _levelName);
		}
		infoArea->row();
		infoArea->cell(new display::Label("Phrases:"));
		display::Grid* productions = new display::Grid();
			for (int i = 0; i < _definition->productions().size(); i++) {
				if (i != 0)
					productions->row();
				display::Label* prod = new display::Label(_definition->productions()[i]);
				if (editable)
					makeProductionField(prod, i, &DefinitionEditor::onProductionCommitted);
				productions->cell(new display::Spacer(2, new display::Bevel(2, true, prod)));
			}
			if (editable) {
				productions->row();
				display::Label* newProduction = new display::Label("");
				makeProductionField(newProduction, _definition->productions().size(), &DefinitionEditor::onNewProductionCommitted);
				productions->cell(new display::Spacer(2, new display::Bevel(2, true, newProduction)));
			}
		productions->complete();
		infoArea->cell(productions);
	infoArea->complete();
	display::Grid* variants = new display::Grid();
		const vector<Variant*>& vars = _definition->variants();
		for (int i = 0; i < vars.size(); i++) {
			Variant* v = vars[i];
			if (v == null)
				break;
			display::Grid* variant = new display::Grid();
				display::DropDown* levelName = new display::DropDown(v->level(), levels);
				if (v->level() == 0)
					levelName->set_textColor(&display::red);
				variant->cell(new display::Label("Level:"));
				display::Canvas* spacer = new display::Spacer(2, levelName);
				variant->cell(new display::Filler(new display::Border(1, spacer)));
				if (editable) {
					spacer->setBackground(&display::editableBackground);
					field(levelName);
					levelName->valueChanged.addHandler(this, &DefinitionEditor::onVariantLevelChanged, levelName, v);
					LevelMapEntry lme;
					lme.label = levelName;
					lme.variant = v;
					_levelMap.push_back(lme);
				}
				variant->row();
				variant->cell(new display::Label("Precedence:"));
				display::DropDown* precedence = new display::DropDown(v->precedence(), precedences);
				spacer = new display::Spacer(2, precedence);
				variant->cell(new display::Filler(new display::Border(1, spacer)));
				if (editable) {
					spacer->setBackground(&display::editableBackground);
					field(precedence);
					precedence->valueChanged.addHandler(this, &DefinitionEditor::onVariantPrecedenceChanged, precedence, v);
					PrecedenceMapEntry pme;
					pme.label = precedence;
					pme.variant = v;
					_precedenceMap.push_back(pme);
				}
				variant->row();

				const vector<string>& formNames = v->patterns();

				for (int j = 0; j < formNames.size(); j++) {
					if (j == 0)
						variant->cell(new display::Label("Patterns:"));
					else
						variant->cell();
					display::Label* formation = new display::Label(formNames[j], 15);
					if (editable)
						makeFormationField(formation, v, j, &DefinitionEditor::onFormationCommitted);
					if (v->recognizers()[j] == 0)
						formation->set_textColor(&display::red);
					display::Grid* formGrid = new display::Grid();
					formGrid->cell(new display::Filler(new display::Spacer(2, new display::Bevel(2, true, formation))));
						if (v->recognizers()[j])
							formGrid->cell(new display::Filler(new display::Border(1, new Picture(v->recognizers()[j]->formation(), 16))));
					formGrid->complete();
					variant->cell(formGrid);
					variant->row();
				}
				if (editable) {
					display::Label* newFormation = new display::Label("");
					makeFormationField(newFormation, v, formNames.size(), &DefinitionEditor::onNewFormationCommitted);
					if (formNames.size() == 0)
						variant->cell(new display::Label("Formations:"));
					else
						variant->cell();
					variant->cell(new display::Spacer(2, new display::Bevel(2, true, newFormation)));
					variant->row();
				}
				display::Grid* parts = new display::Grid();
					for (int pi = 0; pi < v->partCount(); pi++) {
						Part* p = v->part(pi);
						display::Grid* part = new display::Grid();
							part->cell(new display::Label("Repeat:"));
							display::Label* repeat = new display::Label(p->repeat());
							if (editable) {
								repeat->setBackground(&display::editableBackground);
								field(repeat);
								repeat->valueCommitted.addHandler(this, &DefinitionEditor::onRepeatCommitted, repeat, p);
								RepeatMapEntry rme;

								rme.label = repeat;
								rme.part = p;
								_repeatMap.push_back(rme);
							}
							part->cell(new display::Spacer(2, new display::Bevel(2, true, repeat)));
							part->row();
							int limit = p->actions();
							if (editable)				// for editable forms, add a blank action field
								limit++;
							for (int j = 0; j < limit; j++) {
								if (j == 0)
									part->cell(new display::Label("Actions:"));
								else
									part->cell();
								ActionEditor* action = new ActionEditor(this, p, j, editable);
								_actionMap.push_back(action);
								part->cell(new display::Spacer(2, action));
								part->row();
							}
						part->complete();
						display::Canvas* partCanvas;
						if (editable) {
							display::Grid* g = new display::Grid();
								display::Label* deletePartLabel = new display::Label("Delete");
								deletePartLabel->mouseCursor = display::standardCursor(display::HAND);
								deletePartLabel->click.addHandler(this, &DefinitionEditor::deletePart, v, p);
								g->cell(true);
								g->cell(deletePartLabel);
								g->row();
								g->cell(display::dimension(3, 2), part);
							g->complete();
							partCanvas = g;
						} else
							partCanvas = part;
						parts->cell(new display::Spacer(3, new display::Border(1, new display::Spacer(5, partCanvas))));
						parts->row();
					}
					if (editable) {
						display::Label* moreParts = new display::Label("Add another part...");
						moreParts->mouseCursor = display::standardCursor(display::HAND);
						moreParts->click.addHandler(this, &DefinitionEditor::onNewPart, v);
						parts->cell(new display::Filler(moreParts));
					}
				parts->complete();
				variant->cell(new display::Label("Parts:"));
				variant->cell(parts);
			variant->complete();
			display::Canvas* vCanvas;
			if (editable) {
				display::Grid* g = new display::Grid();
					int cols = 2;
					g->cell(true);
					if (i > 0) {
						display::Label* moveUpLabel = new display::Label("Move Up  ");
						moveUpLabel->mouseCursor = display::standardCursor(display::HAND);
						moveUpLabel->click.addHandler(this, &DefinitionEditor::swapVariants, i - 1);
						g->cell(moveUpLabel);
						cols++;
					}
					if (i < vars.size() - 1) {
						display::Label* moveDownLabel = new display::Label("Move Down  ");
						moveDownLabel->mouseCursor = display::standardCursor(display::HAND);
						moveDownLabel->click.addHandler(this, &DefinitionEditor::swapVariants, i);
						g->cell(moveDownLabel);
						cols++;
					}
					display::Label* deleteVariantLabel = new display::Label("Delete");
					deleteVariantLabel->mouseCursor = display::standardCursor(display::HAND);
					deleteVariantLabel->click.addHandler(this, &DefinitionEditor::deleteVariant, v);
					g->cell(deleteVariantLabel);
					g->row();
					g->cell(display::dimension(cols, 1), variant);
				g->complete();
				vCanvas = g;
			} else
				vCanvas = variant;
			variants->cell(new display::Spacer(3, new display::Border(2, new display::Spacer(5, vCanvas))));
			variants->row();
		}
		if (editable) {
			display::Label* s = new display::Label("Add another variation...");
			s->mouseCursor = display::standardCursor(display::HAND);
			s->click.addHandler(this, &DefinitionEditor::addVariant);
			variants->cell(new display::Filler(s));
		} else
			variants->cell(new display::Label(""));
	variants->complete();
	display::Grid* g = new display::Grid();
		g->cell(infoArea);
		g->row();
		g->cell(variants);
	g->complete();
	return g;
}

void DefinitionEditor::makeProductionField(display::Label* production, int i, void (DefinitionEditor::* func)(display::Label* label, int i)) {
	production->setBackground(&display::editableBackground);
	field(production);
	production->valueCommitted.addHandler(this, func, production, i);
	_productionMap.push_back(production);
}

void DefinitionEditor::onNameCommitted(display::Label* label) {
	string old = _definition->name();
	if (old != label->value())
		parent()->undoStack().addUndo(new NameChangeCommand(parent(), _definition, label->value()));
}

void DefinitionEditor::onLevelChanged(display::DropDown* label) {
	int old = _definition->level();
	if (label->value() != old)
		parent()->undoStack().addUndo(new LevelChangeCommand(parent(), _definition, label->value()));
}

void DefinitionEditor::onVariantLevelChanged(display::DropDown* label, Variant* v) {
	int old = v->level();
	if (label->value() != old)
		parent()->undoStack().addUndo(new VariantLevelChangeCommand(parent(), v, label->value()));
}

void DefinitionEditor::onVariantPrecedenceChanged(display::DropDown* label, Variant* v) {
	int old = v->precedence();
	if (label->value() != old)
		parent()->undoStack().addUndo(new VariantPrecedenceChangeCommand(parent(), v, label->value()));
}

void DefinitionEditor::onProductionCommitted(display::Label* label, int i) {
	string old = _definition->productions()[i];
	if (old != label->value())
		parent()->undoStack().addUndo(new ProductionChangeCommand(parent(), _definition, label->value(), i));
}

void DefinitionEditor::onNewProductionCommitted(display::Label* label, int i) {
	if (label->value().size() && _definition->productions().size() == i)
		parent()->undoStack().addUndo(new AddProductionCommand(parent(), _definition, label->value()));
}

void DefinitionEditor::makeFormationField(display::Label* formation, Variant* v, int j, void (DefinitionEditor::* func)(display::Label* label, Variant* v, int i)) {
	formation->setBackground(&display::editableBackground);
	field(formation);
	formation->valueCommitted.addHandler(this, func, formation, v, j);
	FormationMapEntry fme;

	fme.label = formation;
	fme.variant = v;
	fme.formation = j;
	_formationMap.push_back(fme);
}

void DefinitionEditor::onFormationCommitted(display::Label* label, Variant* v, int i) {
	string old = v->patterns()[i];
	if (old != label->value())
		parent()->undoStack().addUndo(new FormationChangeCommand(parent(), label->value(), v, i));
}

void DefinitionEditor::onNewFormationCommitted(display::Label* label, Variant* v, int i) {
	if (label->value().size() && v->patterns().size() == i)
		parent()->undoStack().addUndo(new AddFormationCommand(parent(), label->value(), v));
}

void DefinitionEditor::onRepeatCommitted(display::Label* label, Part* p) {
	string old = p->repeat();
	if (old != label->value())
		parent()->undoStack().addUndo(new RepeatChangeCommand(parent(), label->value(), p));
}

void DefinitionEditor::addVariant(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	parent()->undoStack().addUndo(new AddVariantCommand(parent(), _definition));
}

void DefinitionEditor::deleteVariant(display::MouseKeys mKeys, display::point p, display::Canvas* target, Variant* v) {
	parent()->undoStack().addUndo(new DeleteVariantCommand(parent(), _definition, v));
}

void DefinitionEditor::swapVariants(display::MouseKeys mKeys, display::point p, display::Canvas* target, int firstIndex) {
	parent()->undoStack().addUndo(new SwapVariantsCommand(parent(), _definition, firstIndex));
}

void DefinitionEditor::onNewPart(display::MouseKeys mKeys, display::point p, display::Canvas* target, Variant* v) {
	parent()->undoStack().addUndo(new AddPartCommand(parent(), v));
}

void DefinitionEditor::deletePart(display::MouseKeys mKeys, display::point p, display::Canvas* target, Variant* v, Part* part) {
	parent()->undoStack().addUndo(new DeletePartCommand(parent(), v, part));
}

ActionEditor::~ActionEditor() {
	for (int i = 0; i < _trackMap.size(); i++) {
		TrackMapEntry& tme = _trackMap[i];

		delete tme.finishHandler;
		delete tme.anyHandler;
		delete tme.anyState;
		delete tme.finishState;
	}
	delete _outlineHandler;
	if (!_showingMainLabel)
		delete _mainLabel->parent;			// This deletes the Bevel too.
}

display::dimension ActionEditor::measure() {
	if (child)
		return child->preferredSize();
	else
		return display::dimension(0, 0);
}

void ActionEditor::init(bool editable) {
	_showingMainLabel = true;
	_outline = new display::Outline();
	_mainLabel = new display::Label("", 40);
	if (editable) {
		_mainLabel->valueChanged.addHandler(this, &ActionEditor::onMainLabelChanged);
		_mainLabel->setBackground(&display::editableBackground);
		_mainLabel->valueCommitted.addHandler(this, &ActionEditor::onActionCommitted);
	}
	display::Bevel* b = new display::Bevel(2, true, _mainLabel);
	display::OutlineItem* oi = new display::OutlineItem(_outline, b);
	_outline->set_itemTree(oi);
	oi->expand.addHandler(this, &ActionEditor::onExpand, editable);
	oi->collapse.addHandler(this, &ActionEditor::onCollapse);
	if (editable && _showingMainLabel) {
		_outlineHandler = new display::OutlineHandler(_outline);
		_parent->field(_mainLabel);
		if (_action >= _part->actions())
			onMainLabelChanged();
	} else
		_outlineHandler = null;
	append(_outline);
	update();
}

void ActionEditor::onActionCommitted() {
	string old;

	if (_action < _part->actions()) {
		if (typeid(*_part->action(_action)) == typeid(CompoundAction)) {
			if (_mainLabel->value().size() == 0)
				return;
			// switch to simple action
			_parent->parent()->undoStack().addUndo(new SwitchActionCommand(_parent->parent(), _part, _action, new SimpleAction(_part, _mainLabel->value())));
		}
		old = ((SimpleAction*)_part->action(_action))->action();
	}
	if (old != _mainLabel->value()) {
		if (_action < _part->actions()) {
			_parent->parent()->undoStack().addUndo(new ActionChangeCommand(_parent->parent(), _mainLabel->value(), _part, _action));
		} else
			_parent->parent()->undoStack().addUndo(new AddActionCommand(_parent->parent(), _mainLabel->value(), _part));
	}
}

void ActionEditor::onMainLabelChanged() {
	if (_mainLabel->value().size() == 0) {
		if (_outline->itemTree()->child == null)
			_outline->itemTree()->append(addTrack());
	} else {
		while (_outline->itemTree()->child != null) {
			display::OutlineItem* oi = _outline->itemTree()->child;
			oi->extract();
			delete oi;
		}
	}
}

void ActionEditor::onWhoCommitted(int i) {
	TrackMapEntry* tme = &_trackMap[i];
	if (_action >= _part->actions() || typeid(*_part->action(_action)) == typeid(SimpleAction)) {
		if (tme->who->value().size() == 0)
			return;
		string s = tme->who->value();
		_parent->parent()->undoStack().addUndo(new SwitchActionCommand(_parent->parent(), _part, _action, new CompoundAction(_part)));
		tme = &_trackMap[i];
		tme->who->set_value(s);
	}
	CompoundAction* a = (CompoundAction*)_part->action(_action);
	string who;
	if (i < a->tracks().size())
		who = a->tracks()[i]->who;
	if (who != tme->who->value())
		_parent->parent()->undoStack().addUndo(new WhoChangeCommand(_parent->parent(), _part, _action, i, tme->who->value()));
}

void ActionEditor::onWhatCommitted(int i) {
	TrackMapEntry* tme = &_trackMap[i];
	if (_action >= _part->actions() || typeid(*_part->action(_action)) == typeid(SimpleAction)) {
		if (tme->what->value().size() == 0)
			return;
		string s = tme->what->value();
		_parent->parent()->undoStack().addUndo(new SwitchActionCommand(_parent->parent(), _part, _action, new CompoundAction(_part)));
		tme = &_trackMap[i];
		tme->what->set_value(s);
	}
	CompoundAction* a = (CompoundAction*)_part->action(_action);
	string what;
	if (i < a->tracks().size())
		what = a->tracks()[i]->what;
	if (what!= tme->what->value())
		_parent->parent()->undoStack().addUndo(new WhatChangeCommand(_parent->parent(), _part, _action, i, tme->what->value()));
}

void ActionEditor::onAnyWhoCanChanged(int i) {
	TrackMapEntry* tme = &_trackMap[i];
	if (_action >= _part->actions() || typeid(*_part->action(_action)) == typeid(SimpleAction)) {
		if (!tme->anyWhoCan->state->value())
			return;
		bool b = tme->anyWhoCan->state->value();
		_parent->parent()->undoStack().addUndo(new SwitchActionCommand(_parent->parent(), _part, _action, new CompoundAction(_part)));
		tme = &_trackMap[i];
		tme->anyWhoCan->state->set_value(b);
	}
	CompoundAction* a = (CompoundAction*)_part->action(_action);
	bool anyWhoCan = false;
	if (i < a->tracks().size())
		anyWhoCan = a->tracks()[i]->anyWhoCan;
	if (anyWhoCan != tme->anyWhoCan->state->value())
		_parent->parent()->undoStack().addUndo(new AnyWhoCanChangeCommand(_parent->parent(), _part, _action, i, tme->anyWhoCan->state->value()));
}

void ActionEditor::onFinishTogetherChanged(int i) {
	TrackMapEntry* tme = &_trackMap[i];
	if (_action >= _part->actions() || typeid(*_part->action(_action)) == typeid(SimpleAction)) {
		if (!tme->finishTogether->state->value())
			return;
		bool b = tme->finishTogether->state->value();
		_parent->parent()->undoStack().addUndo(new SwitchActionCommand(_parent->parent(), _part, _action, new CompoundAction(_part)));
		tme = &_trackMap[i];
		tme->finishTogether->state->set_value(b);
	}
	CompoundAction* a = (CompoundAction*)_part->action(_action);
	bool finishTogether = false;
	if (i < a->tracks().size())
		finishTogether = a->tracks()[i]->finishTogether;
	if (finishTogether!= tme->finishTogether->state->value())
		_parent->parent()->undoStack().addUndo(new FinishTogetherChangeCommand(_parent->parent(), _part, _action, i, tme->finishTogether->state->value()));
}

void ActionEditor::onExpand(bool editable) {
	if (_showingMainLabel) {
		display::Grid* g = new display::Grid();
			g->cell(new display::Spacer(5, 0, 0, 0, new display::Label("      Any Who Can?")));
			g->cell(new display::Spacer(5, 0, 0, 0, new display::Label("           Who", 20)));
			g->cell(new display::Spacer(5, 0, 0, 0, new display::Label("           What", 20)));
			g->cell(new display::Spacer(5, 0, 0, 0, new display::Label("      Finish Together?")));
		g->complete();
		// Put the fields in backwards, because these are inserts after a specific point.
		display::RootCanvas* root = rootCanvas();
		if (editable) {
			for (int i = _trackMap.size() - 1; i >= 0; i--) {
				_parent->field(_trackMap[i].what, _mainLabel);
				_parent->field(_trackMap[i].who, _mainLabel);
			}
			if (root) {
				root->run((GrammarElementEditor*)_parent, &GrammarElementEditor::resetKeyFocus);
			}
			// Remove the fields first to clear any keyboard focus while the old canvas is still rooted
			_parent->removeField(_mainLabel);
		}
		_outline->itemTree()->setCanvas(g);
		_showingMainLabel = false;
	}
}

void ActionEditor::onCollapse() {
	if (!_showingMainLabel) {
		display::RootCanvas* root = rootCanvas();
		display::Field* p = root->nthUnder(0, _outline);
		root->fieldAfter(_mainLabel, p->label());
		// Remove the fields first to clear any keyboard focus while the old canvas is still rooted
		root->removeFields(_outline);
		display::Canvas* c = _outline->itemTree()->setCanvas(_mainLabel->parent);
		delete c;
		root->run((GrammarElementEditor*)_parent, &GrammarElementEditor::resetKeyFocus);
		_showingMainLabel = true;
	}
}

void ActionEditor::onTrackChanged() {
	for (int i = 0; i < _trackMap.size(); i++) {
		if (_trackMap[i].who->value().size() > 0 ||
			_trackMap[i].what->value().size() > 0) {
			// the view has valid data in it, we must stay a compound action, so
			// disable input on the outline
			delete _outlineHandler;
			_outlineHandler = null;
			return;
		}
	}
	// The whole view is empty, so we can collapse this to convert to a simple action
	if (_outlineHandler == null)
		_outlineHandler = new display::OutlineHandler(_outline);
}

void ActionEditor::update() {
	if (_action < _part->actions()) {
		Action* a = _part->action(_action);
		if (typeid(*a) == typeid(SimpleAction)) {
			if (!_showingMainLabel)
				onCollapse();
			updateMainLabel(((SimpleAction*)a)->action());
		} else {
			updateTrackMap((CompoundAction*)a);
			if (_showingMainLabel)
				_outline->itemTree()->setExpanded();
			onTrackChanged();
		}
	} else {
		if (!_showingMainLabel)
			_outline->itemTree()->setCollapsed();
		updateMainLabel("");
	}
}

void ActionEditor::updateMainLabel(const string& s) {
	if (!_showingMainLabel)
		_outline->itemTree()->setCanvas(_mainLabel->parent);
	_mainLabel->set_value(s);
}

void ActionEditor::updateTrackMap(CompoundAction* a) {
	if (_trackMap.size() <= a->tracks().size()) {
		do
			_outline->itemTree()->append(addTrack());
			while (_trackMap.size() <= a->tracks().size());
	} else if (_trackMap.size() > a->tracks().size() + 1) {
		do
			deleteLastTrack();
			while (_trackMap.size() > a->tracks().size() + 1);
	}
	for (int i = 0; i < _trackMap.size(); i++) {
		TrackMapEntry& tme = _trackMap[i];
		if (i < a->tracks().size()) {
			Track* t = a->tracks()[i];
			tme.who->set_value(t->who);
			tme.what->set_value(t->what);
			tme.anyWhoCan->state->set_value(t->anyWhoCan);
			tme.finishTogether->state->set_value(t->finishTogether);
		} else {
			tme.who->set_value("");
			tme.what->set_value("");
			tme.anyWhoCan->state->set_value(false);
			tme.finishTogether->state->set_value(false);
		}
	}

}

display::OutlineItem* ActionEditor::addTrack() {
	string lock = getPreference(_parent->parent()->lockPreference());
	bool editable = lock == "false";
	TrackMapEntry tme;
	display::Grid* g = new display::Grid();
		tme.anyState = new data::Boolean(false);
		tme.anyWhoCan = new display::Toggle(tme.anyState);
		g->cell(new display::Filler(new display::Spacer(35, 5, 0, 0, tme.anyWhoCan)));
		tme.who = new display::Label("", 20);
		g->cell(new display::Spacer(55, 0, 0, 0, new display::Bevel(2, true, tme.who)));
		tme.what = new display::Label("", 20);
		g->cell(new display::Spacer(5, 0, 0, 0, new display::Bevel(2, true, tme.what)));
		tme.finishState = new data::Boolean(false);
		tme.finishTogether = new display::Toggle(tme.finishState);
		g->cell(new display::Filler(new display::Spacer(35, 5, 0, 0, tme.finishTogether)));
		if (editable) {
			if (!_showingMainLabel) {
				display::Label* a = _trackMap[_trackMap.size() - 1].what;
				_parent->field(tme.what, a);
				_parent->field(tme.who, a);
			}
			tme.who->setBackground(&display::editableBackground);
			tme.what->setBackground(&display::editableBackground);
			tme.anyWhoCan->setBackground(&display::editableBackground);
			tme.finishTogether->setBackground(&display::editableBackground);
			tme.who->valueChanged.addHandler(this, &ActionEditor::onTrackChanged);
			tme.what->valueChanged.addHandler(this, &ActionEditor::onTrackChanged);
			tme.anyWhoCan->state->changed.addHandler(this, &ActionEditor::onAnyWhoCanChanged, _trackMap.size());
			tme.finishTogether->state->changed.addHandler(this, &ActionEditor::onFinishTogetherChanged, _trackMap.size());
			tme.finishHandler = new display::ToggleHandler(tme.finishTogether);
			tme.anyHandler = new display::ToggleHandler(tme.anyWhoCan);
			tme.who->valueCommitted.addHandler(this, &ActionEditor::onWhoCommitted, _trackMap.size());
			tme.what->valueCommitted.addHandler(this, &ActionEditor::onWhatCommitted, _trackMap.size());
		} else {
			tme.finishHandler = null;
			tme.anyHandler = null;
		}
	g->complete();
	_trackMap.push_back(tme);
	display::OutlineItem* oi = new display::OutlineItem(_outline, g);
	return oi;
}

void ActionEditor::deleteLastTrack() {
	display::OutlineItem* oi = _outline->itemTree()->child;
	if (oi == null)
		return;
	while (oi->sibling)
		oi = oi->sibling;
	TrackMapEntry& tme = _trackMap[_trackMap.size() - 1];
	data::Boolean* b = tme.finishTogether->state;
	oi->extract();
	delete oi;
	delete b;
	_trackMap.resize(_trackMap.size() - 1);
}

}  // namespace dance
