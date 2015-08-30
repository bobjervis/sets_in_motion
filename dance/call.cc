#include "../common/platform.h"
#include "call.h"

#include "../common/timing.h"
#include "dance.h"
#include "motion.h"

namespace dance {

static string baseDancers(const string& designator);

Event1<const Stage*> deletingStage;

Plan::Plan(const Group* start, const Anything* call, const Plan* outer) : _resolution(UNDECIDED) {
	_enclosing = null;
	_start = start;
	_orientedStart = start;
	_call = call;
	_failed = false;
	_cause = null;
	_interval = null;
	_applied = null;
	_matched = null;
	_collected = false;
	if (outer != null)
		_phantomCount = outer->_phantomCount;
	else
		_phantomCount = 0;
}

Plan::~Plan() {
	delete _interval;
}

bool Plan::perform(MotionSet* enclosing, Context* context, TileAction tileAction) {
	timing::Timer t("Plan::perform");
	if (!context->startPlan())
		return fail(context->stage()->newExplanation(DEFINITION_ERROR, "Planning depth exceeded - probable logic loop in definitions"));
	if (construct(context, tileAction)) {
		makeInterval(enclosing, false);
		for (int i = 0; i < _steps.size(); i++)
			if (!performStep(i, context))
				break;
//		combine(context);
	}
	context->finishPlan();
	return !_failed;
}

void Plan::splice(Plan *sub) {
	_applied = sub->_applied;
	_call = sub->_call;
	_collected = sub->_collected;
	_failed = sub->_failed;
	_interval = sub->_interval;
	sub->_interval = null;
	_locals.clear();
	map<const Term, Anyone*>::iterator li = sub->_locals.begin();
	while (li.valid()) {
		const Term* key = li.key();
		Anyone* a = *li;
		_locals.put(key, a);
		li.next();
	}
	_matched = sub->_matched;
	_orientedStart = sub->_orientedStart;
	_phantomCount = sub->_phantomCount;
	_start = sub->_start;
	_steps.clear();
	for (int i = 0; i < sub->_steps.size(); i++)
		_steps.push_back(sub->_steps[i]);
}

bool Plan::construct(Context* context, TileAction tileAction) {
	timing::Timer t("Plan::construct");
	if (_failed)
		return false;
	if (_steps.size() == 0) {
		const Primitive* p = _call->primitive();
		if (p)
			 return p->construct(this, _call, context, tileAction);
		else {
			const Definition* d = _call->definition();
			for (int i = 0; ; i++) {
				const Variant* v = d->variant(i);
				if (v == null)
					break;
				if (tileAction == TILE_WITH_PHANTOMS) {
					const Group* orientedGroup;
					if (v->testAnyPhantomFormations(_start, context, &_matched, &orientedGroup, _call, _enclosing ? _enclosing->enclosing() : null)) {
						_orientedStart = orientedGroup;
						_applied = v;
						if (!_call->inDefinition() && 
							context->stage()->sequence() && 
							context->stage()->sequence()->level() > NO_LEVEL &&
							v->level() > context->stage()->sequence()->level())
							return fail(context->stage()->newExplanation(USER_ERROR, "'" + d->label() + "' in this position is off-level (" + levels[v->level()] + ")"));
						else
							return true;
					}
				} else {
					const Group* orientedGroup;
					if (v->testAnyFormations(_start, context, &_matched, &orientedGroup, _call, _enclosing ? _enclosing->enclosing() : null)) {
						_orientedStart = orientedGroup;
						_applied = v;
						if (!_call->inDefinition() && 
							context->stage()->sequence() && 
							context->stage()->sequence()->level() > NO_LEVEL &&
							v->level() > context->stage()->sequence()->level())
							return fail(context->stage()->newExplanation(USER_ERROR, "'" + d->label() + "' in this position is off-level (" + levels[v->level()] + ")"));
						else if (v->partCount() == 0)
							return fail(context->stage()->newExplanation(USER_ERROR, "'" + d->label() + "' has no definition"));
						else
							return v->construct(this, _call, context, tileAction);
					}
				}
			}
			if (d->tiles().size() > 0) {
				Step* s = context->stage()->newDefinitionStep(this, d, _call, _start);

				_steps.push_back(s);
				s->construct(context, tileAction);
			} else
				return fail(context->stage()->newExplanation(USER_ERROR, "Could not match this call to the dancers"));
		}
		if (_steps.size() == 0)
			return fail(context->stage()->newExplanation(PROGRAM_BUG, "No steps constructed"));
	}
	if (_failed)
		return false;
	else
		return true;
}

bool Plan::construct(const VariantTile* matched, Context* context, TileAction tileAction) {
	if (_failed)
		return false;
	if (_steps.size() == 0) {
		_applied = matched->variant;
		_matched = matched->pattern;
		return matched->variant->construct(this, _call, context, tileAction);
	}
	return true;
}

Step* Plan::constructRawStep(Context* context) {
	Step* s = context->stage()->newStep(this);
	_steps.push_back(s);
	return s;
}

StartTogetherStep* Plan::constructStartTogetherStep(Context* context) {
	StartTogetherStep* s = context->stage()->newStartTogetherStep(this);
	_steps.push_back(s);
	return s;
}

bool Plan::constructStep(const Part* part, Context* context, TileAction tileAction) {
	Step* s = context->stage()->newPartStep(this, part);
	if (_steps.size() == 0) {
		s->startWith(_orientedStart);
		s->construct(context, tileAction);
	}
	_steps.push_back(s);
	return !_failed;
}

Step* Plan::constructStep(const CompoundAction* action, Context* context, TileAction tileAction) {
	Step* s = context->stage()->newCompoundStep(this, action);
	if (_steps.size() == 0) {
		s->startWith(_start);
		s->construct(context, tileAction);
	}
	_steps.push_back(s);
	return s;
}

Step* Plan::constructStep(const Primitive* primitive, const Anything* parent, Context* context) {
	Step* s = context->stage()->newPrimitiveStep(this, primitive, parent);
	if (_steps.size() == 0 && _start)
		s->startWith(_start);
	_steps.push_back(s);
	return s;
}

Step* Plan::newStep(const Anything* call, Context* context) {
	Step* s = context->stage()->newCallStep(this, call);
	_steps.push_back(s);
	return s;
}

Step* Plan::constructStep(const Anything* call, Context* context, TileAction tileAction) {
	Step* s = context->stage()->newCallStep(this, call);
	if (_steps.size() == 0) {
		s->startWith(_start);
		s->construct(context, tileAction);
	}
	_steps.push_back(s);
	if (_failed)
		return null;
	else
		return s;
}

Step* Plan::insertStep(int i, Context* context) {
	Step* s = context->stage()->newStep(this);
	_steps.insert(i, s);
	return s;
}

void Plan::useStep(Step* s) {
	s->setPlan(this);
	_steps.push_back(s);
}

bool Plan::fractionalize(const Fraction* fraction, Context* context) {
	if (_failed)
		return false;
	if (_steps.size() == 0)
		return fail(context->stage()->newExplanation(PROGRAM_BUG, "Not enough constructed to fractionalize"));

	int num = fraction->numerator() * _steps.size();
	int denom = fraction->denominator();
	int parts = num / denom;
	int remainder = num % denom;
	if (remainder > 0) {
		if (verboseOutput) {
			printf("Fractionalizing num = %d denom = %d parts = %d remainder = %d\n", num, denom, parts, remainder);
			_steps[parts]->print(4, false);
		}
		_steps.resize(parts + 1);
		return _steps[parts]->fractionalize(context->stage()->newFraction(0, remainder, denom), context);
	} else {
		_steps.resize(parts);
		return true;
	}
}

bool Plan::performStep(int index, Context* context) {
	timing::Timer t("Plan::performStep");
	Step* s = _steps[index];
	while (s->start() == null) {
		const Group* previous;
		if (index > 0) {
			previous = _steps[index - 1]->combine(false, null, context);
			if (previous == null)
				return false;
			// The combine method may insert a new step, so we can't skip it.
			if (s != _steps[index]) {
				s = _steps[index];
				continue;
			}
			collectMotions(~0, null, context);
		} else
			previous = _start;
		s->startWith(previous);
	}
	if (s->construct(context, TILE_ALL) &&
		s->perform(context)) {
		return true;
	}
	if (s->failed())
		return false;
	else
		return fail(context->stage()->newExplanation(PROGRAM_BUG, "Step failed without explanation"));
}

const Group* Plan::breathe(Context* context) {
	if (_failed)
		return null;
	timing::Timer t("Plan::combine");
	if (!_resolution.resolved()) {
		if (_steps.size() == 0) {
			fail(context->stage()->newExplanation(PROGRAM_BUG, "Plan with no steps defined got to combine without failing"));
			return null;
		}
		for (;;) {
			int oldSize = _steps.size();
			_resolution.resolvePlan(this, context);
			if (_failed)
				return null;
			if (_resolution.final() == null) {
				fail(context->stage()->newExplanation(PROGRAM_BUG, "Step finished with no final dancers formation"));
				return null;
			}
			if (oldSize == _steps.size())
				break;
			if (!performStep(_steps.size() - 1, context))
				return null;
		}
		collectMotions(~0, null, context);
		if (!_resolution.toCommonCoordinates(_start, context)) {
			fail(context->stage()->newExplanation(PROGRAM_BUG, "Final not derived directly from start"));
			return null;
		}
		if (_resolution.dancerCount() != _start->dancerCount()) {
			fail(context->stage()->newExplanation(PROGRAM_BUG, "Plan finished with a different number of dancers"));
			return null;
		}
	}
	if (_failed)
		return null;
	else
		return _resolution.final();
}

void Plan::collectMotions(unsigned mask, MotionSet* enclosing, Context* context) const {
	if (!_failed) {
		// A null interval here means this was an inactive dancers tile.
		if (_interval) {
			if (!_collected) {
				for (int i = 0; i < _steps.size(); i++)
					_steps[i]->collectMotions(_interval, context);
			}
			if (enclosing) {
				_collected = true;
				if (mask != ~0)
					_interval->trim(mask);
				_interval->fillGaps(_start, context);
				enclosing->remember(_interval);
			}
		}
	}
}

void Plan::makeInterval(MotionSet* enclosing, bool startTogether) {
	if (_interval == null)
		_interval = new Interval(enclosing, startTogether); 
}

int Plan::nextPhantom() {
	return MAX_DANCERS + _phantomCount++;
}

unsigned Plan::startingDancersMask() const {
	if (_steps.size() == 0)
		return _start->dancerMask();
	else
		return _steps[0]->startingDancersMask();;
}

void Plan::trimStart(const Group* originalStart, const Group* outerStart, unsigned mask, Context* context) {
	if (verboseOutput) {
		printf("Plan::trimStart:\n");
		printf("  _start:\n");
		_start->printDetails(4, true);
		printf("  originalStart:\n");
		originalStart->printDetails(4, true);
		printf("  outerStart:\n");
		outerStart->printDetails(4, true);
		printf("  new _start:\n");
		originalStart->addTransforms(_start, outerStart->extract(mask & _start->dancerMask(), context), context)->printDetails(4, true);
	}
	const Group* priorStart = _start;
	_start = originalStart->addTransforms(priorStart, outerStart->extract(mask & _start->dancerMask(), context), context);
	if (_steps.size() > 0)
		_steps[0]->trimStart(priorStart, _start, mask, context);
}

unsigned Plan::lastActiveDancersMask(const Step* requester) const {
	int i;

	for (i = 0; i < _steps.size(); i++)
		if (_steps[i] == requester) 
			break;
	if (i)
		return _steps[i - 1]->lastActiveDancersMask();
	else if (_enclosing)
		return _enclosing->lastActiveDancersMask();
	else
		return _start->dancerMask();
}

Interval* Plan::lastInterval(const Step* requester) const {
	if (_interval != null)
		return _interval;

	int i;

	for (i = 0; i < _steps.size(); i++)
		if (_steps[i] == requester) 
			break;
	if (i)
		return _steps[i - 1]->lastInterval();
	else if (_enclosing)
		return _enclosing->lastInterval();
	else
		return null;
}

bool Plan::fail(const Explanation* ex) {
	if (!_failed) {
		_failed = true;
		_cause = ex;
		if (_enclosing &&
			_enclosing->enclosing())
			_enclosing->enclosing()->fail(ex);
	}
	return false;
}

void Plan::warn(const Explanation* ex) {
	if (!_failed && _cause == null)
		_cause = ex;
}

bool Plan::inStage(Stage* stage) const {
	if (_enclosing &&
		_enclosing->enclosing())
		return _enclosing->enclosing()->inStage(stage);
	else
		return false;
}

unsigned Plan::lastActiveMask() const {
	if (_steps.size())
		return _steps[_steps.size() - 1]->lastActiveMask();
	else
		return _resolution.lastActiveMask();
}

void Plan::print(int indent) const {
	if (_start) {
		printf("%*.*c %d phantoms ", indent, indent, ' ', _phantomCount);
		if (_enclosing &&
			_enclosing->enclosing() &&
			!_start->basedOn(_enclosing->enclosing()->start()))
			printf("Start (*** not based on enclosing step start ***):\n");
		else
			printf("Start:\n");
		_start->print(indent);
	}
	if (_call)
		_call->print(indent + 4);
	if (_steps.size() == 0) {
		if (_applied) {
			printf("%*.*cApplied %s:\n", indent, indent, ' ', _applied->definition()->productions()[0].c_str());
			_orientedStart->print(indent + 4);
		}
	}
	for (int i = 0; i < _steps.size(); i++) {
		printf("%*.*cStep %d:\n", indent, indent, ' ', i);
		_steps[i]->print(indent + 4, true);
	}
	if (_failed) {
		printf("%*.*c*** Failed ***\n", indent, indent, ' ');
		if (_cause)
			_cause->print(indent + 4);
	}
	_resolution.print(indent);
	if (_interval)
		_interval->print(indent);
	else
		printf("%*.*c_interval <null>\n", indent, indent, ' ');
}

Step::Step(Plan* plan, BreatheAction breatheAction) : _resolution(breatheAction) {
	_plan = plan;
	_start = null;
	_lastActiveMask = ~0;
	_failed = false;
	_cause = null;
	_interval = null;
	_rotation = 0;
}

Step::~Step() {
	delete _interval;
}

bool Step::construct(Context* context, TileAction tileAction) {
	return true;
}

bool Step::fractionalize(const Fraction* fraction, Context* context) {
	if (_failed)
		return false;
	for (int i = 0; i < _tiles.size(); i++) {
		if (verboseOutput) {
			printf("Fractionalizing tile %d by ", i);
			fraction->print(0);
			_tiles[i]->plan()->print(4);
		}
		if (!_tiles[i]->plan()->fractionalize(fraction, context))
			return false;
	}
	return true;
}

bool Step::perform(Context* context) {
	timing::Timer t("Step::perform");
	if (_failed)
		return false;
	if (_tiles.size() == 0)
		return fail(context->stage()->newExplanation(PROGRAM_BUG, "no tiles to perform"));

	if (_interval == null)
		_interval = new Interval(_plan->interval(), true);
	unsigned lastActiveMask = 0;
	for (int i = 0; i < _tiles.size(); i++) {
		if (_tiles[i]->active()) {
			if (_tiles[i]->enclosing() != this)
				return fail(context->stage()->newExplanation(PROGRAM_BUG, "a tile (" + string(i) + ") is not enclosed by its step"));
			if (!_tiles[i]->perform(context)) {
				if (_failed)
					return false;
				else
					return fail(context->stage()->newExplanation(PROGRAM_BUG, "Tile::perform returned false without failing"));
			}
			lastActiveMask |= _tiles[i]->plan()->lastActiveMask();
		}
	}
	if (_lastActiveMask == ~0)
		_lastActiveMask = lastActiveMask;
	return true;
}

void Step::collectMotions(MotionSet* enclosing, Context* context) const {
	if (!_failed) {
		if (_interval) {
			for (int i = 0; i < _tiles.size(); i++)
				_tiles[i]->plan()->collectMotions(~0, _interval, context);
			_interval->setRotation(_rotation);
			if (enclosing) {
				_interval->fillGaps(_start, context);
				enclosing->remember(_interval);
			}
		}
	}
}

const Group* Step::combine(bool mustBreathe, const Group* abstracted, Context* context) {
	if (_failed)
		return null;
	if (!_resolution.resolved()) {
		if (mustBreathe)
			_resolution.setBreatheAction(BREATHE);
		_resolution.resolveStep(this, abstracted, context);
	}
	return _resolution.final();
}

void Step::collectOutcomes(Context* context) {
	_resolution.collectOutcomes(this, context);
}

void Step::constructOutcomes() {
	_resolution.constructOutcomes(this);
}

const Anything* Step::displayCall() const {
	return null;
}

void Step::print(int indent, bool printChildren) const {
	printCommon(indent, printChildren);
}

void Step::makeInterval(Interval* enclosing, bool startTogether) { 
	if (_interval == null)
		_interval = new Interval(enclosing, startTogether); 
}

void Step::setInterval(Interval* newInterval) {
	_interval = newInterval;
}

Tile* Step::newTile(const Group* start, const Anything* call, Context* context) {
	Tile* t = context->stage()->newTile(this, start, call, null);
	_tiles.push_back(t);
	return t;
}

Tile* Step::constructTile(const Group* start, const Anything* call, Context* context, TileAction tileAction) {
	Tile* t = context->stage()->newTile(this, start, call, null);
	t->construct(context, tileAction);
	_tiles.push_back(t);
	return t;
}

Tile* Step::constructTile(const Group* start, const Anything* call, Context* context, unsigned activeDancers) {
	Tile* t = context->stage()->newTile(this, start, call, null);
	t->setLastActiveDancersMask(activeDancers);
	t->construct(context, TILE_ALL);
	_tiles.push_back(t);
	return t;
}

Tile* Step::constructTile(const Group* start, const Group* final, Context* context) {
	Tile* t = context->stage()->newTile(this, start, final);
	_tiles.push_back(t);
	return t;
}

void Step::addTile(Tile* t) {
	t->setEnclosing(this);
	_tiles.push_back(t); 
}

void Step::resolveReduction(const Group* reducedStart, const Group* reducedFinal, unsigned innerActiveMask, Interval* ss, Context* context) {
	unsigned outerActiveMask = 0;
	const Group* starters = _plan->start();
	if (starters == null) {
		fail(context->stage()->newExplanation(PROGRAM_BUG, "Null starters in resolveReduction"));
		return;
	}
	if (verboseOutput) {
		printf("Starters:\n");
		starters->printDetails(4, true);
	}
	Group* start3 = context->stage()->newGroup(starters);
	for (int i = 0; i < reducedFinal->dancerCount(); i++) {
		const Dancer* rf = reducedFinal->dancer(i);
		const Dancer* rs = reducedStart->dancerByIndex(rf->dancerIndex());
		const Group* pairStart = _resolution.outcome(rs->dancerIndex());
		if (innerActiveMask & (1 << rf->dancerIndex()))
			outerActiveMask |= pairStart->dancerMask();
		Group* out = context->stage()->newGroup(starters);
		ss->currentDancers(out);
		for (int j = 0; j < pairStart->dancerCount(); j++) {
			const Dancer* ps = pairStart->dancer(j);

			// From the original pair dancer, calculate the forward/lateral relation to the reduced dancer
			int forward, lateral;
			ps->displace(rs->x - ps->x, rs->y - ps->y, &forward, &lateral);
			// Then calculate the relative location of the final pair dancer using the forward/lateral calucations
			int dx, dy;
			rf->displace(forward, lateral, &dx, &dy);

			const Dancer* pf = new Dancer(rf->x - dx, rf->y - dy, rf->facing, ps->gender, ps->couple, ps->dancerIndex());

			start3->insert(pf);

			if (verboseOutput) {
				printf("forward=%d lateral=%d\n", forward, lateral);
				printf("Starting dancer:\n");
				ps->print(4);
				printf("which reduced to:\n");
				rs->print(4);
				printf("then moved to:\n");
				rf->print(4);
				printf("Reconstituting:\n");
				pf->print(4);
			}
			ss->matchTo(pf, context);
			out->insert(pf->clone());
		}
		out->done();
		_resolution.replaceOutcome(rs->dancerIndex(), out);
	}
	start3->done();
	_start = start3;
	makeInterval(_plan->interval(), true);
	ss->fillGaps(starters, context);
	_interval->remember(ss);				// We've now swept the s1 and s2 motions together
												// under s3, so that when we collect these motions,
												// only s3 will report out.
	combine(true, reducedFinal, context);
	_lastActiveMask = outerActiveMask;
}

unsigned Step::startingDancersMask() const {
	if (_tiles.size() == 0) {
		if (_start)
			return _start->dancerMask();
		else
			return 0;
	} else {
		unsigned mask = 0;
		for (int i = 0;  i < _tiles.size(); i++) {
			if (_tiles[i]->active())
				mask |= _tiles[i]->plan()->startingDancersMask();
		}
		return mask;
	}
}

void Step::trimStart(const Group* originalStart, const Group* outerStart, unsigned mask, Context* context) {
	if (_start) {
		if (verboseOutput) {
			printf("Step::trimStart:\n");
			printf("  _start:\n");
			_start->printDetails(4, true);
			printf("  originalStart:\n");
			originalStart->printDetails(4, true);
			printf("  outerStart:\n");
			outerStart->printDetails(4, true);
			printf("  new _start:\n");
			originalStart->addTransforms(_start, outerStart->extract(mask & _start->dancerMask(), context), context)->printDetails(4, true);
		}
		const Group* priorStart = _start;
		_start = originalStart->addTransforms(priorStart, outerStart->extract(mask & _start->dancerMask(), context), context);
		for (int i = 0;  i < _tiles.size(); i++)
			if (_tiles[i]->active())
				_tiles[i]->plan()->trimStart(priorStart, _start, mask, context);
	}
}

unsigned Step::lastActiveDancersMask() const {
	if (_lastActiveMask != ~0)
		return _lastActiveMask;
	else {
		if (_plan)
			return _plan->lastActiveDancersMask(this);
		if (_start)
			return _start->dancerMask();
	}
	return 0;
}

Interval* Step::lastInterval() const {
	if (_interval != null)
		return _interval;
	else if (_plan)
		return _plan->lastInterval(this);
	return null;
}

bool Step::fail(const Explanation* ex) {
	if (!_failed) {
		_failed = true;
		_cause = ex;
		if (_plan)
			_plan->fail(ex);
	}
	return false;
}

bool Step::inStage(Stage* stage) const {
	if (_plan)
		return _plan->inStage(stage);
	else
		return false;
}

void Step::discardTiles(int newTileCount) {
	_tiles.resize(newTileCount);
}

void Step::printCommon(int indent, bool printChildren) const {
	printf("%*.*c%p %s:", indent, indent, ' ', this, typeid(*this).name());
	printf("\n");
	if (_start) {
		printf("%*.*c", indent, indent, ' ');
		if (_plan &&
			!_start->basedOn(_plan->start()))
			printf("Start (*** not based on enclosing step start ***):\n");
		else
			printf("Start:\n");
		_start->print(indent);
	}
	if (printChildren) {
		for (int i = 0; i < _tiles.size(); i++) {
			printf("%*.*cTile %d:\n", indent, indent, ' ', i);
			_tiles[i]->print(indent + 4);
		}
	} else
		printf("%*.*c%d Tiles.\n", indent, indent, ' ', _tiles.size());
	if (_failed) {
		printf("%*.*c*** Failed ***\n", indent, indent, ' ');
		if (_cause)
			_cause->print(indent + 4);
	}
	if (_lastActiveMask != ~0)
		printf("%*.*cLast Active: %0x\n", indent, indent, ' ', _lastActiveMask);
	_resolution.print(indent);
	if (_interval)
		_interval->print(indent);
	else
		printf("%*.*c<no interval>\n", indent, indent, ' ');
}

bool DefinitionStep::construct(Context* context, TileAction tileAction) {
	timing::Timer t("DefinitionStep::construct");
	if (failed())
		return false;
	if (_tiles.size() == 0) {
		const vector<VariantTile>& tiles = _definition->tiles();
		TileSearch bestTiling[MAX_DANCERS];

		// Try to tile the variants over the dancers.
		int result = _start->buildTiling(tiles, bestTiling, context, _call, this, tileAction);
		if (result < 0)
			return fail(context->stage()->newExplanation(USER_ERROR, "Could not find a unique grouping of dancers"));
		if (tileAction == TILE_ALL) {
			int matchedDancers = 0;
			for (int i = 0; i < result; i++)
				matchedDancers += bestTiling[i].dancers->dancerCount();
			if (matchedDancers != _start->dancerCount())
				return fail(context->stage()->newExplanation(USER_ERROR, "Not all dancers can perform this call"));
		}
		for (int i = 0; i < result; i++) {
			Tile* t = context->stage()->newTile(this, bestTiling[i].dancers, _call, bestTiling[i].matched);
			_tiles.push_back(t);
			if (tileAction != TILE_WITH_PHANTOMS)
				t->construct(context, TILE_ALL);
		}
	}
	return !failed();
}

const Anything* DefinitionStep::displayCall() const {
	return _call;
}

void DefinitionStep::print(int indent, bool printChildren) const {
	if (_definition)
		printf("%*.*cDefinition %s\n", indent, indent, ' ', _definition->label().c_str());
	printCommon(indent, printChildren);
}

bool PartStep::construct(Context* context, TileAction tileAction) {
	timing::Timer t("PartStep::construct");
	if (failed())
		return false;
	if (_tiles.size() == 0) {
		Tile* t = newTile(start(), _plan->call(), context);
		for (int i = 0; i < _part->actions(); i++) {
			const Action* action = _part->action(i);
			if (action->noop())
				continue;
			if (verboseMatching) {
				printf("PartStep::parse action ");
				action->print(0);
				if (_plan->start())
					_plan->start()->print(4);
				else
					printf("No start formation\n");
			}
			Step* s = action->construct(this, context, tileAction);
			if (s == null)
				return fail(context->stage()->newExplanation(PROGRAM_BUG, string(typeid(*action).name()) + "::construct returned null"));

			if (i < _part->actions() - 1)
				s->setBreatheAction(DONT_BREATHE);
		}
//		simplifyStructure();
	}
	return true;
}

bool PartStep::perform(Context* context) {
	if (!Step::perform(context))
		return false;
	if (combine(true, null, context) == null)
		return false;
	if (failed())
		return false;
	if (verboseBreathing)
		printf("About to check same position rule:\n");
	_resolution.checkSamePositionRule(this, context);
	if (failed())
		return false;
	if (_resolution.final())
		return true;
	else
		return fail(context->stage()->newExplanation(PROGRAM_BUG, "Could not apply Same Position Rule"));
}

void PartStep::print(int indent, bool printChildren) const {
	printCommon(indent, printChildren);
}

bool CompoundStep::construct(Context* context, TileAction tileAction) {
	timing::Timer t("CompoundStep::construct");
	if (failed())
		return false;
	if (_tiles.size() == 0) {
		Plan* variantPlan = _plan->enclosing()->enclosing()->plan();
		unsigned usedMask = 0;
		for (int i = 0; i < _action->tracks().size(); i++) {
			Track* t = _action->tracks()[i];
			if (t->noop())
				continue;
			const Term* local = null;
			const Anyone* who = context->grammar()->parseAnyone(variantPlan->orientedStart(), t->who, _plan->call(), context, variantPlan, &local);
			if (who == null)
				return fail(context->stage()->newExplanation(DEFINITION_ERROR, "Unrecognized designator in track " + string(i) + ": " + t->who));
			_lastActiveMask = usedMask;
			unsigned mask = who->match(start(), this, context);
			if (local)
				variantPlan->defineLocal(local, context->stage()->newAnyone(DANCER_MASK, mask, null, null, NO_LEVEL));
			if (mask == 0) {
				if (t->anyWhoCan)
					continue;
				return fail(context->stage()->newExplanation(USER_ERROR, "No '" + baseDancers(t->who) + "' dancers for this call"));
			}
			usedMask |= mask;
			const Anything* what = context->grammar()->parse(variantPlan->orientedStart(), t->what, true, _plan->call(), context, variantPlan);
			if (what == null)
				return fail(context->stage()->newExplanation(DEFINITION_ERROR, "Unrecognized call in track " + string(i)));
			constructTile(start()->extract(mask, context), what, context, mask);
		}
		// If there are un-named dancers, be sure to include them as 'inactives'
		unsigned others = start()->dancerMask() & ~usedMask;
		if (others) {
			const Group* g = start()->extract(others, context);
			constructTile(g, context->stage()->newAnything(&Primitive::nothing), context, others);
		}
		_lastActiveMask = ~0;
	}
	return true;
}

void CompoundStep::print(int indent, bool printChildren) const {
	printCommon(indent, printChildren);
}

void StartTogetherStep::collectOutcomes(Context* context) {
	resolution().collectOutcomes(_mask0, _mask1, this, context);
}

void StartTogetherStep::collectMotions(MotionSet* enclosing, Context* context) const {
	if (!failed()) {
		if (_interval) {
			_tiles[0]->plan()->collectMotions(_mask0, _interval, context);
			_tiles[1]->plan()->collectMotions(_mask1, _interval, context);
			_interval->setRotation(_rotation);
			if (enclosing) {
				_interval->fillGaps(_start, context);
				enclosing->remember(_interval);
			}
		}
	}
}

bool PrimitiveStep::construct(Context* context, TileAction tileAction) {
	return !failed();
}

unsigned PrimitiveStep::startingDancersMask() const {
	unsigned x = _primitive->startingDancersMask(this);
	if (x == INT_MAX)
		return Step::startingDancersMask();
	else
		return x;
}

void PrimitiveStep::trimStart(const Group* originalStart, const Group* outerStart, unsigned mask, Context* context) {
	if (verboseOutput) {
		printf("PrimitiveStep::trimStart:\n");
		printf("  _start:\n");
		_start->printDetails(4, true);
		printf("  originalStart:\n");
		originalStart->printDetails(4, true);
		printf("  outerStart:\n");
		outerStart->printDetails(4, true);
		printf("  new _start:\n");
		originalStart->addTransforms(_start, outerStart->extract(mask & _start->dancerMask(), context), context)->printDetails(4, true);
	}
	_start = originalStart->addTransforms(_start, outerStart->extract(mask & _start->dancerMask(), context), context);
	_primitive->trimStart(this, mask, context);
}

bool PrimitiveStep::fractionalize(const Fraction *fraction, Context* context) {
	if (_primitive->fractionalize(this, _parent, fraction, context)) {
		_fraction = fraction;
		return true;
	} else
		return false;
}

bool PrimitiveStep::perform(Context* context) {
	if (failed())
		return false;
	if (_lastActiveMask == ~0) {
		if (_interval == null)
			_interval = new Interval(_plan->interval(), false);
		const Group* d = _primitive->execute(this, _parent, _fraction, context);
		if (d) {
			_resolution.exclusiveOutcome(d);
			_resolution.resolveStep(this, null, context);
			if (_resolution.final() == null)
				return fail(context->stage()->newExplanation(PROGRAM_BUG, string(primitiveNames[_primitive->index()]) + " failed - no final formation"));
		} else {
			if (failed())
				return false;
			else
				return fail(context->stage()->newExplanation(PROGRAM_BUG, string(primitiveNames[_primitive->index()]) + " failed - no explanation given"));
		}
		if (_lastActiveMask == ~0)
			_lastActiveMask = _resolution.final()->dancerMask();
	}
	return true;
}

const Anything* PrimitiveStep::displayCall() const {
	return _parent;
}

void PrimitiveStep::print(int indent, bool printChildren) const {
	if (_primitive)
		_primitive->print(indent);
	printCommon(indent, printChildren);
}

bool CallStep::construct(Context* context, TileAction tileAction) {
	timing::Timer t("CallStep::construct");
	if (failed())
		return false;
	if (_tiles.size() == 0)
		constructTile(start(), _action, context, tileAction);
	return true;
}

const Anything* CallStep::displayCall() const {
	return _action;
}

void CallStep::print(int indent, bool printChildren) const {
	if (_action)
		_action->print(indent);
	printCommon(indent, printChildren);
}

Tile::Tile(Step* enclosing, const Group* start, const Anything* call, const VariantTile* matched) 
    : _plan(start, call, enclosing != null ? enclosing->plan() : null) {
	_plan._enclosing = this;
	_enclosing = enclosing;
	_matched = matched;
	_active = true;
	_lastActiveDancersMask = ~0;
}

Tile::Tile(Step* enclosing, const Group* start, const Group* final) 
	: _plan(start, null, enclosing != null ? enclosing->plan() : null) {
	_plan._enclosing = this;
	_enclosing = enclosing;
	_matched = null;
	_active = false;
	_plan.setFinal(final);
	_lastActiveDancersMask = ~0;
}

void Tile::construct(Context* context, TileAction tileAction) {
	if (_matched)
		_plan.construct(_matched, context, tileAction);
	else
		_plan.construct(context, tileAction);
}

bool Tile::perform(Context* context) {
	if (_plan.enclosing() != this)
		return _enclosing->fail(context->stage()->newExplanation(PROGRAM_BUG, "Tile not enclosing of embedded plan"));
	return _plan.perform(_enclosing->interval(), context, TILE_ALL);
}

unsigned Tile::lastActiveDancersMask() const {
	if (_lastActiveDancersMask != ~0)
		return _lastActiveDancersMask;
	return _enclosing->lastActiveDancersMask();
}

Interval* Tile::lastInterval() const {
	return _enclosing->lastInterval();
}

void Tile::print(int indent) const {
	if (_active) {
		_plan.print(indent);
	} else {
		printf("%*.*cInactive:\n", indent, indent, ' ');
		_plan.start()->print(indent + 4);
	}
}

bool Tile::failed() const {
	return _plan.failed();
}

bool Tile::isPrimitive() const {
	if (_plan.stepCount() == 1)
		return typeid(*_plan.step(0)) == typeid(PrimitiveStep);
	else
		return false;
}

unsigned StubTile::lastActiveDancersMask() const {
	return _lastActiveDancersMask;
}

void Explanation::print(int indent) const {
	printf("%*.*c%s\n", indent, indent, ' ', _text.c_str());
}

static string baseDancers(const string& designator) {
	int i = designator.find('=');
	if (i == string::npos)
		return designator;
	else
		return designator.substr(i + 1).trim();
}

}  // namesapce dance
