#include "../common/platform.h"
#include "motion.h"

#include "../common/machine.h"
#include "../common/timing.h"
#include "call.h"
#include "dance.h"

namespace dance {

Motion* Interval::lastCurve(const Dancer* dancer, bool lastMotionOnly) const {
	Motion* m = MotionSet::lastCurve(dancer->dancerIndex(), lastMotionOnly);
	if (m)
		return m;
	for (MotionSet* outer = _enclosing; outer != null; outer = outer->enclosing()) {
		m = outer->lastCurve(dancer->dancerIndex(), lastMotionOnly);
		if (m)
			return m;
	}
	return null;
}

static double normalAngle(double angle) {
	if (angle > PI)
		angle -= 2 * PI;
	else if (angle <= -PI)
		angle += 2 * PI;
	return angle;
}

static bool sameAngle(double a, double b) {
	return abs(normalAngle(normalAngle(b) - normalAngle(a))) < EPSILON;
}

MotionSet::MotionSet(bool startTogether) {
	_enclosing = null;
	_startTogether = startTogether;
	_remembered = false;
	_rotation = 0;
}

MotionSet::~MotionSet() {
//	_motions.deleteAll();
}

bool MotionSet::validate(const Group* start, const Group* final) const {
	for (int i = 0; i < _motions.size(); i++) {
		if (_motions[i] == null)
			continue;
		const Dancer* s = start->dancerByIndex(i);
		const Dancer* f = final->dancerByIndex(i);
		float sx = s->x, sy = s->y;
		Facing sFacing = s->facing;
		double sNoseAngle;
		start->convertToAbsolute(&sx, &sy, &sFacing, &sNoseAngle);
		float fx = f->x, fy = f->y;
		Facing fFacing = f->facing;
		double fNoseAngle;
		final->convertToAbsolute(&fx, &fy, &fFacing, &fNoseAngle);
		// Motions are arranged in the list in time reverse order, so we
		// work from back to front.
		if (!_motions[i]->validate(&fx, &fy, &fNoseAngle)) {
			if (verboseOutput) {
				printf("Dancer #%d %s:\n", (i / 2) + 1, genderNames[i & 1]);
				_motions[i]->print(4);
				s->print(4);
				printf("        [%0.2f,%0.2f] nose %0.2f rad\n", sx, sy, sNoseAngle);
				f->print(4);
				printf("        [%0.2f,%0.2f] nose %0.2f rad\n", fx, fy, fNoseAngle);
			}
			return false;
		}
		if (abs(fx - sx) > EPSILON ||
			abs(fy - sy) > EPSILON ||
			!sameAngle(sNoseAngle, fNoseAngle)) {
			if (verboseOutput) {
				printf("Motion start location does not match computed start:\n");
				printf("    computed [%0.2f,%0.2f] nose %0.2f\n", fx, fy, fNoseAngle);
				printf("Dancer #%d %s:\n", (i / 2) + 1, genderNames[i & 1]);
				_motions[i]->print(4);
				s->print(4);
				printf("        actual   [%0.2f,%0.2f] nose %0.2f rad\n", sx, sy, sNoseAngle);
				printf("        computed [%0.2f,%0.2f] nose %0.2f rad\n", fx, fy, fNoseAngle);
				f->print(4);
			}
			return false;
		}
	}
	return true;
}

void MotionSet::checkFlow(FlowState* flowState, Stage *stage) const {
	for (int i = 0; i < _motions.size(); i++) {
		if (_motions[i] == null)
			flowState[i].idle(i, stage->duration(), stage);
		else
			_motions[i]->checkFlow(i, flowState[i], stage, _motions[i]);
	}
}

// Warning: this code relies on the fact that intervals are built with their
// enclosing pointers intact.
Motion* MotionSet::lastCurve(int index, bool lastMotionOnly) const {
	for (Motion* m = motion(index); m != null; m = m->previous()) {
		double d = m->cumulativeNoseMotion();

		if (d != 0)
			return m;
		if (lastMotionOnly)
			return m;
	}
	return null;
}

void MotionSet::remember(MotionSet* i) {
	if (!i->_remembered) {
		beats startAt = duration();
		beats duration = i->duration();
		if (duration == 0)
			duration = 1;
		for (int j = 0; j < i->_motions.size(); j++) {
			if (i->_alsos[j]) {
				Motion* lastAlso = i->_alsos[j];
				while (lastAlso->also() != null) {
					lastAlso->setDuration(duration);
					lastAlso = lastAlso->also();
				}
				lastAlso->setDuration(duration);
				lastAlso->finalizeAlso();
				if (verboseOutput) {
					if (i->_motions[j] == null) {
						printf("Only also for dancer %d\n", j);
						lastAlso->print(4);
					}
				}
				lastAlso->combine(i->_motions[j]);
				i->_motions[j] = i->_alsos[j];
				i->_alsos[j] = null;
			}
			if (i->_motions[j] == null)
				continue;
			if (!_startTogether) {
				if (i->_motions[j])
					i->_motions[j]->scheduleAfter(startAt);
			}
			remember(j, i->_motions[j]);
		}
		i->_remembered = true;
	}
}

void MotionSet::remember(int dancerIndex, Motion* m) {
	if (dancerIndex >= MAX_DANCERS)
		return;
	while (dancerIndex >= _motions.size()) {
		_motions.push_back(null);
		_alsos.push_back(null);
	}
	Motion* endM = m;
	while (endM->previous())
		endM = endM->previous();
	endM->prepend(_motions[dancerIndex]);
	_motions[dancerIndex] = m;
}

void MotionSet::replace(int dancerIndex, Motion* m) {
	if (dancerIndex >= MAX_DANCERS)
		return;
	while (dancerIndex >= _motions.size()) {
		_motions.push_back(null);
		_alsos.push_back(null);
	}
	_motions[dancerIndex] = m;
}

void MotionSet::also(int dancerIndex, Straight* m) {
	if (dancerIndex >= MAX_DANCERS)
		return;
	while (dancerIndex >= _motions.size()) {
		_motions.push_back(null);
		_alsos.push_back(null);
	}
	m->combine(_alsos[dancerIndex]);
	_alsos[dancerIndex] = m;
}

beats MotionSet::duration() const {
	int dur = 0;
	for (int i = 0; i < _motions.size(); i++)
		if (_motions[i] && _motions[i]->endAt() > dur)
			dur = _motions[i]->endAt();
	return dur;
}

Motion* MotionSet::motion(int i) const {
	if (i >= _motions.size())
		return null;
	else
		return _motions[i];
}

void MotionSet::printCommon(int indent) const {
	if (!_remembered) {
		if (_motions.size()) {
			printf("%*.*cMotions", indent, indent, ' ');
			if (_startTogether)
				printf(" start together");
			printf(":\n");
			for (int i = 0; i < _motions.size(); i++) {
				if (_motions[i] || _alsos[i])
					printf("%*.*c   Dancer #%d %s:\n", indent, indent, ' ', (i / 2) + 1, genderNames[i & 1]);
				if (_motions[i])
					_motions[i]->print(indent + 6);
				if (_alsos[i]) {
					printf("%*.*c       alsos:\n", indent, indent, ' ');
					_alsos[i]->print(indent + 6);
				}
			}
		} else
			printf("%*.*c<not remembered>\n", indent, indent, ' ');
	} else
		printf("%*.*c<remembered>\n", indent, indent, ' ');
}

Stage::Stage(const Sequence* sequence, const Group* start) : Plan(start, null, null), _motions(false) {
	_sequence = sequence;
}

Stage::~Stage() {
	_plans.deleteAll();
	_steps.deleteAll();
	_tiles.deleteAll();
	_dancers.deleteAll();
	_terms.deleteAll();
	_explanations.deleteAll();
	_allocedMotions.deleteAll();
}

bool Stage::inStage(Stage* stage) const {
	return stage == this;
}

void Stage::collectMotions() {
	timing::Timer t("Stage::collectMotions");
	Context context(null, null);
	context.startStage(this);
	Plan::collectMotions(~0, &_motions, &context);
	if (!_motions.validate(_start, _resolution.final()))
		fail(newExplanation(PROGRAM_BUG, "Motions are not valid"));
}

void Stage::checkFlow(FlowState* flowState) {
	timing::Timer t("Stage::checkFlow");
	_motions.checkFlow(flowState, this);
}

bool Stage::resolved() const {
	const Group* d = final();
	if (d == null)
		return false;
	return d->atHome();
}

Plan* Stage::newPlan(const Plan* outer, Tile* enclosing, const Group* start, const Anything* call) {
	Plan* p = new Plan(start, call, outer);
	p->_enclosing = enclosing;
	_plans.push_back(p);
	return p;
}

Step* Stage::newStep(Plan* p) {
	Step* s = new Step(p);
	_steps.push_back(s);
	return s;
}

StartTogetherStep* Stage::newStartTogetherStep(Plan* p) {
	StartTogetherStep* s = new StartTogetherStep(p);
	_steps.push_back(s);
	return s;
}

DefinitionStep* Stage::newDefinitionStep(Plan* plan, const Definition* definition, const Anything* call, const Group* start) {
	DefinitionStep* d = new DefinitionStep(plan, definition, call, start);
	_steps.push_back(d);
	return d;
}

CallStep* Stage::newCallStep(Plan* plan, const Anything* action) {
	CallStep* c = new CallStep(plan, action);
	_steps.push_back(c);
	return c;
}

PrimitiveStep* Stage::newPrimitiveStep(Plan* plan, const Primitive* primitive, const Anything* parent) {
	PrimitiveStep* p = new PrimitiveStep(plan, primitive, parent);
	_steps.push_back(p);
	return p;
}

PartStep* Stage::newPartStep(Plan* plan, const Part* part) {
	PartStep* p = new PartStep(plan, part);
	_steps.push_back(p);
	return p;
}

CompoundStep* Stage::newCompoundStep(Plan* plan, const CompoundAction* action) {
	CompoundStep* c = new CompoundStep(plan, action);
	_steps.push_back(c);
	return c;
}

Tile* Stage::newTile(Step* enclosing, const Group* start, const Anything* call, const VariantTile* matched) {
	Tile* t = new Tile(enclosing, start, call, matched);
	_tiles.push_back(t);
	return t;
}

Tile* Stage::newTile(Step* enclosing, const Group* start, const Group* final) {
	Tile* t = new Tile(enclosing, start, final);
	_tiles.push_back(t);
	return t;
}

Group* Stage::newGroup(const Group* base) {
	Group* d = new Group(base);
	_dancers.push_back(d);
	return d;
}

Explanation* Stage::newExplanation(ExplanationClass exClass, const string& text) {
	Explanation* e = new Explanation(exClass, text);
	_explanations.push_back(e);
	return e;
}

Group* Stage::newGroup(Geometry geometry) {
	Group* d = new Group(geometry);
	_dancers.push_back(d);
	return d;
}

Anything* Stage::newAnything(const Primitive* primitive) {
	Anything* a = new Anything(true, primitive, null);
	_terms.push_back(a);
	return a;
}

Anything* Stage::newAnything(bool inDefinition, const Definition* definition) {
	if (!inDefinition && _sequence && _sequence->level() > NO_LEVEL) {
		if (definition->level() > _sequence->level()) {
			fail(newExplanation(USER_ERROR, "'" + definition->label() + "' is off-level (" + levels[definition->level()] + ")"));
			return null;
		}
	}
	Anything* a = new Anything(inDefinition, null, definition);
	_terms.push_back(a);
	return a;
}

Integer* Stage::newInteger(int value) {
	Integer* i = new Integer(value);
	_terms.push_back(i);
	return i;
}

Anyone* Stage::newAnyone(DancerSet dancerSet, unsigned mask, const Anyone* left, const Anyone* right, Level level) {
	Anyone* a = new Anyone(dancerSet, mask, left, right, level);
	_terms.push_back(a);
	return a;
}

Fraction* Stage::newFraction(int whole, int num, int denom) {
	Fraction* f = new Fraction(whole, num, denom);
	_terms.push_back(f);
	return f;
}

Straight* Stage::newStraight(Point start, Point end, double startNose, double noseMotion, beats duration) {
	Straight* s = new Straight(start, end, startNose, noseMotion, duration);
	_allocedMotions.push_back(s);
	return s;
}

Curve* Stage::newCurve(Point center, double motionAngle, double radius, Point start, Point end, double startNose, double noseMotion, beats duration) {
	Curve* c = new Curve(center, motionAngle, radius, start, end, startNose, noseMotion, duration);
	_allocedMotions.push_back(c);
	return c;
}

void Stage::print() const {
	_motions.print();
	Plan::print(0);
}

void Interval::arc(const Dancer* dancer, Point center, double radius, double motionAngle, int noseQuarterTurns, beats duration, Context* context) {
	Point start(dancer->x, dancer->y);
	double startNose;
	Facing facing = dancer->facing;

	Point end = start;
	double deltaX = start.x - center.x;
	double deltaY = start.y - center.y;
	double startLength = sqrt(deltaX * deltaX + deltaY * deltaY);
	double thisAngle;
	if (radius > 0) {
		thisAngle = angle(deltaX, deltaY) + motionAngle;
		end.x = center.x + cos(thisAngle) * radius;
		end.y = center.y + sin(thisAngle) * radius;
	}
	_currentDancers->convertToAbsolute(&start.x, &start.y, &facing, &startNose);
	_currentDancers->convertToAbsolute(&center.x, &center.y, null, null);
	_currentDancers->convertToAbsolute(&end.x, &end.y, null, null);
	_currentDancers->convertToAbsolute(&motionAngle);
	double noseMotion = motionAngle - noseQuarterTurns * PI / 2;

	if (_currentDancers->geometry() == RING && radius > 0) {
		double endDeltaX = end.x - center.x;
		double endDeltaY = end.y - center.y;
		double startDeltaX = start.x - center.x;
		double startDeltaY = start.y - center.y;
		double startAngle = angle(startDeltaX, startDeltaY);
		radius = sqrt(endDeltaX * endDeltaX + endDeltaY * endDeltaY);
		double endAngle = angle(endDeltaX, endDeltaY);
		double deltaAngle = endAngle - startAngle - motionAngle;
		if (deltaAngle > PI)
			deltaAngle -= 2 * PI;
		else if (deltaAngle < -PI)
			deltaAngle += 2 * PI;
		motionAngle += deltaAngle;
		noseMotion += deltaAngle + deltaAngle;
	}
	constructArc(dancer, center, start, end, radius, motionAngle, startNose, noseMotion, duration, context);
}

void Interval::constructArc(const Dancer* dancer, const Point& center, const Point& start, const Point& end, double radius, double motionAngle, double startNose, double noseMotion, beats duration, Context* context) {
	Motion* m = context->stage()->newCurve(center, motionAngle, radius, start, end, startNose, noseMotion, duration);
	perform(dancer->dancerIndex(), m);
}

void Interval::forwardVeer(const Dancer* dancer, Point delta, float startYAdjust, double motionAngleAdjust, int rightQuarterTurns, beats duration, Context* context) {
	Point start(dancer->x, dancer->y);
	switch (dancer->facing) {
	default:
		start.x += startYAdjust;
		break;

	case	BACK_FACING:
		start.y += startYAdjust;
		break;

	case	LEFT_FACING:
		start.x -= startYAdjust;
		break;

	case	FRONT_FACING:
		start.y -= startYAdjust;
	}
	displace(dancer, start.x, start.y, delta.x, delta.y, rightQuarterTurns, motionAngleAdjust, duration, context);
}

void Interval::setToRing(const Dancer* dancer, const Group* ring, const Dancer* ringDancer, int ringX, int ringY, beats duration, Context* context) {
	Point start(dancer->x, dancer->y);
	Point center;
	double startNose;
	double motionAngle;
	double radius;
	double noseMotion;
	Facing facing = dancer->facing;

	_currentDancers->convertToAbsolute(&start.x, &start.y, &facing, &startNose);
	facing = ringDancer->facing;
	Point end(ringX, ringY);
	double endNose;
	ring->convertToAbsolute(&end.x, &end.y, &facing, &endNose);

	noseMotion = endNose - startNose;
	if (noseMotion > PI)
		noseMotion -= 2 * PI;
	else if (noseMotion < -PI)
		noseMotion += 2 * PI;

	Motion* m = context->stage()->newStraight(start, end, startNose, noseMotion, duration);
	perform(dancer->dancerIndex(), m);
}

void Interval::displace(const Dancer* dancer, float startX, float startY, float deltaX, float deltaY, int rightQuarterTurns, double motionAngleAdjust, beats duration, Context* context) {
	Point start(startX, startY);
	double startNose;
	Facing facing = dancer->facing;

	Point end = start;
	double endNose;
	Facing endFacing = quarterRight(dancer->facing, rightQuarterTurns);
	end.x += deltaX - motionAngleAdjust * 8 / PI;
	end.y += deltaY;
//	if (verboseOutput) {
//		dancer->print(0);
//		printf("    start [%g:%g] end [%g:%g] rightQuarterTurns %d motionAngleAdjust %g deltaX = %g\n", start.x, start.y, end.x, end.y, rightQuarterTurns, motionAngleAdjust, deltaX);
//	}
	_currentDancers->convertToAbsolute(&start.x, &start.y, &facing, &startNose);
	_currentDancers->convertToAbsolute(&end.x, &end.y, &endFacing, &endNose);
	double turnAngle = -rightQuarterTurns * PI / 2;
	_currentDancers->convertToAbsolute(&turnAngle);
	Motion* m;
	if (_currentDancers->geometry() == RING) {
		float radius = end.distance();
		Point center(0, 0);
		double motionAngle = motionAngleAdjust - _currentDancers->convertRingXToAngle(deltaX);
		_currentDancers->convertToAbsolute(&motionAngle);
		constructArc(dancer, center, start, end, radius, motionAngle, startNose, turnAngle + motionAngle, duration, context);
	} else {
		m = context->stage()->newStraight(start, end, startNose, turnAngle, duration);
		perform(dancer->dancerIndex(), m);
	}
}

void Interval::ringToSet(const Dancer* dancer, const Group* set, const Dancer* setDancer, beats duration, Context* context) {
	Point start(dancer->x, dancer->y);
	double startNose;
	double noseMotion;
	Facing facing = dancer->facing;

	_currentDancers->convertToAbsolute(&start.x, &start.y, &facing, &startNose);
	facing = setDancer->facing;
	Point end(setDancer->x, setDancer->y);
	double endNose;
	set->convertToAbsolute(&end.x, &end.y, &facing, &endNose);

	noseMotion = endNose - startNose;
	if (noseMotion > PI)
		noseMotion -= 2 * PI;
	else if (noseMotion < -PI)
		noseMotion += 2 * PI;

	Motion* m = context->stage()->newStraight(start, end, startNose, noseMotion, duration);
	perform(dancer->dancerIndex(), m);
}

Motion* Interval::pause(Motion* previous, const Dancer* d, beats duration, Context* context) {
	Point start;
	double startNose;
	Facing facing;

	if (previous == null) {
		start.x = d->x;
		start.y = d->y;
		facing = d->facing;
		_currentDancers->convertToAbsolute(&start.x, &start.y, &facing, &startNose);
	} else {
		start.x = previous->cumulativeEndX();
		start.y = previous->cumulativeEndY();
		startNose = previous->cumulativeEndNose();
	}
	return context->stage()->newStraight(start, start, startNose, 0, duration);
}

void Interval::perform(int dancerIndex, Motion* m) {
	if (motion(dancerIndex))
		m->scheduleAfter(motion(dancerIndex)->endAt());
	remember(dancerIndex, m);
}

void Interval::trim(unsigned mask) {
	for (int i = 0; i < motions().size(); i++) {
		if (mask & (1 << i))
			continue;
		motions()[i] = null;
	}
}

void Interval::fillGaps(const Group* dancers, Context* context) {
	currentDancers(dancers);
	beats duration = this->duration();
	for (int i = 0; i < motions().size(); i++) {
		if (alsos()[i] != null) {
			const Dancer* d = dancers->dancerByIndex(i);
			if (motions()[i] == null)
					perform(d->dancerIndex(), pause(null, d, duration, context));
			fillGaps(motions()[i], d, duration, context);
			if (motions()[i]->endAt() < duration) {
				Motion* p = pause(motions()[i], d, duration - motions()[i]->endAt(), context);
				p->scheduleAfter(motions()[i]->endAt());
				p->prepend(motions()[i]);
				motions()[i] = p;
			}
		}
	}
}

void Interval::fillGaps(Motion* m, const Dancer* d, beats duration, Context* context) {
	if (m->previous() == null) {
		if (m->startAt() > 0)
			m->prepend(pause(null, d, m->startAt(), context));
	} else {
		fillGaps(m->previous(), d, duration, context);
		if (m->previous()->endAt() < m->startAt()) {
			Motion* p = pause(m->previous(), d, m->startAt() - m->previous()->endAt(), context);
			p->scheduleAfter(m->previous()->endAt());
			p->prepend(m->previous());
			m->prepend(p);
		}
	}
}

void Interval::matchTo(const Dancer* dancer, Context* context) {
	int i = dancer->dancerIndex();
	if (i >= motions().size())			// really?  Shouldn't this check against some start loc for the dancer?
		return;
	if (motions()[i] == null)
		return;
	Point end(motions()[i]->cumulativeEndX(), motions()[i]->cumulativeEndY());
	Point loc(dancer->x, dancer->y);
	Facing facing = dancer->facing;
	double startNose;
	_currentDancers->convertToAbsolute(&loc.x, &loc.y, &facing, &startNose);
	if (verboseOutput) {
		printf("matching [%0.2f,%0.2f] to computed [%0.2f,%0.2f]\n", loc.x, loc.y, end.x, end.y);
		dancer->print(4);
	}
	if (loc.x != end.x || loc.y != end.y) {
		loc.x -= end.x;
		loc.y -= end.y;
		MotionSet::also(i, context->stage()->newStraight(Point(0,0), loc, startNose, 0, 0));
	}
}

void Interval::also(const Dancer* dancer, int deltaX, int deltaY, Context* context) {
	Point end(deltaX, deltaY);
	Point origin;
	_currentDancers->convertToAbsolute(&end.x, &end.y, null, null);
	_currentDancers->convertToAbsolute(&origin.x, &origin.y, null, null);
	end.x -= origin.x;
	end.y -= origin.y;
	MotionSet::also(dancer->dancerIndex(), context->stage()->newStraight(Point(0, 0), end, 0, 0, 0));
}

void Interval::print(int indent) const {
	printCommon(indent);
}

double Point::distance() const {
	return sqrt(x * x + y * y);
}

void Motion::scheduleAfter(beats startAt) {
	if (_previous)
		_previous->scheduleAfter(startAt);
	if (_also)
		_also->scheduleAfter(startAt);
	_startAt += startAt;
}

double Motion::effectiveNoseMotion() const {
	if (_also)
		return _noseMotion + _also->cumulativeNoseMotion();
	else
		return _noseMotion;
}

double Motion::effectiveStartNose() const {
	if (_also)
		return _startNose + _also->cumulativeStartNose();
	else
		return _startNose;
}

double Motion::effectiveStartX() const {
	if (_also)
		return _start.x + _also->cumulativeStartX();
	else
		return _start.x;
}

double Motion::effectiveStartY() const {
	if (_also)
		return _start.y + _also->cumulativeStartY();
	else
		return _start.y;
}

double Motion::cumulativeNoseMotion() const {
	double m = _noseMotion;
	if (_previous)
		m += _previous->cumulativeNoseMotion();
	if (_also)
		m += _also->cumulativeNoseMotion();
	return m;
}

double Motion::cumulativeStartNose() const {
	if (_previous)
		return _previous->cumulativeStartNose();
	else if (_also)
		return _startNose + _also->cumulativeStartNose();
	else
		return _startNose;
}

double Motion::cumulativeStartX() const {
	if (_previous)
		return _previous->cumulativeStartX();
	else if (_also)
		return _start.x + _also->cumulativeStartX();
	else
		return _start.x;
}

double Motion::cumulativeStartY() const {
	if (_previous)
		return _previous->cumulativeStartY();
	else if (_also)
		return _start.y + _also->cumulativeStartY();
	else
		return _start.y;
}

double Motion::cumulativeEndX() const {
	if (_also)
		return _end.x + _also->cumulativeEndX();
	else
		return _end.x;
}

double Motion::cumulativeEndY() const {
	if (_also)
		return _end.y + _also->cumulativeEndY();
	else
		return _end.y;
}

double Motion::cumulativeEndNose() const {
	double n = _startNose + _noseMotion;
	if (_also)
		n += _also->cumulativeEndNose();
	return n;
}

double Motion::cumulativeRadius() const {
	if (_also)
		return _also->cumulativeRadius();
	if (typeid(*this) != typeid(Curve)) {
		if (_start.x != _end.x ||
			_start.y != _end.y)
			return 1;
		else
			return 0;				// Any Straight motion that is non-zero distance has 'lateral' flow consistent with nose motion
									// TODO: Need to investigate whether we can induce some contrary motions for this assertion
	}
	const Curve* c = (const Curve*)this;
	return c->radius();
}

bool Motion::validate(float* fx, float* fy, double* fNoseAngle) const {
	if (abs(*fx - cumulativeEndX()) > EPSILON ||
		abs(*fy - cumulativeEndY()) > EPSILON) {
		if (verboseOutput) {
			printf("Motion starting at %d does not end on expected location:\n", _startAt);
			printf("    expected [%0.2f,%0.2f] found [%0.2f,%0.2f]\n", *fx, *fy, cumulativeEndX(), cumulativeEndY());
		}
		return false;
	}
	double computedNoseAngle = effectiveStartNose() + effectiveNoseMotion();
	if (!sameAngle(computedNoseAngle, *fNoseAngle)) {
		if (verboseOutput) {
			printf("Motion starting at %d does not end on expected nose angle:\n", _startAt);
			printf("    expected nose %0.2f rad, found %0.2f rad\n", *fNoseAngle, computedNoseAngle);
		}
		return false;
	}
	if (_also) {
		float afx = *fx - end().x, afy = *fy - end().y;
		double afNoseAngle = *fNoseAngle - _noseMotion;
		_also->validate(&afx, &afy, &afNoseAngle);
		if (abs(afx - effectiveStartX()) > EPSILON ||
			abs(afy - effectiveStartY()) > EPSILON ||
			!sameAngle(afNoseAngle, effectiveStartNose())) {
			if (verboseOutput) {
				printf("Nested motion starting at does not begin in expected position:\n", _also->_startAt);
				printf("    expected start [%0.2f,%0.2f] nose %0.2f\n", effectiveStartX(), effectiveStartY(), effectiveStartNose());
				printf("    computed start [%0.2f,%0.2f] nose %0.2f\n", afx, afy, afNoseAngle);
			}
			return false;
		}
	}
	*fx = effectiveStartX();
	*fy = effectiveStartY();
	*fNoseAngle = effectiveStartNose();
	if (_previous)
		return _previous->validate(fx, fy, fNoseAngle);
	else
		return true;
}

void Motion::checkFlow(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const {
	if (_previous)
		_previous->checkFlow(dancerIndex, flowState, stage, top == this ? _previous : top);
	if (_also)
		_also->checkFlow(dancerIndex, flowState, stage, top);
	else
		checkThis(dancerIndex, flowState, stage, top);
}

Motion* Motion::split(const Group* pair, Point start, Facing* facing, int forward, int lateral, Context* context) const {
	Motion* also = null;
	Motion* previous = null;

	if (_previous) {
		previous = _previous->split(pair, start, facing, forward, lateral, context);
		start = previous->end();
		int leftQuarterTurns = round(previous->noseMotion() * (4 / PI));
		*facing = quarterLeft(*facing, leftQuarterTurns);
	}
	if (_also)
		also = _also->split(pair, start, facing, forward, lateral, context);

	Motion* m = splitThis(pair, start, forward, lateral, context);
	m->scheduleAfter(_startAt);
	m->combine(also);
	m->prepend(previous);
	return m;
}

void Motion::finalizeAlso() {
	_start.x = 0;
	_start.y = 0;
	_startNose = 0;
}

void Motion::convertAlsoToStraight() {
	// No active motion for this dancer, turn this last also into a proper Straight as
	// appropriate
	_end.x += _start.x;
	_end.y += _start.y;
}

void Motion::printCommon(int indent) const {
	if (_previous)
		_previous->print(indent);
	printf("%*.*c<%03d-%03d> [%0.2f,%0.2f] facing %0.2f -> [%0.2f,%0.2f] nose = %0.2f rad duration = %d",
		   indent, indent, ' ', 
		   startAt(),
		   endAt(),
		   start().x,
		   start().y,
		   startNose(),
		   end().x,
		   end().y,
		   noseMotion(),
		   duration());
}

void Straight::location(double fraction, double *x, double *y) const {
	Point s = start();
	Point e = end();
	double deltaX = e.x - s.x;
	double deltaY = e.y - s.y;
	*x = s.x + deltaX * fraction;
	*y = s.y + deltaY * fraction;
}

Motion* Straight::splitThis(const Group* pair, Point start, int dx, int dy, Context* context) const {
	Point e;
	if (also()) {
		if (verboseOutput) {
			printf("Splitting also straight: start=[%0.2f,%0.2f] dx=%d dy=%d\n", start.x, start.y, dx, dy);
			print(4);
		}
		return context->stage()->newStraight(this->start(), end(), startNose(), noseMotion(), duration());
	}
	Point s = start;
	e.x = start.x + end().x - this->start().x;
	e.y = start.y + end().y - this->start().y;
	if (verboseOutput) {
		printf("Splitting straight: start=[%0.2f,%0.2f] dx=%d dy=%d\n", start.x, start.y, dx, dy);
	}
	return context->stage()->newStraight(s, e, startNose(), noseMotion(), duration());
}

void Straight::checkThis(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const {
	double deltaX = (end().x - start().x) / duration();
	double deltaY = (end().y - start().y) / duration();
	if (abs(deltaX) > EPSILON || abs(deltaY) > EPSILON) {
		top->accumulateAlsoMotion(startAt(), &deltaX, &deltaY);
		flowState.straight(dancerIndex, angle(deltaX, deltaY), duration(), stage);
	} else
		flowState.lastMotion = NOT_AN_ANGLE;
	if (abs(noseMotion()) < EPSILON)
		flowState.idle(dancerIndex, duration(), stage);
	else
		flowState.turn(dancerIndex, noseMotion(), duration(), 0, stage);
}

void Straight::accumulateAlsoMotion(int at, double* deltaX, double* deltaY) const {
	// Ignore the last motion in the chain
	if (also() == null)
		return;
	if (at < startAt()) {
		if (previous())
			previous()->accumulateAlsoMotion(at, deltaX, deltaY);
		return;
	}
	*deltaX += (end().x - start().x) / duration();
	*deltaY += (end().y - start().y) / duration();
	also()->accumulateAlsoMotion(at, deltaX, deltaY);
}

void Straight::print(int indent) const {
	printCommon(indent);
	printf(" straight\n");
	if (also())
		also()->print(indent + 4);
}

void Curve::location(double fraction, double *x, double *y) const {
	Point s = start();
	Point center = _center;
	double deltaX = s.x - center.x;
	double deltaY = s.y - center.y;
	double startLength = sqrt(deltaX * deltaX + deltaY * deltaY);
	double thisAngle;
	if (fabs(deltaX) > EPSILON || fabs(deltaY) > EPSILON)
		thisAngle = angle(deltaX, deltaY) + fraction * _motionAngle;
	else
		thisAngle = _motionAngle;
	double thisLength = startLength + fraction * (_radius - startLength);

	*x = center.x + cos(thisAngle) * thisLength;
	*y = center.y + sin(thisAngle) * thisLength;
}

Motion* Curve::splitThis(const Group* pair, Point start, int forward, int lateral, Context* context) const {
	Point s = start;
	Point e;
	Point c;
	double sAngle = startNose();
	double dx = cos(sAngle) * forward + sin(sAngle) * lateral;
	double dy = -cos(sAngle) * lateral + sin(sAngle) * forward;
	double radius = _radius;
	if (radius > EPSILON) {
		double centerOffsetX = _center.x - this->start().x;
		double centerOffsetY = _center.y - this->start().y;
		c.x = start.x + centerOffsetX;
		c.y = start.y + centerOffsetY;
		if (dx > EPSILON) {			// couple, facing head walls
			if (centerOffsetX) {
				if ((dx > 0) != (centerOffsetX > 0)) {
					c.x -= dx + dx;
					radius += 2;
				}
			}
		} else {			// couple, facing side walls
			if (centerOffsetY) {
				if ((dy > 0) != (centerOffsetY > 0)) {
					c.y -= dy + dy;
					radius += 2;
				}
			}
		}
	} else {
		c.x = start.x - dx;
		c.y = start.y - dy;
		radius = 1;
	}
	double dAngle = angle(start.x - c.x, start.y - c.y);
	double eAngle = dAngle + _motionAngle;
	e.x = c.x + cos(eAngle) * radius;
	e.y = c.y + sin(eAngle) * radius;
	if (verboseOutput) {
		printf("Splitting curve: dx=%0.2f dy=%0.2f start=[%0.2f,%0.2f] dAngle=%0.2f eAngle=%0.2f\n", dx, dy, start.x, start.y, dAngle, eAngle);
		printf("    couple start=[%0.2f,%0.2f] _center=[%0.2f,%0.2f]\n", this->start().x, this->start().y, _center.x, _center.y);
	}
	return context->stage()->newCurve(c, _motionAngle, radius, s, e, sAngle, noseMotion(), duration());
}

void Curve::checkThis(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const {
	if (_radius > EPSILON) {
		double alsoX = 0;
		double alsoY = 0;
		top->accumulateAlsoMotion(startAt(), &alsoX, &alsoY);
		double sAngle = angle(start().x - _center.x, start().y - _center.y);
		if (_motionAngle > 0)
			sAngle += PI / 2;
		else
			sAngle -= PI / 2;
		double deltaX = alsoX + cos(sAngle);
		double deltaY = alsoY + sin(sAngle);
		flowState.straight(dancerIndex, angle(deltaX, deltaY), duration(), stage);
		double eAngle = angle(end().x - _center.x, end().y - _center.y);
		if (_motionAngle > 0)
			eAngle += PI / 2;
		else
			eAngle -= PI / 2;
		deltaX = alsoX + cos(eAngle);
		deltaY = alsoY + sin(eAngle);
		flowState.lastMotion = angle(deltaX, deltaY);
	} else
		flowState.lastMotion = NOT_AN_ANGLE;
	if (abs(noseMotion()) < EPSILON)
		flowState.idle(dancerIndex, duration(), stage);
	else
		flowState.turn(dancerIndex, noseMotion(), duration(), _radius, stage);
}

void Curve::accumulateAlsoMotion(int at, double* deltaX, double* deltaY) const {
}

void Curve::print(int indent) const {
	printCommon(indent);
	printf(" arc center [%0.2f,%0.2f] sweep = %0.2f rad radius = %0.2f\n",
		   _center.x,
		   _center.y,
		   _motionAngle,
		   _radius);
	if (also())
		also()->print(indent + 4);
}

void FlowState::idle(int dancerIndex, beats duration, Stage* stage) {
	if (turnState >= OVER_TURN || turnState <= -OVER_TURN) {
		stage->warn(stage->newExplanation(USER_ERROR, string(genderNames[genderOf(dancerIndex)]) + " #" + coupleOf(dancerIndex) + " has turned a little too much"));
		turnState = 0;
	} else if (turnState > duration)
		turnState -= duration;
	else if (turnState < -duration)
		turnState += duration;
	else
		turnState = 0;
}

void FlowState::turn(int dancerIndex, double angle, beats duration, double radius, Stage* stage) {
	if (radius < 1)
		radius = 1;
	turnState += int(round(4 * angle / (radius * PI)));
	if (turnState >= EXCESSIVE_TURN || turnState <= -EXCESSIVE_TURN) {
		stage->warn(stage->newExplanation(USER_ERROR, string(genderNames[genderOf(dancerIndex)]) + " #" + coupleOf(dancerIndex) + " has turned far too much"));
		turnState = 0;
	}
}

void FlowState::straight(int dancerIndex, double angle, beats duration, Stage* stage) {		// motion angle
	if (lastMotion == NOT_AN_ANGLE)
		return;
	double delta = normalAngle(lastMotion - angle);
	if (delta > 13 * PI / 16 || delta < -(13 * PI / 16)) {
		stage->warn(stage->newExplanation(USER_ERROR, "Abrupt change of direction for " + string(genderNames[genderOf(dancerIndex)]) + " #" + coupleOf(dancerIndex)));
	}
	lastMotion = angle;
}

}  // namespace dance
