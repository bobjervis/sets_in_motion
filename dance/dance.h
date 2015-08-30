#pragma once
#include <time.h>
#include "../common/dictionary.h"
#include "../common/string.h"
#include "stored_data.h"

namespace dance {

class Anyone;
class Anything;
class Context;
class Dance;
class Dancer;
class Group;
class Definition;
class Explanation;
class Formation;
class Fraction;
class Grammar;
class Interval;
class Motion;
class MotionSet;
class PartStep;
class Primitive;
class PrimitiveStep;
class Stage;
class Step;
class Term;
class Tile;
class TileSearch;
class Variant;

extern vector<string> levels;
extern vector<string> precedences;
extern dictionary<int> levelValues;

extern const char* primitiveNames[];
extern const char* tokenNames[];
extern const char* genderNames[];
extern const char* dancerSetNames[];
extern const char* pivotNames[];
extern const char* directionNames[];
extern const char* dancerNames[];

extern char facingGlyphs[];

extern unsigned heads[];				// Indexed by DanceType
extern unsigned sides[];				// Indexed by DanceType

extern bool verboseOutput;
extern bool verboseBreathing;
extern bool verboseParsing;
extern bool verboseMatching;
extern bool showUI;

bool anyVerbose();

// Memory leak clean-up
void clearMemory();

const int MAX_DANCERS = 12;

const double PI = 3.14159;

Facing mirrorFacing(Facing facing);

Facing quarterLeft(Facing facing, int amount);

Facing quarterRight(Facing facing, int amount);

Facing reverse(Facing facing);

bool ambiguous(Facing facing);

enum Direction {
	D_AS_YOU_ARE,
	D_LEFT,
	D_RIGHT,
	D_FORWARD,
	D_BACK,
	D_IN,
	D_OUT,
	D_PROMENADE,
	D_REVERSE_PROMENADE,
	D_PARTNER,
	D_CORNER,
	D_ORIGINAL_PARTNER,
	D_ORIGINAL_CORNER,
	D_LAST,
	D_AWAY_FROM_PARTNER,
};

enum Pivot {
	P_CENTER,
	P_BOX_CENTER,
	P_SPLIT_CENTER,
	P_LINE_CENTER,
	P_INSIDE_HAND,
	P_OUTSIDE_HAND,
	P_LAST_HAND,
	P_LEFT_HAND,
	P_INSIDE_DANCER,
	P_OUTSIDE_DANCER,
	P_LEFT_DANCER,
	P_RIGHT_HAND,
	P_RIGHT_DANCER,
	P_HAND,
	P_SELF,
	P_NOSE,
	P_TAIL,
	P_INSIDE_SHOULDER,
	P_LEFT_TWO_DANCERS,
};

enum Rotation {
	UNROTATED,			// Normal alignment, no rotation.  Home spots are really
						// home
	ROTATED_1,			// Rotated counter-clockwize by arbitrary angle (PI/16).  No home
						// spots.
	ROTATED_2,
	ROTATED_3,
	DIAGONAL,			// Rotated counter-clockwise by 45 degrees (PI/4).  No home spots.
	ROTATED_5,			// Rotated counter-clockwize by arbitrary angle.  No home
						// spots.
	ROTATED_6,
	ROTATED_7,
};

enum Gender {
	GIRL,
	BOY,
	UNSPECIFIED_GENDER	// for phantoms
};

enum SpatialRelation {
	IN_FRONT,
	BEHIND,
	LEFT_OF,
	RIGHT_OF,
	OTHER
};

#define dancerIdx(couple, gender) ((((couple)-1) << 1) + (gender))
#define dancerMsk(couple, gender) (1 << dancerIdx(couple, gender))
#define genderOf(idx) ((idx) & 1)
#define coupleOf(idx) (((idx) >> 1) + 1)

const int COUPLE_1 = dancerMsk(1,GIRL)|dancerMsk(1,BOY);
const int COUPLE_2 = dancerMsk(2,GIRL)|dancerMsk(2,BOY);
const int COUPLE_3 = dancerMsk(3,GIRL)|dancerMsk(3,BOY);
const int COUPLE_4 = dancerMsk(4,GIRL)|dancerMsk(4,BOY);
const int COUPLE_5 = dancerMsk(5,GIRL)|dancerMsk(5,BOY);
const int COUPLE_6 = dancerMsk(6,GIRL)|dancerMsk(6,BOY);

