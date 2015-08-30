#pragma once
#include "../common/dictionary.h"
#include "../common/event.h"
#include "../common/file_system.h"
#include "../common/string.h"
#include "../common/vector.h"

namespace dance {

class Action;
class Anyone;
class Anything;
class BuiltIn;
class Context;
class Dance;
class DanceObject;
class Dancer;
class Group;
class Definition;
class DefinitionsContext;
class Designator;
class Formation;
class Grammar;
class GrammarObject;
class Interval;
class ParseObject;
class ParseState;
class Part;
class PartStep;
class Pattern;
class PatternClosure;
class PhraseMeaning;
class Plan;
class PlayList;
class Point;
class Rectangle;
class Sequence;
class Spot;
class Stage;
class Step;
class Synonym;
class Term;
class Token;
class Variant;
class VariantTile;
class Word;

#define VERSION "SiMv1.0alpha4"

typedef int Level;			// 0 = undefined 1 = unspecified 2-N defined levels INT_MAX = maximum level

const int ERROR_LEVEL = 0;				// The 'level' string in the datafile was unrecognizable
const int NO_LEVEL = 1;					// 'level' 1 is reserved for no level specified.

typedef	int	beats;			// arbitrary measure of time used for call timing

enum TileAction {
	TILE_ALL,
	TILE_ANY_WHO_CAN,
	TILE_WITH_PHANTOMS
};

enum DanceType {
	D_UNSPECIFIED,			// On a dance, means don't enforce on the sequences.
							// Not allowed on a sequence.
	D_2COUPLE,
	D_4COUPLE,
	D_6COUPLE,
	D_HEXAGONAL
};

enum PositionType {
	NO_POSITION,			// not found in any pattern
	ACTIVE,					// a
	ACTIVE_BOY,				// b
	ACTIVE_GIRL,			// g
	ACTIVE_DESIGNATED,		// d
	ACTIVE_NONDESIGNATED,	// n
	CENTER,					// c
	END,					// e
	VERY_CENTER,			// C
	VERY_END,				// E

	INACTIVE,				// i

		// Syntactic tokens

	END_OF_ROW,				// end of string

		// Not dancers, but location and/or relationship markers

	EMPTY,					// .
	SAME_ROW,				// -
	SAME_COLUMN,			// |
	TO_THE_LEFT,			// <
	TO_THE_BACK,			// ^
	WRAP,					// wraps to beginning of ring

	FIRST_POSITION = ACTIVE,
	LAST_POSITION = WRAP,
};

const int DANCER_POSITIONS = VERY_END + 1;

enum Facing {
	RIGHT_FACING,	// > caller's right side of hall
	BACK_FACING,	// ^ back of hall
	LEFT_FACING,	// < caller's left side of hall
	FRONT_FACING,	// v front of hall
	HEAD_FACING,	// | front or back
	SIDE_FACING,	// - left or right
	ANY_FACING,		// ? any value
};

enum Geometry {
	UNSPECIFIED_GEOMETRY,	// any geometry
	GRID,					// dancing on the square grid
	HEXAGONAL,				// dancing on the hexagonal grid (for future extension)
	RING,					// dancing in a ring of 8
};

enum DancerSet {
	NONE,			// Not actually a kind of 'ANYONE' match but used to distinguish non-matching positions
	CENTERS,
	ENDS,
	VERY_CENTERS,
	VERY_ENDS,
	LAST_ACTIVE,	// The active dancers in the last call of this interval
	OTHERS,			// The inactive dancers in the last call of this interval
	LEADERS,
	TRAILERS,
	HEADS,
	SIDES,
	BOYS,
	GIRLS,
	BEAUS,
	BELLES,
	FACING_ACROSS,
	FACING_ALONG,
	IN_FACING,
	OUT_FACING,
	DANCER_MASK,	// The exact dancers are named from the dancer mask 
	L_AND_R,
	L_OR_R,
	L_XOR_R,
	NOT_L,
};

enum TokenType {
	ERROR_TOKEN,
	UNKNOWN_WORD,
	WORD,
	INTEGER,
	ORDINAL,
	COUPLE_NUMBER,
	SLASH,
	DASH,
	PLUS,
	ASTERISK,
	NOT,
	NOT_EQ,
	EQ_EQ,
	GTR,
	GTR_EQ,
	LSS,
	LSS_EQ,
	OR,
	AND,
	XOR,
	REMAINDER,
	LPAREN,
	RPAREN,
	COMMA,
	EQ,
	END_OF_STRING,

