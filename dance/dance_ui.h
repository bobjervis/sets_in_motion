#include "../common/map.h"
#include "../display/grid.h"
#include "../display/tabbed_group.h"
#include "../display/undo.h"
#include "dance.h"

namespace script {

class Object;

}  // namespace script

namespace display {

class Cell;
class DropDown;
class Field;
class Font;
class FontSelector;
class Measure;
class PrintJob;
class Outline;
class OutlineHandler;
class ScrollableCanvas;
class ScrollableCanvasHandler;
class Timer;
class Window;

}  // namespace display

namespace dance {

class ActionEditor;
class Animator;
class Browser;
class Dance;
class DanceEditor;
class DanceFileEditor;
class Dancer;
class Group;
class DefinitionEditor;
class Designator;
class DesignatorEditor;
class Formation;
class FormationEditor;
class Grammar;
class GrammarEditor;
class LibraryEditor;
class Meter;
class PageLayout;
class Performance;
class PerformanceTab;
class Picture;
class PlayList;
class PlayListEditor;
class PreferencesEditor;
class Sequence;
class SequenceEditor;

extern const char* PREFERENCES_VERSION_STRING;

extern Grammar* defaultDefinitions;
extern Grammar* myDefinitions;

extern display::Window* danceWindow;

extern display::Font labelFont;
extern display::Font editCallTextFont;
extern display::Font performCallTextFont;
extern display::Font phraseTextFont;
extern display::Font actionTextFont;
extern display::Font formationNameFont;

	// Performance Fonts

extern display::Font performanceCallFont;
extern display::Font performanceOtherFont;

	// Printing fonts

extern display::Font printerCallFont;
extern display::Font printerCommentFont;
extern display::Font printerVersionFont;
extern display::Font printerDateFont;
extern display::Font printerLevelFont;
extern display::Measure topMargin;
extern display::Measure headerMargin;
extern display::Measure leftMargin;
extern display::Measure rightMargin;
extern display::Measure bottomMargin;

extern const char* definitionsLock;
extern const char* myDefinitionsLock;


void launch(int argc, char**argv);

class DanceFrame : public display::FileManager {
public:
	DanceFrame();

	~DanceFrame();

	void shutdown();

	void bind(display::RootCanvas* c);

	void newDance(display::point p, display::Canvas* target);

	void openDefinitions(display::point p, display::Canvas* target);

	void openMyDefinitions(display::point p, display::Canvas* target);

	void openMyLibrary(display::point p, display::Canvas* target);

	void openPreferences(display::point p, display::Canvas* target);

	virtual bool matches(const string& filename) const;

	virtual bool openFile(const string& filename);

	virtual void extendContextMenu(display::ContextMenu* menu);

	bool save();

	SequenceEditor* edit(DanceEditor* danceEditor, int index, Sequence* sequence);

	PlayListEditor* edit(DanceEditor* danceEditor, int index, PlayList* playList);

	SequenceEditor* edit(Sequence* sequence);

	DefinitionEditor* edit(Definition* definition);

	FormationEditor* edit(Formation* formation);

	DesignatorEditor* edit(Designator* designator);

	void refreshSubEditors(GrammarEditor* parent);

	void refreshSequenceEditor(Sequence* sequence);

	void start(Performance* performance);

	void dismiss(Performance* performance);

	void animate(SequenceEditor* editor, int index, Sequence* sequence);

	void drillDown(int index, Sequence* sequence);

	void setStatus(const string& text);

	void onHelp(display::FunctionKey, display::ShiftState);

	display::RootCanvas* rootCanvas() const;

	GrammarEditor* defaultEditor() const { return _defaultEditor; }

	GrammarEditor* myDefinitionsEditor() const { return _myDefinitionsEditor; }

private:
	void onIdle();

	void onSelect(display::TabManager* manager);

	void onClose(display::TabbedGroup* group, display::TabManager* manager);

	int preference(display::TabbedGroup* group, display::TabManager* manager);

	void selectStartup(display::TabManager* manager, int tab);

	void exitIfClean();

	void clearStatus();