	// Special designation for couple/tandem/siamese composite dancers

const int COUPLE_H = dancerMsk(7,GIRL)|dancerMsk(7,BOY);
const int COUPLE_S = dancerMsk(8,GIRL)|dancerMsk(8,BOY);

const int BOYS_MASK = dancerMsk(1,BOY)|
					  dancerMsk(2,BOY)|
					  dancerMsk(3,BOY)|
					  dancerMsk(4,BOY)|
					  dancerMsk(5,BOY)|
					  dancerMsk(6,BOY)|
					  dancerMsk(7,BOY)|
					  dancerMsk(8,BOY);
const int GIRLS_MASK = dancerMsk(1,GIRL)|
					   dancerMsk(2,GIRL)|
					   dancerMsk(3,GIRL)|
					   dancerMsk(4,GIRL)|
					   dancerMsk(5,GIRL)|
					   dancerMsk(6,GIRL)|
					   dancerMsk(7,GIRL)|
					   dancerMsk(8,GIRL);

int oppositeCouple(const Sequence* sequence, int couple);

enum Primitives {
	P_NOTHING,			// $nothing - has no effect on the dancers
	P_IN,				// $in - fails if there is no formation match
	P_ACTIVATE,			// $activate - activate a subset of the dancers
	P_MOVE_IN,			// $move_in - adjusts a ring for the selected dancers to move in
	P_CIRCLE,			// $circle - does circular shifts on rings
	P_CIRCLE_FRACTION,	// $circle_fraction - does circular shifts on rings
	P_CIRCLE_HOME,		// $circle_home - does circular shifts on rings
	P_ROTATE,			// $rotate - rotate the set by a fraction and execute a call as well
	P_FORM_RING,		// $form_ring - forms a ring from a squared set
	P_FORM_SET,			// $form_set - returns a ring to a squared set
	P_FORM_PROMENADE,	// $form_promenade - adjust to promenade position from squared set/almost squared set
	P_FORM_THAR,		// $form_thar - adjust to thar position
	P_FORWARD_AND_BACK,	// $forward_and_back - no-op, placemarker for animation
	P_PULL_BY,			// $pull_by - move facing dancers forward two (half-)spots
	P_FACE,				// $face - change facing right or left
	P_DEFINITION,		// $definition - fails if the first arg is not a call bound to the second definition
	P_BACK_OUT,			// $back_out - move dancers back one full position (if not occupied).
	P_ANY_WHO_CAN,		// $any_who_can - always succeeds.  Those who can do the call do it.  Others do nothing.
						// (returns only the active dancers)
	P_THOSE_WHO_CAN,	// $those_who_can - like $any_who_can, except fails if none can do it.
						// (returns only the active dancers)
	P_FORWARD,			// $forward - move dancers back/forward
	P_ARC,				// $arc - move dancers along an arc
	P_START_TOGETHER,	// $start_together - start two calls at the same time
	P_RUN,				// $run - run selected dancers around appropriate adjacent dancers.
	P_RUN_TO,			// $run_to - run to a specific directions, to clarify possibly ambiguous partners.
	P_FORWARD_VEER,		// $forward_veer - takes both a forward/backward motion and a lateral motion
	P_FORWARD_VEER_FACE,// $forward_veer_face - takes both a forward/backward motion and a lateral motion and a change in facing
	P_FORWARD_PEEL,		// $forward_peel - takes both a forward/backward motion and a lateral peel direction
	P_VEER,				// $veer - veer distance in relative direction
	P_DISPLACE,			// $displace - displace distance in relative direction
	P_ARC_FACE,			// $arc_face - move in an arc and adjust facing as well
	P_MIRROR,			// $mirror - do the call in mirrored coordinates
	P_FRACTIONALIZE,	// $fractionalize - do the specified fraction of the named call
	P_IF,				// $if - try first call, if successful, do second, else do third call
	P_CAN_START,		// $can_start - 'contruct' the plan for the call, and succeed if it succeeds
	P_REDUCE,			// $reduce - reduce the square using the first two parameters as formation names and do the third param
	P_CHECK_SEQUENCE,	// $check_sequence - check that boys are in sequence
	P_ROLL,				// $roll() - designated dancer(s) roll individually
	P_CAN_ROLL,			// $can_roll() - designated dancer(s) roll individually
	P_CLOSER_TO_CENTER,	// $closer_to_center - fails if $1 (dancer) is further from the center than $2 (dancer) 
						//    - fails if dancer sets are not single dancers
	P_HAS_LATERAL_FLOW,	// $has_lateral_flow - fails if the designated dancers do not all share common lateral flow.
						// if the parameter is $as_you_are, then left or right flow succeeds.  If the parameter is left
						// then only left flow succeeds and if the parameter is right, only right flow succeeds.
	P_STRETCH,			// $stretch - implements the stretch concept on the Anything parameter.
	P_DONT_BREATHE,		// $dont_breathe - suppresses breathing for the enclosing part step
	P_NORMALIZE,		// $normalize - regroup as needed and breathe
	P_BREATHE,			// $breathe - performs the parameter call and then breathes immediately.
	P_CONJURE_PHANTOM,	// $conjure_phantom - create a phantom at the designated local coordinates (x, y) - any facing
	P_PHANTOM,			// $phantom - performs the parameter call, using phantoms to fill any vacancies
	P_PRIMITIVE_COUNT
};

class Rectangle {
public:
	Rectangle() {}

	Rectangle(int left, int top, int right, int bottom) {
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}

	int left, top, right, bottom;

	int width() const { return right - left; }
	int height() const { return top - bottom; }