	// Not produced directly by parser

	R_L,
	P_C,
	ANYONE,
	ANYTHING,
	ANYCALL,
	ANYDIRECTION,
	FORMATION,
	FRACTION,
	VARIABLE,
	DANCER_NAME,
	PRIMITIVE,

	FINAL_PARTIAL,

};

enum SequenceStatus {
	SEQ_UNCHECKED,
	SEQ_FAILED,
	SEQ_UNRESOLVED,
	SEQ_READY
};

class Dance {
public:
	Dance(const string& label, const string& filename);

	~Dance();

	bool read();

	Sequence* newSequence();

	PlayList* newPlayList();

	void append(Sequence* sequence);
	// Test API
	bool runAll(bool allowUnresolved, const Grammar* grammar);

	bool save();

	void print();

	void setFilename(const string& filename);

	const string& filename() const { return _filename; }

	const string& label() const { return _label; }

	const vector<Sequence*>& sequences() const { return _sequences; }

	const vector<PlayList*>& playLists() const { return _playLists; }

private:
	string _levelName;
	Level _level;
	DanceType _danceType;
	vector<Sequence*> _sequences;
	vector<PlayList*> _playLists;
	string _filename;
	string _label;
};

class PlayList : public Dance {
public:
	PlayList(Dance* parent) : Dance("", "") {
		_parent = parent;
		created = time(null);
		modified = 0;
		_level = NO_LEVEL;
	}

	time_t			created;
	time_t			modified;
	string			comment;

	fileSystem::TimeStamp lastChecked() const;

	void include(vector<Sequence*>& sequences);

	void fetchSequences(vector<Sequence*>* output) const;

	Dance* parent() const { return _parent; }

	Level level() const { return _level; }
private:
	Dance*			_parent;
	Level			_level;
	vector<Sequence*> _sequences;
};

class Library : public Dance {
public:
	Library(const string& filename) : Dance("Library", filename) {}

private:
	class LibraryContext {
	public:
		LibraryContext() {
			sequence = null;
		}

		int		line;
		string	text;
		const string*	filename;
		Sequence*		sequence;
	};
};

class Sequence {
public:
	time_t			created;
	time_t			modified;
	string			comment;
	string			createdWith;
	SequenceStatus	status;

	Sequence(Dance* dance);

	~Sequence();

	void setLevelName(const string& name);

	void setLevel(int value);

	void setCall(int index, const string& call);

	void setLastChecked(fileSystem::TimeStamp t);

	void append(const string& text);

	void appendNotes(const string& text);
	// Test API
	bool run(bool allowUnresolved, const Grammar* grammar);

	bool current(const Grammar* grammar);

	bool updateStages(const Grammar* grammar);

	bool updateStatus(const Grammar* grammar);

	const vector<const Stage*>& stages() { return _stages; }

	void clearStages();

	void print();

	void write(FILE* fp);

	beats durationThru(int index) const;

	beats duration() const {
		return durationThru(INT_MAX);
	}

	bool resolved() const;

	const vector<string>& text() const { return _text; }

	const vector<string>& notes() const { return _notes; }

	fileSystem::TimeStamp lastChecked() const { return _lastChecked; }

	Dance* dance() const { return _dance; }

	Level level() const { return _level; }

	DanceType danceType() const { return _danceType; }

private:
	Dance* _dance;
	fileSystem::TimeStamp _lastChecked;
	vector<string>	_text;
	vector<string> _notes;
	vector<const Stage*> _stages;
	Level			_level;
	DanceType		_danceType;
};

class VariantTile {
public:
	VariantTile() {}

	VariantTile(const Variant* variant, const Pattern* pattern) {
		this->variant = variant;
		this->pattern = pattern;
	}

	const Variant* variant;
	const Pattern* pattern;

	int compare(const VariantTile* other) const;
};

class ParseState {
public:
	const Term*	term;
	int			matchState;
	int			missState;
	bool		printed;		
};

class Grammar {
	friend GrammarObject;
	friend ParseObject;
	friend BuiltIn;
public:
	Grammar();

	~Grammar();

	bool read(const string& filename);

	void setFilename(const string& filename);

	void touch();

	void compact();

	bool write(const string& filename) const;

	void print();

	void printStateMachines() const;

	void printStateMachineStats() const;

	void addDefinition(Definition* definition);

	void removeDefinition(Definition* definition);

	void addFormation(Formation* formation);

