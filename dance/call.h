#include "../common/event.h"
#include "../common/map.h"
#include "../common/string.h"
#include "../common/vector.h"
#include "dance.h"
#include "motion.h"

namespace dance {

class Anything;
class CallStep;
class CompoundAction;
class CompoundStep;
class Context;
class Group;
class Definition;
class DefinitionStep;
class Explanation;
class Fraction;
class Grammar;
class Integer;
class Interval;
class MotionSet;
class Part;
class PartStep;
class Primitive;
class PrimitiveStep;
class Stage;
class StartTogetherStep;
class Step;
class Tile;
class Variant;

enum BreatheAction {
	UNDECIDED,
	DONT_BREATHE,
	SKIP_BREATHE,			// breathing would be redundant.
	BREATHE,
	NORMALIZE,
	FORM_GRID,
};

enum ExplanationClass {
	GENERAL_ERROR,
	PROGRAM_BUG,
	USER_ERROR,
	DEFINITION_ERROR,
};

extern Event1<const Stage*> deletingStage;

const int MAX_PLAN_DEPTH = 50;

class Context {
public:
	Context(Sequence* sequence, const Grammar* grammar) {
		init();
		_grammar = grammar;
		_sequence = sequence;
		_planDepth = 0;
	}

	Context(const Context* base) {
		init();
		_grammar = base->grammar();
		_sequence = base->sequence();
		copy(base);
	}

	void init() {
		_stage = null;
	}

	void copy(const Context* source) {
		_stage = source->_stage;
		_planDepth = source->_planDepth;
	}

	void startStage(Stage* stage) {
		_stage = stage;
	}

	bool startPlan() {
		_planDepth++;
		return _planDepth < MAX_PLAN_DEPTH;
	}

	void finishPlan() {
		if (_planDepth > 0)
			_planDepth--;
	}

	void endStage() {
		_stage = null;
	}

	Stage* stage() const { return _stage; }

	const Grammar* grammar() const { return _grammar; }

	Sequence* sequence() const { return _sequence; }

private:
	Stage* _stage;
	const Grammar* _grammar;
	Sequence* _sequence;
	int _planDepth;
};

class Resolution {
public:
	Resolution(BreatheAction ba) : _resolved(false), _breatheAction(ba), _final(null), _beforeBreathing(null) {}

	void copy(const Resolution& r);

	void constructOutcomes(Step* step);

	void collectOutcomes(Step* step, Context* context);

	void collectOutcomes(unsigned mask0, unsigned mask1, Step* step, Context* context);

	void collectOutcomes(unsigned mask, const Resolution& r, Context* context);

	void resolvePlan(Plan* plan, Context* context);

	void resolveStep(Step* step, const Group* abstracted, Context* context);

	void resolveReduction();

	bool toCommonCoordinates(const Group* start, Context* context);

	void checkSamePositionRule(PartStep* step, Context* context);

	void setBreatheAction(BreatheAction ba) { _breatheAction = ba; }

	BreatheAction breatheAction() const { return _breatheAction; }

	const char* breatheActionLabel() const;

	void exclusiveOutcome(const Group* g);

	void addOutcome(const Group* g) {
		_outcome.push_back(g);
	}

	void replaceOutcome(int i, const Group* g) {
		_outcome[i] = g;
	}

	int outcomeCount() const { return _outcome.size(); }

	unsigned finalDancerMask() const;

	unsigned lastActiveMask() const;

	int dancerCount() const;

	void print(int indent) const;

	bool resolved() const { return _resolved; }

	const Group* final() const { return _final; }

	void setFinal(const Group* g) { _resolved = true; _final = g; }

	const Group* outcome(int i) const { return _outcome[i]; }

	bool isSimple() const {
		if (!_resolved)
			return true;
		if (_breatheAction == BREATHE ||
			_breatheAction == NORMALIZE)
			return false;
		else
			return true;
	}

	const Group* beforeBreathing() const { return _beforeBreathing; }

private:
	enum CompactifyRelation {
		LEFT_OF_CENTER,
		CENTER,
		RIGHT_OF_CENTER
	};