	// This returns true if the rectangle lies between the two dancers.
	// If the rectangle completely encloses the dancers, this returns
	// true.
	// This assumes a->y <= b->y (which would be true if a and b were taken
	// from a Group object in their index order).
	bool isBetween(const Dancer* a, const Dancer* b) const;
	/*
	 *	coincident
	 *
	 *	Two rectangles are 'coincident' if they have the same center point
	 */
	bool coincident(const Rectangle& other) const;
	/*
	 *	overlaps
	 *
	 *	Two rectangles overlap when their contents' intersection has an area.
	 *	(Two rectangles that merely share an edge do not overlap.)
	 */
	bool overlaps(const Rectangle& other) const;

	void clear() {
		left = 0;
		right = 0;
		top = 0;
		bottom = 0;
	}

	bool intersectsHorizontally(int y) const;

	bool intersectsVertically(int x) const;

	void enclose(const Rectangle& r);

	void print(int indent) const;
};

class Plane {
public:
	Plane(int tile, const Tile* owner, int afterWidth, Plane* lesserEdge, Plane* greaterEdge) {
		this->before = (lesserEdge->before + greaterEdge->before) / 2;
		this->now = (lesserEdge->now + greaterEdge->now) / 2;
		this->after = INT_MAX;
		this->tile = tile;
		this->owner = owner;
		this->afterWidth = afterWidth;
		this->lesserEdge = lesserEdge;
		this->greaterEdge = greaterEdge;
		this->lesser = false;
		this->tileCenter = null;
		this->centerLine = false;
		lesserEdge->tileCenter = this;
		greaterEdge->tileCenter = this;
		lesserEdge->lesser = true;
	}

	Plane(int edge, int nowEdge, int tile, const Tile* owner) {
		this->before = edge;
		this->now = nowEdge;
		this->after = INT_MAX;
		this->tile = tile;
		this->owner = owner;
		this->lesserEdge = null;
		this->greaterEdge = null;
		this->afterWidth = 0;
		this->lesser = false;
		this->tileCenter = null;
		this->centerLine = false;
	}

	Plane(bool centerLine) {
		this->before = 0;
		this->now = 0;
		this->after = 0;
		this->owner = null;
		this->tile = 0;
		this->lesserEdge = null;
		this->greaterEdge = null;
		this->afterWidth = 0;
		this->lesser = false;
		this->tileCenter = null;
		this->centerLine = centerLine;
	}

	int before;								// coordinate of this plane before the call
	int now;								// coordinate of this plane after the call, before breathing
	int after;								// coordinate of this plane after breathing
	int afterWidth;							// for edge planes, the width to the corresponding center plane
	int tile;								// The tile number for debugging
	const Tile* owner;						// This plane is from this group
	Plane* lesserEdge;						// lesser edge if this is the center plane
	Plane* greaterEdge;						// greater edge if this is the center plane
	bool lesser;							// true if this is a lesser edge, false otherwise
	bool centerLine;						// true if this is a center line plane.
	Plane* tileCenter;						// for edge planes, this is the tile center plane

	int compare(const Plane* other) const;

	void print(int indent) const;
};

class Token {
public:
	TokenType		type;
	int				value;
	const Term*		term;
	string			text;

	static Token endOfString;

	void print() const;
};

class Term {
public:
	virtual ~Term() { }

	virtual const Term* negate(Context* context) const;

	virtual const Term* not(Context* context) const;

	virtual const Term* positive(Context* context) const;

	virtual const Term* add(const Term* operand, Context* context) const;

	virtual const Term* subtract(const Term* operand, Context* context) const;

	virtual const Term* multiply(const Term* operand, Context* context) const;

	virtual const Term* divide(const Term* operand, Context* context) const;

	virtual const Term* remainder(const Term* operand, Context* context) const;

	virtual const Term* or(const Term* operand, Context* context) const;

	virtual const Term* and(const Term* operand, Context* context) const;

	virtual const Term* xor(const Term* operand, Context* context) const;

	virtual bool compare(const Term* operand, int* output) const;

	virtual void token(Token* output) const;

	virtual void print(int indent) const = 0;

	virtual int tokenWeight() const;

	virtual int match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const;
	/*
	 *	RETURNS:
	 *		true if the text can be a prefix for the given term.
	 */
	virtual bool partialMatch(Context* context, const string& text, vector<string>* output) const;

	virtual int sortIndex() const;

	const string& spelling() const { return _spelling; }

protected:
	Term(const string& spelling) {
		_spelling = spelling;
	}

private:
	string	_spelling;
};

class Anypivot : public Term {
public:
	Anypivot(Pivot pivot) : Term(pivotNames[pivot]) {
		_pivot = pivot;
	}

	Pivot pivot() const { return _pivot; }

	virtual void print(int indent) const;

private:
	Pivot _pivot;
};

class Anydirection : public Term {
public:
	Anydirection(Direction d) : Term(directionNames[d]) {
		_direction = d;
	}

	Direction direction() const { return _direction; }

	virtual void print(int indent) const;

	virtual int match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const;

private:
	Direction _direction;
};

class Anything : public Term {
	friend Stage;
public:
	Level designatorLevel() const;

	virtual void print(int indent) const;

