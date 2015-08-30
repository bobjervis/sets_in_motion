#include "../common/platform.h"
#include "dance.h"

#include "call.h"
#include "stored_data.h"

namespace dance {

const char* dancerSetNames[] = {
	"none",
	"centers",
	"ends",
	"very centers",
	"very ends",
	"$last_active",
	"others",
	"leaders",
	"trailers",
	"heads",
	"sides",
	"boys",
	"girls",
	"beaus",
	"belles",
	"facing across",
	"facing along",
	"in facing",
	"out facing",
	"dancer_mask",
	"left AND right",
	"left OR right",
	"left xor RIGHT",
	"not LEFT",
};

unsigned heads[] = {
	0,										// D_UNSPECIFIED
	0,										// D_2COUPLE
	COUPLE_1|COUPLE_3,						// D_4COUPLE
	COUPLE_1|COUPLE_2|COUPLE_4|COUPLE_5,	// D_6COUPLE
	COUPLE_1|COUPLE_3|COUPLE_5,				// D_HEXAGONAL
};

unsigned sides[] = {
	0,										// D_UNSPECIFIED
	0,										// D_2COUPLE
	COUPLE_2|COUPLE_4,						// D_4COUPLE
	COUPLE_3|COUPLE_6,						// D_6COUPLE
	COUPLE_2|COUPLE_4|COUPLE_6,				// D_HEXAGONAL
};

unsigned Anyone::match(const Group* dancers, const Step* step, Context* context) const {
	unsigned dancerMask = dancers->dancerMask();
	unsigned mask;
	Rectangle r;
	switch (_dancerSet) {
	case	LAST_ACTIVE:
		return dancerMask & step->lastActiveDancersMask();

	case	OTHERS:
		return dancerMask & ~step->lastActiveDancersMask();

	case	FACING_ACROSS:
	case	FACING_ALONG:
		dancers->boundingBox(&r);
		mask = 0;
		for (int i = 0; i < dancers->dancerCount(); i++) {
			const Dancer* d = dancers->dancer(i);
			switch (d->facing) {
			case	RIGHT_FACING:
			case	LEFT_FACING:
				if (r.width() > r.height()) {
					if (_dancerSet == FACING_ALONG)
						mask |= d->dancerMask();
				} else if (r.width() < r.height()) {
					if (_dancerSet == FACING_ACROSS)
						mask |= d->dancerMask();
				}
				break;

			case	FRONT_FACING:
			case	BACK_FACING:
				if (r.width() > r.height()) {
					if (_dancerSet == FACING_ACROSS)
						mask |= d->dancerMask();
				} else if (r.width() < r.height()) {
					if (_dancerSet == FACING_ALONG)
						mask |= d->dancerMask();
				}
				break;
			}
		}
		return mask;

	case	IN_FACING:
	case	OUT_FACING:
		mask = 0;
		if (dancers->geometry() == RING) {
			for (int i = 0; i < dancers->dancerCount(); i++) {
				const Dancer* d = dancers->dancer(i);
				switch (d->facing) {
				case	FRONT_FACING:
					if (_dancerSet == IN_FACING)
						mask |= d->dancerMask();
					break;

				case	BACK_FACING:
					if (_dancerSet == OUT_FACING)
						mask |= d->dancerMask();
					break;
				}
			}
		} else {
			for (int i = 0; i < dancers->dancerCount(); i++) {
				const Dancer* d = dancers->dancer(i);
				switch (d->facing) {
				case	RIGHT_FACING:
					if (d->x < 0) {
						if (_dancerSet == IN_FACING)
							mask |= d->dancerMask();
					} else if (d->x > 0) {
						if (_dancerSet == OUT_FACING)
							mask |= d->dancerMask();
					}
					break;

				case	LEFT_FACING:
					if (d->x < 0) {
						if (_dancerSet == OUT_FACING)
							mask |= d->dancerMask();
					} else if (d->x > 0) {
						if (_dancerSet == IN_FACING)
							mask |= d->dancerMask();
					}
					break;

				case	FRONT_FACING:
					if (d->y < 0) {
						if (_dancerSet == OUT_FACING)
							mask |= d->dancerMask();
					} else if (d->y > 0) {
						if (_dancerSet == IN_FACING)
							mask |= d->dancerMask();
					}
					break;

				case	BACK_FACING:
					if (d->y < 0) {
						if (_dancerSet == IN_FACING)
							mask |= d->dancerMask();
					} else if (d->y > 0) {
						if (_dancerSet == OUT_FACING)
							mask |= d->dancerMask();
					}
					break;
				}
			}
		}
		return mask;

	case	LEADERS:
	case	TRAILERS: {
		const vector<VariantTile>& forms = context->grammar()->leadersTrailers();
		TileSearch tiles[MAX_DANCERS];
		mask = 0;

		int result = dancers->buildTiling(forms, tiles, context, null, null, TILE_ALL);
		if (result <= 0)
			return 0;
		for (int i = 0; i < result; i++) {
			const Group* d = tiles[i].dancers;
			if (d->dancerCount() == 4) {				// must be a box
				static DancerSet classification[4][4] = {
					{	// dancer 0:
						TRAILERS,		//	RIGHT_FACING,	// > caller's right side of hall
						LEADERS,		//	BACK_FACING,	// ^ back of hall
						LEADERS,		//	LEFT_FACING,	// < caller's left side of hall
						TRAILERS,		//	FRONT_FACING,	// v front of hall
					},
					{	// dancer 1:
						LEADERS,		//	RIGHT_FACING,	// > caller's right side of hall
						LEADERS,		//	BACK_FACING,	// ^ back of hall
						TRAILERS,		//	LEFT_FACING,	// < caller's left side of hall
						TRAILERS,		//	FRONT_FACING,	// v front of hall
					},
					{	// dancer 2:
						TRAILERS,		//	RIGHT_FACING,	// > caller's right side of hall
						TRAILERS,		//	BACK_FACING,	// ^ back of hall
						LEADERS,		//	LEFT_FACING,	// < caller's left side of hall
						LEADERS,		//	FRONT_FACING,	// v front of hall
					},
					{	// dancer 3:
						LEADERS,		//	RIGHT_FACING,	// > caller's right side of hall
						TRAILERS,		//	BACK_FACING,	// ^ back of hall
						TRAILERS,		//	LEFT_FACING,	// < caller's left side of hall
						LEADERS,		//	FRONT_FACING,	// v front of hall
					},
				};
				for (int j = 0; j < 4; j++)
					if (_dancerSet == classification[j][d->dancer(j)->facing])
						mask |= d->dancer(j)->dancerMask();
			} else {									// must be a twosome
				static DancerSet classification[2][4] = {
					{	// dancer 0:
						TRAILERS,		//	RIGHT_FACING,	// > caller's right side of hall
						NONE,			//	BACK_FACING,	// ^ back of hall
						LEADERS,		//	LEFT_FACING,	// < caller's left side of hall
						NONE,			//	FRONT_FACING,	// v front of hall
					},
					{	// dancer 1:
						LEADERS,		//	RIGHT_FACING,	// > caller's right side of hall
						NONE,			//	BACK_FACING,	// ^ back of hall
						TRAILERS,		//	LEFT_FACING,	// < caller's left side of hall
						NONE,			//	FRONT_FACING,	// v front of hall
					},
				};
				for (int j = 0; j < 2; j++)
					if (_dancerSet == classification[j][d->dancer(j)->facing])
						mask |= d->dancer(j)->dancerMask();
			}
		}
		return dancerMask & mask;
	}

	case	VERY_CENTERS:
	case	VERY_ENDS:
	case	CENTERS:
	case	ENDS: {
		const vector<const Pattern*>& forms = context->grammar()->centersEnds();

		for (int i = 0; i < forms.size(); i++) {
			const Group* d = dancers->match(forms[i], context, null);
			if (d) {
				static PositionType pos[] = {
					NO_POSITION,	//	NONE
					CENTER,			//	CENTERS,
					END,			//	ENDS,
					VERY_CENTER,	//	VERY_CENTERS,
					VERY_END,		//	VERY_ENDS
				};
				static PositionType altPos[] = {
					NO_POSITION,	//	NONE
					VERY_CENTER,	//	CENTERS,
					VERY_END,		//	ENDS,
					NO_POSITION,	//	VERY_CENTERS,
					NO_POSITION,	//	VERY_ENDS
				};
				return dancerMask & forms[i]->formation()->extract(d, dancers, pos[_dancerSet], altPos[_dancerSet]);
			}
		}
	}	break;

	case	BEAUS:
	case	BELLES: {
		unsigned mask = 0;
		vector<const Dancer*> partners;
		dancers->partnershipOp(_dancerSet, null, context, &partners);
		for (int i = 0; i < partners.size(); i++)
			mask |= partners[i]->dancerMask();
		return dancerMask & mask;
	}

	case	HEADS:
		return dancerMask & heads[D_4COUPLE];

	case	SIDES:
		return dancerMask & sides[D_4COUPLE];

	case	GIRLS:
		return dancerMask & GIRLS_MASK;

	case	BOYS:
		return dancerMask & BOYS_MASK;

	case	DANCER_MASK:
		return dancerMask & _mask;

	case	NOT_L:
		return dancerMask & ~_left->match(dancers, step, context);

	case	L_AND_R:
		mask = _left->match(dancers, step, context);
		if (mask == 0)
			return 0;
		return dancerMask & mask & _right->match(dancers, step, context);

	case	L_OR_R:
		mask = _left->match(dancers, step, context);
		if (mask == dancerMask)
			return dancerMask;
		return dancerMask & (mask | _right->match(dancers, step, context));

	case	L_XOR_R:
		return dancerMask & (_left->match(dancers, step, context) ^ _right->match(dancers, step, context));

	default:
		break;
	}
	return 0;
}

const Term* Anyone::not(Context* context) const {
	return context->stage()->newAnyone(NOT_L, 0, this, null, _level);
}

const Term* Anyone::and(const Term* operand, Context* context) const {
	if (typeid(*operand) != typeid(Anyone))
		return null;
	const Anyone* right = (const Anyone*)operand;
	Level max = _level > right->_level ? _level : right->_level;
	return context->stage()->newAnyone(L_AND_R, 0, this, right, max);
}

const Term* Anyone::or(const Term* operand, Context* context) const {
	if (typeid(*operand) != typeid(Anyone))
		return null;
	const Anyone* right = (const Anyone*)operand;
	Level max = _level > right->_level ? _level : right->_level;
	return context->stage()->newAnyone(L_OR_R, 0, this, right, max);
}

const Term* Anyone::xor(const Term* operand, Context* context) const {
	if (typeid(*operand) != typeid(Anyone))
		return null;
	const Anyone* right = (const Anyone*)operand;
	Level max = _level > right->_level ? _level : right->_level;
	return context->stage()->newAnyone(L_XOR_R, 0, this, right, max);
}

string Anyone::label() const {
	string s;
	switch (_dancerSet) {
	case	DANCER_MASK:
		for (int i = 0; i < MAX_DANCERS; i++) {
			if (_mask & (1 << i))
				s.printf("#%d %s ", coupleOf(i), genderNames[genderOf(i)]);
		}
		break;

	case	L_AND_R:
		s = "(" + _left->label() + ")and(" + _right->label() + ")";
		break;

	case	L_OR_R:
		s = "(" + _left->label() + ")or(" + _right->label() + ")";
		break;

	case	L_XOR_R:
		s = "(" + _left->label() + ")xor(" + _right->label() + ")";
		break;

	case	NOT_L:
		s = "not(" + _left->label() + ")";
		break;

	default:
		s = dancerSetNames[_dancerSet];
	}
	return s;
}

void Anyone::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("Anyone{ %s }\n", label().c_str());
}

void DancerName::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("$dancer%c\n", 'A' + _index);
}

void DancerName::token(Token* output) const {
	output->type = DANCER_NAME;
	output->value = _index;
}

}  // namespace dance