	struct CompactifyInfo {
		vector<int> groupIds;
		vector<const Rectangle*> startingBoxes;
		vector<const Rectangle*> finalBoxes;
		CompactifyRelation vertical;
		CompactifyRelation horizontal;
		int verticalPosition;
		int horizontalPosition;
		Rectangle startBox;
		Rectangle finalBox;

		CompactifyInfo() {}

		CompactifyInfo(CompactifyRelation v, int vp, CompactifyRelation h, int hp) {
			vertical = v;
			verticalPosition = vp;
			horizontal = h;
			horizontalPosition = hp;
		}
		/*
		 * Returns true if the groups needed any displacement to satisfy the
		 * vertical and horizontal positioning requirements.
		 */
		bool displaceIfNeeded(Resolution* that, Step* step, unsigned rootMask, Context* context);

		void print(const char* label);
	};

	void normalize(Step* step, Context* context);

	void breathe(Step* step, const Group* abstracted, bool preserveRelativePositions, Context* context);

	bool compactify(Step* enclosing, CompactifyInfo* info, unsigned rootMask, Context* context);

	void decideBreatheAction(Step* step, Context* context);

	bool					_resolved;
	BreatheAction			_breatheAction;
	vector<const Group*>	_outcome;
	const Group*			_final;
	const Group*			_beforeBreathing;
};

class Plan {
	friend Stage;
	friend Tile;
public:
	bool perform(MotionSet* enclosing, Context* context, TileAction tileAction);

	void splice(Plan *sub);

	const Group* breathe(Context* context);

	bool construct(Context* context, TileAction tileAction);

	bool construct(const VariantTile* variant, Context* context, TileAction tileAction);

	Step* constructRawStep(Context* context);

	StartTogetherStep* constructStartTogetherStep(Context* context);

	bool constructStep(const Part* part, Context* context, TileAction tileAction);

	Step* constructStep(const CompoundAction* action, Context* context, TileAction tileAction);

	Step* constructStep(const Primitive* primitive, const Anything* parent, Context* context);

	Step* newStep(const Anything* action, Context* context);

	Step* constructStep(const Anything* action, Context* context, TileAction tileAction);

	Step* insertStep(int after, Context* context);

	void useStep(Step* s);

	bool fractionalize(const Fraction* fraction, Context* context);

	bool performStep(int index, Context* context);

	void collectMotions(unsigned mask, MotionSet* enclosing, Context* context) const;

	void makeInterval(MotionSet* enclosing, bool startTogether);

	int nextPhantom();

	void setStart(const Group* dancers) { _start = dancers; }

	void setCall(const Anything* call) { _call = call; }

	unsigned startingDancersMask() const;

	void trimStart(const Group* originalStart, const Group* outerStart, unsigned mask, Context* context);

	unsigned lastActiveDancersMask(const Step* requester) const;

	Interval* lastInterval(const Step* requester) const;

	bool fail(const Explanation* ex);

	void warn(const Explanation* ex);

	void print(int indent) const;

	virtual bool inStage(Stage* stage) const;

	unsigned lastActiveMask() const;

	bool defineLocal(const Term* name, Anyone* value) { return _locals.insert(name, value); }

	Anyone* get(const Term* name) const { return *_locals.get(name); }

	const Group* start() const { return _start; }

	const Group* orientedStart() const { return _orientedStart; }

	const Anything* call() const { return _call; }

	int stepCount() const { return _steps.size(); }

	Step* step(int i) const { return _steps[i]; }

	const Group* final() const { return _resolution.final(); }

	unsigned finalDancerMask() const { return _resolution.finalDancerMask(); }

	void setFinal(const Group* g) { _resolution.setFinal(g); }

	bool failed() const { return _failed; }

	const Explanation* cause() const { return _cause; }

	Interval* interval() const { return _interval; }

	Tile* enclosing() const { return _enclosing; }

	const Variant* applied() const { return _applied; }

	const Pattern* matched() const { return _matched; }