	void clear() {
		_variables.clear();
	}

	void variable(const Term* value) {
		_variables.push_back(value);
	}

	const vector<const Term*>& variables() const { return _variables; }

	const Definition* definition() const { return _definition; }

	const Primitive* primitive() const { return _primitive; }

	bool inDefinition() const { return _inDefinition; }

private:
	Anything(bool inDefinition, const Primitive* primitive, const Definition* definition) : Term(string()) {
		_inDefinition = inDefinition;
		_definition = definition;
		_primitive = primitive;
	}

	bool					_inDefinition;			// True if this object was constructed from text in a definition,
													// false if parsed from a sequence.
	const Definition*		_definition;
	const Primitive*		_primitive;
	vector<const Term*>		_variables;
};

class Integer : public Term {
	friend Stage;
public:
	int value() const { return _value; }

	virtual void token(Token* output) const;

	virtual void print(int indent) const;

	virtual int match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const;

	virtual bool compare(const Term* operand, int* output) const;

private:
	Integer(int value) : Term(string()) {
		_value = value;
	}

	int	_value;
};

class Word : public Term {
public:
	Word(const string& text) : Term(text) {
	}

	virtual void print(int indent) const;

private:
};

class Fraction : public Term {
	friend Stage;
public:
	/*
	 *	improperNumerator
	 *
	 *	Returns the numerator of the improper fraction formed when the
	 *	value is expressed with the given denominator.  If the resulting
	 *	numerator would not be a whole integer, the function returns false.
	 *
	 *	The fraction argument, if not null, is multiplied by this object before
	 *	computing the resulting numerator.
	 *
	 *	On success, the function returns true.
	 */
	bool improperNumerator(int denominator, const Fraction* fraction, int* result) const;

	virtual const Term* negate(Context* context) const;

	virtual const Term* not(Context* context) const;

	virtual const Term* positive(Context* context) const;

	virtual const Term* add(const Term* operand, Context* context) const;

	virtual const Term* subtract(const Term* operand, Context* context) const;

	virtual const Term* multiply(const Term* operand, Context* context) const;

	virtual const Term* divide(const Term* operand, Context* context) const;

	virtual const Term* remainder(const Term* operand, Context* context) const;

	virtual const Term* or(const Term* operand, Context* context) const;

	virtual const Term* and(const Term* operand, Context* context) const;

	virtual const Term* xor(const Term* operand, Context* context) const;

	virtual bool compare(const Term* operand, int* output) const;
	/*
	 *	normalize
	 *
	 *	A normalized fraction has a non-negative denominator,
	 *	a numerator between 0 (inclusive) and the denominator (exclusive).
	 *	The GCD of numerator and denominator may be greater than 1.
	 */
	const Fraction* normalize(Context* context) const;

	virtual void token(Token* output) const;

	virtual void print(int indent) const;

	virtual int match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const;

	int whole() const { return _whole; }

	int numerator() const { return _numerator; }

	int denominator() const { return _denominator; }

	static Fraction* untilHome() {
		return new Fraction(0, 1 ,0);
	}

private:
	Fraction(int whole, int numerator, int denominator) : Term(string()) {
		_whole = whole;
		_numerator = numerator;
		_denominator = denominator;
	}

	int	_whole;
	int _numerator;
	int _denominator;
};

class BuiltIn : public Term {
public:
	BuiltIn(TokenType index) : Term(string(tokenNames[index]).tolower()) {
		_index = index;
	}

	virtual int tokenWeight() const;

	virtual void print(int indent) const;

	virtual int match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const;

	virtual bool partialMatch(Context* context, const string& text, vector<string>* output) const;

	virtual int sortIndex() const;

	TokenType index() const { return _index; }

private:
	TokenType	_index;
};

class Primitive : public Term {
public:
	Primitive(Primitives index) : Term(primitiveNames[index]) {
		_index = index;
	}

	static Primitive nothing;

	bool construct(Plan* p, const Anything* parent, Context* context, TileAction action) const;

	unsigned startingDancersMask(const PrimitiveStep* s) const;

	void trimStart(PrimitiveStep* s, unsigned mask, Context* context) const;

	const Group* execute(PrimitiveStep* s, const Anything* parent, const Fraction* fraction, Context* context) const;

	virtual void print(int indent) const;

	bool fractionalize(PrimitiveStep* step, const Anything* parent, const Fraction* f, Context* context) const;

	Primitives index() const { return _index; }

	static const int	ANY = -1;

private:
	Primitives	_index;
};

class Anyone : public Term {
	friend Stage;
	friend Grammar;
public:
	unsigned match(const Group* dancers, const Step* step, Context* context) const;

	virtual const Term* not(Context* context) const;

	virtual const Term* or(const Term* operand, Context* context) const;

	virtual const Term* and(const Term* operand, Context* context) const;

	virtual const Term* xor(const Term* operand, Context* context) const;

	virtual void print(int indent) const;

	string label() const;

	const Anyone* left() const { return _left; }

	const Anyone* right() const { return _right; }

	Level level() const { return _level; }

