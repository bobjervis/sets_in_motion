#include "../common/platform.h"
#include "dance.h"

#include "../common/machine.h"
#include "../common/timing.h"
#include "call.h"
#include "motion.h"
#include "stored_data.h"

namespace dance {

struct PrimitiveDescriptor {
	int					largestDenominator;					// 0 = no fractionalizing allowed
															// -1 = any fraction allowed
	const type_info*	types[5];							// Make sure this is one more than are actually used
};

const int ANY = Primitive::ANY;

static PrimitiveDescriptor preCheck[] = {
	{	ANY },								//	P_NOTHING
	{	0,	{ &typeid(Word) } },			//	P_IN
	{	0,	{ &typeid(Anyone), 
			  &typeid(Anything) } },		//	P_ACTIVATE
	{	0,	{ &typeid(Anyone) } },			//	P_MOVE_IN
	{	ANY,{ &typeid(Anydirection) } },	//	P_CIRCLE
	{	0,	{ &typeid(Anydirection), 
			  &typeid(Fraction) } },		//	P_CIRCLE_FRACTION
	{	0,	{ &typeid(Anydirection) } },	//	P_CIRCLE_HOME
	{	0,	{ &typeid(Fraction), 
			  &typeid(Anything) } },		//	P_ROTATE
	{	0 },								//	P_FORM_RING
	{	0 },								//	P_FORM_SET
	{	0,  { &typeid(Anydirection) } },	//	P_FORM_PROMENADE
	{	0 },								//	P_FORM_THAR
	{	0 },								//	P_FORWARD_AND_BACK
	{	2,  { &typeid(Anydirection) } },	//	P_PULL_BY
	{	0,	{ &typeid(Anydirection), 
			  &typeid(Anypivot) } },		//	P_FACE
	{	0,	{ &typeid(Anything), 
			  &typeid(Word) } },			//	P_DEFINITION
	{	0,	{ &typeid(Anyone), 
			  &typeid(Group) } },			//	P_BACK_OUT
	{	ANY,{ &typeid(Anything) } },		//	P_ANY_WHO_CAN
	{	0,	{ &typeid(Anything) } },		//	P_THOSE_WHO_CAN
	{	2,	{ &typeid(Fraction) } },		//	P_FORWARD
	{	2,	{ &typeid(Anypivot), 
			  &typeid(Anydirection), 
			  &typeid(Fraction) } },		//	P_ARC
	{	ANY,{ &typeid(Anything), 
			  &typeid(Anything) } },		//	P_START_TOGETHER
	{	0,	{ &typeid(Anyone) } },			//	P_RUN
	{	0,	{ &typeid(Anyone),
			  &typeid(Anydirection) } },	//	P_RUN_TO
	{	2,	{ &typeid(Fraction), 
			  &typeid(Fraction) } },		//	P_FORWARD_VEER
	{	2,	{ &typeid(Fraction), 
			  &typeid(Fraction), 
			  &typeid(Fraction) } },		//	P_FORWARD_VEER_FACE
	{	2,	{ &typeid(Fraction), 
			  &typeid(Anydirection) } },	//	P_FORWARD_PEEL
	{	2,	{ &typeid(Fraction), 
			  &typeid(Anydirection) } },	//	P_VEER
	{	0,	{ &typeid(Fraction), 
			  &typeid(Anydirection) } },	//	P_DISPLACE
	{	2,	{ &typeid(Anypivot), 
			  &typeid(Anydirection), 
			  &typeid(Fraction), 
			  &typeid(Fraction) } },		//	P_ARC_FACE
	{	ANY,{ &typeid(Anything) } },		//	P_MIRROR
	{	0,	{ &typeid(Fraction), 
			  &typeid(Anything) } },		//	P_FRACTIONALIZE
	{	0,	{ &typeid(Term), 
			  &typeid(Anything), 
			  &typeid(Anything) } },		//	P_IF
	{	0,	{ &typeid(Anything) } },		//	P_CAN_START
	{	ANY,{ &typeid(Word), 
			  &typeid(Word), 
			  &typeid(Anything) } },		//	P_REDUCE
	{	0,	{ &typeid(Anydirection) } },	//	P_CHECK_SEQUENCE
	{	0 },								//	P_ROLL
	{	0 },								//	P_CAN_ROLL
	{	0,	{ &typeid(Anyone), 
			  &typeid(Anyone) } },			//	P_CLOSER_TO_CENTER
	{	0,	{ &typeid(Anydirection) } },	//	P_HAS_LATERAL_FLOW
	{	0,	{ &typeid(Anything) } },		//	P_STRETCH
	{	0 },								//	P_DONT_BREATHE
	{	0 },								//	P_NORMALIZE
	{	0,	{ &typeid(Anything) } },		//	P_BREATHE
	{	0,	{ &typeid(Fraction), 
			  &typeid(Fraction) } },		//	P_CONJURE_PHANTOM
	{	0,	{ &typeid(Anything) } },		//	P_YOUR_PART
};

bool Primitive::fractionalize(PrimitiveStep* step, const Anything* parent, const Fraction* fraction, Context* context) const {
	if (preCheck[_index].largestDenominator == 0)
		return step->fail(context->stage()->newExplanation(USER_ERROR, "Cannot fractionalize this call"));

	int x;
	// Is it an appropriate fraction for the primitive?
	if (preCheck[_index].largestDenominator != Primitive::ANY &&
		!fraction->improperNumerator(preCheck[_index].largestDenominator, null, &x))
		return step->fail(context->stage()->newExplanation(USER_ERROR, "Improper fraction for this call"));
	const Fraction* f;
	switch (_index) {
	case	P_REDUCE:
		return step->tiles()[0]->plan()->step(1)->fractionalize(fraction, context);

	case	P_BREATHE:
	case	P_MIRROR:
	case	P_START_TOGETHER:
	case	P_ANY_WHO_CAN:
	case	P_PHANTOM:
		for (int i = 0; i < step->tiles().size(); i++) {
			if (!step->tiles()[i]->plan()->fractionalize(fraction, context))
				return false;
		}
		return true;

	case	P_NOTHING:
	case	P_FORWARD:
		return true;

	case	P_ARC:
		f = (const Fraction*)parent->variables()[2];
		if (f->denominator() == 0)
			return false;
		return true;

	case	P_CIRCLE:
	case	P_PULL_BY:
	case	P_FORWARD_VEER:
	case	P_FORWARD_VEER_FACE:
	case	P_FORWARD_PEEL:
	case	P_VEER:
	case	P_ARC_FACE:
		return step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Unfinished: fractionalized"));
		break;

	default:
		return step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Internal error: missing primitive " + string(_index) + " fractionalize"));
	}
	return true;
}

bool Primitive::construct(Plan* p, const Anything* parent, Context* context, TileAction action) const {
	timing::Timer tx("Primitive::construct");
	const Term* variables[3];
	const Group* d2;
	Step* s;
	Step* s2;
	Step* s3;
	Tile* t;
	Tile* t2;
	Plan* p2;
	unsigned mask;
	const Fraction* f;

	for (int i = 0; i < dimOf(preCheck[0].types); i++) {
		if (preCheck[_index].types[i] == null) {
			if (i < parent->variables().size())
				return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Too many variables in the call"));
			break;
		}
		if (i >= parent->variables().size())
			return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Too few variables in the call"));
		variables[i] = parent->variables()[i];
		if (&typeid(*variables[i]) != preCheck[_index].types[i]) {
			if (preCheck[_index].types[i] != &typeid(Term))
				return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Variable " + string(i + 1) + " is not a " + preCheck[_index].types[i]->name()));
		}
	}
	switch (_index) {
	default:
		p->constructStep(this, parent, context);
		return true;

	case	P_DONT_BREATHE:
		t = p->enclosing();
		while (t) {
			Step* s = t->enclosing();
			if (typeid(*s) == typeid(PartStep)) {
				s->setBreatheAction(DONT_BREATHE);
				break;
			}
			t = s->plan()->enclosing();
		}
		p->constructStep(this, parent, context);
		return true;

	case	P_NORMALIZE:
		t = p->enclosing();
		while (t) {
			Step* s = t->enclosing();
			if (typeid(*s) == typeid(PartStep)) {
				s->setBreatheAction(NORMALIZE);
				break;
			}
			t = s->plan()->enclosing();
		}
		s = p->constructStep(this, parent, context);
		s->setBreatheAction(NORMALIZE);
		return true;

	case	P_IF: {
		bool outcome;
		if (typeid(*variables[0]) == typeid(Integer)) {
			const Integer* i = (const Integer*)variables[0];
			outcome = i->value() != 0;
		} else if (typeid(*variables[0]) == typeid(Anything)) {
			Interval* i = p->lastInterval(null);
			StubTile stub(p->lastActiveDancersMask(null), i);

			Plan* sub = context->stage()->newPlan(p, &stub, p->start(), (const Anything*)variables[0]);
			outcome = sub->perform(i, context, TILE_ALL);
			if (verboseOutput) {
				printf("$if test = %s\n", outcome ? "true" : "false");
				sub->print(4);
			}
		} else
			return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Variable 0 is neither an integer or a call"));
		const Anything* call;
		if (outcome)
			call = (const Anything*)variables[1];
		else
			call = (const Anything*)variables[2];
		t = p->enclosing();
		if (t) {
			s = t->enclosing();
			if (typeid(*s) == typeid(CallStep)) {
				CallStep* cs = (CallStep*)s;
				cs->setAction(call);
				p->setCall(call);
				t->construct(context, TILE_ALL);
				return true;
			}
		}
		// Not sure how we got here, but let's just build an extra layer of Plan's and let the general mechanism
		// do its thing.  The Call Analysis drill down may well be weird, but things should work.
		p->constructStep(call, context, TILE_ALL);
		return true;
	}

	case	P_ACTIVATE:
		s = p->constructRawStep(context);
		mask = ((const Anyone*)variables[0])->match(p->start(), s, context);
		if (mask == 0)
			return p->fail(context->stage()->newExplanation(USER_ERROR, "No one matched the designated dancers"));
		d2 = p->start()->extract(mask, context);
		s->startWith(p->start());
		s->constructTile(d2, (const Anything*)variables[1], context, action);
		s->setBreatheAction(DONT_BREATHE);
		return true;

	case	P_CAN_START: {
		StubTile stub(p->lastActiveDancersMask(null), p->lastInterval(null));

		p2 = context->stage()->newPlan(p, &stub, p->start(), (const Anything*)variables[0]);
		p2->construct(context, TILE_ALL);
		if (!p2->failed()) {
			s = p->constructRawStep(context);
			s->startWith(p->start());
			s->constructTile(p->start(), context->stage()->newAnything(&Primitive::nothing), context, TILE_ALL);
			return true;
		} else {
			if (verboseOutput) {
				printf("$can_start failed:\n");
				p2->print(4);
			}
			return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "$can_start failed: " + p2->cause()->text()));
		}
	}