	display::TabbedGroup*								_primary;
	display::TabbedGroup*								_secondary;
	display::TabbedGroupHandler*						_primaryHandler;
	display::TabbedGroupHandler*						_secondaryHandler;
	GrammarEditor*										_defaultEditor;
	int													_defaultStart;
	GrammarEditor*										_myDefinitionsEditor;
	int													_myDefinitionsStart;
	LibraryEditor*										_libraryEditor;
	int													_libraryStart;
	PreferencesEditor*									_preferencesEditor;
	int													_preferencesStart;
	vector<DanceFileEditor*>							_danceEditors;
	vector<Performance*>								_performances;
	map<Sequence, SequenceEditor*>						_sequenceEditors;
	map<PlayList, PlayListEditor*>						_playListEditors;
	map<Definition, DefinitionEditor*>					_definitionEditors;
	map<Formation, FormationEditor*>					_formationEditors;
	map<Designator, DesignatorEditor*>					_designatorEditors;
	Animator*											_animator;
	int													_animatorStart;
	Browser*											_drillDown;
	int													_drillDownStart;
	display::Label*										_status;
	Event2<display::FunctionKey, display::ShiftState>	_helpFunction;
	display::VerticalSliderHandler*						_sliderHandler;
	bool												_shuttingDown;
	void*												_idleHandler;
};

class PreferencesEditor : public display::TabManager {
public:
	PreferencesEditor(DanceFrame* frame);

	~PreferencesEditor();

	virtual bool deleteTab();

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

	virtual bool needsSave();

	virtual bool save();

	void freshenPreferences();
private:
	void onSelected();

	void onUnselected();

	void onSpeedChanged();

	void onLockChanged(data::Boolean* variable, GrammarEditor* editor);

	DanceFrame* _frame;
	display::Canvas* _body;
	display::Field* _firstField;
	display::FontSelector* _performanceCallFont;
	display::FontSelector* _performanceOtherFont;
	display::FontSelector* _printerCallFont;
	display::FontSelector* _printerCommentFont;
	display::FontSelector* _printerDateFont;
	display::FontSelector* _printerVersionFont;
	display::FontSelector* _printerLevelFont;
	PageLayout* _pageLayout;
	data::Integer	_speed;
	data::Boolean	_definitionsLock;
	data::Boolean	_myDefinitionsLock;
	display::ScrollableCanvasHandler* _scrollerHandler;
	display::ToggleHandler* _defsToggleHandler;
	display::ToggleHandler* _myDefsToggleHandler;
};

class GrammarEditor : public display::TabManager {
public:
	GrammarEditor(DanceFrame* frame, const string& lockPreference, Grammar* grammar);

	~GrammarEditor();

	virtual bool deleteTab();

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

	virtual void extendContextMenu(display::ContextMenu* menu);

	virtual bool needsSave();

	virtual bool save();

	void touchDefinition(Definition* definition);

	void touchFormation(Formation* formation);

	void touchDesignator(Designator* designator);

	void onFunctionKey(display::FunctionKey fk, display::ShiftState ss);

	void editDefinition(Definition* definition);

	void editFormation(Formation* formation);

	void editDesignator(Designator* designator);

	display::OutlineItem* newDefinitionOI(Definition* definition);

	void addDefinitionOI(display::OutlineItem* oi);

	display::OutlineItem* newFormationOI(Formation* formation);

	void addFormationOI(display::OutlineItem* oi);

	display::OutlineItem* newDesignatorOI(Designator* designator);

	void addDesignatorOI(display::OutlineItem* oi);

	const string& lockPreference() const { return _lockPreference; }

	display::UndoStack& undoStack() { return _undoStack; }

	DanceFrame* frame() const { return _frame; }

	Grammar* grammar() const { return _grammar; }

private:
	class DefinitionMapEntry {
	public:
		display::Label*		key;
		Definition*			definition;
	};

	class FormationMapEntry {
	public:
		display::Label*		key;
		Formation*			formation;
	};

	class DesignatorMapEntry {
	public:
		display::Label*		key;
		Designator*			designator;
	};

	void onGrammarChanged();

	void onDefinitionClick(display::MouseKeys mKeys, display::point p, display::Canvas* target, Definition* definition);

	void onFormationClick(display::MouseKeys mKeys, display::point p, display::Canvas* target, Formation* formation);

	void onDesignatorClick(display::MouseKeys mKeys, display::point p, display::Canvas* target, Designator* designator);

	void onUnlock(display::point p, display::Canvas* target);

	void onLock(display::point p, display::Canvas* target);

	void onDefinitionsOpenContextMenu(display::point p, display::Canvas* target);

	void onDefinitionOpenContextMenu(display::point p, display::Canvas* target, Definition* definition, display::OutlineItem* oi);

	void onFormationsOpenContextMenu(display::point p, display::Canvas* target);

	void onFormationOpenContextMenu(display::point p, display::Canvas* target, Formation* formation, display::OutlineItem* oi);

	void onDesignatorsOpenContextMenu(display::point p, display::Canvas* target);

	void onDesignatorOpenContextMenu(display::point p, display::Canvas* target, Designator* designator, display::OutlineItem* oi);

	void newDefinition(display::point p, display::Canvas* target);

	void deleteDefinition(display::point p, display::Canvas* target, Definition* definition, display::OutlineItem* oi);

	void newFormation(display::point p, display::Canvas* target);