	DancerSet dancerSet() const { return _dancerSet; }

private:
	Anyone(DancerSet dancerSet, unsigned mask, const Anyone* left, const Anyone* right, Level level) : Term(dancerSetNames[dancerSet]) {
		_dancerSet = dancerSet;
		_mask = mask;
		_left = left;
		_right = right;
		_level = level;
	}

	DancerSet		_dancerSet;
	unsigned		_mask;
	const Anyone*	_left;
	const Anyone*	_right;
	Level			_level;
};

class DancerName : public Term {
public:
	DancerName(int index) : Term(string(dancerNames[index]).tolower()) {
		_index = index;
	}

	virtual void print(int indent) const;

	virtual void token(Token* output) const;

private:
	int _index;
};

class Pattern {
public:
	Pattern(const Formation* formation, const string& parameterList) {
		_formation = formation;
		_parameterList = parameterList;
	}

	bool match(const Group* dancers, const PatternClosure* closure) const;

	unsigned matchSome(const Group* dancers, int startWith, const PatternClosure* closure) const;

	Group* matchWithPhantoms(const Group* dancers, const PatternClosure* closure) const;

	static Pattern* parse(const Grammar* grammar, const string& expression);

	const Formation* formation() const { return _formation; }

	const string& parameterList() const { return _parameterList; }

private:
	const Formation*	_formation;
	string				_parameterList;
};

class PatternClosure {
public:
	PatternClosure(const Pattern* pattern, const Anything* call, const Step* step, Context* context) {
		_pattern = pattern;
		_call = call;
		_designatorTerm = null;
		_step = step;
		_context = context;
	}
	/*
	 *	Returns true if the pattern discriminates between designated and non-designated dancers
	 */
	bool discriminates() const { return _pattern->parameterList().size() > 0; }

	bool designates(const Dancer* dancer) const;

	Context* context() const { return _context; }

	const Step* step() const { return _step; }

private:
	const Pattern*			_pattern;
	const Anything*			_call;
	const Step*				_step;
	Context*				_context;

	mutable const Anyone*	_designatorTerm;
};

// Note: only implements 90 degree rotations (0 = no rotate, 1 = 90 degrees, etc.)
class Transform {
public:
	Transform(int x0, int x1, int x2, int y0, int y1, int y2) :
		_x0(x0), _x1(x1), _x2(x2),
		_y0(y0), _y1(y1), _y2(y2),
		_allocated(false) {}

	static const Transform identity;
	static const Transform rotate90;
	static const Transform rotate180;
	static const Transform rotate270;
	static const Transform mirror;

	static const Transform* translate(int offsetX, int offsetY);

	void apply(int* x, int* y, Facing* facing) const;

	void apply(float* x, float* y, Facing* facing) const;

	void revert(int* x, int* y, Facing* facing) const;

	void revert(float* x, float* y, Facing* facing) const;

	void revert(double* angle) const;

	const Dancer* apply(const Dancer* source) const;

	const Dancer* revert(const Dancer* source) const;

	void print(int indent) const {
		if (indent)
			printf("%*.*c", indent, indent, ' ');
		printf("| %3d %3d %3d |\n", _x0, _x1, _x2);
		if (indent)
			printf("%*.*c", indent, indent, ' ');
		printf("| %3d %3d %3d |\n", _y0, _y1, _y2);
	}

	bool isMirror() const {
		return _x0 < 0 && _y1 > 0;
	}
	// _x0 = N, _x1 = 0, _y0 = 0, _y1 = N -> no rotation (0)
	// _x0 = 0, _x1 = -N, _y0 = N, _y1 = 0 -> 270 degree   (left 1/4 / right 3/4)
	// _x0 = -N, _x1 = 0, _y0 = 0, _y1 = -N -> 180 degree (left 2/4 / right 2/4)
	// _x0 = 0, _x1 = N, _y0 = -N, _y1 = 0 -> 90 degree (left 3/4 / right 1/4)
	int leftQuarterTurns() const {
		if (_x0 > 0)
			return 0;
		else if (_x1 < 0)
			return 1;
		else if (_x0 < 0)
			return 2;
		else // if (_x1 > 0)
			return 3;
	}

	const Transform* clone() const;

	void dispose() const;
private:
	int		_x0, _x1, _x2;
	int		_y0, _y1, _y2;
	bool	_allocated;
};

const int NO_NOSE = 1000000;		// value used to indicate no nose angle in dancer drawings.

/*
	Note on coordinate systems
 
	Square Dancing represents a square as a grid of points on the floor.
	Nominally, the space between the center of one dancer and
	the center of adjacent dancer is always 1.  However, formations and
	calls allow dancers to move either in full 1 distances or in half
	units.  Therefore, the granularity of 'square dance space' is 1/2 a
	nominal dancer width.  The space itself is a square cartesian grid
	of points spaced 1/2 unit apart.

	For convenience, the code will store x and y coordinates as integers
	and hold a value actually twice as large as the external coordinate value.  Thus,
	a dancer at the actual location of [1/2, 1/2] would appear in memory as
	[1, 1].  The code never displays these values anywhere.  The values specified
	in definitions (which use the 1/2-unit coordinate system) will be converted
	when they are parsed.
 */
class Dancer {
	friend Group;
public:
	int	x, y;			// Location in 'square-dance-space'
	Facing facing;		// 
	Gender gender;		// specified
	int couple;			// couple number (1-4), or 0 for a phantom
	