	case	P_ANY_WHO_CAN: {
		StubTile stub(p->lastActiveDancersMask(null), p->lastInterval(null));

		p2 = context->stage()->newPlan(p, &stub, p->start(), (const Anything*)variables[0]);
		s = context->stage()->newStep(p2);
		s->startWith(p->start());
		t = s->constructTile(p->start(), (const Anything*)variables[0], context, TILE_ANY_WHO_CAN);
		if (s->failed()) {
			s = p->newStep(context->stage()->newAnything(&Primitive::nothing), context);
			s->startWith(context->stage()->newGroup(p->start()));
			s->construct(context, TILE_ANY_WHO_CAN);
		} else
			p->useStep(s);
		return true;
	}

	case	P_THOSE_WHO_CAN:
		s = p->constructRawStep(context);
		s->startWith(p->start());
		t = s->constructTile(p->start(), (const Anything*)variables[0], context, TILE_ANY_WHO_CAN);
		return true;

	case	P_PHANTOM:
		s = p->constructStep(this, parent, context);
		s->constructTile(p->start(), (const Anything*)variables[0], context, TILE_WITH_PHANTOMS);
		return true;

	case	P_START_TOGETHER: {
		StartTogetherStep* branch = p->constructStartTogetherStep(context);
		branch->startWith(p->start());
		t = branch->constructTile(p->start(), (const Anything*)variables[0], context, action);
		if (t->failed())
			return false;

		StubTile stub(t->plan()->startingDancersMask(), t->plan()->lastInterval(null));
		p2 = context->stage()->newPlan(p, &stub, p->start(), (const Anything*)variables[0]);
		s2 = context->stage()->newStep(p2);
		s2->startWith(p->start());
		t2 = s2->constructTile(p->start(), (const Anything*)variables[1], context, action);
		branch->addTile(t2);
		if (t2->failed())
			return false;

		unsigned m1 = t->plan()->startingDancersMask();
		unsigned m2 = t2->plan()->startingDancersMask();
		if (m1 & m2) {
			branch->addTile(t2);
 			return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Cannot direct the same people to do two things at the same time"));
		}
		branch->setOutcomeMasks(m1, m2);
		return true;
	}