	bool hasLocals() const { return _locals.size(); }

	const Resolution& resolution() const { return _resolution; }

	~Plan();
private:
	Plan(const Group* start, const Anything* call, const Plan* outer);

	Interval*					_interval;
	mutable bool				_collected;
	Tile*						_enclosing;
	const Group*				_start;
	const Group*				_orientedStart;
	const Anything*				_call;
	vector<Step*>				_steps;
	Resolution					_resolution;
	bool						_failed;
	const Explanation*			_cause;
	const Variant*				_applied;
	const Pattern*				_matched;
	map<const Term, Anyone*>	_locals;
	int							_phantomCount;

};
/*
 *	Stage
 *
 *	A stage is the data generated from a single call within a Sequence.  The _dancers
 *	object describes the state and geometry of the final position for all dancers when the
 *	call is completed.
 */
class Stage : public Plan {
public:
	Stage(const Sequence* sequence, const Group* start);

	~Stage();

	virtual bool inStage(Stage* stage) const;

	void collectMotions();

	void checkFlow(FlowState* flowState);

	void print() const;

	bool resolved() const;

	Plan* newPlan(const Plan* outer, Tile* enclosing, const Group* start, const Anything* call);

	Step* newStep(Plan* p);

	StartTogetherStep* newStartTogetherStep(Plan* p);

	DefinitionStep* newDefinitionStep(Plan* plan, const Definition* definition, const Anything* call, const Group* start);

	CallStep* newCallStep(Plan* plan, const Anything* action);

	PrimitiveStep* newPrimitiveStep(Plan* plan, const Primitive* primitive, const Anything* parent);

	PartStep* newPartStep(Plan* plan, const Part* part);

	CompoundStep* newCompoundStep(Plan* plan, const CompoundAction* action);

	Tile* newTile(Step* enclosing, const Group* start, const Anything* call, const VariantTile* matched);

	Tile* newTile(Step* enclosing, const Group* start, const Group* final);

	Explanation* newExplanation(ExplanationClass exClass, const string& text);

	Group* newGroup(const Group* base);

	Group* newGroup(Geometry geometry);

//	Group* combine(const vector<const Group*>& groups);

	Anything* newAnything(const Primitive* primitive);

	Anything* newAnything(bool inDefinition, const Definition* definition);

	Integer* newInteger(int value);

	Anyone* newAnyone(DancerSet dancerSet, unsigned mask, const Anyone* left, const Anyone* right, Level level);

	Fraction* newFraction(int whole, int num, int denom);

	Straight* newStraight(Point start, Point end, double startNose, double noseMotion, beats duration);

	Curve* newCurve(Point center, double motionAngle, double radius, Point start, Point end, double startNose, double noseMotion, beats duration);

	int dancerCount() const { return _motions.dancerCount(); }

	Motion* motion(int index) const { return _motions.motion(index); }

	beats duration() const { return _motions.duration(); }

	const MotionSet* motions() const { return &_motions; }

	const Sequence* sequence() const { return _sequence; }

private:
	const Sequence*			_sequence;
	MotionSet				_motions;
	vector<Plan*>			_plans;
	vector<Step*>			_steps;
	vector<Tile*>			_tiles;
	vector<Group*>			_dancers;
	vector<Term*>			_terms;
	vector<Explanation*>	_explanations;
	vector<Motion*>			_allocedMotions;
};

class Tile {
	friend Stage;
public:
	virtual unsigned lastActiveDancersMask() const;

	virtual Interval* lastInterval() const;

	void construct(Context* context, TileAction tileAction);

	bool perform(Context* context);

	void print(int indent) const;

	bool failed() const;

	bool isPrimitive() const;

	void setEnclosing(Step* s) { _enclosing = s; }

	Step* enclosing() const { return _enclosing; }

	Plan* plan() { return &_plan; }

	const Plan* plan() const { return &_plan; }

	bool active() const { return _active; }

	const VariantTile* matched() const { return _matched; }