	void removeFormation(Formation* formation);

	void changeFormationName(Formation* formation, const string& newName);

	void addDesignator(Designator* designator);

	void removeDesignator(Designator* designator);

	void compileStateMachines() const;

	void define(const string& key, const Term* term);

	bool parsePartial(TokenType goalSymbol, const string& text, Level level, vector<string>* output) const;

	const Anything* parse(const Group* dancers, const string& text, bool inDefinition, const Anything* call, Context* context, const Plan* variantPlan) const;

	const Anyone* parseAnyone(const Group* dancers, const string& text, const Anything* call, Context* context, const Plan* variantPlan, const Term** local) const;

	const Anyone* parseDesignatingPattern(const Group* dancers, const string& text, bool inDefinition, const Anything* call, Context* context) const;

	const Anyone* parseDesignatorExpression(const string& expression, const vector<const Term*>* variables, Context* context) const;

	int matchR_L(const vector<Token>& tokens, int tIndex) const;

	int matchP_C(const vector<Token>& tokens, int tIndex) const;

	const Anything* matchAnycall(bool inDefinition, const vector<Token>& tokens, int tIndex, bool fullMatch, int* matched, vector<int>* partialStates, Context* context) const;

	const Term* stateMachine(int initialState, bool inDefinition, const vector<Token>& tokens, int tIndex, bool fullMatch, int* matched, vector<int>* partialStates, Context* context) const;

	const Anything* matchPrimitive(const vector<Token>& tokens, int tIndex, int* matched, Context* context) const;

	const Formation* formation(const string& key) const;

	const Synonym* synonym(const string& key) const;

	void setBackupGrammar(Grammar* g);

	const Term* lookup(const string& key) const { return *_words.get(key); }

	bool error() const { return _error; }

	const Term* and() const { return _and; }

	const vector<VariantTile>& leadersTrailers() const;

	const vector<const Pattern*>& centersEnds() const;

	const vector<VariantTile>& partners() const;

	const vector<VariantTile>& couples() const;

	fileSystem::TimeStamp lastChanged() const;

	const vector<Synonym*>& synonyms() const { return _synonyms; }

	const vector<Definition*>& definitions() const { return _definitions; }

	const vector<Formation*>& formations() const { return _formations; }

	const vector<Designator*>& designators() const { return _designators; }

	const string& filename() const { return _filename; }

	Stage* termStorage() const { return _termStorage; }

	Event			changed;

private:
	class Reduction {
	public:
		TokenType				type;
		const string*			production;
		bool					definitionsOnly;
		const PhraseMeaning*	meaning;
	};

	bool collectReductions(int state, const Token& finalPartial, Context* context, vector<string>* output) const;

	void collectReductionsAnon(int state, vector<string>* output, Context* context) const;

	void partialMatch(TokenType type, const string& text, Context* context, vector<string>* output) const;

	bool tokenize(const Group* dancers, 
				  const string& text, 
				  bool inDefinition, 
				  const vector<const Term*>* variables, 
				  Context* context,
				  const Plan* variantPlan,
				  vector<Token>& tokens,
				  Token* finalPartial) const;

	void anyone(const string& word, DancerSet value, Level level = NO_LEVEL);

	void includeBackDefinitions(const Grammar* master) const;

	void includeProduction(TokenType initialStateProduction, const string& production, const Definition* def) const;

	void includeProduction(const string& production, const Designator* des) const;

	int buildProductionTables(TokenType* initialStateProduction, const string& production, bool* definitionsOnly) const;

	void recurseState(int i, int indent) const;

	void printState(int i, int indent) const;

	void processText(DefinitionsContext& ctx);

	void processLine(DefinitionsContext& ctx, int startLine, int endLine);

	void processDesignators(DefinitionsContext& ctx, int start);

	void processDiagrams(DefinitionsContext& ctx, int start);

	void writeContents(FILE* fp) const;

	const Term* keyword(const string& word);

	int matchPrimitiveParameters(Anything* instance, const vector<Token>& tokens, int tIndex, Context* context) const;

	fileSystem::TimeStamp _lastChanged;
	DanceType _danceType;
	mutable dictionary<const Term*>	_words;
	vector<Synonym*> _synonyms;
	vector<Definition*>	_definitions;
	vector<Formation*> _formations;
	vector<Designator*> _designators;
	bool _error;
	dictionary<Synonym*> _synonymDictionary;
	dictionary<Formation*> _formationDictionary;