	case	P_MIRROR:
		d2 = p->start()->apply(&Transform::mirror, context);
		if (d2 == null)
			return p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Couldn't apply mirror transform"));
		s = p->constructRawStep(context);
		s->startWith(d2);
		s->constructTile(d2, (const Anything*)variables[0], context, action);
		return true;

	case	P_FRACTIONALIZE:
		f = ((const Fraction*)variables[0])->normalize(context);
		if (f->whole() < 0 || f->denominator() == 0)
			return p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Only positive fractions allowed with this call"));
		for (int i = 0; i < f->whole(); i++)
			if (p->constructStep((const Anything*)variables[1], context, action) == null)
				return false;
		if (f->numerator() != 0) {
			if (f->whole() > 0) {
				Anything* subCall = context->stage()->newAnything(this);
				subCall->variable(context->stage()->newFraction(0, f->numerator(), f->denominator()));
				subCall->variable(variables[1]);
				p->constructStep(subCall, context, action);
			} else {
				s = p->constructStep((const Anything*)variables[1], context, action);
				if (s == null)
					return false;
				if (verboseOutput) {
					printf("Fractionalizing by ");
					f->print(0);
					s->print(4, false);
				}
				for (int i = 0; i < s->tiles().size(); i++) {
					if (!s->tiles()[i]->plan()->construct(context, action))
						return false;
					if (verboseOutput) {
						printf("Fractionalizing tile %d:\n", i);
						s->tiles()[i]->plan()->print(4);
					}
					if (!s->tiles()[i]->plan()->fractionalize(f, context))
						return p->fail(context->stage()->newExplanation(USER_ERROR, "Cannot fractionalize this call"));
				}
			}
		}
		return true;