	void deleteFormation(display::point p, display::Canvas* target, Formation* formation, display::OutlineItem* oi);

	void newDesignator(display::point p, display::Canvas* target);

	void deleteDesignator(display::point p, display::Canvas* target, Designator* designator, display::OutlineItem* oi);

	DanceFrame* _frame;
	Grammar*	_grammar;
	string		_lockPreference;

	display::Canvas* _body;
	display::Outline* _outline;
	display::OutlineHandler* _outlineHandler;
	display::OutlineItem* _synonyms;
	display::OutlineItem* _definitions;
	display::OutlineItem* _formations;
	display::OutlineItem* _designations;
	display::ScrollableCanvasHandler* _scrollerHandler;
	display::UndoStack _undoStack;
	vector<DefinitionMapEntry> _definitionMap;
	vector<FormationMapEntry> _formationMap;
	vector<DesignatorMapEntry> _designatorMap;
	bool _listening;
};

class GrammarElementEditor : public display::TabManager {
public:
	GrammarElementEditor(DanceFrame* frame, GrammarEditor* parent);

	virtual ~GrammarElementEditor();

	void rebuildView();

	void refreshView();

	void setModified(time_t modified);

	virtual display::Canvas* createView(bool editable) = 0;

	virtual bool deleteTab();

	virtual display::Canvas* tabBody();

	virtual void extendContextMenu(display::ContextMenu* menu);

	GrammarEditor* parent() const { return _parent; }

	display::Label* name() const { return _name; }

	display::Label* modifiedLabel() const { return _modifiedLabel; }

	display::RootCanvas* root() const { return _root; }

	void field(display::Label* label, display::Label* prior = null);

	void removeField(display::Label* label);

	void resetKeyFocus();

private:
	void onSelected();

	void onUnselected();

	void onFunctionKey(display::FunctionKey fk, display::ShiftState ss, display::Label* label);

	void composeView();

	void restoreKeyFocus();

	DanceFrame* _frame;
	GrammarEditor* _parent;
	display::Label* _name;
	display::Label* _modifiedLabel;
	display::Canvas* _body;
	display::Canvas* _view;
	display::ScrollableCanvas* _scroller;
	display::ScrollableCanvasHandler* _scrollerHandler;
	display::RootCanvas* _root;
	int _keyFocus;						// ordinal of the key focus field in tab order
	int _scrollX;						// value of the horizontal scroll bar
	int _scrollY;						// value of the vertical scroll bar
};

class DefinitionEditor : public GrammarElementEditor {
public:
	DefinitionEditor(DanceFrame* frame, GrammarEditor* parent, Definition* definition) : GrammarElementEditor(frame, parent) {
		_definition = definition;
	}

	void touchName();

	void touchLevel();

	void touchProduction(int production);

	void touchVariantLevel(Variant* v);

	void touchVariantPrecedence(Variant* v);

	void touchPattern(Variant* v, int pattern);

	void touchRepeat(Part* p);

	void touchAction(Part* p, int action);

	virtual const char* tabLabel();

	virtual display::Canvas* createView(bool editable);

	Definition* definition() const { return _definition; }

private:
	class RepeatMapEntry {
	public:
		display::Label*	label;
		Part*			part;
	};

	class FormationMapEntry {
	public:
		display::Label*	label;
		Variant*		variant;
		int				formation;
	};

	class LevelMapEntry {
	public:
		display::DropDown* label;
		Variant*		variant;
	};

	class PrecedenceMapEntry {
	public:
		display::DropDown* label;
		Variant*		variant;
	};

	void makeProductionField(display::Label* production, int i, void (DefinitionEditor::* func)(display::Label* label, int i));

	void makeFormationField(display::Label* formation, Variant* v, int j, void (DefinitionEditor::* func)(display::Label* label, Variant* v, int i));

	void onNameCommitted(display::Label* label);

	void onLevelChanged(display::DropDown* label);

	void onVariantLevelChanged(display::DropDown* label, Variant* v);

	void onVariantPrecedenceChanged(display::DropDown* label, Variant* v);

	void onProductionCommitted(display::Label* label, int i);

	void onNewProductionCommitted(display::Label* label, int i);

	void onFormationCommitted(display::Label* label, Variant* v, int i);

	void onNewFormationCommitted(display::Label* label, Variant* v, int i);

	void onRepeatCommitted(display::Label* label, Part* p);