	mutable Pattern* _couple;
	mutable vector<VariantTile> _leadersTrailers;
	mutable vector<VariantTile> _partners;
	mutable vector<VariantTile> _couples;
	mutable vector<const Pattern*> _centersEnds;

	Grammar*			_backupGrammar;
	void*				_changeHandler;

	mutable vector<ParseState> _parseStates;
	mutable vector<int> _initialState;
	mutable vector<int> _suffixes;
	mutable vector<Reduction> _reductions;
	const Term* _and;

	string _filename;
	Stage* _termStorage;			// allocator for definition productions
};

const int NULL_STATE = -1;
const int REDUCE_TOS = -1;

class PhraseMeaning {
public:
	PhraseMeaning(Grammar* grammar) {
		_grammar = grammar;
		_created = time(null);
		_modified = 0;
	}

	virtual const string& label() const = 0;

	virtual Level level() const = 0;

	void setCreated(const string& value);

	void setModified(const string& value);

	void setModified(time_t value);

	Grammar* grammar() const { return _grammar; }

	time_t created() const { return _created; }

	time_t modified() const { return _modified; }

private:
	Grammar* _grammar;						// The grammar, if any, that contains this definition ...
	time_t _created;
	time_t _modified;
};

class Definition : public PhraseMeaning {
	friend DanceObject;
public:
	Definition(Grammar* grammar) : PhraseMeaning(grammar) {
		_dance = null;
		_level = 0;
		_tilesBuilt = false;
	}

	Definition(Dance* dance) : PhraseMeaning(null) {
		_dance = dance;
		_level = 0;
		_tilesBuilt = false;
	}

	~Definition();

	void setLevel(const string& levelName);

	int compare(const Definition* other) const;

	int addProduction();

	bool setProduction(int i, const string& text);

	bool compileProduction(int i);

	void addPattern(const string& text);

	void action(const string& text);

	void addCompound();

	bool compoundWho(bool finishTogether, const string& text);

	bool compoundWhat(bool anyWhoCan, const string& text);

	void nextPart(const string& text);

	void setVariantPrecedence(const string& text);

	void setVariantLevel(const string& text);

	void nextVariant();

	void insertVariant(int index, Variant* v);

	void deleteVariant(int index);

	void removeLastVariant();

	void setName(const string& name);

	void verify(const Grammar* grammar);

	void write(FILE* fp);

	void print() const;

	void clearTiles() { _tilesBuilt = false; }

	const vector<VariantTile>& tiles() const;

	const Variant* variant(int i) const { return i < _variants.size() ? _variants[i] : null; }

	const string& name() const { return _name; }

	virtual const string& label() const;

	string callText() const;

	const vector<string>& productions() const { return _productions; }

	const string& levelName() const { return _levelName; }

	const vector<Variant*>& variants() const { return _variants; }

	virtual Level level() const;

private:
	static string _emptyLabel;

	string _levelName;
	Level _level;
	vector<string>	_productions;
	vector<Variant*> _variants;
	mutable vector<VariantTile> _tiles;
	mutable bool _tilesBuilt;
	string _name;
	Dance* _dance;							// else, the imported Dance file that contains this definition.
											// One will be null
};

const int PRECEDENCE_SHIFT = 3;				// precedence varies from 0-10, and up to 7 tiles can be added together
											// by converting the precedence as (1 << (precedence * PRECEDENCE_SHIFT))
											// we guarantee that any tile with better precedence trumps all tiles
											// with lesser precedence, and any tiling with the most number of highest
											// precedence tiles wins.
class Variant {
public:
	Variant(Definition* definition) {
		_definition = definition;
		_level = NO_LEVEL;							// Variants default to empty (no) level
		_precedence = 0;
	}

	~Variant();

	void addPattern(const string& text) {
		_patterns.push_back(text);
		_recognizers.push_back(null);
	}

	void setPattern(int index, const string& text);

	int compare(const Variant* variant) const;

	void action(const string& text);

	void addCompound();

	bool compoundWho(bool finishTogether, const string& text);

	bool compoundWhat(bool anyWhoCan, const string& text);

	void addPart(Part* p) { _parts.push_back(p); }

	void removeLastPart() { _parts.resize(_parts.size() - 1); }

	void insertPart(int index, Part* p);

	void deletePart(int index);

	Part* nextPart(const string& text);

	void setPrecedence(const string& text);

	void setLevel(const string& text);

	void verify(const Grammar* grammar);