	case	P_ROTATE: {
		f = (const Fraction*)variables[0];
		int adjust;
		if (!f->improperNumerator(16, null, &adjust))
			return p->fail(context->stage()->newExplanation(USER_ERROR, "Rotation must be in 1/16ths"));
		s = p->constructStep((const Anything*)variables[1], context, action);
		if (s == null)
			return false;
		s->setRotation(adjust);
		return true;
	}

	case	P_REDUCE:
		s = p->constructStep(this, parent, context);
		if (p->start() != null)
			p->start()->constructReduce((PrimitiveStep*)s, context);
		return true;

	case	P_BREATHE:
	case	P_STRETCH: {
		const Anything* call = (const Anything*)variables[0];
		s = p->constructStep(this, parent, context);
		if (p->start() != null) {
			s->startWith(p->start());
			s->constructTile(p->start(), call, context, action);
			return !s->failed();
		}
		return true;
	}
	}
	return true;
}

unsigned Primitive::startingDancersMask(const PrimitiveStep* s) const {
	if (s->failed())
		return 0;
	switch (_index){
	case	P_REDUCE:
		return s->start()->startingDancersMaskReduce(s);

	case	P_FRACTIONALIZE:
	case	P_STRETCH:
	case	P_BREATHE:
		return s->tiles()[0]->plan()->startingDancersMask();

	default:
		return INT_MAX;
	}
}

void Primitive::trimStart(PrimitiveStep* s, unsigned mask, Context* context) const {
	if (s->failed())
		return;
	switch (_index){
	case	P_REDUCE:
		s->start()->trimStartReduce(s, mask, context);
		break;

	case	P_FRACTIONALIZE:
	case	P_STRETCH:
	case	P_BREATHE:
		s->tiles()[0]->plan()->trimStart(s->start(), s->start(), mask, context);
		break;
	}
}