	Dancer(int x, int y, Facing facing, Gender gender, int couple) {
		this->x = x;
		this->y = y;
		this->facing = facing;
		this->gender = gender;
		this->couple = couple;
		_dancerIndex = int(gender) + 2 * (couple - 1);
	}

	Dancer(int x, int y, Facing facing, Gender gender, int couple, int dancerIndex) {
		this->x = x;
		this->y = y;
		this->facing = facing;
		this->gender = gender;
		this->couple = couple;
		_dancerIndex = dancerIndex;
	}

	int compare(const Dancer* other) const;

	bool identical(const Dancer* other) const;

	bool opposite(const Sequence* sequence, const Dancer* other) const;

	bool matches(const Spot& spot, const PatternClosure* closure) const;

	const Dancer* arc(Geometry geometry, Point center, double radius, int rightSixteenthTurns, double angleAdjust, int noseQuarterTurns, Interval* interval, Context* context) const;

	const Dancer* forwardVeer(int amount, int veer, int rightQuarterTurns, Interval* interval, Context* context) const;

	const Dancer* face(int amount, Interval* interval, Context* context) const;

	const Dancer* displace(int deltaX, int deltaY, Interval* interval, Context* context) const;

	void displace(int forward, int lateral, int* x, int *y) const;

	void forward(Point* location, int amount);

	bool adjacentY(const Dancer* other) const;

	bool adjacentX(const Dancer* other) const;

	int quarterTurnsTo(const Anydirection* direction, int x, int y) const;

	void calculateArcParameters(const Group* container, 
								const Anypivot* center, 
								const Anydirection* direction, 
								Point* centerPoint, 
								double* radius, 
								Rotation newRotation, 
								int* rightSixteenthTurns, 
								double* angleAdjust, 
								Interval* motionData) const;

	void print(int indent) const;

	const Dancer* clone() const;

	const Dancer* cloneToCoordinates(const Group* base, const Group* coordinateSystem) const;

	const Dancer* cloneOffsetX(int amount) const;

	bool isPhantom() const { return couple == 0; }

	int leftTurnsNeededToFace(Geometry geometry, const Dancer* other) const;

	SpatialRelation relativeLocationOf(Geometry geometry, const Dancer* other) const;

	unsigned dancerMask() const { return 1 << _dancerIndex; }

	int dancerIndex() const { return _dancerIndex; }

private:
	Dancer() {}

	int _dancerIndex;
};

// Note: A Group object always keeps the dancers sorted in ascending y then ascending x.
// Under the 'common spot' concept, it is possible for mapped dancers to occupy the same
// x and y locations, but normally a given x,y combination is unique.

class Group : public Term {
	friend Stage;
public:
	~Group() {
		_dancers.deleteAll();
		_transform->dispose();
	}

	static const Group* home;

	bool equals(const Group* dancers) const;

	bool contains(const Dancer* dancer) const;

	bool sameShape(const Group* before) const;

	const Group* apply(const Anything* call, Context* context, TileAction tileAction) const;

	Group* apply(const Transform* transform, Context* context) const;

	void convertToLocal(const Group* coordinateSystem, Dancer* dancer) const;

	void convertToAbsolute(float* x, float* y, Facing* facing, double* noseAngle) const;

	void convertFromAbsolute(float* x, float* y) const;

	void applyTransforms(float* x, float* y) const;

	void convertToAbsolute(double *angle) const;

	void convertFromAbsolute(double *angle) const;

	double convertRingXToAngle(float x) const;
	/*
	 *	match
	 *
	 *	This method searches this set of dancers for any pattern matching the
	 *	given formation.  If a transform had to be applied (such as a rotation
	 *	or translation) to properly orient the resulting dancer set, it is attached
	 *	to the returned dancers.  Also, for Ring geometry, the dancers coordinates
	 *	may get re-written to re-align the dancers in the set.
	 */
	const Group* match(const Pattern* pattern, Context* context, const PatternClosure* closure) const;

	const Group* matchThisOrder(const Pattern* pattern, Context* context, const PatternClosure* closure) const;

	const Group* matchSome(int startWith, const Pattern* pattern, Context* context, const PatternClosure* closure, TileAction tileAction) const;

	const Group* matchSomeThisOrder(int startWith, const Pattern* pattern, Context* context, const PatternClosure* closure, TileAction tileAction) const;

	const Group* moveIn(PrimitiveStep* step, const Anyone* anyone, Context* context) const;

	const Group* forwardAndBack(Interval* interval, Context* context) const;

	Group* formRing(Interval* interval, Context* context) const;

	bool shouldBeRing() const;

	const Group* formGrid(Interval* interval, vector<const Group*>& affected, Context* context) const;