	void write(FILE* fp);

	bool testAnyFormations(const Group* d, Context* context, const Pattern** matched, const Group** orientedGroup, const Anything* call, const Step* step) const;

	bool testAnyPhantomFormations(const Group* d, Context* context, const Pattern** matched, const Group** orientedGroup, const Anything* call, const Step* step) const;

	bool construct(Plan* p, const Anything* call, Context* context, TileAction tileAction) const;

	Level effectiveLevel() const;

	Definition* definition() const { return _definition; }

	int partCount() const { return _parts.size(); }

	Part* part(int i) const { return _parts[i]; }

	const vector<string>& patterns() const { return _patterns; }

	const vector<const Pattern*>& recognizers() const { return _recognizers; }

	const string& levelName() const { return _levelName; }

	Level level() const { return _level; }

	int precedence() const { return _precedence; }

private:
	Definition* _definition;
	string _levelName;
	Level _level;
	int _precedence;				// 0 = normal
	vector<string> _patterns;
	vector<const Pattern*> _recognizers;
	vector<Part*>	_parts;
};

class Part {
public:
	Part(const Variant* variant, const string& text) {
		_variant = variant;
		setRepeat(text);
	}

	~Part();

	void setRepeat(const string& text);

	void addAction(const string& text);

	void addAction(Action* action);

	bool compoundWho(bool finishTogether, const string& text);

	bool compoundWhat(bool anyWhoCan, const string& text);

	void setAction(int i, const string& text);

	void setAction(int i, Action* newAction);

	int repeatCount(Plan* p, const Anything* call, Context* context) const;

	void write(FILE* fp);

	const Variant* variant() const { return _variant; }

	int actions() const { return _actions.size(); }

	Action* action(int i) const { return _actions[i]; }

	const string& repeat() const { return _repeat; }

private:
	const Variant* _variant;
	vector<Action*> _actions;
	string _repeat;
};

class Action {
public:
	Action(Part* parent) {
		_parent = parent;
	}

	virtual ~Action() {}

	virtual Step* construct(PartStep* step, Context* context, TileAction tileAction) const = 0;

	virtual bool noop() const = 0;

	virtual void write(FILE* fp) const = 0;

	virtual void print(int indent) const = 0;

	Part* parent() const { return _parent; }

private:
	Part*	_parent;
};

class SimpleAction : public Action {
public:
	SimpleAction(Part* parent, const string& text) : Action(parent) {
		_action = text;
	}

	void set_action(const string& text) { _action = text; }

	virtual Step* construct(PartStep* step, Context* context, TileAction tileAction) const;

	virtual bool noop() const;

	virtual void write(FILE* fp) const;

	virtual void print(int indent) const;

	const string& action() const { return _action; }

private:
	string			_action;
};

class Track {
public:
	Track() {
		finishTogether = false;
		anyWhoCan = false;
	}

	bool noop() const;

	string			who;
	string			what;
	bool			finishTogether;
	bool			anyWhoCan;
};

class CompoundAction : public Action {
public:
	CompoundAction(Part* parent) : Action(parent) {
	}

	~CompoundAction();

	Track* addTrack();

	void setWho(int track, const string& who);

	void setWhat(int track, const string& what);

	void setFinishTogether(int track, bool finishTogether);

	void setAnyWhoCan(int track, bool anyWhoCan);

	bool noop(int track);

	virtual Step* construct(PartStep* step, Context* context, TileAction tileAction) const;

	virtual bool noop() const;

	virtual void write(FILE* fp) const;

	virtual void print(int indent) const;

	const vector<Track*>& tracks() const { return _tracks; }

private:
	vector<Track*>	_tracks;
};

class Spot {
public:
	Spot() {}

	Spot(PositionType p, Facing f) {
		position = p;
		facing = f;
	}

	PositionType	position;
	Facing			facing;

	static Spot empty;
};

struct PhantomMatch {
	int	x;
	int	y;
	const Dancer* dancer;
};

class Formation {
public:
	Formation(Grammar* grammar, const string& name, Geometry geometry) {
		_grammar = grammar;
		_geometry = geometry;
		_name = name;
		_maxPositions = 0;
		_dancerCount = 0;
		_significantCount = 0;
		for (int i = 0; i < DANCER_POSITIONS; i++)
			_dancerTypes[i] = 0;
		_rotationalSymmetry = 0;
		_created = time(null);
		_modified = 0;
		_biasX = 0;
		_biasY = 0;
		_firstDancerColumn = -1;
		_firstDancerRow = -1;
		_firstSignificantColumn = -1;
	}

