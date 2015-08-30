#include "../common/platform.h"
#include "dance_ui.h"

#include <stdlib.h>
#include <typeinfo.h>
#include "../common/atom.h"
#include "../common/execute.h"
#include "../common/file_system.h"
#include "../common/machine.h"
#include "../common/parser.h"
#include "../common/process.h"
#include "../display/background.h"
#include "../display/context_menu.h"
#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/outline.h"
#include "../display/scrollbar.h"
#include "../display/tabbed_group.h"
#include "../display/window.h"
#include "../engine/global.h"
#include "../sd/sd.h"
#include "call.h"
#include "dance.h"
#include "motion.h"

namespace dance {

static display::Canvas* callVariables(const Anything* call);
static display::Canvas* termCanvas(const Term* t);

const char* PREFERENCES_VERSION_STRING = "1";				

const char* definitionsLock = "definitions_lock";
const char* myDefinitionsLock = "myDefinitions_lock";

Grammar* defaultDefinitions;
Grammar* myDefinitions;
Library* library;

bool loadState = true;

static string findUserFolder();
static void loadStateFile();

script::Atom* preferences;
display::Window* danceWindow;
DanceFrame* danceFrame;

void launch(int argc, char** argv) {
	if (loadState)
		loadStateFile();
	initializePrecedences();
	if (!loadLevels())
		warningMessage("Couldn't load levels file - levels not enforced");
	if (!loadDefinitions())
		warningMessage("Couldn't load default definitions");
	if (!loadMyDefinitions())
		warningMessage("Couldn't load my definitions");
	myDefinitions->setBackupGrammar(defaultDefinitions);
	myDefinitions->compileStateMachines();
	if (!loadLibrary())
		warningMessage("Couldn't load my sequence library");
	if (showUI) {
		danceWindow = new display::Window();
		danceFrame = new DanceFrame();
		danceFrame->bind(danceWindow);
		danceWindow->show();
	}
	for (int i = 0; i < argc; i++) {
		if (showUI) {
			string filename = fileSystem::absolutePath(argv[i]);
			danceFrame->openFile(filename);
		}
	}
	atexit(clearMemory);
	if (!showUI)
		exit(0);
}

DanceFrame::DanceFrame() {
	_animator = null;
	_animatorStart = 1;
	_drillDown = null;
	_drillDownStart = 1;
	_idleHandler = process::currentThread()->idle.addHandler(this, &DanceFrame::onIdle);
	_preferencesEditor = null;
	_libraryEditor = null;
	_defaultEditor = null;
	_myDefinitionsEditor = null;
	_primaryHandler = null;
	_secondaryHandler = null;
	_sliderHandler = null;
	_shuttingDown = false;
}

DanceFrame::~DanceFrame() {
	process::currentThread()->idle.removeHandler(_idleHandler);
	_definitionEditors.deleteAll();
	delete _preferencesEditor;
	delete _libraryEditor;
	delete _defaultEditor;
	delete _myDefinitionsEditor;
	delete _primaryHandler;
	delete _secondaryHandler;
	delete _sliderHandler;
	delete _drillDown;
	delete _animator;
	_sequenceEditors.deleteAll();
	_danceEditors.deleteAll();
}

void DanceFrame::shutdown() {
	_shuttingDown = true;
}

void DanceFrame::bind(display::RootCanvas* c) {
	c->onFunctionKey(display::FK_F1, 0, &_helpFunction); 
	_helpFunction.addHandler(this, &DanceFrame::onHelp);
	c->close.addHandler(this, &DanceFrame::exitIfClean);
	_status = new display::Label("");
	_primary = new display::TabbedGroup();
	_primaryHandler = new display::TabbedGroupHandler(_primary);
	_secondary = new display::TabbedGroup();
	_secondaryHandler = new display::TabbedGroupHandler(_secondary);
	display::Bevel* v = display::slider();
	display::Grid* bottomRow = new display::Grid();
		bottomRow->cell(_status, true);
		bottomRow->cell(new display::Label(VERSION));
	bottomRow->complete();
	bottomRow->setBackground(&display::buttonFaceBackground);
	display::Grid* form = new display::Grid();
		form->row(true);
			display::Cell* secondaryCell = form->cell(_secondary);
			form->cell(v);
			form->cell(_primary);
		form->row();
			form->cell(display::dimension(3, 1), bottomRow);
		form->columnWidth(0, -30);	// % of total width
	form->complete();
	_sliderHandler = new display::VerticalSliderHandler(v, form, 1);
	c->append(form);
	_primary->addFileFilter(this);
	_libraryEditor = new LibraryEditor(library, this);
	_defaultEditor = new GrammarEditor(this, definitionsLock, defaultDefinitions);
	_myDefinitionsEditor = new GrammarEditor(this, myDefinitionsLock, myDefinitions);
	_preferencesEditor = new PreferencesEditor(this);
	string p = getPreference("tabs");
	if (p.size() > 0) {
		for (int i = 0; i < p.size() - 2; i += 3) {
			int last = p[i + 1] - '0';
			int current = p[i + 2] - '0';
			switch (p[i]) {
			case	'a':
				_animatorStart = last * 10;
				break;

			case	'b':
				_drillDownStart = last * 10;
				break;

			case	'd':
				_defaultStart = last * 10 + current;
				selectStartup(_defaultEditor, current);
				break;

			case	'm':
				_myDefinitionsStart = last * 10 + current;
				selectStartup(_myDefinitionsEditor, current);
				break;

			case	'l':
				_libraryStart = last * 10 + current;
				selectStartup(_libraryEditor, current);
				break;

			case	'p':
				_preferencesStart = last * 10 + current;
				selectStartup(_preferencesEditor, current);
				break;
			}
		}
	} else {
		_animatorStart = 1;
		_drillDownStart = 1;
		_preferencesStart = 1;
		_libraryStart = 1;
		_defaultStart = 2;
		_myDefinitionsStart = 2;
		_primary->select(_preferencesEditor);
		_primary->select(_libraryEditor);
		_secondary->select(_defaultEditor);
		_secondary->select(_myDefinitionsEditor);
	}
	_libraryEditor->selected.addHandler(this, &DanceFrame::onSelect, (display::TabManager*)_libraryEditor);
	_defaultEditor->selected.addHandler(this, &DanceFrame::onSelect, (display::TabManager*)_defaultEditor);
	_preferencesEditor->selected.addHandler(this, &DanceFrame::onSelect, (display::TabManager*)_preferencesEditor);
	_myDefinitionsEditor->selected.addHandler(this, &DanceFrame::onSelect, (display::TabManager*)_myDefinitionsEditor);
	_libraryEditor->closed.addHandler(this, &DanceFrame::onClose, (display::TabManager*)_libraryEditor);
	_defaultEditor->closed.addHandler(this, &DanceFrame::onClose, (display::TabManager*)_defaultEditor);
	_preferencesEditor->closed.addHandler(this, &DanceFrame::onClose, (display::TabManager*)_preferencesEditor);
	_myDefinitionsEditor->closed.addHandler(this, &DanceFrame::onClose, (display::TabManager*)_myDefinitionsEditor);
	c->beforeInputEvent.addHandler(this, &DanceFrame::clearStatus);
}

void DanceFrame::newDance(display::point p, display::Canvas* target) {
	Dance* dance = new Dance("", "");
	DanceFileEditor* de = new DanceFileEditor(dance, this);
	_danceEditors.push_back(de);
	_primary->select(de);
}

void DanceFrame::openDefinitions(display::point p, display::Canvas* target) {
	if (_defaultEditor->owner() == null)
		selectStartup(_defaultEditor, _defaultStart / 10);
	else
		_defaultEditor->owner()->select(_defaultEditor);
}

void DanceFrame::openMyDefinitions(display::point p, display::Canvas* target) {
	if (_myDefinitionsEditor->owner() == null)
		selectStartup(_myDefinitionsEditor, _myDefinitionsStart / 10);
	else
		_myDefinitionsEditor->owner()->select(_myDefinitionsEditor);
}

void DanceFrame::openMyLibrary(display::point p, display::Canvas* target) {
	if (_libraryEditor->owner() == null)
		selectStartup(_libraryEditor, _libraryStart / 10);
	else
		_libraryEditor->owner()->select(_libraryEditor);
}

void DanceFrame::openPreferences(display::point p, display::Canvas* target) {
	if (_preferencesEditor->owner() == null)
		selectStartup(_preferencesEditor, _preferencesStart / 10);
	else
		_preferencesEditor->owner()->select(_preferencesEditor);
}

bool DanceFrame::matches(const string &filename) const {
	return true;
}

bool DanceFrame::openFile(const string &filename) {
	if (filename.tolower().endsWith(".dnc")) {
		for (int i = 0; i < _danceEditors.size(); i++) {
			if (_danceEditors[i]->dance()->filename() == filename) {
				warningMessage("Dance already open");
				return true;
			}
		}
		Dance* d = new Dance(fileSystem::basename(filename), filename);
		if (!d->read()) {
			warningMessage("Errors in file, contents might be corrupted");
			return true;
		}
		DanceFileEditor* de = new DanceFileEditor(d, this);
		_danceEditors.push_back(de);
		_primary->select(de);
		return true;
	} else if (filename.tolower().endsWith(".cdf")) {
	} else {
		string danceName = fileSystem::constructPath("", filename, ".dnc");
		for (int i = 0; i < _danceEditors.size(); i++) {
			if (_danceEditors[i]->dance()->filename() == danceName) {
				warningMessage("Dance already open");
				return true;
			}
		}
		Dance* d = parseSdFile(filename);
		if (d == null) {
			warningMessage("Could not load " + filename);
			return false;
		}
		DanceFileEditor* de = new DanceFileEditor(d, this);
		_danceEditors.push_back(de);
		_primary->select(de);
		return true;
	}
	return false;
}

void DanceFrame::extendContextMenu(display::ContextMenu *menu) {
	menu->choice("New Dance")->click.addHandler(this, &DanceFrame::newDance);
	menu->choice("Open Definitions Tab")->click.addHandler(this, &DanceFrame::openDefinitions);
	menu->choice("Open My Definitions Tab")->click.addHandler(this, &DanceFrame::openMyDefinitions);
	menu->choice("Open Library Tab")->click.addHandler(this, &DanceFrame::openMyLibrary);
	menu->choice("Open Preferences Tab")->click.addHandler(this, &DanceFrame::openPreferences);
}

bool DanceFrame::save() {
	bool result = true;
	if (_libraryEditor->needsSave() &&
		!_libraryEditor->save())
		result = false;
	if (_defaultEditor->needsSave() &&
		!_defaultEditor->save())
		result = false;
	if (_myDefinitionsEditor->needsSave() &&
		!_myDefinitionsEditor->save())
		result = false;
	if (_preferencesEditor->needsSave() &&
		!_preferencesEditor->save())
		result = false;

	for (int i = 0; i < _danceEditors.size(); i++)
		if (_danceEditors[i]->needsSave() &&
			!_danceEditors[i]->save())
			result = false;
	return result;
}

SequenceEditor* DanceFrame::edit(DanceEditor* danceEditor, int index, Sequence* sequence) {
	SequenceEditor** editor = _sequenceEditors.get(sequence);
	SequenceEditor* se;
	if (*editor != null)
		se = *editor;
	else {
		se = new SequenceEditor(this, danceEditor, sequence);
		_sequenceEditors.insert(sequence, se);
	}
	if (se->owner())
		se->owner()->select(se);
	else
		_secondary->select(se);
	return se;
}

SequenceEditor* DanceFrame::edit(Sequence* sequence) {
	SequenceEditor** editor = _sequenceEditors.get(sequence);
	SequenceEditor* se;
	if (*editor != null)
		se = *editor;
	else
		return null;
	if (se->owner())
		se->owner()->select(se);
	else
		_secondary->select(se);
	return se;
}

PlayListEditor* DanceFrame::edit(DanceEditor* danceEditor, int index, PlayList* playList) {
	PlayListEditor** editor = _playListEditors.get(playList);
	PlayListEditor* pe;
	if (*editor != null)
		pe = *editor;
	else {
		pe = new PlayListEditor(this, danceEditor, playList);
		_playListEditors.insert(playList, pe);
	}
	if (pe->owner())
		pe->owner()->select(pe);
	else
		_secondary->select(pe);
	return pe;
}

DefinitionEditor* DanceFrame::edit(Definition* definition) {
	DefinitionEditor** editor = _definitionEditors.get(definition);
	DefinitionEditor* de;
	if (*editor != null)
		de = *editor;
	else {
		Grammar* grammar = definition->grammar();
		de = new DefinitionEditor(this, grammar == defaultDefinitions ? _defaultEditor : _myDefinitionsEditor, definition);
		_definitionEditors.insert(definition, de);
	}
	if (de->owner())
		de->owner()->select(de);
	else
		_secondary->select(de);
	return de;
}

FormationEditor* DanceFrame::edit(Formation* formation) {
	FormationEditor** editor = _formationEditors.get(formation);
	FormationEditor* fe;
	if (*editor != null)
		fe = *editor;
	else {
		Grammar* grammar = formation->grammar();
		fe = new FormationEditor(this, grammar == defaultDefinitions ? _defaultEditor : _myDefinitionsEditor, formation);
		_formationEditors.insert(formation, fe);
	}
	if (fe->owner())
		fe->owner()->select(fe);
	else
		_primary->select(fe);
	return fe;
}

DesignatorEditor* DanceFrame::edit(Designator* designator) {
	DesignatorEditor** editor = _designatorEditors.get(designator);
	DesignatorEditor* de;
	if (*editor != null)
		de = *editor;
	else {
		Grammar* grammar = designator->grammar();
		de = new DesignatorEditor(this, grammar == defaultDefinitions ? _defaultEditor : _myDefinitionsEditor, designator);
		_designatorEditors.insert(designator, de);
	}
	if (de->owner())
		de->owner()->select(de);
	else
		_secondary->select(de);
	return de;
}

void DanceFrame::refreshSubEditors(GrammarEditor* parent) {
	for (map<Definition, DefinitionEditor*>::iterator i = _definitionEditors.begin(); i.valid();i.next()) {
		DefinitionEditor* de = *i;
		if (de->parent() == parent)
			de->refreshView();
	}
	for (map<Formation, FormationEditor*>::iterator i = _formationEditors.begin(); i.valid();i.next()) {
		FormationEditor* fe = *i;
		if (fe->parent() == parent)
			fe->refreshView();
	}
	for (map<Designator, DesignatorEditor*>::iterator i = _designatorEditors.begin(); i.valid();i.next()) {
		DesignatorEditor* de = *i;
		if (de->parent() == parent)
			de->refreshView();
	}
	_preferencesEditor->freshenPreferences();
}

void DanceFrame::refreshSequenceEditor(Sequence* sequence) {
	for (map<Sequence, SequenceEditor*>::iterator i = _sequenceEditors.begin(); i.valid(); i.next()) {
		SequenceEditor* se = *i;
		if (se->sequence() == sequence) {
			se->updateAllCalls();
			return;
		}
	}
}

void DanceFrame::start(Performance* performance) {
	_performances.push_back(performance);
	performance->show();
}

void DanceFrame::dismiss(Performance* performance) {
	for (int i = 0; i < _performances.size(); i++)
		if (_performances[i] == performance) {
			_performances.remove(i);
			break;
		}
	delete performance;
}

void DanceFrame::animate(SequenceEditor* editor, int index, Sequence* sequence) {
	if (_animator == null) {
		_animator = new Animator();
		_animator->selected.addHandler(this, &DanceFrame::onSelect, (display::TabManager*)_animator);
		_animator->closed.addHandler(this, &DanceFrame::onClose, (display::TabManager*)_animator);
	}
	_animator->setup(editor, index, sequence);
	if (_animator->owner())
		_animator->owner()->select(_animator);
	else
		selectStartup(_animator, _animatorStart / 10);
}

void DanceFrame::drillDown(int index, Sequence* sequence) {
	if (_drillDown == null) {
		_drillDown = new Browser();
		_drillDown->selected.addHandler(this, &DanceFrame::onSelect, (display::TabManager*)_drillDown);
		_drillDown->closed.addHandler(this, &DanceFrame::onClose, (display::TabManager*)_drillDown);
	}
	if (_drillDown->owner())
		_drillDown->owner()->select(_drillDown);
	else
		selectStartup(_drillDown, _drillDownStart / 10);
	const Stage* s = sequence->stages()[index];
	if (s->stepCount() > 1 || s->step(0)->tiles().size() == 1)
		_drillDown->show(new PlanSeed(_drillDown, s));
	else
		_drillDown->show(new TileSeed(_drillDown, s->step(0), s->call()));
}

void DanceFrame::setStatus(const string& text) {
	_status->set_value(text);
}

display::RootCanvas* DanceFrame::rootCanvas() const {
	return _status->rootCanvas(); 
}

void DanceFrame::onIdle() {
	if (_animator) {
		if (_animator->playing())
			return;
	}
	do {
		if (!_libraryEditor->calculateOneSequence()) {
			bool fixedOne = false;
			for (int i = 0; i < _danceEditors.size(); i++) {
				if (_danceEditors[i]->calculateOneSequence()) {
					fixedOne = true;
					break;
				}
			}
			if (!fixedOne)
				return;
		}
	} while (!display::messagesWaiting());
}

int DanceFrame::preference(display::TabbedGroup* group, display::TabManager* manager) {
	if (manager->owner() != null)
		group = manager->owner();
	int base = 0;
	// Tens digit is the last owner (if now closed)
	if (group == _primary)
		base = 10;
	else if (group == _secondary)
		base = 20;
	// Ones digit is the current owner
	if (manager->owner() == _primary)
		base += 1;
	else if (manager->owner() == _secondary)
		base += 2;
	return base;
}

void DanceFrame::selectStartup(display::TabManager* manager, int tab) {
	tab %= 10;			// only look at the low-order value (start-up state).
	if (tab == 1)
		_primary->select(manager);
	else if (tab == 2)
		_secondary->select(manager);
}

void DanceFrame::onSelect(display::TabManager* manager) {
	int tab = preference(null, manager);
	if (manager == _defaultEditor) {
		if (_defaultStart == tab)
			return;
		_defaultStart = tab;
	} else if (manager == _myDefinitionsEditor) {
		if (_myDefinitionsStart == tab)
			return;
		_myDefinitionsStart = tab;
	} else if (manager == _libraryEditor) {
		if (_libraryStart == tab)
			return;
		_libraryStart = tab;
	} else if (manager == _preferencesEditor) {
		if (_preferencesStart == tab)
			return;
		_preferencesStart = tab;
	} else if (manager == _animator) {
		if (_animatorStart == tab)
			return;
		_animatorStart = tab;
	} else if (manager == _drillDown) {
		if (_drillDownStart == tab)
			return;
		_drillDownStart = tab;
	}

	onClose(manager->owner(), manager);
}

void DanceFrame::onClose(display::TabbedGroup* group, display::TabManager* manager) {
	if (_shuttingDown)
		return;
	string s;

	s.printf("a%02db%02dd%02dl%02dm%02dp%02d", _animator ? preference(group, _animator) : _animatorStart,
		_drillDown ? preference(group, _drillDown) : _drillDownStart,
		preference(group, _defaultEditor),
		preference(group, _libraryEditor),
		preference(group, _myDefinitionsEditor),
		preference(group, _preferencesEditor));
	setPreference("tabs", s);
}

void DanceFrame::exitIfClean() {
	bool dirty = false;
	if (_libraryEditor->needsSave() ||
		_defaultEditor->needsSave() ||
		_myDefinitionsEditor->needsSave() ||
		_preferencesEditor->needsSave())
		dirty = true;
	else {
		for (int i = 0; i < _danceEditors.size(); i++)
			if (_danceEditors[i]->needsSave()) {
				dirty = true;
				break;
			}
	}
	if (dirty) {
		int a = display::messageBox(null, "Unsaved work - save it?", "Warning", MB_YESNOCANCEL|MB_ICONQUESTION);
		if (a == IDYES) {
			if (!save()) {
				warningMessage("Could not save");
				return;
			}
		} else if (a == IDCANCEL)
			return;
	}
	exit(0);
}

void DanceFrame::clearStatus() {
	if (_status->value().size())
		_status->set_value("");
}

void DanceFrame::onHelp(display::FunctionKey, display::ShiftState) {
	if (!launchUrl(_status->rootCanvas(), global::dataFolder + "/help/SetsInMotion.html"))
		warningMessage("Could not launch Help URL");
}

Seed::~Seed() {
}

void Seed::hidePage() {
}

bool Seed::purge(const void* handle) {
	return false;
}

PlanSeed::~PlanSeed() {
	delete _scrollerHandler;
}

display::Canvas* PlanSeed::page() {
	if (_page == null) {
		display::Canvas* callText = termCanvas(_plan->call());
		display::Grid* g = new display::Grid();
			g->cell(callText);
			for (int i = 0; i < _plan->stepCount(); i++) {
				const Step* s = _plan->step(i);
				g->row();
				g->cell(new Picture(s->start(), 16));
				g->row();
				g->cell(new display::Spacer(3, new display::Border(2, new display::Spacer(5, stepCanvas(s)))));
			}
			if (_plan->final()) {
				g->row();
				g->cell(new Picture(_plan->final(), 16));
			}
			if (_plan->failed()) {
				g->row();
				display::Label* cause = new display::Label(_plan->cause()->text());
				cause->set_textColor(&display::red);
				g->cell(cause);
			}
			g->row();
			g->cell();
			g->cell();
		g->complete();
		display::ScrollableCanvas* c = new display::ScrollableCanvas();
		c->setBackground(&display::editableBackground);
		c->append(g);
		_scrollerHandler = new display::ScrollableCanvasHandler(c);
		_page = c;
	}
	return _page;
}

bool PlanSeed::purge(const void* handle) {
	return _plan->inStage((Stage*)handle);
}

TileSeed::~TileSeed() {
	delete _scrollerHandler;
}

display::Canvas* TileSeed::page() {
	if (_page == null) {
		display::Canvas* callText = termCanvas(_call);
		display::Grid* g = new display::Grid();
			g->cell(callText);
			g->row();
			if (_step->start())
				g->cell(new Picture(_step->start(), 16));
			else
				g->cell();
			g->row();
			display::Grid* tiles = new display::Grid();
				for (int i = 0; i < _step->tiles().size(); i++) {
					const Tile* t = _step->tiles()[i];

					display::Canvas* c = new Picture(t->plan()->start(), 16);

					if (!t->active())
						c = new display::Spacer(2, new display::Border(1, c));
					tiles->cell(new display::Filler(c));
				}
				tiles->row();
				for (int i = 0; i < _step->tiles().size(); i++) {
					const Tile* t = _step->tiles()[i];

					if (t->active()) {
						display::Grid* c = new display::Grid();
							c->cell(drillDownButton(t->plan()));
							c->cell(termCanvas(t->plan()->call()));
						c->complete();
						tiles->cell(new display::Spacer(3, new display::Border(2, new display::Spacer(5, c))));
					} else
						tiles->cell();
				}
				tiles->row();
				for (int i = 0; i < _step->tiles().size(); i++) {
					const Tile* t = _step->tiles()[i];

					if (t->active() && t->plan()->final())
						tiles->cell(new display::Filler(new Picture(t->plan()->final(), 16)));
					else
						tiles->cell();
				}
			tiles->complete();
			g->cell(new display::Spacer(10, 0, 0, 0, new display::Border(1, new display::Spacer(3, tiles))));
			resolutionPane(g, _step);
			if (_step->final()) {
				g->row();
				g->cell(new Picture(_step->final(), 16));
			}
			if (_step->failed()) {
				g->row();
				display::Label* cause = new display::Label(_step->cause()->text());
				cause->set_textColor(&display::red);
				g->cell(cause);
			}
			g->row();
			g->cell();
			g->cell();
		g->complete();
		display::ScrollableCanvas* c = new display::ScrollableCanvas();
		c->setBackground(&display::editableBackground);
		c->append(g);
		_scrollerHandler = new display::ScrollableCanvasHandler(c);
		_page = c;
	}
	return _page;
}

bool TileSeed::purge(const void* handle) {
	return _step->inStage((Stage*)handle);
}

void DancingSeed::onDrillDownPlan(display::Bevel* b, const Plan* plan) {
	_drillDown->show(new PlanSeed(_drillDown, plan));
}

void DancingSeed::onDrillDownTiles(display::Bevel* b, const Step* step, const Anything* call) {
	_drillDown->show(new TileSeed(_drillDown, step, call));
}

display::Canvas* DancingSeed::drillDownButton(const Plan* p) {
	display::Bevel* b = new display::Bevel(2, new display::Label("?"));
	b->setBackground(&display::buttonFaceBackground);
	display::ButtonHandler* bh = new display::ButtonHandler(b, 0);
	bh->click.addHandler(this, &PlanSeed::onDrillDownPlan, p);
	return new display::Filler(b);
}

display::Canvas* DancingSeed::drillDownButton(const Step* step, const Anything* call) {
	display::Bevel* b = new display::Bevel(2, new display::Label("?"));
	b->setBackground(&display::buttonFaceBackground);
	display::ButtonHandler* bh = new display::ButtonHandler(b, 0);
	bh->click.addHandler(this, &PlanSeed::onDrillDownTiles, step, call);
	return new display::Filler(b);
}

display::Canvas* DancingSeed::stepCanvas(const Step* s) {
	display::Grid* step = new display::Grid();
		if (typeid(*s) == typeid(PrimitiveStep)) {
			const PrimitiveStep* ps = (const PrimitiveStep*)s;
			display::Grid* g;
			if (s->tiles().size() != 0)
				g = new display::Grid();
			else
				g = step;
			const Primitive* p = ps->primitive();
			g->cell(new display::Label(primitiveNames[p->index()]));
			if (ps->parent()->variables().size()) {
				g->row();
				g->cell(new display::Spacer(3, new display::Border(1, new display::Spacer(5, callVariables(ps->parent())))));
			}
			if (s->tiles().size() != 0) {
				g->complete();
				display::Grid* g2 = new display::Grid();
					g2->cell(drillDownButton(ps->tiles()[0]->plan()));
					g2->cell(g);
				g2->complete();
				step->cell(g2);
			}
			resolutionPane(step, s);
		} else {
			if (s->specialLabel().size() > 0) {
				step->cell(new display::Label(s->specialLabel()));
				step->row();
			}
			const Anything* call = s->displayCall();
			if (call) {
				display::Grid* g = new display::Grid();
					if (s->tiles().size() > 1)
						g->cell(drillDownButton(s, call));
					else
						g->cell();
					g->cell(termCanvas(call));
				g->complete();
				step->cell(g);
				step->row();
			}
			if (s->tiles().size() == 1) {
				step->cell(partCanvas(s));
				resolutionPane(step, s);
			} else if (call == null) {
				display::Grid* g = new display::Grid();
					g->cell(drillDownButton(s, call));
					g->cell(new display::Label("..."));
				g->complete();
				step->cell(g);
			}
		}
		if (s->failed()) {
			step->row();
			display::Label* cause = new display::Label(s->cause()->text());
			cause->set_textColor(&display::red);
			step->cell(cause);
		}
	step->complete();
	return step;
}

display::Canvas* DancingSeed::partCanvas(const Step* step) {
	const Plan* p = step->tiles()[0]->plan();
	display::Grid* g = new display::Grid();
		g->row();
		for (int i = 0; i < p->stepCount(); i++) {
			const Step* s = p->step(i);
			display::Canvas* step = stepCanvas(s);
			g->row();
			g->cell(new display::Spacer(3, new display::Border(2, new display::Spacer(5, step))));
			if (i < p->stepCount() - 1 && s->final()) {
				g->row();
				g->cell(new Picture(s->final(), 16));
			}
		}
		g->row();
		g->cell();
		g->cell();
	g->complete();
	return new display::Spacer(10, 0, 0, 0, g);
}

void DancingSeed::resolutionPane(display::Grid* g, const Step* s) {
	const Resolution& r = s->resolution();
	if (r.isSimple())
		return;
	display::Grid* bb = new display::Grid();
		bb->cell(new display::Label(string("Before Breathing (") + r.breatheActionLabel() + "):"));
		if (r.beforeBreathing()) {
			bb->row();
			bb->cell(new Picture(r.beforeBreathing(), 16));
		}
		bb->row();
		display::Grid* outcomes = new display::Grid();
			for (int i = 0; i < r.outcomeCount(); i++) {
				display::Canvas* c = new Picture(r.outcome(i), 16);
				outcomes->cell(new display::Filler(new display::Spacer(2, new display::Border(1, c))));
			}
		outcomes->complete();
		bb->cell(outcomes);
	bb->complete();
	g->row();
	g->cell(new display::Spacer(10, 5, 0, 0, new display::Border(1, new display::Spacer(2, bb))));
}

static display::Canvas* termCanvas(const Term* t) {
	if (t == null)
		return new display::Label("...");
	if (typeid(*t) == typeid(Anyone)) {
		const Anyone* a = (const Anyone*)t;
		return new display::Label(a->label());
	} else if (typeid(*t) == typeid(Anypivot)) {
		const Anypivot* p = (const Anypivot*)t;
		return new display::Label(pivotNames[p->pivot()]);
	} else if (typeid(*t) == typeid(Anydirection)) {
		const Anydirection* d = (const Anydirection*)t;
		return new display::Label(directionNames[d->direction()]);
	} else if (typeid(*t) == typeid(Anything)) {
		const Anything* c = (const Anything*)t;
		display::Label* callText;
		if (c->primitive())
			callText = new display::Label(primitiveNames[c->primitive()->index()]);
		else if (c->definition()) {
			callText = new display::Label(c->definition()->callText());
		} else {
			callText = new display::Label("<no primitive or definition>");
			callText->set_textColor(&display::red);
		}
		display::Grid* g = new display::Grid();
			g->cell(callText);
			if (c->variables().size()) {
				g->row();
				g->cell(new display::Spacer(3, new display::Border(1, new display::Spacer(5, callVariables(c)))));
				g->cell();
			}
			g->row();
			g->cell();
		g->complete();
		return g;
	} else if (typeid(*t) == typeid(Integer)) {
		const Integer* it = (const Integer*)t;
		return new display::Label(string(it->value()));
	} else if (typeid(*t) == typeid(Word)) {
		return new display::Label(t->spelling());
	} else if (typeid(*t) == typeid(Fraction)) {
		const Fraction* f = (const Fraction*)t;
		string s;
		if (f->denominator() == 0)
			return new display::Label("$until_home");
		if (f->numerator() == 0)
			s = string(f->whole());
		else {
			s.printf("%d/%d", f->numerator(), f->denominator());
			if (f->whole() != 0)
				s = string(f->whole()) + "+" + s;
		}
		return new display::Label(s);
	} else
		return new display::Label("xxx");
}

static display::Canvas* callVariables(const Anything* call) {
	display::Grid* variables = new display::Grid();
		for (int i = 0; i < call->variables().size(); i++) {
			if (i > 0)
				variables->row();
			variables->cell(new display::Label("$" + string(i + 1), 2));
			variables->cell(termCanvas(call->variables()[i]));
		}
	variables->complete();
	return variables;
}

Browser::Browser() {
	_body = null;
	_current = null;
	_deletingHandler = deletingStage.addHandler(this, &Browser::onDeletingStage);
}

Browser::~Browser() {
	deletingStage.removeHandler(_deletingHandler);
	delete _body;
}

void Browser::show(Seed* s) {
	_future.deleteAll();
	_future.push_back(s);
	onForward(null);
}

bool Browser::deleteTab() {
	return false;
}

const char* Browser::tabLabel() {
	return "Call Analysis";
}

display::Canvas* Browser::tabBody() {
	if (_body == null) {
		display::Bevel* backButton = new display::Bevel(2, new display::Label("Back"));
		_backHandler = new display::ButtonHandler(backButton, 0);
		_backHandler->click.addHandler(this, &Browser::onBack);
		display::Bevel* forwardButton = new display::Bevel(2, new display::Label("Forward"));
		_forwardHandler = new display::ButtonHandler(forwardButton, 0);
		_forwardHandler->click.addHandler(this, &Browser::onForward);
		display::Grid* navBar = new display::Grid();
			navBar->cell(new display::Spacer(4, backButton));
			navBar->cell(new display::Spacer(4, forwardButton));
			navBar->cell();
		navBar->complete();
		navBar->setBackground(&display::buttonFaceBackground);
		display::Grid* g = new display::Grid();
			g->cell(navBar);
			g->row();
			_pageHolder = g->cell();
		g->complete();
		_body = g;
	}
	return _body;
}

void Browser::onDeletingStage(const Stage* stage) {
	for (int j = _history.size() - 1; j >= 0; j--)
		if (_history[j]->purge(stage)) {
			Seed* s = _history[j];
			_history.remove(j);
			delete s;
		}
	for (int j = _future.size() - 1; j >= 0; j--)
		if (_future[j]->purge(stage)) {
			Seed* s = _future[j];
			_future.remove(j);
			delete s;
		}

	if (_current && _current->purge(stage)) {
		Seed* s = _current;
		hideCurrentPage();
		delete s;
		_current = null;
	}
	// If we purged the current page, back up to any remaining history pages
	if (_current == null) {
		if (_history.size())
			onBack(null);
		else
			onForward(null);
	}
}

void Browser::onBack(display::Bevel* back) {
	if (_history.size()) {
		Seed* prev = _history[_history.size() - 1];
		_history.resize(_history.size() - 1);
		if (_current) {
			_future.push_back(_current);
			hideCurrentPage();
		}
		_current = prev;
		showCurrentPage();
	}
}

void Browser::onForward(display::Bevel* forward) {
	if (_future.size()) {
		Seed* next = _future[_future.size() - 1];
		_future.resize(_future.size() - 1);
		if (_current) {
			_history.push_back(_current);
			hideCurrentPage();
		}
		_current = next;
		showCurrentPage();
	}
}

void Browser::hideCurrentPage() {
	if (_current) {
		_pageHolder->setCanvas(null);
		_current->hidePage();
		_current = null;
	}
}

void Browser::showCurrentPage() {
	if (_current)
		_pageHolder->setCanvas(_current->page());
}

bool loadDefinitions() {
	string defsFile = getPreference("definitions");
	if (defsFile.size() == 0)
		defsFile = global::dataFolder + "/dance/calls.cdf";
	delete defaultDefinitions;
	defaultDefinitions = new Grammar();
	return defaultDefinitions->read(defsFile);
}

bool loadMyDefinitions() {
	delete myDefinitions;
	myDefinitions = new Grammar();
	string myDefsFile = getPreference("myDefinitions");
	if (myDefsFile.size() == 0) {
		myDefsFile = findUserFolder() + "/myDefinitions.cdf";
		// Not having a 'my definitions' file in the default location means its empty
		if (!fileSystem::exists(myDefsFile)) {
			myDefinitions->touch();
			myDefinitions->setFilename(myDefsFile);
			return true;
		}
	}
	return myDefinitions->read(myDefsFile);
}

bool loadLibrary() {
	delete library;
	string libFile = getPreference("library");
	if (libFile.size() == 0) {
		libFile = findUserFolder() + "/library.dnc";
		// Not having a library at the default location means its empty
	}
	library = new Library(libFile);
	if (getPreference("library").size() == 0 && !fileSystem::exists(libFile))
		return true;
	else
		return library->read();
}

static void writeFont(display::Font* font, const char* preference) {
	setPreference(preference, font->pack());
}

static void writeMeasure(display::Measure* measure, const char* preference) {
	setPreference(preference, measure->pack());
}

static bool loadFont(display::Font* font, const char* preference) {
	script::Object* o;

	o = getObjectPreference(preference);
	if (o && !font->unpack(o))
		return false;
	font->familyChanged.addHandler(writeFont, font, preference);
	font->size.changed.addHandler(writeFont, font, preference);
	font->bold.changed.addHandler(writeFont, font, preference);
	font->italic.changed.addHandler(writeFont, font, preference);
	font->underlined.changed.addHandler(writeFont, font, preference);
	return true;
}

static bool loadMeasure(display::Measure* measure, const char* preference) {
	script::Object* o;

	o = getObjectPreference(preference);
	if (o && !measure->unpack(o))
		return false;
	measure->changed.addHandler(writeMeasure, measure, preference);
	return true;
}

bool loadPrinterFonts() {
	static bool succeeded = false;

	if (succeeded)
		return true;
	if (!loadFont(&printerCallFont, "pCallFont"))
		return false;
	if (!loadFont(&printerDateFont, "pDateFont"))
		return false;
	if (!loadFont(&printerCommentFont, "pCommentFont"))
		return false;
	if (!loadFont(&printerVersionFont, "pVersionFont"))
		return false;
	if (!loadFont(&printerLevelFont, "pLevelFont"))
		return false;
	if (!loadMeasure(&topMargin, "pTopMargin"))
		return false;
	if (!loadMeasure(&headerMargin, "pHeaderMargin"))
		return false;
	if (!loadMeasure(&leftMargin, "pLeftMargin"))
		return false;
	if (!loadMeasure(&rightMargin, "pRightMargin"))
		return false;
	if (!loadMeasure(&bottomMargin, "pBottomMargin"))
		return false;
	succeeded = true;
	return true;
}

bool loadPerformanceFonts() {
	static bool succeeded = false;

	if (succeeded)
		return true;
	if (!loadFont(&performanceCallFont, "perfCallFont"))
		return false;
	if (!loadFont(&performanceOtherFont, "perfOtherFont"))
		return false;
	succeeded = true;
	return true;
}

void loadStateFile() {
	string stateFile = findUserFolder() + "/state.ets";
	vector<script::Atom*> atoms;

	script::Parser* p = script::Parser::load(stateFile);
	if (p != null) {
		p->content(&atoms);
		if (p->parse()) {
			for (int i = 0; i < atoms.size(); i++) {
				script::Atom* a = atoms[i]->get("tag");
				if (a == null)
					continue;
				string tag = a->toString();
				if (tag == "preferences") {
					preferences = atoms[i];
					atoms[i] = null;
				}
			}
		} else
			warningMessage("Saved program state is garbled, ignored.");
		delete p;
	}
	atoms.deleteAll();
}

bool saveStateFile() {
	string userFolder = findUserFolder();
	string stateFile = userFolder + "/state.ets";

	if (!fileSystem::ensure(userFolder)) {
		warningMessage("Could not create application data folder at " + userFolder);
		return false;
	}
	FILE* out = fileSystem::createTextFile(stateFile);
	if (out == null) {
		warningMessage("Could not save state");
		return false;
	}
	if (preferences) {
		preferences->put("version", new script::String(PREFERENCES_VERSION_STRING));
		fprintf(out, "%s\n", preferences->toSource().c_str());
	}
	fclose(out);
	return true;
}

string findUserFolder() {
	const char* cp = getenv("HOME");
	if (cp == null) {
		cp = getenv("APPDATA");
		if (cp == null)
			cp = global::dataFolder.c_str();
	}
	return fileSystem::absolutePath(string(cp) + "/sets_in_motion");
}

string getPreference(const string& prop) {
	if (preferences) {
		script::Atom* a = preferences->get(prop);
		if (a)
			return a->toString();
	}
	return "";
}

bool setPreference(const string& prop, const string& value) {
	if (preferences == null) {
		preferences = new script::Object();
		preferences->put("tag", new script::String("preferences"));
	}
	preferences->put(prop, new script::String(value));
	return saveStateFile();
}

script::Object* getObjectPreference(const string& prop) {
	if (preferences) {
		script::Atom* a = preferences->get(prop);
		if (a && typeid(*a) == typeid(script::Object))
			return (script::Object*)a;
	}
	return null;
}

bool setPreference(const string& prop, script::Object* value) {
	if (preferences == null) {
		preferences = new script::Object();
		preferences->put("tag", new script::String("preferences"));
	}
	preferences->put(prop, value);
	return saveStateFile();
}

void clearMemory() {
	if (danceFrame)
		danceFrame->shutdown();
	delete danceWindow;
	delete danceFrame;
	delete preferences;
	delete myDefinitions;
	delete defaultDefinitions;
	delete library;
	delete Group::home;
}

}  // namespace dance