	Group* formSet(Interval* interval, Context* context) const;

	Group* formPromenade(const Anydirection* direction, Context* context, Interval* interval) const;

	Group* formThar(Interval* interval, Context* context) const;

	const Group* run(const Group* runners, const Anydirection* direction, Context* context, Interval* interval) const;

	Group* fractionalize(const Anything* call, const Fraction* fraction, Context* context) const;

	void adjacentTo(const Group* subset, const Anydirection* direction, Context* context, vector<const Dancer*>* output) const;
	/*
	 *	From the current set of dancers, identify one of the following partnershipRelations:
	 *
	 *		which		subset		Meaning
	 *		NONE		non-null	subset is a selection of dancers from the current set, possibly
	 *								transformed.  This function places in the output array the set
	 *								of Dancer objects in this set that are the respective partners
	 *								of each dancer in the subset.  Note that if no partner can be
	 *								found, the output value will be null.
	 *
	 *		BEAUS		null		Each beau in the current set is copied to the output vector.
	 *
	 *		BELLES		null		Each belle in the current set is copied to the output vector.
	 */
	void partnershipOp(DancerSet which, const Group* subset, Context* context, vector<const Dancer*>* output) const;
	/*
	 *	circle
	 *
	 *	This forces a RING to circle by full danncer positions, the amount
	 *	parameter specifies the number of positions.  A positive amount
	 *	indicates a shift to the right (an increase in the angle), and
	 *	a negative amount indicates a shift to the left (a decrease in the
	 *	angle).
	 *
	 *	interval == null implies that this is bookkeeping and no actual movement
	 *	of the dancers should be reported.
	 */
	const Group* circle(const Anydirection* leftRight, int amount, Rotation rotation, Interval* interval, Context* context) const;

	Group* rotate(Rotation rotation, Context* context) const;

	Group* veer(int amount, const Anydirection* direction, Interval* interval, Context* context) const;

	Group* displace(int amount, const Anydirection* direction, Interval* interval, Context* context) const;

	Group* forwardVeer(int amount, int veer, int rightQuarterTurns, Interval* interval, Context* context) const;

	Group* arc(const Anypivot* center, const Anydirection* direction, Rotation rotation, int amount, int noseQuarterTurns, Interval* interval, Context* context) const;

	Group* face(const Anydirection* dir, const Anypivot* pivot, Interval* interval, Context* context) const;

	const Group* backOut(PrimitiveStep* s, const Anyone* movers, const Group* start, Context* context, Interval* interval) const;

	const Group* roll(PrimitiveStep* s, Context* context, bool failOnCantRoll) const;

	bool closerToCenter(const Group* other) const;

	bool hasLateralFlow(const Anydirection* direction, const Interval* interval, Context* context) const;

	unsigned startingDancersMaskReduce(const PrimitiveStep* s) const;

	void trimStartReduce(const PrimitiveStep* s, unsigned mask, Context* context) const;

	void constructReduce(PrimitiveStep* s, Context* context) const;

	const Group* reduce(PrimitiveStep* s, Context* context) const;

	const Group* stretch(PrimitiveStep* s, Context* context) const;

	const Group* removePhantoms(Context* context) const;

	bool hasAmbiguousFacing() const;

	bool isSymmetric(const Sequence* sequence) const;

	const Group* disambiguateFromRoot(Context* context) const;

	const Group* root() const;

	void splitMotion(int pairX, int pairY, Motion* m, Interval* interval, Context* context) const;

	Group* displace(int deltaX, int deltaY, Interval* interval, Context* context) const;
	/*
	 *	merge
	 *
	 *	This takes each of the dancers in the current set, reverts their
	 *	current coordinates to the base set, then takes whatever other dancers
	 *	are in the base set and returns the merged dancers.
	 */
	const Group* merge(Context* context) const;
	/*
	 *	toCommonCoordinates
	 *
	 *	The current dancers are converted to a common coordinate
	 *	system with the starting dancers.  If the current set is
	 *	not derived from the starting set, this operation may not
	 *	produce well defined results.
	 */
	const Group* toCommonCoordinates(const Group* start, Context* context) const;

	bool sameCoordinateSpace(const Group* other) const;

	bool basedOn(const Group* base) const;
	/*
	 *	addTransforms
	 *
	 *	This method takes the 'newStart' group, and then adds each transform found in
	 *	the chain of groups from this object to priorStart (priorStart is assumed in
	 *	some way to be based on this object).
	 */
	const Group* addTransforms(const Group* priorStart, const Group* newStart, Context* context) const;
	/*
	 *	unwind
	 *
	 *	This takes each of the dancers in the current set and reverts
	 *	their current coordinates to the base set.  Any additional dancers
	 *	found in the base set are ignored.
	 */
	Group* unwind(Context* context) const;
	/*
	 *	combine
	 *
	 *	This takes the dancers of all of the groups vector (which are in the
	 *	same coordinates system as this set).  Any dancers in the current set
	 *	that do not appear in any of the groups will also be included in the
	 *	output.
	 */
	const Group* combine(vector<const Group*>& groups, Context* context) const;
	/*
	 *	combineSubGroups
	 *
	 *	This takes the dancers of all of the groups vector (which are in the
	 *	same coordinates system as this set).  Any dancers in the current set
	 *	that do not appear in any of the groups will NOT be included in the
	 *	output.
	 */
	const Group* combineSubGroups(vector<const Group*>& groups, Context* context) const;
	/*
	 *	breathe
	 */
	void breathe(Step* enclosing, vector<const Group*>& affected, unsigned rootMask, const vector<Rectangle>& boundingBoxes, bool preserveRelativePositions, Context* context) const;