	void addVariant(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void deleteVariant(display::MouseKeys mKeys, display::point p, display::Canvas* target, Variant* v);

	void swapVariants(display::MouseKeys mKeys, display::point p, display::Canvas* target, int firstIndex);

	void onNewPart(display::MouseKeys mKeys, display::point p, display::Canvas* target, Variant* v);

	void deletePart(display::MouseKeys mKeys, display::point p, display::Canvas* target, Variant* v, Part* part);

	Definition* _definition;
	vector<FormationMapEntry> _formationMap;
	vector<RepeatMapEntry> _repeatMap;
	vector<ActionEditor*> _actionMap;
	vector<display::Label*> _productionMap;
	vector<LevelMapEntry> _levelMap;
	vector<PrecedenceMapEntry> _precedenceMap;
	display::DropDown* _levelName;
};

class ActionEditor : public display::Canvas {
public:
	ActionEditor(DefinitionEditor* parent, Part* part, int action, bool editable) {
		_parent = parent;
		_part = part;
		_action = action;
		init(editable);
	}

	~ActionEditor();

	virtual display::dimension measure();

	void update();

	Part* part() const { return _part; }

	int action() const { return _action; }

private:
	class TrackMapEntry {
	public:
		display::Label*			who;
		display::Label*			what;
		display::Toggle*		finishTogether;
		display::ToggleHandler*	finishHandler;
		data::Boolean*			finishState;
		display::Toggle*		anyWhoCan;
		display::ToggleHandler*	anyHandler;
		data::Boolean*			anyState;
	};

	void init(bool editable);

	display::OutlineItem* addTrack();

	void deleteLastTrack();

	void updateMainLabel(const string& s);

	void updateTrackMap(CompoundAction* a);

	void onActionCommitted();

	void onMainLabelChanged();

	void onExpand(bool editable);

	void onCollapse();

	void onTrackChanged();

	void onWhoCommitted(int i);

	void onWhatCommitted(int i);

	void onAnyWhoCanChanged(int i);

	void onFinishTogetherChanged(int i);

	DefinitionEditor*			_parent;
	Part*						_part;
	int							_action;
	display::Outline*			_outline;
	display::OutlineHandler*	_outlineHandler;
	display::Label*				_mainLabel;
	bool						_showingMainLabel;
	vector<TrackMapEntry>		_trackMap;
};

class FormationEditor : public GrammarElementEditor {
public:
	FormationEditor(DanceFrame* frame, GrammarEditor* parent, Formation* formation) : GrammarElementEditor(frame, parent) {
		_formation = formation;
	}

	virtual const char* tabLabel();

	virtual display::Canvas* createView(bool editable);

	void touchName();

	void touchGeometry();

	void touchDiagram();

private:
	void onNameCommitted(display::Label* label);

	void onGeometryChanged();

	void onMouseMove(display::point p, display::Canvas* target);

	void onClick(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void onOpenContextMenu(display::point p, display::Canvas* target);

	void setPosition(display::point p, display::Canvas* target, int newValue);

	Formation* _formation;
	display::DropDown* _geometryName;
	Picture* _diagram;
};

class DesignatorEditor : public GrammarElementEditor {
public:
	DesignatorEditor(DanceFrame* frame, GrammarEditor* parent, Designator* designator) : GrammarElementEditor(frame, parent) {
		_designator = designator;
	}

	virtual const char* tabLabel();

	virtual display::Canvas* createView(bool editable);

	void touchLevel();

	void touchExpression();

	void touchPhrase(int index);

private:
	void makeProductionField(display::Label* production, int i, void (DesignatorEditor::* func)(display::Label* label, int i));

	void onLevelChanged(display::DropDown* label);

	void onProductionCommitted(display::Label* label, int i);

	void onExpressionCommitted();

	void onNewProductionCommitted(display::Label* label, int i);

	void onMouseMove(display::point p, display::Canvas* target);

	void onClick(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void onOpenContextMenu(display::point p, display::Canvas* target);

	vector<display::Label*> _phraseMap;
	display::Label* _expression;
	Designator* _designator;
	display::DropDown* _levelName;
};

class DanceEditor : public display::TabManager {
public:
	~DanceEditor();

	void touch(Sequence* sequence);

	void onFunctionKey(display::FunctionKey fk, display::ShiftState ss);

	bool startPrinting();

	void print(Sequence* sequence);

	void donePrinting();

	virtual bool deleteTab();

	virtual bool needsSave();

	virtual bool save();

	bool calculateOneSequence();

	display::UndoStack& undoStack() { return _undoStack; }

	DanceFrame* frame() const { return _frame; }

	Dance* dance() const { return _dance; }

protected:
	DanceEditor(Dance* dance, DanceFrame* frame);

	display::Canvas* commonTabBody(bool editPlayLists);

	void onSelected();