	void setLastActiveDancersMask(unsigned m) { _lastActiveDancersMask = m; }

protected:
	Tile() : _plan(null, null, null) {
		_enclosing = null;
		_active = false;
		_matched = null;
		_lastActiveDancersMask = ~0;
	}

	unsigned			_lastActiveDancersMask;

private:
	Tile(Step* enclosing, const Group* start, const Anything* call, const VariantTile* matched);

	Tile(Step* enclosing, const Group* start, const Group* final);

	Step*				_enclosing;
	const VariantTile*	_matched;
	Plan				_plan;
	bool				_active;
};

class StubTile : public Tile {
public:
	StubTile(unsigned lastActiveDancersMask, Interval* interval) {
		_lastActiveDancersMask = lastActiveDancersMask;
		_interval = interval;
	}

	virtual unsigned lastActiveDancersMask() const;

	virtual Interval* lastInterval() const { return _interval; }

private:
	Interval* _interval;
};

class Step {
	friend Stage;
public:
	virtual bool construct(Context* context, TileAction tileAction);

	virtual bool perform(Context* context);

	virtual void collectMotions(MotionSet* enclosing, Context* context) const;

	virtual const Anything* displayCall() const;

	virtual void print(int indent, bool printChildren) const;

	virtual bool fractionalize(const Fraction* fraction, Context* context);

	void startWith(const Group* dancers) { _start = dancers; }

	void makeInterval(Interval* enclosing, bool startTogether);

	void setInterval(Interval* newInterval);

	Tile* newTile(const Group* start, const Anything* call, Context* context);

	Tile* constructTile(const Group* start, const Anything* call, Context* context, TileAction action);

	Tile* constructTile(const Group* start, const Anything* call, Context* context, unsigned activeDancers);

	Tile* constructTile(const Group* start, const Group* final, Context* context);

	void addTile(Tile* t);

	void setRotation(int rotation) { _rotation = rotation; }

	void setBreatheAction(BreatheAction ba) { _resolution.setBreatheAction(ba); }

	const Group* combine(bool mustBreathe, const Group* abstracted, Context* context);

	virtual void collectOutcomes(Context* context);

	void constructOutcomes();

	bool outcomeGeometryDiffers(const Group* start);

	BreatheAction breatheAction() const { return _resolution.breatheAction(); }

	bool buildAffectedVector(vector<const Group*>* affected, Context* context);

	virtual unsigned startingDancersMask() const;

	virtual void trimStart(const Group* originalStart, const Group* outerStart, unsigned mask, Context* context);

	unsigned lastActiveDancersMask() const;

	Interval* lastInterval() const;

	bool fail(const Explanation* ex);

	bool inStage(Stage* stage) const;

	void discardTiles(int newTileCount);

	void setPlan(Plan* p) { _plan = p; }

	void addOutcome(const Group* g) {
		_resolution.addOutcome(g);
	}

	void resolveReduction(const Group* reducedStart, const Group* reducedFinal, unsigned lastActiveDancersMask, Interval* ss, Context* context);

	const Group* outcome(int i) const { return _resolution.outcome(i); }

	const Resolution& resolution() const { return _resolution; }

	Resolution& resolution() { return _resolution; }

	Plan* plan() const { return _plan; }

	const Group* start() const { return _start; }

	void setLastActiveMask(unsigned m) { _lastActiveMask = m; }

	unsigned lastActiveMask() const { return _lastActiveMask; }

	const Group* final() const { return _resolution.final(); }

	const vector<Tile*>& tiles() const { return _tiles; }

	bool failed() const { return _failed; }

	const Explanation* cause() const { return _cause; }

	Interval* interval() const { return _interval; }

	const vector<Plan*>& collapsed() const { return _collapsed; }

	void setSpecialLabel(const string& label) { _specialLabel = label; }

	const string& specialLabel() const { return _specialLabel; }

	~Step();

protected:
	Step(Plan* plan, BreatheAction breatheAction = UNDECIDED);

	void printCommon(int indent, bool printChildren) const;