	const Group* normalize(Step* enclosing, vector<const Group*>& affected, Context* context) const;

	Step* checkSamePositionRule(PartStep* step, Context* context) const;

	Group* clone(Context* context) const;

	Group* rotateDancers(int by, Context* context) const;

	Group* cloneNonDancerData(Context* context) const;

	Group* cloneCoordinateSystem(Context* context) const;

	Group* minus(const Group* y, Context* context) const;		// result = this->minus(that); - constructs the set of dancers in this that are not in that.

	unsigned dancerMask() const;

	Group* extract(unsigned mask, Context* context) const;		// constructs the set of dancers in this that correspond to the mask values (bit corresponds to _dancers index).
	/*
	 *	extractIn
	 *
	 *	Create a new group by selecting the dancers covered by 'mask' in this group
	 *	and then convert them to the coordinateSystem (which must have this group as a base).
	 */
	Group* extractIn(const Group* coordinateSystem, unsigned mask, Context* context) const;

	Group* trim(unsigned mask, Context* context) const;

	Group* normalizeRingCoordinates(Context* context) const;

	bool inSequence(const Anydirection* direction) const;

	bool atHome() const;

	void computeCenter(int* x, int* y) const;

	bool computeSplitCenter(int* x, int* y) const;

	virtual void print(int indent) const;

	void printDetails(int indent, bool includeDetails) const;

	int compare(const Group* other) const;

	void boundingBox(Rectangle* box) const;

	Gender gender() const;

	int couple() const;

	Group* findTile(const vector<Rectangle>& boundingBoxes, Context* context) const;

	bool singleTile(const vector<Rectangle>& boundingBoxes) const;

	int dancerCount() const { return _dancers.size(); }

	int realDancerCount() const;

	const Dancer* dancerByIndex(int dancerIndex) const;

	const Dancer* dancerByLocation(int x, int y, bool inBase) const;

	const Dancer* dancer(int i) const { return _dancers[i]; }

	void include(const Group* input, unsigned mask);

		// These methods populate the _dancers of this object

	void insert(const Dancer* dancer);

		// Clone those dancers from x that are selected by mask

	void intersection(const Group* x, unsigned mask);

		// Clone those dancers from x that are not selected by mask

	void subtraction(const Group* x, unsigned mask);

	void done();

	void clear();
	/*
	 * Note: for tiles that are known to not use designations, call and step can be null.  If a
	 * formation with ACTIVE_DESIGNATED or ACTIVE_NONDESIGNATED spots are matched, those spots
	 * are treated as ACTIVE spots.
	 */
	int buildTiling(const vector<VariantTile>& tiles, TileSearch* out, Context* context, const Anything* call, Step* step, TileAction tileAction) const;

	void buildDancerArray(const Dancer** output) const;

	Rotation rotation() const { return _rotation; }

	Geometry geometry() const { return _geometry; }

	const Group* base() const { return _base; }

	void setTiled() { _tiled = true; }

	bool tiled() const { return _tiled; }

private:

	int buildPhantom4Dancer(const VariantTile& tile, TileSearch* out, Context* context, const Anything* call, Step* step) const;

	static int compare(const void* ts1, const void* ts2);

	Group* formCrossedCouplesFromRing(int minRadius, Context* context, Interval* interval) const;

	Group(const Group* base) : Term(string()) {
		_geometry = base->_geometry;
		_homeGeometry = base->_homeGeometry;
		_rotation = base->_rotation;
		_transform = null;
		_base = base;
		_tiled = false;
	}

	Group(Geometry geometry) : Term(string()) {
		_geometry = geometry;
		_homeGeometry = geometry;
		_rotation = UNROTATED;
		_transform = null;
		_base = null;
		_tiled = false;
	}

	static Group* makeHome();

	vector<const Dancer*>	_dancers;
	Geometry				_geometry;
	Geometry				_homeGeometry;
	Rotation				_rotation;
	bool					_tiled;
	const Transform*		_transform;
	const Group*			_base;
};

Rotation rotateBy(int n);

double rotationAngle(Rotation r);
/*
 *	angle
 *
 *	This calculates the angle theta, in radians, formed be the vector [x,y] with the X axis.
 *
 *  The angle lies between -PI and PI.
 */
double angle(double x, double y);

double round(double x);

void initTestObjects();

void initializePrecedences();

bool loadLevels();

__int64 parseLongLong(const string& s, int offset);

Gender gender(PositionType position);

}  // namespace dance