	void onAddSequence(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void onAddPlayList(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void grammarChanged();

	display::Canvas* _body;
private:
	class PlayListMapEntry {
	public:
		PlayList*			playList;
		display::Label*		level;
		display::Label*		comment;
		data::Boolean*		checked;
	};
	class SequenceMapEntry {
	public:
		Sequence*				sequence;
		display::Label*			status;
		display::Label*			level;
		display::Label*			comment;
		data::Boolean*			checked;
		display::ToggleHandler*	checkedHandler;
	};

	void showSequenceInfo(int i, Sequence* sequence);

	void showPlayListInfo(int i, PlayList* playList);

	void setSequenceStatus(display::Label* label, Sequence* sequence);

	void onOpenContextMenu(display::point p, display::Canvas* target);

	void onClick(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void onChecked(data::Boolean* b, int index);

	void onOpenPlayListContextMenu(display::point p, display::Canvas* target);

	void onPlayListClick(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void onPlayListChecked(data::Boolean* b, int index);

	int hitSequence(display::point p);

	int hitPlayList(display::point p);

	void editSequence(display::point p, display::Canvas* target);

	void editPlayList(display::point p, display::Canvas* target);

	void printSequence(display::point p, display::Canvas* target, Sequence* sequence);

	void printPlayList(display::point p, display::Canvas* target, PlayList* playList);

	void performSequence(display::point p, display::Canvas* target, Sequence* sequence);

	void performPlayList(display::point p, display::Canvas* target, PlayList* playList);

	void recheckSequence(display::point p, display::Canvas* target);

	void recheckAllSequences(display::point p, display::Canvas* target);

	void recheckPlayList(display::point p, display::Canvas* target);

	void recheckSequence(Sequence* sequence);

	void printSelected(display::point p, display::Canvas* target);

	void performSelected(display::point p, display::Canvas* target);

	DanceFrame* _frame;
	Dance* _dance;
	display::UndoStack _undoStack;
	display::Grid* _sequenceArea;
	display::Grid* _playListArea;
	display::Canvas* _commonBody;
	display::PrintJob* _printJob;
	vector<SequenceMapEntry> _sequenceMap;
	vector<PlayListMapEntry> _playListMap;
	bool _listening;
	display::ScrollableCanvasHandler* _scrollerHandler;
};

class PlayListEditor : public DanceEditor {
public:
	PlayListEditor(DanceFrame* frame, DanceEditor* parent, PlayList* playList) : DanceEditor(playList, frame) {
		_playList = playList;
		_parent = parent;
	}

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

private:
	PlayList*		_playList;
	DanceEditor*	_parent;
};

class LibraryEditor : public DanceEditor {
public:
	LibraryEditor(Library* library, DanceFrame* frame) : DanceEditor(library, frame) {
		_library = library;
	}

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

private:
	Library*	_library;
};

class DanceFileEditor : public DanceEditor {
public:
	DanceFileEditor(Dance* dance, DanceFrame* frame);

	~DanceFileEditor();

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

private:
	string			_label;
	display::Label* _nameLabel;
};
/*
 *	SequenceEditor
 *
 *	This editor must provide a surface on which you can create a sequence of
 *	square dance calls, constrained to be within a specific dance level.
 *
 *	Various annotations are allowed both on the sequence as a whole, and on each
 *	stage (one text line).
 *
 *	The body of the sequence is a column of text lines.  Text focus can tab between
 *	each line and across the general information fields (comment and level) at
 *	the top.
 *
 *	When a text line contains the text of a syntactically correct line, a drilldown
 *	button is displayed next to the text.  This will direct the user into the Call
 *	Analysis browser.
 *
 *	Whenever text focus reaches a specific line, or after any change to the selected
 *	line, a separate area below the calls being edited is updated with a relevant
 *	set of syntax option.
 *
 *	The order in which changes to the text are applied are immaterial.  Only the current
 *	state of the text, and the position of the cursor affect the syntax list.
 *
 *	The portion of the text to the left of the cursor forms a 'sentence prefix'.  This
 *	prefix is then matched to the governing grammar.  If the prefix does not end in a
 *	space, then the match is done both with and without the space.  Without a space, the
 *	final token must be recorded as partial if there is any possibility of a continuation
 *	that would be legal.
 *
 *	Each production that has been matched is listed below.  Double-clicking on one of the
 *	available productions then pastes the unmatched suffix of that production onto the
 *	current line and makes that new text the current selection.
 *
 *	If the pasted text contains non-terminal symbols, these must be treated differently.
 *	First, if a text line contains a non-terminal, the text should not be run as a call
 *	(by definition the call is incomplete).  Next, if the text cursor is placed over the 
 *	non-terminal symbol, the interaction changes.  The list of productions below is changed
 *	from the ANYTHING/ANYCALL list normally shown, to a list specific to the non-terminal.
 *
 *	Here are the non-terminals:
 *
 *		ANYTHING					A list of productions taken from the definitions lists
 *		ANYCALL						in the governing grammar(s).
 *
 *		ANYONE						A list of productions taken from the designators lists
 *									in the governing grammar(s).
 *
 *		ANYDIRECTION				in/out/left/right/as you are
 *
 *		R_L							right/left.
 *
 *		P_C							partner/corner.
 *
 *		ORDINAL						1st/2nd/3rd/4th/5th/6th/7th/8th/9th/10th
 *
 *		INTEGER						1/2/3/4/5/6/7/8/9/10
 *
 *		COUPLE_NUMBER				#1/#2/#3/#4/#5/#6
 *
 *		FRACTION					INTEGER, INTEGER / INTEGER, INTEGER and INTEGER / INTEGER
 *
 *	When you are typing in a partial match to one of these non-terminals, the text must be
 *	decorated with some kind of markup so that the display will clearly convey what you are doing.
 *
 *	If you select a production for a prefix that partially matches an embedded non-terminal,
 *	the text of the partial non-terminal is marked up exactly as if you had begun with the
 *	bare production and then typed in the partial text of that non-terminal.
 *
 *	This can lead to a phenomenon in which a line of text has been manually typed in, but no
 *	production has been selected.  Markup should then be applied whenever the string enters a
 *	state where all matching productions agree on a particular text run being a particular
 *	non-terminal.
 *
 *	For example, the prefix '1/2 t' could match '1/2 tag' or 'FRACTION ANYTHING'.  Note: the
 *	algorithm for generating the set of productions will automatically do one level of trailing
 *	recursive expansion of an ANYTHING or ANYCALL non-terminal if there is only one production
 *	ending in that token.
 */
class SequenceEditor : public display::TabManager {
public:
	SequenceEditor(DanceFrame* frame, DanceEditor* parent, Sequence* sequence);

	~SequenceEditor();

	void select(int index);

	void touchComment();

	void touchLevel();

	void touchCall(int index);

	void touchInsertDeleteCall(int startingAt);

	void touchModified(bool affectsOutcomes);

	void updateAllCalls();

	void updateCallData(int index);

	void resetKeyFocus();

	void restoreKeyFocus();

	virtual bool deleteTab();

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

	Sequence* sequence() const { return _sequence; }

private:
	class CallMapEntry {
	public:
		int						row;
		display::Cell*			statusCell;
		display::Label*			status;
		display::Cell*			drillDown;
		display::ButtonHandler*	drillDownHandler;
		display::Cell*			cause;
		display::Cell*			callCell;
		display::Label*			call;
		display::Cell*			notesCell;
		display::Label*			notes;
	};

	CallMapEntry* defineCall();

	void defineField(display::Label* c, int index);

	void clearField(display::Label* c);

	void updateCallList();

	void onSelected();

	void onGotKeyboardFocus(int index);

	void onAppearanceChanged(int index);

	void onOpenContextMenu(display::point p, display::Canvas* target);

	void onDrillDown(display::Bevel* target, int index);

	void onCallChanged(int index);					// index in stages array, not _callMap

	void onFunctionKey(display::FunctionKey fk, display::ShiftState ss, display::Label* label);

	void insertCall(display::point p, display::Canvas* target, int i);

	void deleteCall(display::point p, display::Canvas* target, int i);

	void onCommentChanged();

	void onLevelChanged();

	int hitCall(display::point p);

	DanceFrame* _frame;
	DanceEditor* _parent;
	Sequence* _sequence;
	display::Canvas* _body;
	display::Canvas* _view;
	display::RootCanvas* _root;
	display::Grid* _callArea;
	display::Canvas* _choicesList;
	display::ScrollableCanvasHandler* _choicesHandler;
	vector<CallMapEntry> _callMap;
	int _selectedCall;					// index in array of stages, NOT _callMap
	display::Label* _comment;
	display::Label* _modified;
	display::DropDown* _levelName;
	int _keyFocus;
	bool _selectedInProgress;
};

class Performance {
public:
	Performance(DanceFrame* frame, Dance* parent, vector<PlayList*>& playLists, vector<Sequence*>& sequences);

	~Performance();

	void show();

private:
	void onClose();

	DanceFrame*				_frame;
	Dance*					_parent;
	vector<PerformanceTab*>	_performanceTabs;
	display::Window*		_window;
	display::TabbedGroup*	_group;
	PlayList*				_allocated;
};


class PerformanceTab : public display::TabManager {
public:
	PerformanceTab(DanceFrame* frame, const string& label, PlayList* playList);

	~PerformanceTab();

	void display(int index);

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

	const string& label() const { return _label; }

	PlayList* playList() const { return _playList; }

private:
	void advance();

	void backOneSequence();

	void forwardOneSequence();

	void onSelected();

	void onCharacter(char c);

	void onFunctionKey(display::FunctionKey fk, display::ShiftState ss);

	DanceFrame*							_frame;
	string								_label;
	PlayList*							_playList;
	vector<Sequence*>					_sequences;
	int									_index;
	int									_call;
	display::Canvas*					_body;
	display::ScrollableCanvas*			_callArea;
	display::ScrollableCanvasHandler*	_callAreaHandler;
	vector<display::Label*>				_callMap;
	display::Label*						_level;
	display::Label*						_description;
};

class Seed {
public:
	Seed() {}

	virtual ~Seed();

	virtual void hidePage();

	virtual bool purge(const void* handle);

	virtual display::Canvas* page() = 0;
};

class DancingSeed: public Seed {
public:
	DancingSeed(Browser* drillDown) {
		_drillDown = drillDown;
		_page = null;
	}

	~DancingSeed() {
		delete _page;
	}

protected:
	display::Canvas* partCanvas(const Step* step);

	display::Canvas* stepCanvas(const Step* s);

	void resolutionPane(display::Grid* g, const Step* s);

	void onDrillDownPlan(display::Bevel* b, const Plan* plan);

	void onDrillDownTiles(display::Bevel* b, const Step* step, const Anything* call);

	display::Canvas* drillDownButton(const Plan* p);

	display::Canvas* drillDownButton(const Step* step, const Anything* call);

	Browser*			_drillDown;
	display::Canvas*	_page;
};

class PlanSeed : public DancingSeed {
public:
	PlanSeed(Browser* drillDown, const Plan* plan) : DancingSeed(drillDown) {
		_plan = plan;
		_scrollerHandler = null;
	}

	~PlanSeed();

	virtual display::Canvas* page();

	virtual bool purge(const void* handle);

private:
	const Plan*			_plan;
	display::ScrollableCanvasHandler* _scrollerHandler;
};

class TileSeed : public DancingSeed {
public:
	TileSeed(Browser* drillDown, const Step* step, const Anything* call) : DancingSeed(drillDown) {
		_step = step;
		_call = call;
		_scrollerHandler = null;
	}

	~TileSeed();

	virtual display::Canvas* page();

	virtual bool purge(const void* handle);

private:
	const Step*			_step;
	const Anything*		_call;
	display::ScrollableCanvasHandler* _scrollerHandler;
};

class Browser : public display::TabManager {
public:
	Browser();

	~Browser();

	void show(Seed* s);

	virtual bool deleteTab();

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

private:
	void onDeletingStage(const Stage* stage);

	void onBack(display::Bevel* back);

	void onForward(display::Bevel* forward);

	void hideCurrentPage();

	void showCurrentPage();

	display::Canvas*		_body;
	display::Cell*			_pageHolder;
	display::ButtonHandler*	_backHandler;
	display::ButtonHandler*	_forwardHandler;
	void*					_deletingHandler;
	Seed*					_current;
	vector<Seed*>			_history;			// Last page visited has highest index
	vector<Seed*>			_future;			// Last page visited has highest index
};

const int TICKS_PER_BEAT = 20;

class Animator : public display::TabManager {
public:
	Animator();

	~Animator();

	void setup(SequenceEditor* editor, int index, Sequence* sequence);

	virtual bool deleteTab();

	virtual const char* tabLabel();

	virtual display::Canvas* tabBody();

	bool playing() const { return _playing; }

private:
	void startTimer();

	void stopTimer();

	void onSelected();

	void onUnselected();

	void onDeletingStage(const Stage* stage);

	void onStartOverClick(display::Bevel* button);

	void onBackOneCall(display::Bevel* button);

	void onPlayPauseClick(display::Bevel* button);

	void onForwardOneCall(display::Bevel* button);

	void onTick();

	void onSpeedChanged(data::Integer* variable);

	void onMeterDrag(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	void onMeterDrop(display::MouseKeys mKeys, display::point p, display::Canvas* target);

	display::Timer* _tick;
	bool _playing;
	SequenceEditor* _editor;
	Sequence* _sequence;
	display::Canvas* _body;
	Meter* _meter;
	Picture* _picture;
	int		_index; 
	display::Label* _playPauseLabel;
	display::Bevel*	_startOverButton;
	display::Bevel*	_backOneCallButton;
	display::Bevel*	_playPauseButton;
	display::Bevel*	_forwardOneCallButton;
	data::Integer* _speed;
	display::RadioHandler* _speedRadioHandler;
	display::ButtonHandler* _startOverHandler;
	display::ButtonHandler* _backOneHandler;
	display::ButtonHandler* _forwardOneHandler;
	display::ButtonHandler* _playPauseHandler;
};

class Meter : public display::Canvas {
public:
	Meter(Picture* picture) {
		_editor = null;
		_picture = picture;
		_timeIndex = 0;
		_ticksPerBeat = 0;
		setSpeed(3);
	}

	void reset(int index);

	bool tick();

	void setup(Sequence* sequence);

	void setSpeed(int speed);

	void setEditor(SequenceEditor* editor) { _editor = editor; }

	void refreshEditor();

	virtual display::dimension measure();

	virtual void paint(display::Device* device);

	virtual bool bufferedDisplay(display::Device* device);

	int timeRemaining() const { return _duration * _ticksPerBeat - _timeIndex; }

	int ticksPerBeat() const { return _ticksPerBeat; }

	int timeIndex() const { return _timeIndex; }

private:
	SequenceEditor* _editor;
	Picture* _picture;
	int _timeIndex;
	int _ticksPerBeat;
	vector<beats> _stageDurations;
	beats _duration;
};

enum FigureZone {
	FZ_CENTER,					// The 'interior' of the figure
	FZ_LEFT,					// Left side of the figure
	FZ_RIGHT,					// Right side of the figure
	FZ_BACK,					// Back side (top)
	FZ_FRONT,					// Front side (bottom)
};

class PictureHit {
public:
	int			x;			// column of figure
	int			y;			// row of figure
	FigureZone	zone;		// where in this figure is the mouse
};

class Picture : public display::Canvas {
public:
	Picture() : _boundingBox(-8, 8, 8, -8) {
		init();
	}

	Picture(const Group* dancers, int minimumHalfSpot) {
		init();
		drawDancers(dancers);
		setHalfSpot(minimumHalfSpot);
	}

	Picture(const Formation* formation, int minimumHalfSpot) : _boundingBox(-10, 10, 10, -10) {
		init();
		_formation = formation;
		_formation->boundingBox(&_formationBox);
		setHalfSpot(minimumHalfSpot);
	}

	void drawDancers(const Group* dancers);

	void setHalfSpot(int pixels);

	void setup(Sequence* sequence);

	void setIndex(int index);

	void setPartial(double beats);

	void setSpeed(int beatsPerMinute) {
		_speed = beatsPerMinute;
	}

	void startPlaying();

	beats duration() const;

	bool hitTest(display::point p, PictureHit* output);

	virtual display::dimension measure();

	virtual void paint(display::Device* device);

	bool paintMotion(display::Device* device, int dancerIndex, const Motion* motion, double partial);

	const Motion* combineMotion(const Motion* motion, double partial, double* x, double* y, double* noseAngle);

	void paintFinishedMotion(display::Device* device, int dancerIndex, const Motion* m);

	virtual bool bufferedDisplay(display::Device* device);

	int index() const { return _index; }

	double partial() const { return _partial; }

private:
	void init() {
		_sequence = null;
		_speed = 60;
		_font = null;
		_start = null;
		_partial = 0;
		_minimumHalfSpot = 0;
		_ignoreRotation = false;
		_formation = null;
	}

	double getBaseAngle() const;

	void facingToAngles(Facing facing, double* noseAngle, double* secondNoseAngle);

	void drawDancer(display::Device* device, double baseAngle, const Dancer* dancer);

	void drawDancer(display::Device* device, Gender gender, int couple, double x, double y, double noseAngle, double secondNoseAngle);

	void drawFigure(display::Device* device, bool square, display::Color* color, const string& caption, double x, double y, 
						 double noseAngle, 
						 double secondNoseAngle);

	void drawNose(display::Device* device, int x, int y, double noseAngle);

	int		_index;
	double	_partial;
	Sequence* _sequence;
	int _minimumHalfSpot;
	const Group* _start;
	const Formation* _formation;
	Rectangle _boundingBox;		// Area covered by the image
	Rectangle _formationBox;	// Initial area covered by the formation
	int _speed;					// beats per minute
	display::BoundFont* _font;
	bool _ignoreRotation;

			// Used during paint only

	int _halfSpot;
	display::point _center;
	bool _painted[MAX_DANCERS];
	const Motion* _lastMotion[MAX_DANCERS];
};

class SpeedControl : public display::Grid {
public:
	SpeedControl(data::Integer* value);

	~SpeedControl();

private:
	display::RadioHandler* _speedRadioHandler;
};

SpeedControl* speedSelector(data::Integer* value);

bool saveStateFile();

bool loadDefinitions();

bool loadMyDefinitions();

bool loadLibrary();

bool loadPrinterFonts();

bool loadPerformanceFonts();

string getPreference(const string& prop);
/*
 *	setPreference
 *
 *	Sets the given preference property and then writes the state file.
 *	Returns false if the state file failed to save properly.
 */
bool setPreference(const string& prop, const string& value);
/*
 *	getObjectPreference
 */
script::Object* getObjectPreference(const string& prop);

bool setPreference(const string& prop, script::Object* object);

}  // namespace dance