	Interval*				_interval;
	Plan*					_plan;
	const Group*			_start;
	vector<Tile*>			_tiles;
	unsigned				_lastActiveMask;
	Resolution				_resolution;
	int						_rotation;

private:
	bool					_failed;
	string					_specialLabel;
	const Explanation*		_cause;
	vector<Plan*>			_collapsed;
};

class DefinitionStep : public Step {
	friend Stage;
public:
	virtual bool construct(Context* context, TileAction tileAction);

	virtual const Anything* displayCall() const;

	virtual void print(int indent, bool printChildren) const;

	const Anything* call() const { return _call; }
private:
	DefinitionStep(Plan* plan, const Definition* definition, const Anything* call, const Group* start) : Step(plan) {
		_definition = definition;
		_call = call;
		startWith(start);
	}

	const Definition*		_definition;
	const Anything*			_call;
};

class PartStep : public Step {
	friend Stage;
public:
	virtual bool construct(Context* context, TileAction tileAction);

	virtual bool perform(Context* context);

	virtual void print(int indent, bool printChildren) const;

	const Part* part() const { return _part; }

private:
	PartStep(Plan* plan, const Part* part) : Step(plan) {
		_part = part;
	}

	const Part*			_part;
};

class CompoundStep : public Step {
	friend Stage;
public:
	virtual bool construct(Context* context, TileAction tileAction);

	virtual void print(int indent, bool printChildren) const;

private:
	CompoundStep(Plan* plan, const CompoundAction* action) : Step(plan) {
		_action = action;
	}

	const CompoundAction*	_action;
};

class StartTogetherStep : public Step {
	typedef Step super;
	friend Stage;
public:
	virtual void collectOutcomes(Context* context);

	virtual void collectMotions(MotionSet* enclosing, Context* context) const;

	void setOutcomeMasks(unsigned mask0, unsigned mask1) { 
		_mask0 = mask0;
		_mask1 = mask1;
	}

private:
	StartTogetherStep(Plan* plan) : Step(plan) {
	}

	unsigned _mask0;					// selection mask for tile 0
	unsigned _mask1;					// selection mask for tile 1
};

class PrimitiveStep : public Step {
	friend Stage;
public:
	virtual bool construct(Context* context, TileAction tileAction);

	virtual unsigned startingDancersMask() const;

	virtual void trimStart(const Group* originalStart, const Group* outerStart, unsigned mask, Context* context);

	virtual bool fractionalize(const Fraction* fraction, Context* context);

	virtual bool perform(Context* context);

	virtual const Anything* displayCall() const;

	virtual void print(int indent, bool printChildren) const;

	const Primitive* primitive() const { return _primitive; }

	const Anything* parent() const { return _parent; }

	const Fraction* fraction() const { return _fraction; }

private:
	PrimitiveStep(Plan* plan, const Primitive* primitive, const Anything* parent) : Step(plan) {
		_fraction = null;
		_primitive = primitive;
		_parent = parent;
	}

	const Fraction*		_fraction;
	const Primitive*	_primitive;
	const Anything*		_parent;
};

class CallStep : public Step {
	friend Stage;
public:
	virtual bool construct(Context* context, TileAction tileAction);

	virtual const Anything* displayCall() const;

	virtual void print(int indent, bool printChildren) const;

	void setAction(const Anything* action) {
		_action = action;
	}

	const Anything* action() const { return _action; }

private:
	CallStep(Plan* plan, const Anything* action) : Step(plan) {
		_action = action;
	}

	const Anything*			_action;
};

struct TileSearch {
	const Group*		dancers;
	const VariantTile*	matched;
};

class Explanation {
	friend Stage;
public:
	void print(int indent) const;

	const string& text() const { return _text; }

	ExplanationClass explanationClass() const { return _class; }

private:
	Explanation(ExplanationClass exClass, const string& text) {
		_text = text;
		_class = exClass;
	}

	Explanation(const string& text) {
		_text = text;
		_class = GENERAL_ERROR;
	}

	string	_text;
	ExplanationClass _class;
};

}  // namespace dance