	void setName(const string& name);

	void setCreated(const string& value);

	void setModified(const string& value);

	void setModified(time_t value);

	void setGeometry(Geometry value);

	void boundingBox(Rectangle* result) const;

	int compare(const Formation* other) const;

	bool match(const Group* dancers, const PatternClosure* closure) const;

	unsigned extract(const Group* inFormation, const Group* dancers, PositionType pos, PositionType altPos) const;

	unsigned matchSome(const Group* dancers, int startWith, const PatternClosure* closure) const;

	Group* matchWithPhantoms(const Group* dancers, const PatternClosure* closure) const;

	bool row(const string& text, int start, int end);
	/*
	 * Calculate the rotational symmetry of this formation:
	 *	1 = no radial symmetry
	 *	2 = 2-fold radial symmetry
	 *	4 = 4-fold radial symmetry
	 */
	int rotationalSymmetry() const;

	void compact();

	void write(FILE* fp) const;

	void print();

	bool hasCentersOrEnds() const;

	const Group* recenter(Group* d, Context* context) const;

	const Spot& spot(int x, int y) const;

	void setSpot(int x, int y, const Spot& s);

	bool blocked(int x, int y) const;

	time_t created() const { return _created; }

	time_t modified() const { return _modified; }

	Geometry geometry() const { return _geometry; }

	const string& name() const { return _name; }

	int dancerCount() const { return _dancerCount; }

	Grammar* grammar() const { return _grammar; }
private:
	void calculateSymmetry() const;

	bool has4FoldSymmetry() const;

	void nextSignificantSpot(int* row, int* column) const;

	Group* matchNext(vector<PhantomMatch>& matching, int spot, int row, int column, const Group* dancers, int startWith, const PatternClosure* closure) const;

	Grammar* _grammar;
	time_t _created;
	time_t _modified;
	Geometry _geometry;
	string _name;
	vector<string> _rows;
	vector<vector<Spot>> _spotRows;
	int _maxPositions;
	int _dancerCount;
	int _significantCount;
	int _firstSignificantColumn;
	int _firstDancerRow;
	int _firstDancerColumn;
	int _dancerTypes[DANCER_POSITIONS];
	mutable int _rotationalSymmetry;		// n-fold rotational symmetry
											// 0 = symmetry has not been calculated
											// 1 = not symmetric, try all four rotations
											// 2 = symmetric when turned 180 degrees, try only identity and rotate90
											// 4 = symmetric at all 90 degree rotations, try only identity
	// _biasX and _biasY are used to allow expansion of a formation during editing
	// As rows or columns are added to the FRONT of the formation, these two variables keep track
	// of where the original formation started.  Thus, if we see a setSpot call with a negative x or y,
	// and those are numerically smaller than the current biases, we must insert rows/columns to compensate
	int _biasX;							
	int _biasY;
};

class Designator : public PhraseMeaning {
public:
	Designator(Grammar* grammar) : PhraseMeaning(grammar) {
		_level = NO_LEVEL;
	}

	Designator(Grammar* grammar, const Anyone* a, const string& word, Level level);

	void write(FILE* fp) const;

	bool processLine(DefinitionsContext& ctx, int* start);

	virtual const string& label() const;

	int compare(const Designator* other) const;

	int addPhrase();
	
	void setPhrase(int index, const string& s);

	void setLevel(const string& level);

	void setExpression(const string& expression);

	const Anyone* compile(vector<const Term*>& terms, Context* context) const;

	const vector<string>& phrases() const { return _phrases; }

	virtual Level level() const;

	const string& expression() const { return _expression; }

private:
	time_t _created;
	time_t _modified;
	vector<string> _phrases;
	Level _level;
	string _levelString;
	string _expression;
};

class Synonym {
public:
	Synonym(const string& synonym, const string& value) {
		_synonym = synonym;
		_value = value;
	}

	int compare(const Synonym* other) const {
		return _synonym.compare(&other->_synonym);
	}

	void write(FILE* fp);

	void print();

	const string& synonym() const { return _synonym; }

	const string& value() const { return _value; }

private:
	string	_synonym;
	string	_value;
};

extern bool loadState;					// Defaults to true.  When true, loads saved state file to restore windows.

const bool isDancer(PositionType position);

const bool isSignificantSpot(PositionType position);

void loadSavedState();

}  // namespace dance