const Group* Primitive::execute(PrimitiveStep* s, const Anything* parent, const Fraction* fraction, Context* context) const {
	timing::Timer t("Primitive::execute");
	const vector<const Term*>& variables = parent->variables();
	const Word* w;
	const Formation* form;
	const Group* out;
	const Group* d = s->start();
	const Group* d2;
	const Group* d3;
	int adjust;
	int x, y;
	const Anything* call;
	const Anything* call2;
	const Anydirection* dir;
	const Anypivot* pivot;
	Rotation rotation;
	const Anyone* anyone;
	const Fraction* f;
	vector<const Group*> v;
	Plan* p;
	unsigned mask;

	switch (_index) {
	case	P_NOTHING:
		return d;

	case	P_IN:
		w = (const Word*)variables[0];
		form = context->grammar()->formation(w->spelling());
		if (form == null) {
			printf("Unknown formation name: %s\n", w->spelling().c_str());
			return null;
		}
		out = d->match(&Pattern(form, null), context, null);
		if (out)
			return out;
		return null;

	case	P_MOVE_IN:
		return d->moveIn(s, (const Anyone*)variables[0], context);

	case	P_FORM_RING:
		return d->formRing(s->interval(), context);

	case	P_FORM_SET:
		return d->formSet(s->interval(), context);

	case	P_FORM_PROMENADE:
		return d->formPromenade((const Anydirection*)variables[0], context, s->interval());

	case	P_FORM_THAR:
		return d->formThar(s->interval(), context);

	case	P_CHECK_SEQUENCE:
		if (d->inSequence((const Anydirection*)variables[0]))
			return d;
		else
			return null;

	case	P_CIRCLE:
		rotation = d->rotation();
		if (fraction) {
			// We shift formation by one dancer for each 1/8th
			if (!fraction->improperNumerator(8, null, &adjust))
				return null;
		} else {
			adjust = 3;
			rotation = ROTATED_1;
		}

		return d->circle((const Anydirection*)variables[0], adjust, rotation, s->interval(), context);

	case	P_CIRCLE_FRACTION:
		f = (const Fraction*)variables[1];

		// We shift formation by one dancer for each 1/8th
		if (!f->improperNumerator(8, null, &adjust))
			return null;

		return d->circle((const Anydirection*)variables[0], adjust, d->rotation(), s->interval(), context);

	case	P_CIRCLE_HOME:
		dir = (const Anydirection*)variables[0];
		d = d->normalizeRingCoordinates(context);
		for (int i = 0; i < d->dancerCount(); i++) {
			const Dancer* dx = d->dancer(i);
			if (dx->gender == GIRL && dx->couple == 4) {
				int couple = 4;
				Gender gender = BOY;
				for (int j = 1; j < 8; j++) {
					dx = d->dancer((i + j) & 7);
					if (dx->couple != couple ||
						dx->gender != gender)
						return null;
					if (gender == GIRL)
						gender = BOY;
					else {
						gender = GIRL;
						couple--;
					}
				}
				adjust = i;
				// flip the amount for circling left, since i is the right-circle amount needed.
				if (dir->direction() == D_LEFT)
					adjust = 8 - adjust;
				break;
			}
		}
		return d->circle((const Anydirection*)variables[0], adjust, UNROTATED, s->interval(), context);

	case	P_PULL_BY:
		if (fraction) {
			fraction->improperNumerator(2, null, &adjust);
			if (adjust > 2)
				return null;
		} else
			adjust = 2;
		return d->forwardVeer(adjust, 0, 0, s->interval(), context);

	case	P_FACE:
		dir = (const Anydirection*)variables[0];
		pivot = (const Anypivot*)variables[1];
		return d->face(dir, pivot, s->interval(), context);

	case	P_DEFINITION:
		call = (const Anything*)variables[0];
		w = (const Word*)variables[1];
		if (call->definition()->name() == w->spelling())
			return d;
		else
			return null;

	case	P_BACK_OUT:
		return d->backOut(s, (const Anyone*)variables[0], (const Group*)variables[1], context, s->interval());

	case	P_FORWARD:
		f = (const Fraction*)variables[0];
		if (!f->improperNumerator(2, s->fraction(), &adjust)) {
			if (verboseOutput)
				printf("$forward needs a forward/back amount in 1/2 increments.\n");
			return null;
		}
		return d->forwardVeer(adjust, 0, 0, s->interval(), context);

	case	P_FORWARD_VEER: {
		int amount = 0;
		int veer = 0;
		f = (const Fraction*)variables[0];

		if (!f->improperNumerator(2, null, &amount)) {
			if (verboseOutput)
				printf("$forward needs a forward/back amount in 1/2 increments.\n");
			return null;
		}
		f = (const Fraction*)variables[1];

		if (!f->improperNumerator(2, null, &veer)) {
			if (verboseOutput)
				printf("$forward needs a veer amount in 1/2 increments.\n");
			return null;
		}
		return d->forwardVeer(amount, veer, 0, s->interval(), context);
	}

	case	P_FORWARD_VEER_FACE: {
		int amount = 0;
		int veer = 0;
		int turns = 0;
		f = (const Fraction*)variables[0];

		if (!f->improperNumerator(2, null, &amount)) {
			if (verboseOutput)
				printf("$forward needs a forward/back amount in 1/2 increments.\n");
			return null;
		}
		f = (const Fraction*)variables[1];

		if (!f->improperNumerator(2, null, &veer)) {
			if (verboseOutput)
				printf("$forward needs a veer amount in 1/2 increments.\n");
			return null;
		}

		f = (const Fraction*)variables[2];

		if (!f->improperNumerator(4, null, &turns)) {
			if (verboseOutput)
				printf("$forward needs a turn amount in 1/4 increments.\n");
			return null;
		}

		return d->forwardVeer(amount, veer, turns, s->interval(), context);
	}

	case	P_FORWARD_PEEL: {
		int amount = 0;
		int veer = 0;
		int turns = 0;
		f = (const Fraction*)variables[0];
		dir = (const Anydirection*)variables[1];

		if (!f->improperNumerator(2, null, &amount)) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Forward/back amount must be in 1/2 increments"));
			return null;
		}
		if (dir->direction() == D_LEFT) {
			veer = -1;
			turns = -2;
		} else if (dir->direction() == D_RIGHT) {
			veer = 1;
			turns = 2;
		} else {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Direction must be 'left' or 'right'"));
			return null;
		}
		return d->forwardVeer(amount, veer, turns, s->interval(), context);
	}

	case	P_VEER: {
		int veer = 0;
		f = (const Fraction*)variables[0];
		dir = (const Anydirection*)variables[1];

		if (!f->improperNumerator(2, null, &veer)) {
			s->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Veer amount must be in 1/2 increments"));
			return null;
		}
		if (dir->direction() == D_IN ||
			dir->direction() == D_OUT) {
			return d->veer(veer, dir, s->interval(), context);
		} else if (dir->direction() == D_LEFT) {
			veer = -veer;
		} else if (dir->direction() == D_RIGHT) {
			// veer is fine as is.
		} else {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Direction limited to 'in', 'out', 'left', 'right'"));
			return null;
		}
		return d->forwardVeer(0, veer, 0, s->interval(), context);
	}

	case	P_DISPLACE: {
		int amount = 0;
		f = (const Fraction*)variables[0];
		dir = (const Anydirection*)variables[1];

		if (!f->improperNumerator(2, null, &amount)) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Displace amount must be in 1/2 increments"));
			return null;
		}
		if (dir->direction() == D_IN ||
			dir->direction() == D_OUT) {
			return d->displace(amount, dir, s->interval(), context);
		} else {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Direction limited to 'in', 'out'"));
			return null;
		}
	}

	case	P_ARC: {
		int amount = 0;
		const Anypivot* pivot = (const Anypivot*)variables[0];
		Rotation rotation = d->rotation();
		f = (const Fraction*)variables[2];

		if (f->denominator() == 0) {		// 'special' fractions have meaning.
											// numerator:
											//		1			$until_home (assumes
			switch (f->numerator()) {
			case	1:
				// We need to have the incoming group appear in the base coordinate system.
				while (d->base())
					d = d->unwind(context);
				const Dancer* dx = d->dancerByIndex(dancerIdx(1, BOY));

				if (pivot->pivot() != P_CENTER) {
					s->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Can only arc $until_home around $center"));
					return null;
				}
				// Valid from single-file promenade formation
				if (d->geometry() == RING) {
					// The dancers had better be in single-file.
					if (dx->y != 3)
						return null;
					if (dx->facing == LEFT_FACING)
						amount = dx->x + 2;
					else
						amount = 14 - dx->x;
					if (amount < 4)
						amount += 16;
				} else {
					int adjustAmount = d->rotation();
					// Fail the primitive unless the #1 boy off center on some axis
					if (dx->x == 0) {
						if (dx->y > 0)
							amount = 8;
						else if (dx->y < 0) {
							if (adjustAmount > 0 && dx->facing == RIGHT_FACING)
								amount = 16;
							else
								amount = 0;
						} else
							return null;
					} else if (dx->y == 0) {
						if (dx->x > 0) {
							if (dx->facing == BACK_FACING)
								amount = 12;
							else
								amount = 4;
						} else if (dx->x < 0) {
							if (dx->facing == FRONT_FACING)
								amount = 4;
							else
								amount = 12;
						} else
							return null;
					} else
						return null;
				}
				rotation = UNROTATED;
			}
		} else if (pivot->pivot() == P_CENTER) {
			if (!f->improperNumerator(8, s->fraction(), &amount)) {
				s->fail(context->stage()->newExplanation(USER_ERROR, "Turning around the center needs a fraction in eighths"));
				return null;
			}
			amount *= 2;
		} else {
			if (!f->improperNumerator(4, s->fraction(), &amount)) {
				s->fail(context->stage()->newExplanation(USER_ERROR, "Turning needs a fraction in quarters"));
				return null;
			}
			amount *= 4;
		}
		if (verboseOutput && d->geometry() == RING) {
			printf("Ring $arc %d sixteenths:\n", amount);
			d->printDetails(4, true);
		}
		return d->arc(pivot, (const Anydirection*)variables[1], rotation, amount, 0, s->interval(), context);
	}

	case	P_ARC_FACE: {
		int amount = 0;
		int noseAmount = 0;
		f = (const Fraction*)variables[2];

		if (!f->improperNumerator(4, null, &amount)) {
			if (verboseOutput)
				printf("$arc needs a turning amount in 1/4 increments.\n");
			return null;
		}
		f = (const Fraction*)variables[3];

		if (!f->improperNumerator(4, null, &noseAmount)) {
			if (verboseOutput)
				printf("$arc needs a nose turning amount in 1/4 increments.\n");
			return null;
		}
		return d->arc((const Anypivot*)variables[0], (const Anydirection*)variables[1], d->rotation(), amount * 4, noseAmount, s->interval(), context);
	}

	case	P_RUN:
		anyone = (const Anyone*)variables[0];
		mask = anyone->match(d, s, context);
		if (mask == 0) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "No dancers can run"));
			return null;
		}
		d2 = d->extract(mask, context);
		d3 = d->run(d2, null, context, s->interval());
		if (d3 == null)
			s->fail(context->stage()->newExplanation(USER_ERROR, "Not all the designated dancers have unique partners"));
		return d3;

	case	P_RUN_TO:
		anyone = (const Anyone*)variables[0];
		mask = anyone->match(d, s, context);
		if (mask == 0) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "No dancers can run"));
			return null;
		}
		d2 = d->extract(mask, context);
		dir = (const Anydirection*)variables[1];
		if (dir->direction() != D_RIGHT &&
			dir->direction() != D_LEFT) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Run direction must be left or right"));
			return null;
		}
		d3 = d->run(d2, dir, context, s->interval());
		if (d3 == null)
			s->fail(context->stage()->newExplanation(USER_ERROR, "Not all the designated dancers have unique partners"));
		return d3;

	case	P_FORWARD_AND_BACK:
		return d->forwardAndBack(s->interval(), context);

	case	P_REDUCE:
		return d->reduce(s, context);

	case	P_STRETCH:
		s->tiles()[0]->plan()->perform(s->interval(), context, TILE_ALL);
		if (s->failed())
			return null;
		d = s->tiles()[0]->plan()->breathe(context);
		return d->stretch(s, context);

	case	P_PHANTOM:
		// 1. check for ambiguous facing and try alternative arrangements
		p = s->tiles()[0]->plan();
		if (p->stepCount() == 0) {
			if (p->orientedStart()->hasAmbiguousFacing()) {
				// First see if we can resolve by symmetry.  If we can, we take it.
				if (verboseOutput) {
					printf("Disambiguate:\n");
					p->start()->printDetails(4, true);
				}
				if (p->start()->isSymmetric(context->sequence())) {
					const Group* g = p->start();
					const Group* disambiguated = g->disambiguateFromRoot(context);

					if (disambiguated != null) {
						Interval* i = p->lastInterval(null);
						StubTile stub(p->lastActiveDancersMask(null), i);
						Plan* sub = context->stage()->newPlan(p, &stub, disambiguated, p->call());
						if (sub->perform(i, context, TILE_ALL)) {
							p->splice(sub);
						} else
							p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Disambiguated facing, symmetric whole group unfinished"));
						return p->final();
					} else
						p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Ambiguous facing, symmetric whole group unfinished"));
					return null;
				} else {
					p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Ambiguous facing, whole group unfinished"));
					return null;
				}
			}
			p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Full group match unfinished"));
			return null;
		} else {
			Step* s = p->step(0);
			for (int i = 0; i < s->tiles().size(); i++) {
				p = s->tiles()[i]->plan();
				if (p->start()->hasAmbiguousFacing()) {
					if (verboseOutput) {
						printf("Disambiguate:\n");
						p->start()->printDetails(4, true);
					}
					if (p->start()->root()->isSymmetric(context->sequence())) {
						const Group* g = p->start();
						const Group* disambiguated = g->disambiguateFromRoot(context);

						if (disambiguated != null) {
							Interval* i = p->lastInterval(null);
							StubTile stub(p->lastActiveDancersMask(null), i);
							Plan* sub = context->stage()->newPlan(p, &stub, disambiguated, p->call());
							if (sub->perform(i, context, TILE_ALL)) {
								p->splice(sub);
							} else
								p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Disambiguated facing, symmetric tile unfinished"));
						} else
							p->fail(context->stage()->newExplanation(PROGRAM_BUG, "disambiguate from root did not work"));
					} else {
						p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Ambiguous facing, tile unfinished"));
					}
					// First see if we can resolve by symmetry.  If we can, we take it.
				} else {
					p->fail(context->stage()->newExplanation(PROGRAM_BUG, "Unambiguous facing, tile unfinished"));
				}
			}
			if (s->failed())
				return null;
			if (s->interval() == null)
				s->makeInterval(p->interval(), false);
			return s->combine(false, null, context);
		}
		// 2. remove phantoms from result group
		p->perform(s->interval(), context, TILE_ALL);
		if (s->failed())
			return null;
		d = s->tiles()[0]->plan()->breathe(context);
		return d->removePhantoms(context);

	case	P_ROLL:
		return d->roll(s, context, false);

	case	P_CAN_ROLL:
		return d->roll(s, context, true);

	case	P_CLOSER_TO_CENTER:
		anyone = (const Anyone*)variables[0];
		d2 = d->extract(anyone->match(d, s, context), context);
		anyone = (const Anyone*)variables[1];
		d3 = d->extract(anyone->match(d, s, context), context);
		if (d2->closerToCenter(d3))
			return d;
		else
			return null;

	case	P_HAS_LATERAL_FLOW:
		dir = (const Anydirection*)variables[0];
		if (d->hasLateralFlow(dir, s->interval(), context))
			return d;
		else {
			switch (dir->direction()) {
			case	D_AS_YOU_ARE:
				s->fail(context->stage()->newExplanation(USER_ERROR, "Not all dancers moving in a consistent direction"));
				break;

			case	D_LEFT:
				s->fail(context->stage()->newExplanation(USER_ERROR, "Not all dancers moving leftward"));
				break;

			case	D_RIGHT:
				s->fail(context->stage()->newExplanation(USER_ERROR, "Not all dancers moving rightward"));
				break;

			default:
				s->fail(context->stage()->newExplanation(DEFINITION_ERROR, "$hasLateralFlow must use 'left', 'right', or '$as_you_are'"));
				break;
			}
			return null;
		}

	case	P_BREATHE:
		s->tiles()[0]->perform(context);
		if (s->failed())
			return null;
		else
			return s->tiles()[0]->plan()->breathe(context);

	case	P_DONT_BREATHE:
		s->setLastActiveMask(s->plan()->lastActiveDancersMask(s));
		return d;

	case	P_NORMALIZE:
		s->setLastActiveMask(s->plan()->lastActiveDancersMask(s));
		return d;

	case	P_CONJURE_PHANTOM:
		s->fail(context->stage()->newExplanation(PROGRAM_BUG, "Unfinished: $conjure_phantom()"));
		return null;

	default:
		printf("Unknown primitive: ");
		print(0);
		return null;
	}
}

const char* primitiveNames[] = {
	"$nothing",
	"$in",
	"$activate",
	"$move_in",
	"$circle",
	"$circle_fraction",
	"$circle_home",
	"$rotate",
	"$form_ring",
	"$form_set",
	"$form_promenade",
	"$form_thar",
	"$forward_and_back",
	"$pull_by",
	"$face",
	"$definition",
	"$back_out",
	"$any_who_can",
	"$those_who_can",
	"$forward",
	"$arc",
	"$start_together",
	"$run",
	"$run_to",
	"$forward_veer",
	"$forward_veer_face",
	"$forward_peel",
	"$veer",
	"$displace",
	"$arc_face",
	"$mirror",
	"$fractionalize",
	"$if",
	"$can_start",
	"$reduce",
	"$check_sequence",
	"$roll",
	"$can_roll",
	"$closer_to_center",
	"$has_lateral_flow",
	"$stretch",
	"$dont_breathe",
	"$normalize",
	"$breathe",
	"$conjure_phantom",
	"$phantom",
};

void Primitive::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("%s\n", primitiveNames[_index]);
}

}  // namespace dance
