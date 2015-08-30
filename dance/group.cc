#include "../common/platform.h"
#include "dance.h"

#include <math.h>
#include "../common/timing.h"
#include "call.h"

namespace dance {

static void assignAfterCoordinates(vector<Plane*>& planes);
static bool isBetween(const vector<Rectangle>& boundingBoxes, const Dancer* a, const Dancer* b);
static Motion* lastCurve(const Dancer* d, const Interval* interval, Context* context);
static Rotation rotateBy(Rotation r, int n);
static bool anyOverlapping(const vector<Rectangle>& boundingBoxes);

bool Group::equals(const Group* dancers) const {
	if (_rotation != dancers->_rotation ||
		_geometry != dancers->_geometry ||
		_dancers.size() != dancers->_dancers.size())
		return false;
	for (int i = 0; i < _dancers.size(); i++)
		if (!_dancers[i]->identical(dancers->_dancers[i]))
			return false;

	return true;
}

bool Group::contains(const Dancer* dancer) const {
	for (int i = 0; i < _dancers.size(); i++)
		if (_dancers[i]->dancerIndex() == dancer->dancerIndex())
			return true;
	return false;
}

const Group* Group::apply(const Anything* call, Context* context, TileAction tileAction) const {
	return null;
}
//
// 1. Restore diamond point dancers to their 'proper' spacing.
//
//		At the end of each call, dancers who are perpendicular to 
//		the others in their half of the square are moved 1/2 space away
//		from their center line (see the step 7 below).  Any such
//		dancers who are not participating in a diamond formation
//		are moved back toward their center line to remove this distortion
//		before performing the call.
//
// 2. Perform the actions of the call.
//
// 3. Find the x and y 'planes' in the 'restored' dancers (the results of step 1)
//
//		We take the tiles that we formed from the call as one set of tiles.
//		The next thing we do is we form tiles of the inactive dancers by
//		combining all sets of inactive dancers that are on the same
//		uninterrupted line into common tiles.  Thus inactive points of diamonds
//		are separate tiles, but inactive outside couples in a quarter tag formation
//		are treated as a unit.
//		If we look along each axis, then the center of the set forms one plane.
//		The center of each tile and each inactive dancer also forms a plane.  
//		Lastly, the 'outer edge' of each tile and each inactive dancer forms
//		another plane.  The 'outer edge' exists 1/2 space beyond the furthest 
//		dancer in that tile.
//
//		Example:
//			formation: hourglass
//			call: very centers hinge
//
//					   i
//					i . . i
//					  a a 
//					i . . i
//					   i
//
//			with y-axis planes marked:
//
//				A  -------------
//				B  --    i    --
//				C  -- i . . i --
//				D  --   a a   --
//				F  -- i . . i --
//				F  --    i    --
//				G  -------------
//
// 4. Find the corresponding planes in the final dancers:
//
//				A  -------------
//				B  --    i    --
//				C  -------------
//					  i  a  i
//				D  --         --
//					  i  a  i
//				E  -------------
//				F  --    i    --
//				G  -------------
//
// 5. The distance between each plane is calculated
//
//	Each 'center' plane is separated by its own 'edge'
//	planes by at least 1/2 the size of the tile.
//	Within these constraints, the sizes of each gap between
//	planes is made as small as possible.
//  If no tiles cross two given planes (a space that
//	would be occupied by phantoms in a high-order matrix),
//	the ending distance between those two planes is zero.
//
// 6. Group are assigned positions.
//
//  The center plane (D in the above example) becomes
//	center in the final formation.  The corresponding
//	x and y coordinates are then assigned as you work from
//	the center line out to the various other planes.
//	The center of each tile is assigned to the coordinates
//	of their final center plane.
//
//				         x
//					  x  x  x
//					  x  x  x
//				         x
//
// 7. Center dancers on hand-holds.
//
//				         x
//					  y  x  y
//					----------
//					  y  x  y
//				         x
//
//	Note that dancers marked x and y in this example are
//	perpendicular to one another.  Since the x's have hand
//	holds, the y's must center themselves on the x's hands.
//	This is done by moving away from the center line by 1/2
//	space.  If the hands are farther away than 1/2 space,
//	then no adjustment takes place.
//
//	Yielding:
//
//				         x
//					  y     y
//					     x
//					----------
//					     x
//					  y     y
//				         x
//
//	(Note that this turns an hourglass into twin diamonds.)
//
//	Another example:
//
//	An inactive dancer or narrow tile may find that its
//	edge planes have been pushed farther apart than in the
//	starting formation.  If the two edge planes have been
//	pushed apart by equal amounts, the the center plane will
//	naturally center the tiles within the new space.
//
//	Before:
//		formation: parallel waves.
//		Call: centers hinge
//
//					   i a a i
//					   i a a i
//
//
//				  A	-------------
//				  B -- i a a i --
//				  C	-------------
//				  D -- i a a i --
//				  E	-------------
//
//
//				  A	-------------
//						  a
//				  B -------------
//					   i  a  i
//				  C	-------------
//					   i  a  i
//				  D -------------
//						  a
//				  E	-------------
//
//		Since the inactives are perpendicular to the actives:
//
//					      a
//					   i     i
//					      a
//					      a
//					   i     i
//					      a
//
//
//	After:
//		formation: twin diamonds
//
//		Note that the inactive dancers in this case are in
//		twosomes, so they tile separately.  The end dancers
//		move slightly apart to center themselves on the diamond
//		centers' hands.
//
//	Another example:
//
//	Before:
//
//		formation: T-bones (e.g. from a squared set after heads move in)
//		call: centers hinge
//
//					   i a a i
//					   i a a i
//
//
//				  A	-------------
//				  B -- i a a i --
//				  C	-------------
//				  D -- i a a i --
//				  E	-------------
//
//
//				  A	-------------
//						  a
//				  B --    .    --
//					   i  a  i
//				  C	-------------
//					   i  a  i
//				  D --    .    --
//						  a
//				  E	-------------
//
//		Note how in this example, the inactive dancers form two tiles.
//		Their tiles' center plane is C, so they take up positions
//		around it and don't move.  Since no one is perpendicular, the
//		dancers stay in place.
//
//	Another example:
//
//		Formation: H
//		Call: center wave lockit
//
//					  i . . i
//					  a a a a
//					  i . . i
//
//		
//					  i . . i
//					-----------
//					  a a a a
//					-----------
//					  i . . i
//
//					  i . . i
//					-----------
//					     a
//					     a
//					     a
//					     a
//					-----------
//					  i . . i
//
//		After:
//					  i . . i
//					     a
//					     a
//					     a
//					     a
//					  i . . i

//	A problematic situation:
//
//		Before:
//					i . . i
//				  a | a a | a
//					i . . i
//
// Note: all dancers facing along the long axis.
//
//		Call: 'active dancers' face right.
//
//		After:
//				No adjustments.  Dancers are in point-to-point diamonds.
//
//	The difficulty of this situation is that, depending on how the call is
//	written (and the 'face right' was written that way), the active dancers may appear as
//	a single tile, overlapping the inactive dancer tiles.
//
//	The correct action seems to be that when the tiles are overlapping, the
//	dancers should be re-grouped into non-overlapping tiles.  Then, any adjustments
//	can be applied.
//
//	In this case, the active dancers must be broken into three groups.
//	Once re-grouped, the formation is compact and requires no adjustment.
//	(Note: since the determination of co-incidence is determined by reconstructing
//	the a priori position of the group, this should not interfere with that step).

void Group::breathe(Step* enclosing, vector<const Group*>& affected, unsigned rootMask, const vector<Rectangle>& boundingBoxes, bool preserveRelativePositions, Context* context) const {

		// Now compute the planes

	vector<Plane*> xPlanes;
	vector<Plane*> yPlanes;

		// Seed with the center-line planes

	xPlanes.push_back(new Plane(true));
	yPlanes.push_back(new Plane(true));

	for (int i = 0; i < affected.size(); i++) {
		unsigned mask = affected[i]->dancerMask();
		if (mask == 0)
			continue;

		Rectangle boxNow;

		affected[i]->boundingBox(&boxNow);

		Plane* left;
		Plane* right;
		Plane* top;
		Plane* bottom;
		Plane* xCenter; 
		Plane* yCenter;
		if (preserveRelativePositions && boxNow.coincident(boundingBoxes[i])) {
			left = new Plane(boundingBoxes[i].left, boxNow.left, i, null);
			right = new Plane(boundingBoxes[i].right, boxNow.right, i, null);
			top = new Plane(boundingBoxes[i].top, boxNow.top, i, null);
			bottom = new Plane(boundingBoxes[i].bottom, boxNow.bottom, i, null);
		} else {
			left = new Plane(boxNow.left, boxNow.left, i, null);
			right = new Plane(boxNow.right, boxNow.right, i, null);
			top = new Plane(boxNow.top, boxNow.top, i, null);
			bottom = new Plane(boxNow.bottom, boxNow.bottom, i, null);
		}
		xCenter = new Plane(i, null, boxNow.width(), left, right); 
		yCenter = new Plane(i, null, boxNow.height(), bottom, top);
		xPlanes.push_back(left);
		xPlanes.push_back(right);
		xPlanes.push_back(xCenter);
		yPlanes.push_back(top);
		yPlanes.push_back(bottom);
		yPlanes.push_back(yCenter);
	}

	xPlanes.sort();
	yPlanes.sort();

	assignAfterCoordinates(xPlanes);
	assignAfterCoordinates(yPlanes);
	if (verboseBreathing) {
		printf("X Planes:\n");
		for (int i = 0; i < xPlanes.size(); i++)
			xPlanes[i]->print(4);
		printf("Y Planes:\n");
		for (int i = 0; i < yPlanes.size(); i++)
			yPlanes[i]->print(4);
	}

	vector<int> deltaX;
	vector<int> deltaY;
	deltaX.resize(affected.size());
	deltaY.resize(affected.size());

	for (int i = 0; i < xPlanes.size(); i++)
		if (xPlanes[i]->lesserEdge)
			deltaX[xPlanes[i]->tile] = xPlanes[i]->after - xPlanes[i]->now;
	for (int i = 0; i < yPlanes.size(); i++)
		if (yPlanes[i]->lesserEdge)
			deltaY[yPlanes[i]->tile] = yPlanes[i]->after - yPlanes[i]->now;
	xPlanes.deleteAll();
	yPlanes.deleteAll();

	// The last step is to copy out the adjustments into 'displace' operations.
	// Then, combine the tiles into the final dancer set.

	for (int i = 0; i < affected.size(); i++) {
		if (deltaX[i] == 0 && deltaY[i] == 0)
			continue;
		if (verboseBreathing)
			printf("Adjusting Tile %d by [%d,%d]\n", i, deltaX[i], deltaY[i]);
		const Group* x = affected[i];
		if (x->dancerMask() & rootMask) {
			if (verboseBreathing)
				printf("Group contains ghosted dancers - aborting adjustment.\n");
			continue;
		}
		if (x->dancerCount() == 0)
			continue;
		affected[i] = x->displace(deltaX[i], deltaY[i], enclosing->interval(), context);
		if (verboseBreathing)
			affected[i]->printDetails(4, true);
	}
}

bool Group::shouldBeRing() const {
	if (_dancers.size() != 8)
		return false;
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* a = _dancers[i];
		int absX = abs(a->x);
		if (absX == 1) {
			if (abs(a->y) != 2)
				return false;
			if (a->facing != FRONT_FACING &&
				a->facing != BACK_FACING)
				return false;
		} else if (absX == 2) {
			if (abs(a->y) != 1)
				return false;
			if (a->facing != RIGHT_FACING &&
				a->facing != LEFT_FACING)
				return false;
		} else
			return false;
	}
	return true;
}

Step* Group::checkSamePositionRule(PartStep* step, Context* context) const {
	Step* samePositionStep = null;

	for (int i = 0; i < _dancers.size() - 1; i++) {
		const Dancer* d1 = _dancers[i];
		const Dancer* d2 = _dancers[i + 1];
		if (d1->x == d2->x &&
			d1->y == d2->y) {
			static Facing oppositeFacing[] = {
				LEFT_FACING,	//	RIGHT_FACING,	// > caller's right side of hall
				FRONT_FACING,	//	BACK_FACING,	// ^ back of hall
				RIGHT_FACING,	//	LEFT_FACING,	// < caller's left side of hall
				BACK_FACING,	//	FRONT_FACING,	// v front of hall
				HEAD_FACING,	//	HEAD_FACING,	// | front or back
				SIDE_FACING,	//	SIDE_FACING,	// - left or right
				ANY_FACING,		//	ANY_FACING,		// ? any value
			};

			if (verboseBreathing)
				printf("Group on same position: [%d, %d] (#%d %s & #%d %s)\n", d1->x,
																				 d1->y,
																				 d1->couple,
																				 genderNames[d1->gender],
																				 d2->couple,
																				 genderNames[d2->gender]);
			Facing f1 = d1->facing;
			Facing f2 = d2->facing;
			if (f1 == ANY_FACING || f2 == ANY_FACING)	// at least one dancer is a phantom, do nothing
				continue;
			if (f1 != oppositeFacing[f2])
				return null;
			Group* pair = context->stage()->newGroup(this);
			int x1, y1, x2, y2;
			d1->displace(0, -1, &x1, &y1);
			d2->displace(0, -1, &x2, &y2);
			if (samePositionStep == null) {
				Plan* p = step->tiles()[0]->plan();
				samePositionStep = p->constructRawStep(context);
				samePositionStep->setSpecialLabel("Same Position Rule");
				samePositionStep->startWith(this);
				samePositionStep->makeInterval(null, true);
				samePositionStep->setBreatheAction(BREATHE);
				step->collectMotions(null, context);
				samePositionStep->interval()->currentDancers(this);
			}
			const Dancer* ad1 = d1->displace(x1, y1, samePositionStep->interval(), context);
			const Dancer* ad2 = d2->displace(x2, y2, samePositionStep->interval(), context);
			pair->insert(ad1);
			pair->insert(ad2);
			pair->done();
			samePositionStep->addOutcome(pair);
		}
	}
	if (verboseBreathing && samePositionStep) {
		printf("After same position rule applied:\n");
		step->print(4, false);
	}
	return samePositionStep;
}

bool Group::sameShape(const Group* before) const {
	if (_dancers.size() != before->_dancers.size())
		return false;
	if (_dancers.size() == 1)
		return true;
	for (int i = 0; i < _dancers.size(); i++)
		if (_dancers[i]->x != before->_dancers[i]->x ||
			_dancers[i]->y != before->_dancers[i]->y)
			return false;
	return true;
}

int Group::compare(const void *ts1, const void *ts2) {
	const TileSearch* t1 = (const TileSearch*)ts1;
	const TileSearch* t2 = (const TileSearch*)ts2;

	return t1->dancers->compare(t2->dancers);
}

Group* Group::makeHome() {
	Group* d = new Group(GRID);
	d->_dancers.push_back(new Dancer(-1, 3, FRONT_FACING, GIRL, 3));
	d->_dancers.push_back(new Dancer(1, 3, FRONT_FACING, BOY, 3));
	d->_dancers.push_back(new Dancer(-3, 1, RIGHT_FACING, BOY, 4));
	d->_dancers.push_back(new Dancer(3, 1, LEFT_FACING, GIRL, 2));
	d->_dancers.push_back(new Dancer(-3, -1, RIGHT_FACING, GIRL, 4));
	d->_dancers.push_back(new Dancer(3, -1, LEFT_FACING, BOY, 2));
	d->_dancers.push_back(new Dancer(-1, -3, BACK_FACING, BOY, 1));
	d->_dancers.push_back(new Dancer(1, -3, BACK_FACING, GIRL, 1));
	return d;
}

int Group::buildTiling(const vector<VariantTile>& tiles, TileSearch* out, Context* context, const Anything* call, Step* step, TileAction tileAction) const {
	timing::Timer tx("Group::buildTiling");

	if (tileAction == TILE_WITH_PHANTOMS) {
		if (verboseOutput) {
			printf("buildTiling(TILE_WITH_PHANTOMS)\n");
			for (int i = 0; i < tiles.size(); i++) {
				printf("Tile %d: %s dancers %d\n", i, tiles[i].pattern->formation()->name().c_str(), tiles[i].pattern->formation()->dancerCount());
			}
			printf("Matching:\n");
			printDetails(4, true);
		}
		int result;
		for (int i = 0; i < tiles.size(); i++) {
			if (tiles[i].pattern->formation() == null)
				continue;
			switch (tiles[i].pattern->formation()->dancerCount()) {
			case	1:
			case	2:
				step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Unfinished: buildTiling(TILE_WITH_PHANTOMS)"));
				return -1;

			case	4:
				result = buildPhantom4Dancer(tiles[i], out, context, call, step);
				if (result < 0)
					continue;
				return result;

			default:
				step->fail(context->stage()->newExplanation(USER_ERROR, "Don't know how to do that call with phantoms"));
				return -1;
			}
		}
		step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Unfinished: buildTiling(TILE_WITH_PHANTOMS)"));
		return -1;
	}

	int bestResult = 0;
	int bestScore = 0;
	int dancersInBest = 0;
	bool unique = true;
	bool bestSorted = false;

	for (int i = 0; i < _dancers.size(); i++) {
		for (int j = 0; j < tiles.size(); j++) {
			if (tiles[j].pattern->formation() == null)
				continue;
			PatternClosure closure(tiles[j].pattern, call, step, context);
			const Group* subGroup = matchSome(i, tiles[j].pattern, context, &closure, tileAction);
			if (subGroup == null)
				continue;

			if (subGroup->dancerCount() == _dancers.size()) {
				out->dancers = subGroup;
				out->matched = &tiles[j];
				return 1;
			}

			TileSearch cover[MAX_DANCERS];

			cover[0].dancers = subGroup;
			cover[0].matched = &tiles[j];

			const Group* r = this->minus(subGroup, context);
			int result = r->buildTiling(tiles, cover + 1, context, call, step, tileAction);
			if (result < 0)
				continue;
			result++;
			int dancersInThis = 0;
			int score = 0;
			for (int i = 0; i < result; i++) {
				dancersInThis += cover[i].dancers->dancerCount();
				if (cover[i].matched->variant)
					score += 1 << (cover[i].matched->variant->precedence() * PRECEDENCE_SHIFT);
			}
			if (dancersInBest == dancersInThis && bestResult == result && bestScore == score) {
				if (!bestSorted) {
					qsort(out, bestResult, sizeof (TileSearch), Group::compare);
					bestSorted = true;
				}
				qsort(cover, result, sizeof (TileSearch), Group::compare);
				for (int i = 0; i < result; i++)
					if (Group::compare(&out[i], &cover[i])) {
						unique = false;
						if (verboseOutput) {
							printf("Non-unique tiling: score = %d\n", score);
							for (int i = 0; i < result; i++) {
								printf("    out[%d]: precedence %d\n", i, out[i].matched->variant->precedence());
								out[i].dancers->printDetails(4, true);
								printf("    cover[%d]: precedence %d\n", i, cover[i].matched->variant->precedence());
								cover[i].dancers->printDetails(4, true);
							}
						}
						break;
					}
			} else {
				bool thisImprovesOverBest = false;
				// A 'better' result covers more dancers or covers the same number of dancers in fewer tiles
				if (dancersInBest < dancersInThis)
					thisImprovesOverBest = true;
				else if (dancersInBest == dancersInThis) {
					if (bestResult > result)
						thisImprovesOverBest = true;
					else if (bestResult == result) {
						if (bestScore < score)
							thisImprovesOverBest = true;
					}
				}
				if (thisImprovesOverBest) {
					dancersInBest = dancersInThis;
					memcpy(out, cover, result * sizeof (TileSearch));
					bestResult = result;
					bestScore = score;
					unique = true;
					bestSorted = false;
				}
			}
		}
	}
	if (unique)
		return bestResult;
	else
		return -1;
}

int Group::buildPhantom4Dancer(const VariantTile& tile, TileSearch* out, Context* context, const Anything* call, Step* step) const {
	int goalCount = dancerCount();
	const Group* g = this;
	PatternClosure closure(tile.pattern, call, step, context);
	Rectangle r;
	const Formation* f = tile.pattern->formation();

	int symmetry = f->rotationalSymmetry();
	f->boundingBox(&r);
	int dx = (r.width() + 1) >> 1;
	int dy = (r.height() + 1) >> 1;
	out[0].matched = &tile;
	out[1].matched = &tile;

	// First iteration straight up, then rotated 90 degrees.
	for (int i = 0; ; i++, g = g->apply(&Transform::rotate90, context)) {
		if (i >= 2)
			return -1;
		const Group* gTop = g->apply(Transform::translate(0, dy), context);
		const Group* gBottom = g->apply(Transform::translate(0, -dy), context);

		out[0].dancers = gTop->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
		if (out[0].dancers != null) {

			out[1].dancers = gBottom->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
			if (out[1].dancers != null) {
				int used = out[0].dancers->realDancerCount() + out[1].dancers->realDancerCount();
				if (used == goalCount)
					break;
			}
		}

		const Group* gLeft = g->apply(Transform::translate(-dx, 0), context);
		const Group* gRight = g->apply(Transform::translate(dx, 0), context);

		out[0].dancers = gLeft->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
		if (out[0].dancers != null) {

			out[1].dancers = gRight->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
			if (out[1].dancers != null) {
				int used = out[0].dancers->realDancerCount() + out[1].dancers->realDancerCount();
				if (used == goalCount)
					break;
			}
		}
		if (symmetry == 1) {
			gTop = gTop->apply(&Transform::rotate180, context);

			out[0].dancers = gTop->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
			if (out[0].dancers != null) {
				gBottom = gBottom->apply(&Transform::rotate180, context);

				out[1].dancers = gBottom->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
				if (out[1].dancers != null) {
					int used = out[0].dancers->realDancerCount() + out[1].dancers->realDancerCount();
					if (used == goalCount)
						break;
				}
			}

			gLeft = gLeft->apply(&Transform::rotate180, context);

			out[0].dancers = gLeft->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
			if (out[0].dancers != null) {
				gRight = gRight->apply(&Transform::rotate180, context);

				out[1].dancers = gRight->matchSome(0, tile.pattern, context, &closure, TILE_WITH_PHANTOMS);
				if (out[1].dancers != null) {
					int used = out[0].dancers->realDancerCount() + out[1].dancers->realDancerCount();
					if (used == goalCount)
						break;
				}
			}
		}
		// If the tile has 4-fold symmetry, we've tried all the possible fits, so give up.
		if (symmetry == 4)
			return -1;
	}
	return 2;
}

Group* Group::apply(const Transform* transform, Context* context) const {
	Group* out = cloneNonDancerData(context);
	out->_transform->dispose();
	out->_transform = transform;
	out->_base = this;
	for (int i = 0; i < _dancers.size(); i++)
		out->_dancers.push_back(transform->apply(_dancers[i]));
	out->_dancers.sort();
	return out;
}

void Group::convertToLocal(const Group* coordinateSystem, Dancer* dancer) const {
	if (this != coordinateSystem) {
		convertToLocal(coordinateSystem->_base, dancer);
		if (coordinateSystem->_transform)
			coordinateSystem->_transform->apply(&dancer->x, &dancer->y, &dancer->facing);
	}
}

void Group::convertToAbsolute(float* x, float* y, Facing* facing, double* noseAngle) const {
	for (const Group* g = this; g != null; g = g->_base) {
		if (g->_transform)
			g->_transform->revert(x, y, facing);
	}
	double baseAngle = rotationAngle(_rotation);
	if (noseAngle)
		*noseAngle = NO_NOSE;
	if (_geometry == RING) {
		double angle = baseAngle + (9 - *x) * PI / 8;
		double radius = *y;
		*x = radius * cos(angle);
		*y = radius * sin(angle);
		if (facing) {
			switch (*facing) {
			case	RIGHT_FACING:
				*noseAngle = angle - PI / 2;
				break;

			case	BACK_FACING:
				*noseAngle = angle;
				break;

			case	LEFT_FACING:
				*noseAngle = angle + PI / 2;
				break;

			case	FRONT_FACING:
				*noseAngle = angle - PI;
				break;

			case	HEAD_FACING:
				*noseAngle = angle;
				break;

			case	SIDE_FACING:
				*noseAngle = angle - PI / 2;
				break;

			case	ANY_FACING:
				*noseAngle = NO_NOSE;
			}
		}
	} else {
		double rawX = *x;
		double rawY = *y;
		*x = cos(baseAngle) * rawX - sin(baseAngle) * rawY;
		*y = sin(baseAngle) * rawX + cos(baseAngle) * rawY;
		if (facing) {
			switch (*facing) {
			case	RIGHT_FACING:
				*noseAngle = baseAngle;
				break;

			case	BACK_FACING:
				*noseAngle = baseAngle + PI / 2;
				break;

			case	LEFT_FACING:
				*noseAngle = baseAngle + PI;
				break;

			case	FRONT_FACING:
				*noseAngle = baseAngle - PI / 2;
				break;

			case	HEAD_FACING:
				*noseAngle = baseAngle + PI / 2;
				break;

			case	SIDE_FACING:
				*noseAngle = baseAngle;
				break;

			case	ANY_FACING:
				*noseAngle = NO_NOSE;
			}
		}
	}
}

void Group::convertFromAbsolute(float* x, float* y) const {
	applyTransforms(x, y);
	double baseAngle = 0;
	switch (_rotation) {
	case	DIAGONAL:
		baseAngle = PI / 4;
		break;

	case	ROTATED_1:
		baseAngle = PI /16;
		break;

	case	ROTATED_2:
		baseAngle = -3 * PI /16;
	}
	if (_geometry == RING) {
		if (*x == 0 && *y == 0)
			return;
		double a = angle(*x, *y) - baseAngle;
		double radius = sqrt(*x * *x + *y * *y);
		*x = 9 - 8 * a / PI;
		*y = radius;
	} else {
		double rawX = *x;
		double rawY = *y;
		*x = cos(baseAngle) * rawX + sin(baseAngle) * rawY;
		*y = sin(baseAngle) * rawX + cos(baseAngle) * rawY;
	}
}

void Group::applyTransforms(float* x, float* y) const {
	if (_base) {
		_base->applyTransforms(x, y);
		if (_transform)
			_transform->apply(x, y, null);
	}
}

void Group::convertToAbsolute(double *angle) const {
	if (_base) {
		if (_transform)
			_transform->revert(angle);
		_base->convertToAbsolute(angle);
	}
}

void Group::convertFromAbsolute(double *angle) const {
	if (_base) {
		_base->convertFromAbsolute(angle);
		if (_transform)
			_transform->revert(angle);
	}
}

double Group::convertRingXToAngle(float x) const {
	if (_base) {
		double angle = _base->convertRingXToAngle(x);
		if (_transform) {
			if (_transform->leftQuarterTurns() == 2)
				angle = -angle;
			_transform->revert(&angle);
		}
		return angle;
	}
	return x * PI / 8;
}

const Group* Group::match(const Pattern* pattern, Context* context, const PatternClosure* closure) const {
	timing::Timer t("Group::match");
	if (_geometry == RING) {
		int n = pattern->formation()->dancerCount();
		if (n <= _dancers.size()) {
			const Group* d = matchThisOrder(pattern, context, closure);
			if (d)
				return d;
			for (int i = 0; i < _dancers.size(); i++) {
				if (_dancers[i]->x < 0 || _dancers[i]->x > 15) {
					Group* temp = normalizeRingCoordinates(context);
					d = temp->matchThisOrder(pattern, context, closure);
					if (d)
						return d;
					for (int i = 1; i < n; i++) {
						d = temp->rotateDancers(i, context);
						d = d->matchThisOrder(pattern, context, closure);
						if (d)
							return d;
					}
					return null;
				}
			}
			for (int i = 1; i < n; i++) {
				d = rotateDancers(i, context);
				d = d->matchThisOrder(pattern, context, closure);
				if (d)
					return d;
			}
		}
		return null;
	} else
		return matchThisOrder(pattern, context, closure);
}

const Group* Group::matchThisOrder(const Pattern* pattern, Context* context, const PatternClosure* closure) const {
	if (pattern->match(this, closure))
		return pattern->formation()->recenter(clone(context), context);
	int s = pattern->formation()->rotationalSymmetry();
	Group* rotated;
	switch (s) {
	case	1:
		rotated = apply(&Transform::rotate180, context);
		if (pattern->match(rotated, closure))
			return pattern->formation()->recenter(rotated, context);
		rotated = apply(&Transform::rotate270, context);
		if (pattern->match(rotated, closure))
			return pattern->formation()->recenter(rotated, context);

		// Fall through to the 2-fold symmetric case
	case	2:
		rotated = apply(&Transform::rotate90, context);
		if (pattern->match(rotated, closure))
			return pattern->formation()->recenter(rotated, context);
	}
	return null;
}

const Group* Group::matchSome(int startWith, const Pattern* pattern, Context* context, const PatternClosure* closure, TileAction tileAction) const {
	timing::Timer t("Group::matchSome");
	if (_geometry == RING) {
		int n = pattern->formation()->dancerCount();
		if (n <= _dancers.size()) {
			const Group* d = matchSomeThisOrder(startWith, pattern, context, closure, tileAction);
			if (d)
				return d;
			for (int i = 1; i < n; i++) {
				d = rotateDancers(i, context);
				if (verboseMatching) {
					printf("Trying this grouping:\n");
					d->printDetails(4, true);
				}
				d = d->matchSomeThisOrder(startWith, pattern, context, closure, tileAction);
				if (d) {
					if (verboseMatching) {
						printf("Found this grouping:\n");
						d->printDetails(4, true);
					}
					return d;
				}
			}
		}
		return null;
	} else
		return matchSomeThisOrder(startWith, pattern, context, closure, tileAction);
}

const Group* Group::matchSomeThisOrder(int startWith, const Pattern* pattern, Context* context, const PatternClosure* closure, TileAction tileAction) const {
	if (tileAction == TILE_WITH_PHANTOMS) {
		Group* g = pattern->matchWithPhantoms(this, closure);
		if (g)
			return g;
		int s = pattern->formation()->rotationalSymmetry();
		Group* rotated;
		switch (s) {
		case	1:
			rotated = apply(&Transform::rotate180, context);
			g = pattern->matchWithPhantoms(rotated, closure);
			if (g)
				return g;
			rotated = apply(&Transform::rotate270, context);
			g = pattern->matchWithPhantoms(rotated, closure);
			if (g)
				return g;

			// Fall through to the 2-fold symmetric case
		case	2:
			rotated = apply(&Transform::rotate90, context);
			return pattern->matchWithPhantoms(rotated, closure);
		}
	} else {
		unsigned mask = pattern->matchSome(this, startWith, closure);
		if (mask)
			return pattern->formation()->recenter(extract(mask, context), context);
		int s = pattern->formation()->rotationalSymmetry();
		Group* rotated;
		switch (s) {
		case	1:
			rotated = apply(&Transform::rotate180, context);
			mask = pattern->matchSome(rotated, startWith, closure);
			if (mask)
				return pattern->formation()->recenter(rotated->extract(mask, context), context);
			rotated = apply(&Transform::rotate270, context);
			mask = pattern->matchSome(rotated, startWith, closure);
			if (mask)
				return pattern->formation()->recenter(rotated->extract(mask, context), context);

			// Fall through to the 2-fold symmetric case
		case	2:
			rotated = apply(&Transform::rotate90, context);
			mask = pattern->matchSome(rotated, startWith, closure);
			if (mask)
				return pattern->formation()->recenter(rotated->extract(mask, context), context);
		}
	}
	return null;
}

const Group* Group::moveIn(PrimitiveStep* step, const Anyone* anyone, Context* context) const {
	if (_dancers.size() != 8) {
		step->fail(context->stage()->newExplanation(DEFINITION_ERROR, "$move_in only makes sense from a full ring"));
		return null;
	}
	const Group* d = this;
	if (_geometry == RING) {
		int startAt;

		unsigned mask = anyone->match(this, step, context);
		if (mask == 0) {
			step->fail(context->stage()->newExplanation(USER_ERROR, "Could not find the named dancers"));
			return null;
		}
		d = extract(mask, context);
		// We have to select 4 dancers and they have to include someone from the first three
		// slots.
		if (d->dancerCount() != 4) {
			step->fail(context->stage()->newExplanation(USER_ERROR, "You did not select four dancers"));
			return null;
		}
		d = d->normalizeRingCoordinates(context);
		if (verboseOutput) {
			printf("moveIn:\n");
			d->printDetails(4, true);
		}
		// First verify that we select opposite couples
		// and discover the first position they occupy
		if (d->dancer(0)->x + 2 == d->dancer(1)->x) {
			if (d->dancer(1)->x + 6 != d->dancer(2)->x ||
				d->dancer(2)->x + 2 != d->dancer(3)->x) {
				step->fail(context->stage()->newExplanation(USER_ERROR, "Dancers are not opposite couples"));
				return null;
			}
			startAt = d->dancer(0)->x / 2;
		} else {
			if (d->dancer(0)->x + 6 != d->dancer(1)->x ||
				d->dancer(1)->x + 2 != d->dancer(2)->x ||
				d->dancer(2)->x + 6 != d->dancer(3)->x) {
				step->fail(context->stage()->newExplanation(USER_ERROR, "Dancers are not opposite couples"));
				return null;
			}
			startAt = 3;
		}
		if (verboseOutput) {
			printf("startAt=%d rotateBy: %d\n", startAt, rotateBy(_rotation, 4));
		}
		// An odd starting point requires that you do some tricks
		// with the rotation to put people in the right places.
		if (startAt & 1) {
			int amount = 1;
			int newRotation = _rotation + 4;
			if (newRotation >= 8) {
				newRotation -= 8;
				amount -= 2;
			}
			d = circle(null, amount, (Rotation)newRotation, null, context);
			if (verboseOutput)
				d->print(0);
		} else
			return this;
	}
	return d;
}

const Group* Group::forwardAndBack(Interval* interval, Context* context) const {
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* dancer = _dancers[i];
		int deltaX, deltaY;

		dancer->displace(1, 0, &deltaX, &deltaY);
		interval->forwardVeer(dancer, Point(deltaX * 0.4, deltaY * 0.4), 0, 0, 0, 1, context);
		interval->forwardVeer(dancer, Point(deltaX * -0.4, deltaY * -0.4), 0.4, 0, 0, 1, context);
	}
	return this;
}

Group* Group::formRing(Interval* interval, Context* context) const {
	static int xValue[] = { 4, 6, 2, 8, 0, 10, 14, 12 };
	static int leftTurns[] = { 0, 0, 3, 1, 3, 1, 2, 2 };
	if (_dancers.size() != 8)
		return null;
	if (verboseOutput) {
		printf("form ring:\n");
		printDetails(4, true);
	}
	const Group* g = this;
	while (g->_base) {
		g = g->unwind(context);
		if (g == null)
			return null;
	}

	Group* out = g->cloneNonDancerData(context);
	out->_geometry = RING;
	interval->currentDancers(g);
	if (g->_dancers[0]->y == 3) {				// squared set, thar/star or lines/columns
		if (g->_dancers[0]->x == 0) {			// a thar/star
			static int xValue[] = { 5, 5, 1, 1, 9, 9, 13, 13 };
			static int yValue[] = { 4, 2, 4, 2, 2, 4, 2, 4 };
			static int leftTurns[] = { 0, 0, 3, 3, 1, 1, 2, 2 };

			for (int i = 0; i < g->_dancers.size(); i++) {
				const Dancer* d = new Dancer(xValue[i], yValue[i], quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
				interval->setToRing(g->_dancers[i], out, d, xValue[i], yValue[i], 1, context);
				out->_dancers.push_back(d);
			}
		} else if (g->_dancers[0]->x == -1) {	// a squared set or lines/columns
			if (g->_dancers[2]->x == -1) {		// line/column
				static int xValue[] = { 4, 6, 2, 8, 0, 10, 14, 12 };
				static int leftTurns[] = { 3, 1, 3, 1, 3, 1, 3, 1 };
				for (int i = 0; i < g->_dancers.size(); i++) {
					const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
					interval->setToRing(g->_dancers[i], out, d, xValue[i], 3, 1, context);
					out->_dancers.push_back(d);
				}
			} else {						// squared set
				for (int i = 0; i < g->_dancers.size(); i++) {
					const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
					interval->setToRing(g->_dancers[i], out, d, xValue[i], 3, 1, context);
					out->_dancers.push_back(d);
				}
			}
		} else
			return null;
	} else if (g->_dancers[0]->y == 2) {
		for (int i = 0; i < g->_dancers.size(); i++) {
			const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
			interval->setToRing(g->_dancers[i], out, d, xValue[i], 3, 1, context);
			out->_dancers.push_back(d);
		}
	} else if (g->_dancers[0]->y == 4) {	// a sausage line - oriented vertically
		static int xValue[] = { 5, 3, 7, 1, 9, 15, 11, 13 };
		static int leftTurns[] = { 0, 3, 1, 3, 1, 3, 1, 2 };

		for (int i = 0; i < g->_dancers.size(); i++) {
			const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
			interval->setToRing(g->_dancers[i], out, d, xValue[i], 3, 1, context);
			out->_dancers.push_back(d);
		}
	} else if (g->_dancers[3]->x == -4) {	// a sausage line - oriented horizontally
		static int xValue[] = { 3, 5, 7, 1, 9, 15, 13, 11 };
		static int leftTurns[] = { 0, 0, 0, 3, 1, 2, 2, 2 };

		for (int i = 0; i < g->_dancers.size(); i++) {
			const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
			interval->setToRing(g->_dancers[i], out, d, xValue[i], 3, 1, context);
			out->_dancers.push_back(d);
		}
	} else if (g->_dancers[0]->y == 1) {		// line or oclumn
		static int xValue[] = { 2, 4, 6, 8, 0, 14, 12, 10 };
		static int leftTurns[] = { 0, 0, 0, 0, 2, 2, 2, 2 };
		for (int i = 0; i < g->_dancers.size(); i++) {
			const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(g->_dancers[i]->facing, leftTurns[i]), g->_dancers[i]->gender, g->_dancers[i]->couple, g->_dancers[i]->dancerIndex());
			interval->setToRing(g->_dancers[i], out, d, xValue[i], 3, 1, context);
			out->_dancers.push_back(d);
		}
	} else {
		const Dancer* dancerMap[8];

		dancerMap[0] = g->_dancers[1];
		dancerMap[1] = g->_dancers[2];
		dancerMap[2] = g->_dancers[0];
		dancerMap[3] = g->_dancers[3];
		dancerMap[4] = g->_dancers[4];
		dancerMap[5] = g->_dancers[7];
		dancerMap[6] = g->_dancers[5];
		dancerMap[7] = g->_dancers[6];
		for (int i = 0; i < 8; i++) {
			const Dancer* d = new Dancer(xValue[i], 3, quarterLeft(dancerMap[i]->facing, leftTurns[i]), dancerMap[i]->gender, dancerMap[i]->couple, dancerMap[i]->dancerIndex());
			interval->setToRing(dancerMap[i], out, d, xValue[i], 3, 1, context);
			out->_dancers.push_back(d);
		}
	}
	out->done();
	return out;
}

const Group* Group::formGrid(Interval* interval, vector<const Group*>& groups, Context* context) const {
	// Okay, we think that dancers are off the ring.  If all dancers fall along spokes at 90 degree angles
	// from one another, we are golden.
	int spoke = INT_MAX;
	bool confirmGrid = false;
	for (int i = 0; i < groups.size(); i++) {
		for (int j = 0; j < groups[i]->dancerCount(); j++) {
			float ax = groups[i]->dancer(j)->x;
			float ay = groups[i]->dancer(j)->y;
			groups[i]->convertToAbsolute(&ax, &ay, null, null);
			if (abs(ay - 3) < EPSILON)
				confirmGrid = true;
			int x = groups[i]->dancer(j)->x;
			while (x < 0)
				x += 16;

			if (spoke == INT_MAX)
				spoke = x & 3;
			else if (spoke != (x & 3))
				spoke = -1;
		}
	}
	// If all the dancers are on the ring longitude (y == 3), we have nothing to do.
	if (!confirmGrid)
		return combine(groups, context);
	// If we got conflicting spoke values, we need to abandon ship now.
	if (spoke == -1) {
		if (verboseOutput) {
			printf("spoke == -1:\n");
			printDetails(4, true);
			for (int i = 0; i < groups.size(); i++) {
				printf("  Group %d:\n", i);
				groups[i]->printDetails(6, true);
			}
		}
		return null;
	}
	// Not sure how this could happen, but it would mean there are no dancers in
	// the affected tiles.  We can then just return this set, unchanged.
	if (spoke == INT_MAX)
		return this;
	if (_dancers.size() != 8)
		return combine(groups, context);
	// Okay, the spoke says this:
	//	0 = All dancers are on spoke 0, 4, 8 or 12 - ROTATED_6
	//	1 = All dancers are on spoke 1, 5, 9 or 13 - UNROTATED
	//	2 = All dancers are on spoke 2, 6, 10 or 14 - ROTATED_2
	//  3 = All dancers are on spoke 3, 7, 11 or 15 - DIAGONAL
	return null;
}

Group* Group::formSet(Interval* interval, Context* context) const {
	static int xValue[] = { -3, -3, -1, 1, 3, 3, 1, -1 };
	static int yValue[] = { -1, 1, 3, 3, 1, -1, -3, -3 };
	static int leftTurns[] = { 1, 1, 0, 0, 3, 3, 2, 2 };
	Group* normal = normalizeRingCoordinates(context);
	Group* out = cloneNonDancerData(context);
	out->_geometry = GRID;
	interval->currentDancers(this);

	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* d = new Dancer(xValue[i], yValue[i], 
									 quarterLeft(normal->_dancers[i]->facing, leftTurns[i]), 
									 normal->_dancers[i]->gender, 
									 normal->_dancers[i]->couple,
									 normal->_dancers[i]->dancerIndex());
		interval->ringToSet(normal->_dancers[i], out, d, 1, context);
		out->insert(d);
	}
	out->done();
	return out;
}

Group* Group::formPromenade(const Anydirection* direction, Context* context, Interval* interval) const {
	bool forward;
	switch (direction->direction()) {
	case	D_FORWARD:
		forward = true;
		break;

	case	D_BACK:
		forward = false;
		break;

	default:
		return null;
	}
	if (_dancers.size() != 8)
		return null;
	const Group* dn;
	if (_geometry == RING) {
		for (int i = 0; i < _dancers.size(); i++) {
			const Dancer* d = _dancers[i];

			if (forward) {
				if (d->facing != LEFT_FACING)
					return null;
			} else {
				if (d->facing != RIGHT_FACING)
					return null;
			}
		}
		return formCrossedCouplesFromRing(2, context, interval);
	} else
		dn = this;
	TileSearch tiles[MAX_DANCERS];

	int result = dn->buildTiling(context->grammar()->couples(), tiles, context, null, null, TILE_ALL);
	if (result < 4)
		return null;

	int lateral[MAX_DANCERS];
	const Group* d[MAX_DANCERS];
	bool posX = false;
	bool posY = false;
	bool negX = false;
	bool negY = false;
	for (int i = 0; i < result; i++) {
		d[i] = tiles[i].dancers->toCommonCoordinates(dn, context);
		const Dancer* a = d[i]->dancer(0);
		const Dancer* b = d[i]->dancer(1);

		if (a->x == 0) {
			if (a->y > 0) {
				if (posY)
					return null;
				posY = true;
				// we know that because this is a couple facing left, and the a dancer is first,
				// then the b dancer must be at a lesser y value.
				if (b->y <= 0)
					return null;
				if (forward) {
					if (a->facing != LEFT_FACING)
						return null;
					lateral[i] = 2 - b->y;
				} else {
					if (a->facing != RIGHT_FACING)
						return null;
					lateral[i] = b->y - 2;
				}
			} else if (a->y < 0) {
				if (negY)
					return null;
				negY = true;
				// we know that because this is a couple facing right, and the a dancer is first,
				// then the b dancer must be at a lesser y value.
				if (forward) {
					if (a->facing != RIGHT_FACING)
						return null;
					lateral[i] = 2 + a->y;
				} else {
					if (a->facing != LEFT_FACING)
						return null;
					lateral[i] = -2 - a->y;
				}
			} else
				return null;				// Anyone on the center spot means we can't make a promenade formation
		} else if (a->y == 0) {
			if (a->x > 0) {
				if (posX)
					return null;
				posX = true;
				// we know that because this is a couple facing back, and the a dancer is first,
				// then the b dancer must be at a greater x value.
				if (forward) {
					if (a->facing != BACK_FACING)
						return null;
					lateral[i] = 2 - a->x;
				} else {
					if (a->facing != FRONT_FACING)
						return null;
					lateral[i] = a->x - 2;
				}
			} else if (a->x < 0) {
				if (negX)
					return null;
				negX = true;
				// we know that because this is a couple facing heads and the a dancer is first,
				// then the b dancer must be at a greater x value.
				if (b->x >= 0)
					return null;
				if (forward) {
					if (a->facing != FRONT_FACING)
						return null;
					lateral[i] = 2 + b->x;
				} else {
					if (a->facing != BACK_FACING)
						return null;
					lateral[i] = -2 - b->x;
				}
			} else
				return null;				// Anyone on the center spot means we can't make a promenade formation
		} else
			return null;
	}
	Group* out = context->stage()->newGroup(dn);
	interval->currentDancers(this);
	for (int i = 0; i < result; i++) {
		for (int j = 0; j < d[i]->dancerCount(); j++) {
			const Dancer* a = d[i]->dancer(j);
			const Dancer* pa;
			if (lateral[i])
				pa = a->forwardVeer(0, lateral[i], 0, interval, context);
			else
				pa = a->clone();
			out->insert(pa);
		}
	}
	out->done();
	return out;
}

Group* Group::formThar(Interval* interval, Context* context) const {
	if (_dancers.size() != 8)
		return null;
	const Group* dn;
	if (_geometry == RING)
		return formCrossedCouplesFromRing(1, context, interval);
	dn = this;
	int lateral[MAX_DANCERS];
	Group* out = cloneNonDancerData(context);
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* a = _dancers[i];
		int lateral = 0;

		if (a->x == 0) {
			if (a->y > 0) {
				if (a->facing != LEFT_FACING &&
					a->facing != RIGHT_FACING)
					return null;
				if (a->y == 2 ||
					a->y == 4)
					lateral = a->facing == LEFT_FACING ? -1 : 1;
			} else if (a->y < 0) {
				if (a->facing != RIGHT_FACING &&
					a->facing != LEFT_FACING)
					return null;
				if (a->y == -2 ||
					a->y == -4)
					lateral = a->facing == LEFT_FACING ? 1 : -1;
			} else
				return null;				// Anyone on the center spot means we can't make a thar formation
		} else if (a->y == 0) {
			if (a->x > 0) {
				if (a->facing != BACK_FACING &&
					a->facing != FRONT_FACING)
					return null;
				if (a->x == 2 ||
					a->x == 4)
					lateral = a->facing == BACK_FACING ? -1 : 1;
			} else if (a->x < 0) {
				if (a->facing != FRONT_FACING &&
					a->facing != BACK_FACING)
					return null;
				if (a->x == -2 ||
					a->x == -4)
					lateral = a->facing == BACK_FACING ? 1 : -1;
			} else
				return null;				// Anyone on the center spot means we can't make a promenade formation
		} else
			return null;
		const Dancer* pa;
		if (lateral != 0)
			pa = a->forwardVeer(0, lateral, 0, interval, context);
		else
			pa = a->clone();
		out->insert(pa);
	}
	out->done();
	return out;
}

Group* Group::formCrossedCouplesFromRing(int minRadius, Context* context, Interval* interval) const {
	Group* dn = normalizeRingCoordinates(context);
	int diagonal = dn->dancer(0)->x & 3;			// If the x value is not 1 mod 4, we must be in a diagonal formation
													// otherwise, we are square to the (possibly rotated) walls.
	if (verboseOutput) {
		printf("formCrossedCouplesFromRing from RING: rotation=%d diagonal=%d\n", _rotation, diagonal);
		dn->printDetails(4, true);
	}
	Group* out = cloneNonDancerData(context);
	out->_geometry = GRID;
	out->_rotation = rotateBy(_rotation, 2 - diagonal * 2);
	int displacement = minRadius - 2;
	if (displacement != 0)
		interval->currentDancers(dn);
	for (int i = 0; i < dn->_dancers.size(); i++) {
		const Dancer* a = dn->_dancers[i];

		if (a->facing == BACK_FACING ||
			a->facing == FRONT_FACING) {
			if (verboseOutput) {
				printf("Dancer %d facing wrong\n", i);
				a->print(8);
				out->printDetails(4, true);
			}
			return null;
		}
		if (a->y != 2 && a->y != 4) {
			if (verboseOutput) {
				printf("Dancer %d on wrong y\n", i);
				a->print(8);
				out->printDetails(4, true);
			}
			return null;
		}
		int rawX = a->x;
		int rawY = a->y + displacement;
		int setX, setY;
		int leftTurns;
		switch (rawX) {
		case	14:					// -5 PI / 8
		case	15:					// -3 PI / 4
		case	0:					//  9 PI / 8
		case	1:					//    PI
			setX = -rawY;
			setY = 0;
			leftTurns = 1;
			break;

		case	2:					//  7 PI / 8
		case	3:					//  3 PI / 4
		case	4:					//  5 PI / 8
		case	5:					//    PI / 2
			setX = 0;
			setY = rawY;
			leftTurns = 0;
			break;

		case	6:					//  3 PI / 8
		case	7:					//    PI / 4
		case	8:					//    PI / 8
		case	9:					//    0
			setX = rawY;
			setY = 0;
			leftTurns = -1;
			break;

		case	10:					//   -PI / 8
		case	11:					//   -PI / 4
		case	12:					// -3 PI / 8
		case	13:					//   -PI / 2
			setX = 0;
			setY = -rawY;
			leftTurns = 2;
			break;

		default:
			if (verboseOutput) {
				printf("rawX=%d\n", rawX);
				a->print(4);
				out->printDetails(4, true);
			}
			return null;				// How did a normalized ring dancer get an invalid x coordinate like this?
		}
		if (verboseOutput) {
			printf("raw=[%d,%d] set=[%d,%d] leftTurns=%d\n", rawX, rawY, setX, setY, leftTurns);
			a->print(4);
		}
		const Dancer* setA = new Dancer(setX, setY, 
								 quarterLeft(a->facing, leftTurns), 
								 a->gender, 
								 a->couple,
								 a->dancerIndex());
		if (displacement != 0)
			interval->ringToSet(a, out, setA, 1, context);
		out->insert(setA);
	}
	out->done();
	if (verboseOutput) {
		printf("Formed up:\n");
		out->printDetails(4, true);
	}
	return out;
}

const Group* Group::run(const Group* runners, const Anydirection* direction, Context* context, Interval* interval) const {
	unsigned mask = 0;

	unsigned runnersMask = runners->dancerMask();
	unsigned inactiveMask = ~runnersMask;
	vector<const Dancer*> pairings;
	if (direction)
		adjacentTo(runners, direction, context, &pairings);
	else
		partnershipOp(NONE, runners, context, &pairings);
	if (pairings.size() < runners->dancerCount())
		return null;
	if (verboseOutput) {
		printf("Runners:\n");
		runners->printDetails(4, true);
		printf("    Partners:\n");
		for (int i = 0; i < pairings.size(); i++) {
			if (pairings[i])
				pairings[i]->print(8);
			else
				printf("        <none>\n");
		}
	}
	Group* output = context->stage()->newGroup(this);
	interval->currentDancers(this);
	for (int i = 0; i < pairings.size(); i++)
		if (pairings[i] == null)
			return null;
		else {
			Point center;
			int rightSixteenthTurns = 8;

			center.x = (pairings[i]->x + runners->dancer(i)->x) / 2.0;
			center.y = (pairings[i]->y + runners->dancer(i)->y) / 2.0;
			// Make adjustments to the arc center in ring where the pair is wrapping.
			if (runners->geometry() == RING) {
				if (pairings[i]->x >= 14 && runners->dancer(i)->x < 2)
					center.x = -1;
				else if (pairings[i]->x < 2 && runners->dancer(i)->x >= 14)
					center.x = 15;
			}
			switch (runners->dancer(i)->facing) {
			case	RIGHT_FACING:
				if (runners->dancer(i)->y < center.y)
					rightSixteenthTurns = -rightSixteenthTurns;
				break;

			case	BACK_FACING:
				if (runners->dancer(i)->x > center.x)
					rightSixteenthTurns = -rightSixteenthTurns;
				break;

			case	LEFT_FACING:
				if (runners->dancer(i)->y > center.y)
					rightSixteenthTurns = -rightSixteenthTurns;
				break;

			case	FRONT_FACING:
				if (runners->dancer(i)->x < center.x)
					rightSixteenthTurns = -rightSixteenthTurns;
				break;

			}
			output->insert(runners->dancer(i)->arc(runners->geometry(), center, 1, rightSixteenthTurns, 0, 0, interval, context));
			// Make further adjustments to the arc center in ring where the pair is wrapping.
			if (runners->geometry() == RING) {
				if (pairings[i]->x >= 14 && runners->dancer(i)->x < 2)
					center.x = 15;
				else if (pairings[i]->x < 2 && runners->dancer(i)->x >= 14)
					center.x = -1;
			}
			if ((pairings[i]->dancerMask() & runnersMask) == 0) {
				int amount = 2;
				bool lateral = true;

				switch (pairings[i]->facing) {
				case	RIGHT_FACING:
					if (pairings[i]->x < center.x)
						lateral = false;
					else if (pairings[i]->x > center.x) {
						lateral = false;
						amount = -amount;
					} else if (pairings[i]->y < center.y)
						amount = -amount;
					break;

				case	BACK_FACING:
					if (pairings[i]->y < center.y)
						lateral = false;
					else if (pairings[i]->y > center.y) {
						lateral = false;
						amount = -amount;
					} else if (pairings[i]->x > center.x)
						amount = -amount;
					break;

				case	LEFT_FACING:
					if (pairings[i]->x > center.x)
						lateral = false;
					else if (pairings[i]->x < center.x) {
						lateral = false;
						amount = -amount;
					} else if (pairings[i]->y > center.y)
						amount = -amount;
					break;

				case	FRONT_FACING:
					if (pairings[i]->y > center.y)
						lateral = false;
					else if (pairings[i]->y < center.y) {
						lateral = false;
						amount = -amount;
					} else if (pairings[i]->x < center.x)
						amount = -amount;
					break;
				}
				if (lateral)
					output->insert(pairings[i]->forwardVeer(0, amount, 0, interval, context));
				else
					output->insert(pairings[i]->forwardVeer(amount, 0, 0, interval, context));
			}
		}
	if (verboseOutput) {
		printf("Before merge of run:\n");
		output->printDetails(4, true);
	}
	return output->merge(context);
}

Group* Group::fractionalize(const Anything* call, const Fraction* fraction, Context* context) const {
	return null;
}

void Group::adjacentTo(const Group* subset, const Anydirection* direction, Context* context, vector<const Dancer*>* output) const {
	for (int i = 0; i < subset->dancerCount(); i++) {
		const Dancer* d = subset->dancer(i);

		int targetX, targetY;
		switch (d->facing) {
		case	RIGHT_FACING:
			targetX = d->x;
			if (direction->direction() == D_RIGHT)
				targetY = d->y - 2;
			else
				targetY = d->y + 2;
			break;

		case	BACK_FACING:
			targetY = d->y;
			if (direction->direction() == D_RIGHT)
				targetX = d->x + 2;
			else
				targetX = d->x - 2;
			break;

		case	LEFT_FACING:
			targetX = d->x;
			if (direction->direction() == D_RIGHT)
				targetY = d->y + 2;
			else
				targetY = d->y - 2;
			break;

		case	FRONT_FACING:
			targetY = d->y;
			if (direction->direction() == D_RIGHT)
				targetX = d->x - 2;
			else
				targetX = d->x + 2;
			break;
		}

		for (int j = 0; j < _dancers.size(); j++) {
			const Dancer* b = _dancers[j];
			if (_geometry == RING) {
				if (((b->x - targetX) & 0xf) == 0 &&
					b->y == targetY) {
					output->push_back(b);
					break;
				}
			} else {
				if (b->x == targetX &&
					b->y == targetY) {
					output->push_back(b);
					break;
				}
			}
		}
		// If the output size didn't increase, we had no one to run around in the given direction.
		if (output->size() == i)
			break;
	}
}

void Group::partnershipOp(DancerSet which, const Group* subset, Context* context, vector<const Dancer*>* output) const {
	if (_geometry == RING) {
		const Formation* f = context->grammar()->formation("infacing_ring");
		if (f) {
			if (f->match(this, null)) {
				for (int i = 0; i < _dancers.size() - 1; i += 2) {
					if (_dancers[i]->gender != _dancers[0]->gender ||
						_dancers[i]->gender == _dancers[i + 1]->gender)
						return;
				}
				switch (which) {
				case	NONE:
					for (int i = 0; i < subset->dancerCount(); i++) {
						for (int j = 0; j < _dancers.size(); j++) {
							if (subset->dancer(i)->dancerIndex() == _dancers[j]->dancerIndex()) {
								if (_dancers[j]->gender == BOY) {
									if (j == 0)
										output->push_back(_dancers[_dancers.size() - 1]);
									else
										output->push_back(_dancers[j - 1]);
								} else
									output->push_back(_dancers[(j + 1) % _dancers.size()]);
								break;
							}
						}
					}
					break;

				case	BEAUS:
					for (int i = 0; i < _dancers.size(); i++) {
						if (_dancers[i]->gender == BOY)
							output->push_back(_dancers[i]);
					}
					break;

				case	BELLES:
					for (int i = 0; i < _dancers.size(); i++) {
						if (_dancers[i]->gender == GIRL)
							output->push_back(_dancers[i]);
					}
					break;
				}
			}
		}
		return;			// All other rings have no way to resolve 'partner': you can pair any two
						// adjacent people in a ring (because they are by definition in a couple or 
						// mini-wave).  So any given dancer can be the partner of either their left- or
						// right-hand dancer.
	}

	const vector<VariantTile>& forms = context->grammar()->partners();
	TileSearch tiles[MAX_DANCERS];
	unsigned tileMasks[MAX_DANCERS];

	int result = buildTiling(forms, tiles, context, null, null, TILE_ALL);
	if (result <= 0)
		return;			// if we couldn't get a unique tiling of 'partners' patterns, give up
						// Also, if we can't find any partners give up as well.

	const Dancer* fullSet[MAX_DANCERS];
	buildDancerArray(fullSet);

	for (int i = 0; i < result; i++)
		tileMasks[i] = tiles[i].dancers->dancerMask();
	if (which == NONE) {
		for (int i = 0; i < subset->dancerCount(); i++)
			output->push_back(null);
	} else
		subset = this;
	for (int i = 0; i < subset->dancerCount(); i++) {
		const Dancer* d = subset->dancer(i);

		int x0, y0, x1, y1;

		x1 = x0 = d->x;
		y1 = y0 = d->y;
		switch (d->facing) {
		case	RIGHT_FACING:
			if (which == BEAUS) {
				y0 -= 2;
				y1 = INT_MAX;
			} else if (which == BELLES) {
				y0 = INT_MAX;
				y1 += 2;
			} else {
				y0 -= 2;
				y1 += 2;
			}
			break;

		case	LEFT_FACING:
			if (which == BEAUS) {
				y0 = INT_MAX;
				y1 += 2;
			} else if (which == BELLES) {
				y0 -= 2;
				y1 = INT_MAX;
			} else {
				y0 -= 2;
				y1 += 2;
			}
			break;

		case	FRONT_FACING:
			if (which == BEAUS) {
				x0 -= 2;
				x1 = INT_MAX;
			} else if (which == BELLES) {
				x0 = INT_MAX;
				x1 += 2;
			} else {
				x0 -= 2;
				x1 += 2;
			}
			break;

		case	BACK_FACING:
			if (which == BEAUS) {
				x0 = INT_MAX;
				x1 += 2;
			} else if (which == BELLES) {
				x0 -= 2;
				x1 = INT_MAX;
			} else {
				x0 -= 2;
				x1 += 2;
			}
		}

		// now [x0, y0] and [x1, y1] are the possible locations of any partner.

		const Dancer* partner = null;
		bool inMasks = false;
		for (int j = 0; j < result; j++) {
			// If dancer is in this tile, so is their 'partner'
			if (tileMasks[j] & d->dancerMask()) {
				for (int k = 0; k < tiles[j].dancers->dancerCount(); k++) {
					if (tiles[j].dancers->dancer(k)->dancerIndex() == d->dancerIndex()) {
						int p;
						switch (tiles[j].dancers->dancer(k)->facing) {
						case	RIGHT_FACING:
						case	LEFT_FACING:
							p = (k + 2) & 3;
							break;

						default:
							p = k ^ 1;
						}
						partner = fullSet[tiles[j].dancers->dancer(p)->dancerIndex()];
						inMasks = true;
						if (which == BEAUS || which == BELLES) {
							if ((partner->x == x0 && partner->y == y0) ||
								(partner->x == x1 && partner->y == y1))
								partner = fullSet[d->dancerIndex()];
							else
								partner = null;
						}
						break;
					}
				}
				break;
			}
		}
		if (!inMasks) {
			// Okay, we didn't find the subset dancer in the tiles.  Try to find a 'partner' geometrically.
			// We go back to the full set at this point in order to be sure we are looking at a common
			// coordinate system (the subset dancer facing may be locally different than in the full set).
			d = fullSet[d->dancerIndex()];

			for (int j = 0; j < _dancers.size(); j++) {
				d = _dancers[j];

				if ((d->x == x0 && d->y == y0) ||
					(d->x == x1 && d->y == y1)) {
					if (partner == null)
						partner = d;
					else {
						output->clear();			// We already have one match, this means 'partner' is ambiguous - so reject
						return;
					}
				}
			}
		}
		if (which == NONE)
			(*output)[i] = partner;
		else if (partner)
			output->push_back(partner);
	}
}

const Group* Group::circle(const Anydirection* leftRight, int amount, Rotation rotation, Interval* interval, Context* context) const {
	amount &= 7;			// Make it less than one full turn.
	if (amount == 0 && rotation == _rotation)
		return this;
	Group* out = cloneNonDancerData(context);
	out->_rotation = rotation;
	if (leftRight != null && 
		leftRight->direction() == D_RIGHT)
		amount = -amount;		// left circle's are negative.
	if (interval)
		interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* d = _dancers[i];
		if (interval) {
			double motionAngleAdjust = rotationAngle(Rotation(rotation - _rotation));
			interval->forwardVeer(_dancers[i], Point(amount * 2, 0), 0, motionAngleAdjust, 0, abs(amount), context);
		}
		out->_dancers.push_back(new Dancer((d->x + amount * 2) & 0xf, d->y, d->facing, d->gender, d->couple, d->dancerIndex()));
	}
	out->done();
	return out;
}

Group* Group::rotate(Rotation rotation, Context* context) const {
	Group* out = cloneNonDancerData(context);
	out->_rotation = rotation;
	for (int i = 0; i < _dancers.size(); i++)
		out->_dancers.push_back(_dancers[i]->clone());
	return out;
}

Group* Group::veer(int amount, const Anydirection* direction, Interval* interval, Context* context) const {
	Group* out = cloneNonDancerData(context);
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* in = _dancers[i];
		int veer = 0;
		switch (in->facing) {
		case	LEFT_FACING:
			if (in->y > 0) {
				if (direction->direction() == D_IN)
					veer = -amount;
				else // D_OUT
					veer = amount;
			} else {
				if (direction->direction() == D_IN)
					veer = amount;
				else // D_OUT
					veer = -amount;
			}
			break;

		case	RIGHT_FACING:
			if (in->y > 0) {
				if (direction->direction() == D_IN)
					veer = amount;
				else // D_OUT
					veer = -amount;
			} else {
				if (direction->direction() == D_IN)
					veer = -amount;
				else // D_OUT
					veer = amount;
			}
			break;

		case	FRONT_FACING:
			if (in->x > 0) {
				if (direction->direction() == D_IN)
					veer = amount;
				else // D_OUT
					veer = -amount;
			} else {
				if (direction->direction() == D_IN)
					veer = -amount;
				else // D_OUT
					veer = amount;
			}
			break;

		case	BACK_FACING:
			if (in->x > 0) {
				if (direction->direction() == D_IN)
					veer = -amount;
				else // D_OUT
					veer = amount;
			} else {
				if (direction->direction() == D_IN)
					veer = amount;
				else // D_OUT
					veer = -amount;
			}
			break;
		}
		const Dancer* d = in->forwardVeer(0, veer, 0, interval, context);
		out->_dancers.push_back(d);
	}
	out->done();
	return out;
}

Group* Group::displace(int amount, const Anydirection* direction, Interval* interval, Context* context) const {
	Rectangle r;
	boundingBox(&r);
	int w = r.width();
	int h = r.height();

	bool wider;

	if (w < h)
		wider = false;
	else if (w > h)
		wider = true;
	else
		return null;
	Group* out = cloneNonDancerData(context);
	interval->currentDancers(this);
	// Moving out is the negation of moving in, in all cases
	if (direction->direction() == D_OUT)
		amount = -amount;
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* in = _dancers[i];
		int veer = 0;
		int forward = 0;
		if (wider) {
			switch (in->facing) {
			case	LEFT_FACING:
				veer = 0;
				if (in->x < 0)
					forward = -amount;
				else
					forward = amount;
				break;

			case	RIGHT_FACING:
				veer = 0;
				if (in->x < 0)
					forward = amount;
				else
					forward = -amount;
				break;

			case	FRONT_FACING:
				forward = 0;
				if (in->x < 0)
					veer = -amount;
				else
					veer = amount;
				break;

			case	BACK_FACING:
				forward = 0;
				if (in->x < 0)
					veer = amount;
				else
					veer = -amount;
				break;
			}
		} else {
			switch (in->facing) {
			case	LEFT_FACING:
				forward = 0;
				if (in->y < 0)
					veer = amount;
				else
					veer = -amount;
				break;

			case	RIGHT_FACING:
				forward = 0;
				if (in->y < 0)
					veer = -amount;
				else
					veer = amount;
				break;

			case	FRONT_FACING:
				veer = 0;
				if (in->y < 0)
					forward = -amount;
				else
					forward = amount;
				break;

			case	BACK_FACING:
				veer = 0;
				if (in->y < 0)
					forward = amount;
				else
					forward = -amount;
				break;
			}
		}
		const Dancer* d = in->forwardVeer(forward, veer, 0, interval, context);
		out->_dancers.push_back(d);
	}
	out->done();
	return out;
}

Group* Group::forwardVeer(int amount, int veer, int rightQuarterTurns, Interval* interval, Context* context) const {
	Group* out = cloneNonDancerData(context);
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* d = _dancers[i]->forwardVeer(amount, veer, rightQuarterTurns, interval, context);
		out->_dancers.push_back(d);
	}
	out->done();
	return out;
}

Group* Group::arc(const Anypivot* center, const Anydirection* direction, Rotation newRotation, int amount, int noseQuarterTurns, Interval* interval, Context* context) const {
	if (_geometry == RING && center->pivot() == P_CENTER) {
		return forwardVeer(amount, 0, 0, interval, context);
	}
	Group* out = cloneNonDancerData(context);
	if (center->pivot() == P_CENTER && amount % 4 != 0) {
		newRotation = (Rotation)(newRotation + 2 * (amount % 4));
		if (newRotation >= 8)
			newRotation = (Rotation)(newRotation - 8);
	}
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		Point centerPoint;
		double radius;
		int rightSixteenthTurns = amount;
		double angleAdjust = 0;
		_dancers[i]->calculateArcParameters(this, center, direction, &centerPoint, &radius, newRotation, &rightSixteenthTurns, &angleAdjust, interval);
		if (radius < 0)
			return null;
		const Dancer* d = _dancers[i]->arc(_geometry, centerPoint, radius, rightSixteenthTurns, angleAdjust, noseQuarterTurns, interval, context);
		out->_dancers.push_back(d);
	}
	out->_dancers.sort();
	out->_rotation = newRotation;
	return out;
}

Group* Group::face(const Anydirection* dir, const Anypivot* pivot, Interval* interval, Context* context) const {
	static int originalPartners[MAX_DANCERS] = {
		dancerIdx(1, BOY),
		dancerIdx(1, GIRL),
		dancerIdx(2, BOY),
		dancerIdx(2, GIRL),
		dancerIdx(3, BOY),
		dancerIdx(3, GIRL),
		dancerIdx(4, BOY),
		dancerIdx(4, GIRL),
		dancerIdx(5, BOY),
		dancerIdx(5, GIRL),
		dancerIdx(6, BOY),
		dancerIdx(6, GIRL),
	};
	static int originalCorners8[MAX_DANCERS] = {
		dancerIdx(2, BOY),
		dancerIdx(4, GIRL),
		dancerIdx(3, BOY),
		dancerIdx(1, GIRL),
		dancerIdx(4, BOY),
		dancerIdx(2, GIRL),
		dancerIdx(1, BOY),
		dancerIdx(3, GIRL),
	};
	static int originalCorners12[MAX_DANCERS] = {
		dancerIdx(2, BOY),
		dancerIdx(6, GIRL),
		dancerIdx(3, BOY),
		dancerIdx(1, GIRL),
		dancerIdx(4, BOY),
		dancerIdx(2, GIRL),
		dancerIdx(5, BOY),
		dancerIdx(3, GIRL),
		dancerIdx(6, BOY),
		dancerIdx(4, GIRL),
		dancerIdx(5, BOY),
		dancerIdx(6, GIRL),
	};

	Group* out = cloneNonDancerData(context);
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		int amount = 0;
		int index;
		const Dancer* other;
		SpatialRelation sr;

		switch (dir->direction()) {
		case	D_LEFT:
			amount = 1;
			break;

		case	D_RIGHT:
			amount = -1;
			break;

		case	D_IN:
		case	D_OUT: {
			int x;
			int y;
			switch (pivot->pivot()) {
			case	P_CENTER:
				computeCenter(&x, &y);
				break;

			case	P_BOX_CENTER:
				x = 0;
				y = 0;
				break;

			case	P_SPLIT_CENTER:
				x = _dancers[i]->x;
				y = _dancers[i]->y;
				if (!computeSplitCenter(&x, &y))
					return null;
				break;

			default:
				return null;
			}
			amount = _dancers[i]->quarterTurnsTo(dir, x, y);
			break;
		}

		case	D_PROMENADE:
			if (_geometry == RING) {
				switch (_dancers[i]->facing) {
				case	FRONT_FACING:
					amount = -1;
					break;

				case	BACK_FACING:
					amount = 1;
					break;

				case	LEFT_FACING:
					amount = 0;
					break;

				case	RIGHT_FACING:
					amount = -2;
					break;
				}
				break;
			} else {
				const Dancer* dx = _dancers[i];
				if (dx->x == 0) {
					if (dx->y > 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = 1;
							break;

						case	BACK_FACING:
							amount = -1;
							break;

						case	LEFT_FACING:
							amount = 0;
							break;

						case	RIGHT_FACING:
							amount = -2;
							break;
						}
					} else if (dx->y < 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = -1;
							break;

						case	BACK_FACING:
							amount = 1;
							break;

						case	LEFT_FACING:
							amount = -2;
							break;

						case	RIGHT_FACING:
							amount = 0;
							break;
						}
					} else
						return null;
				} else if (dx->y == 0) {
					if (dx->x > 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = -2;
							break;

						case	BACK_FACING:
							amount = 0;
							break;

						case	LEFT_FACING:
							amount = 1;
							break;

						case	RIGHT_FACING:
							amount = -1;
							break;
						}
					} else if (dx->x < 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = 0;
							break;

						case	BACK_FACING:
							amount = -2;
							break;

						case	LEFT_FACING:
							amount = -1;
							break;

						case	RIGHT_FACING:
							amount = 1;
							break;
						}
					} else
						return null;
				} else
					return null;
				break;
			}

		case	D_REVERSE_PROMENADE:
			if (_geometry == RING) {
				switch (_dancers[i]->facing) {
				case	FRONT_FACING:
					amount = 1;
					break;

				case	BACK_FACING:
					amount = -1;
					break;

				case	LEFT_FACING:
					amount = -2;
					break;

				case	RIGHT_FACING:
					amount = 0;
					break;
				}
				break;
			} else {
				const Dancer* dx = _dancers[i];
				if (dx->x == 0) {
					if (dx->y > 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = -1;
							break;

						case	BACK_FACING:
							amount = 1;
							break;

						case	LEFT_FACING:
							amount = -2;
							break;

						case	RIGHT_FACING:
							amount = 0;
							break;
						}
					} else if (dx->y < 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = 1;
							break;

						case	BACK_FACING:
							amount = -1;
							break;

						case	LEFT_FACING:
							amount = 0;
							break;

						case	RIGHT_FACING:
							amount = -2;
							break;
						}
					} else
						return null;
				} else if (dx->y == 0) {
					if (dx->x > 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = 0;
							break;

						case	BACK_FACING:
							amount = -2;
							break;

						case	LEFT_FACING:
							amount = -1;
							break;

						case	RIGHT_FACING:
							amount = 1;
							break;
						}
					} else if (dx->x < 0) {
						switch (dx->facing) {
						case	FRONT_FACING:
							amount = -2;
							break;

						case	BACK_FACING:
							amount = 0;
							break;

						case	LEFT_FACING:
							amount = 1;
							break;

						case	RIGHT_FACING:
							amount = -1;
							break;
						}
					} else
						return null;
				} else
					return null;
				break;
			}

		case	D_PARTNER: {
			vector<const Dancer*> partners;
			partnershipOp(NONE, extract(_dancers[i]->dancerMask(), context), context, &partners);
			if (partners.size() == 0 || partners[0] == null)
				return null;							// this dancer has no partner
			amount = _dancers[i]->leftTurnsNeededToFace(_geometry, partners[0]);
			break;
		}
		case	D_AWAY_FROM_PARTNER: {
			vector<const Dancer*> partners;
			partnershipOp(NONE, extract(_dancers[i]->dancerMask(), context), context, &partners);
			if (partners.size() == 0 || partners[0] == null)
				return null;							// this dancer has no partner
			amount = _dancers[i]->leftTurnsNeededToFace(_geometry, partners[0]);
			switch (amount) {
			case	-1:
				amount = 1;
				break;

			case	1:
				amount = -1;
				break;

			case	0:
				amount = 2;
				break;

			case	2:
				amount = 0;
				break;
			}
			break;
		}


		case	D_ORIGINAL_PARTNER:
			index = originalPartners[_dancers[i]->dancerIndex()];
			other = dancerByIndex(index);
			if (other == null)
				return null;
			amount = _dancers[i]->leftTurnsNeededToFace(_geometry, other);
			break;

		case	D_CORNER:
			if (_geometry == RING) {
				switch (_dancers[i]->facing) {
				case	FRONT_FACING:
					if (_dancers[i]->gender == BOY)
						amount = 1;
					else
						amount = -1;
					break;

				case	BACK_FACING:
					if (_dancers[i]->gender == BOY)
						amount = -1;
					else
						amount = 1;
					break;

				case	LEFT_FACING:
					if (_dancers[i]->gender == BOY)
						amount = 2;
					else
						amount = 0;
					break;

				case	RIGHT_FACING:
					if (_dancers[i]->gender == BOY)
						amount = 0;
					else
						amount = 2;
					break;
				}
				break;
			}

		case	D_ORIGINAL_CORNER:
			index = originalCorners8[_dancers[i]->dancerIndex()];
			other = dancerByIndex(index);
			if (other == null)
				return null;
			amount = _dancers[i]->leftTurnsNeededToFace(_geometry, other);
			break;

		case	D_LAST: {
			Motion* m = interval->lastCurve(_dancers[i], false);
			if (m == null)
				return null;
			double d = m->cumulativeNoseMotion();
			convertFromAbsolute(&d);
			if (d == 0) {
				if (verboseOutput) {
					printf("lastCurve was 0:\n");
					interval->print(4);
				}
				return null;
			}
			if (d > 0)
				amount = 1;		// Positive angle is a left turn
			else
				amount = -1;
			break;
		}
		default:
			return null;
		}
		if (amount == 0)
			out->insert(_dancers[i]->clone());
		else if (amount < 2)
			out->insert(_dancers[i]->face(amount, interval, context));
		else
			return null;
	}
	return out;
}

const Group* Group::backOut(PrimitiveStep* s, const Anyone* movers, const Group* start, Context* context, Interval* interval) const {
	if (_geometry != start->_geometry ||
		_rotation != start->_rotation)
		return this;
	unsigned mask = movers->match(start, s, context);
	if (mask == 0)
		return this;
	const Group* activesStart = start->extract(mask, context);
	const Group* activesNow = extract(movers->match(this, s, context), context);
	Group* inactivesStart = context->stage()->newGroup(start);
	inactivesStart->subtraction(start, activesStart->dancerMask());
	Group* inactivesNow = context->stage()->newGroup(this);
	inactivesNow->subtraction(this, activesNow->dancerMask());
	// If we have no inactives now, it is because
	if (inactivesNow->dancerCount() > 0 && !inactivesNow->equals(inactivesStart))
		return this;
	Facing facing = activesStart->_dancers[0]->facing;
	bool startedFacingHeads;
	if (facing == BACK_FACING ||
		facing == FRONT_FACING)
		startedFacingHeads = true;
	else
		startedFacingHeads = false;
	for (int i = 0; i < activesNow->_dancers.size(); i++) {
		const Dancer* d = activesNow->_dancers[i];
		if (startedFacingHeads) {
			if (d->facing != BACK_FACING &&
				d->facing != FRONT_FACING)
				return this;
		} else {
			if (d->facing != LEFT_FACING &&
				d->facing != RIGHT_FACING)
				return this;
		}
		if (d->x != -1 && d->x != 1)
			return this;
		if (d->y != -1 && d->y != 1)
			return this;
	}
	// The movers must move out of the center.
	Group* output = context->stage()->newGroup(this);

	interval->currentDancers(activesNow);
	for (int i = 0; i < activesNow->_dancers.size(); i++) {
		const Dancer* d = activesNow->_dancers[i];

		if (startedFacingHeads) {
			if (d->facing == FRONT_FACING) {
				if (d->y < 0)
					output->insert(d->forwardVeer(2, 0, 0, interval, context));
				else
					output->insert(d->forwardVeer(-2, 0, 0, interval, context));
			} else {
				if (d->y < 0)
					output->insert(d->forwardVeer(-2, 0, 0, interval, context));
				else
					output->insert(d->forwardVeer(2, 0, 0, interval, context));
			}
		} else {
			if (d->facing == RIGHT_FACING) {
				if (d->x < 0)
					output->insert(d->forwardVeer(-2, 0, 0, interval, context));
				else
					output->insert(d->forwardVeer(2, 0, 0, interval, context));
			} else {
				if (d->x < 0)
					output->insert(d->forwardVeer(2, 0, 0, interval, context));
				else
					output->insert(d->forwardVeer(-2, 0, 0, interval, context));
			}
		}
	}
	output->done();
	return output->merge(context);
}

const Group* Group::roll(PrimitiveStep* s, Context* context, bool failOnCantRoll) const {
	Group* output = context->stage()->newGroup(this);
	s->interval()->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* d = _dancers[i];
		Motion* m = lastCurve(d, s->interval(), context);
		if (m) {
			double n = m->cumulativeNoseMotion();
			convertFromAbsolute(&n);
			if (n < 0) {
				output->insert(d->face(-1, s->interval(), context));
				continue;
			} else if (n > 0) {
				output->insert(d->face(1, s->interval(), context));
				continue;
			} else if (failOnCantRoll) {
				s->fail(context->stage()->newExplanation(USER_ERROR, "Can't roll"));
				return null;
			}
		} else if (failOnCantRoll) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Can't roll"));
			return null;
		}
		output->insert(d->clone());
	}
	// no need to call done when dancers are only changing facing directions
	return output->merge(context);
}

bool Group::closerToCenter(const Group* other) const {
	if (_dancers.size() != 1 ||
		other->_dancers.size() != 1)
		return false;
	float x = 0, y = 0;
	convertFromAbsolute(&x, &y);
	const Dancer* d1 = _dancers[0];
	const Dancer* d2 = other->_dancers[0];
	double dx, dy;

	dx = d1->x - x;
	dy = d1->y - y;
	double distThis = sqrt(dx * dx + dy * dy);
	dx = d2->x - x;
	dy = d2->y - y;
	double distOther = sqrt(dx * dx + dy * dy);
	return distThis < distOther;
}

bool Group::hasLateralFlow(const Anydirection* direction, const Interval* interval, Context* context) const {
	if (_dancers.size() == 0)
		return false;
	const Dancer* d = _dancers[0];
	Motion* m = lastCurve(d, interval, context);
	if (m == null)
		return false;
	double nose = m->cumulativeNoseMotion();
	if (nose == 0)
		return false;
	if (m->cumulativeRadius() < EPSILON)
		return false;
	bool negative = nose < 0;					// negative nose motion is clockwise movement
	switch (direction->direction()) {
	case	D_LEFT:
		if (!negative)
			return false;
		break;

	case	D_RIGHT:
		if (negative)
			return false;
		break;

	case	D_AS_YOU_ARE:
		break;

	default:
		return false;
	}
	for (int i = 0; i < _dancers.size(); i++) {
		d = _dancers[i];
		m = lastCurve(d, interval, context);
		if (m == null)
			return false;
		bool iNeg = m->cumulativeNoseMotion() < 0;
		if (negative != iNeg)
			return false;
	}
	return true;
}

void Group::constructReduce(PrimitiveStep* s, Context* context) const {
	const Word* form0 = (const Word*)s->parent()->variables()[0];
	const Word* form1 = (const Word*)s->parent()->variables()[1];
	const Formation* f0 = context->grammar()->formation(form0->spelling());
	if (f0 == null) {
		s->fail(context->stage()->newExplanation(DEFINITION_ERROR, "No form named '" + form0->spelling() + "'"));
		return;
	}
	const Formation* f1 = context->grammar()->formation(form1->spelling());
	if (f1 == null) {
		s->fail(context->stage()->newExplanation(DEFINITION_ERROR, "No form named '" + form1->spelling() + "'"));
		return;
	}
	vector<VariantTile> forms;
	Pattern p0(f0, null);
	forms.push_back(VariantTile(null, &p0));
	Pattern p1(f1, null);
	if (f1 != f0)
		forms.push_back(VariantTile(null, &p1));
	TileSearch tiles[MAX_DANCERS];
	int result = buildTiling(forms, tiles, context, null, null, TILE_ALL);
	if (result < 0) {
		s->fail(context->stage()->newExplanation(USER_ERROR, "No unique grouping of dancers"));
		return;
	}
	int dancerCount = 0;
	for (int i = 0; i < result; i++)
		dancerCount += tiles[i].dancers->dancerCount();
	if (dancerCount != _dancers.size()) {
		s->fail(context->stage()->newExplanation(USER_ERROR, "Not all dancers could be grouped"));
		return;
	}
	Tile* seq = s->newTile(this, s->parent(), context);
	Plan* seqPlan = seq->plan();
	seqPlan->makeInterval(s->interval(), false);
	Step* s1 = seqPlan->constructRawStep(context);
	s1->setSpecialLabel("Abstract formation");
	Group* abstracted = cloneCoordinateSystem(context);
	for (int i = 0; i < result; i++) {
		Rectangle bb;
		const Group* pair = tiles[i].dancers->toCommonCoordinates(this, context);
		tiles[i].dancers = pair;
		pair->boundingBox(&bb);
		int x = (bb.left + bb.right) / 2;
		int y = (bb.top + bb.bottom) / 2;
		const Dancer* d = new Dancer(x, y, pair->dancer(0)->facing, pair->gender(), pair->couple(), i);
		abstracted->insert(d);
	}
	abstracted->done();

	s1->startWith(abstracted);
	s1->makeInterval(null, false);
	for (int i = 0; i < result; i++) {
		const Group* d = abstracted->extract(1 << i, context);
		s1->constructTile(d, d, context);
	}
	s1->constructOutcomes();
	const Group* compacted = s1->combine(true, null, context);
	if (s1->failed())
		return;

	Step* s2 = seqPlan->newStep((const Anything*)s->parent()->variables()[2], context);
	s2->startWith(compacted);
	s2->construct(context, TILE_ALL);
	Step* s3 = seqPlan->constructRawStep(context);
	s3->setSpecialLabel("Breathing final position");
	for (int i = 0; i < result; i++)
		s3->addOutcome(tiles[i].dancers);
	if (s2->failed())
		return;
}

unsigned Group::startingDancersMaskReduce(const PrimitiveStep* s) const {
	if (s->tiles().size() == 0)
		return 0;
	Tile* seq = s->tiles()[0];
	Plan* seqPlan = seq->plan();
	Step* s2 = seqPlan->step(1);
	unsigned innerMask = s2->startingDancersMask();

	Step* s3 = seqPlan->step(2);
	unsigned outerMask = 0;
	for (int i = 0; i < s3->tiles().size(); i++) {
		if ((1 << i) & innerMask) {
			Tile* t = s3->tiles()[i];

			outerMask |= t->plan()->start()->dancerMask();
		}
	}
	return outerMask;
}

void Group::trimStartReduce(const PrimitiveStep* s, unsigned mask, Context* context) const {
	if (s->tiles().size() == 0)
		return;
	Tile* seq = s->tiles()[0];
	Plan* seqPlan = seq->plan();
	Step* s2 = seqPlan->step(1);
	Step* s3 = seqPlan->step(2);
	unsigned innerMask = 0;
	for (int i = 0; i < s3->tiles().size(); i++) {
		if (mask & s3->tiles()[i]->plan()->start()->dancerMask())
			innerMask |= (1 << i);
	}
	s2->trimStart(s2->start(), s2->start(), innerMask, context);
}

const Group* Group::reduce(PrimitiveStep* s, Context* context) const {
	if (s->tiles().size() == 0) {
		constructReduce(s, context);
		if (s->failed())
			return null;
	}
	Tile* seq = s->tiles()[0];
	Plan* seqPlan = seq->plan();
	Step* s1 = seqPlan->step(0);
	const Group* abstracted = s1->start();
	Step* s2 = seqPlan->step(1);
	Step* s3 = seqPlan->step(2);

	if (verboseOutput) {
		printf("About to perform:\n");
		s2->print(0, false);
	}
	s2->perform(context);
	if (s2->failed())
		return null;
	s2->setBreatheAction(BREATHE);
	s2->combine(false, null, context);
	if (s2->failed())
		return null;
	if (verboseOutput) {
		printf("Abstracted group (start):\n");
		abstracted->printDetails(4, true);
		printf("breathed group (start):\n");
		s2->start()->printDetails(4, true);
		printf("Reduced group motions:\n");
		s2->interval()->print(4);
		printf("Abstracted group (final):\n");
		s2->final()->printDetails(4, true);
	}
	s2->collectMotions(s1->interval(), context);
	Interval* nested = new Interval(null, false);
	s1->collectMotions(nested, context);

	if (verboseOutput) {
		printf("nested interval:\n");
		nested->print(4);
	}
	Interval* ss = new Interval(null, false);

	for (int i = 0; i < abstracted->dancerCount(); i++) {
		const Dancer* d = abstracted->dancer(i);
		Motion* m = nested->motion(d->dancerIndex());
		if (m) {
			const Group* pair = s3->outcome(d->dancerIndex());
			pair->splitMotion(d->x, d->y, m, ss, context);
		}
	}
	delete nested;

	if (verboseOutput) {
		printf("ss interval:\n");
		ss->print(4);
	}

	s3->resolveReduction(abstracted, s2->final(), s2->lastActiveDancersMask(), ss, context);
	delete ss;
	s->setLastActiveMask(s3->lastActiveMask());
	if (verboseOutput) {
		printf("s3 interval:\n");
		if (s3->interval())
			s3->interval()->print(4);
		else
			printf("    <null>\n");
	}
	return s3->final();
}

const Group* Group::stretch(PrimitiveStep *s, Context *context) const {
	Rectangle r;
	boundingBox(&r);
	const Group* d;
	if (_geometry == RING || r.width() == r.height()) {
		s->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Cannot determine a stretch direction"));
		return null;
	}
	// Make sure the x axis is the major axis.
	if (r.width() < r.height())
		d = this->apply(&Transform::rotate90, context);
	else
		d = this;
	d->boundingBox(&r);
	int centerLine = (r.left + r.right) / 2;
	int leftCenter = (r.left + centerLine) / 2;
	int rightCenter = (r.right + centerLine) / 2;
	Group* out = d->cloneNonDancerData(context);
	s->interval()->currentDancers(d);
	for (int i = 0; i < d->_dancers.size(); i++) {
		const Dancer* dancer = d->_dancers[i];
		if (dancer->x == centerLine) {
			s->fail(context->stage()->newExplanation(USER_ERROR, "Cannot stretch this call: dancers finish on centerline"));
			return null;
		}
		if (dancer->x <= leftCenter || dancer->x >= rightCenter)
			out->insert(dancer->clone());
		else if (dancer->x < centerLine)
			out->insert(dancer->displace(centerLine - leftCenter, 0, s->interval(), context));
		else
			out->insert(dancer->displace(centerLine - rightCenter, 0, s->interval(), context));
	}
	out->done();
	return out;
}

const Group* Group::removePhantoms(Context* context) const {
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* dancer = _dancers[i];
		if (dancer->isPhantom()) {
			Group* out = cloneNonDancerData(context);
			for (int i = 0; i < _dancers.size(); i++) {
				const Dancer* dancer = _dancers[i];
				if (!dancer->isPhantom())
					out->insert(dancer->clone());
			}
			out->done();
			return out;
		}
	}
	return this;
}

bool Group::hasAmbiguousFacing() const {
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* dancer = _dancers[i];
		if (ambiguous(dancer->facing))
			return true;
	}
	return false;
}

bool Group::isSymmetric(const Sequence* sequence) const {
	// Odd number of dancers mean not symmetric
	if ((_dancers.size() & 1) != 0)
		return false;
	for (int i = 0; i < _dancers.size() / 2; i++) {
		const Dancer* dancerA = _dancers[i];
		const Dancer* dancerB = _dancers[_dancers.size() - i - 1];
		if (!dancerA->opposite(sequence, dancerB))
			return false;
	}
	return true;
}

const Group* Group::disambiguateFromRoot(Context* context) const {
	Group* g = cloneNonDancerData(context);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* dancer = _dancers[i];
		if (ambiguous(dancer->facing)) {
			const Dancer* d = dancerByLocation(dancer->x, dancer->y, true);
			if (d == null || ambiguous(d->facing))
				return null;
			g->insert(new Dancer(dancer->x, dancer->y, d->facing, dancer->gender, dancer->couple, dancer->dancerIndex()));
		} else
			g->insert(dancer->clone());
	}
	g->done();
	return g;
}

const Group* Group::root() const {
	if (_base != null)
		return _base->root();
	else
		return this;
}

void Group::splitMotion(int pairX, int pairY, Motion* m, Interval* interval, Context* context) const {
	if (verboseOutput) {
		printf("pair=[%d,%d]\n", pairX, pairY);
		printDetails(4, true);
	}
	for (int i = 0; i < _dancers.size(); i++) {
		int forward, lateral;
		Point start(_dancers[i]->x, _dancers[i]->y);
		Facing facing = _dancers[i]->facing;

		convertToAbsolute(&start.x, &start.y, null, null);
		_dancers[i]->displace(_dancers[i]->x - pairX, _dancers[i]->y - pairY, &forward, &lateral);
		if (verboseOutput) {
			printf("dancer=[%d,%d]\n", _dancers[i]->x, _dancers[i]->y);
			printf("Splitting for:  forward=%d lateral=%d\n", forward, lateral);
			_dancers[i]->print(4);
		}
		Motion* md = m->split(this, start, &facing, forward, lateral, context);
		interval->remember(_dancers[i]->dancerIndex(), md);
	}
}

Group* Group::displace(int deltaX, int deltaY, Interval* interval, Context* context) const {
	Group* out = cloneNonDancerData(context);
	interval->currentDancers(this);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* d = _dancers[i]->displace(deltaX, deltaY, interval, context);
		out->insert(d);
	}
	return out;
}

const Group* Group::merge(Context* context) const {
	if (_base == null)
		return this;
	if (_base->_rotation != _rotation)
		return null;
	if (_base->_geometry != _geometry)
		return null;
	if (_dancers.size() == 0)
		return _base;
	Group* out = _base->cloneNonDancerData(context);
	for (int i = 0; i < _base->_dancers.size(); i++) {
		const Dancer* dancer = _base->_dancers[i];
		for (int j = 0; j < _dancers.size(); j++) {
			const Dancer* subsetDancer = _dancers[j];
			if (dancer->dancerIndex() == subsetDancer->dancerIndex()) {
				dancer = subsetDancer;
				break;
			}
		}
		const Dancer* baseD;
		if (_transform)
			baseD = _transform->revert(dancer);
		else
			baseD = dancer->clone();
		out->_dancers.push_back(baseD);
	}
	out->done();
	return out;
}

const Group* Group::toCommonCoordinates(const Group* start, Context* context) const {
	if (start && _base == start && _geometry == RING && start->_geometry != RING) {
		Group* g = cloneNonDancerData(context);
		g->_base = start->_base;
		g->_transform->dispose();
		g->_transform = null;
		for (int i = 0; i < _dancers.size(); i++)
			g->insert(_dancers[i]->clone());
		if (verboseOutput) {
			printf("toCommonCoordinates:\n");
			printf("this:\n");
			printDetails(4, true);
			printf("start:\n");
			start->printDetails(4, true);
			printf("g:\n");
			g->printDetails(4, true);
		}
		return g;
	}
	const Group* d = this;
	const Group* baseCoordinates;
	if (start != null)
		baseCoordinates = start->_base;
	else
		baseCoordinates = null;
	while (d->_base != baseCoordinates) {
		if (d->_base == null)
			return null;
		d = d->unwind(context);
		if (d == null)
			return null;
	}
	return d;
}

bool Group::sameCoordinateSpace(const Group* other) const {
	if (_geometry != other->_geometry)
		return false;
	return _rotation == other->_rotation;
}

bool Group::basedOn(const Group* base) const {
	for (const Group* d = this; d != null; d = d->_base)
		if (d == base)
			return true;
	return false;
}

const Group* Group::addTransforms(const Group* priorStart, const Group* newStart, Context* context) const {
	if (priorStart == this)
		return newStart;
	if (priorStart->base() == null)
		return newStart;
	const Group* g = addTransforms(priorStart->base(), newStart, context);
	if (priorStart->_transform != null)
		return g->apply(priorStart->_transform, context);
	else
		return g;
}

Group* Group::unwind(Context* context) const {
	if (_base->_rotation != _rotation)
		return null;
	if (_base->_geometry != _geometry)
		return null;
	Group* out = _base->cloneNonDancerData(context);
	for (int i = 0; i < _dancers.size(); i++) {
		const Dancer* d = _dancers[i];
		const Dancer* baseD;
		if (_transform)
			baseD = _transform->revert(d);
		else
			baseD = d->clone();
		out->_dancers.push_back(baseD);
	}
	out->_dancers.sort();
	return out;
}

const Group* Group::combine(vector<const Group*>& groups, Context* context) const {
	Group* output = context->stage()->newGroup(this);

	for (int i = 0; i < groups.size(); i++) {
		const Group* d = groups[i];
		if (d->geometry() != output->geometry()) {
			if (verboseOutput)
				printf("Conflicting merge\n");
			return null;
		}
		for (int j = 0; j < d->dancerCount(); j++)
			output->insert(d->dancer(j)->clone());
	}
	return output->merge(context);
}

const Group* Group::combineSubGroups(vector<const Group*>& groups, Context* context) const {
	Group* output = context->stage()->newGroup(this);

	for (int i = 0; i < groups.size(); i++) {
		const Group* d = groups[i];
		if (d->geometry() != output->geometry()) {
			if (verboseOutput)
				printf("Conflicting merge\n");
			return null;
		}
		for (int j = 0; j < d->dancerCount(); j++)
			output->insert(d->dancer(j)->clone());
	}
	output->done();
	return output;
}

Group* Group::clone(Context* context) const {
	Group* d = cloneNonDancerData(context);
	for (int i = 0; i < _dancers.size(); i++)
		d->_dancers.push_back(_dancers[i]->clone());
	return d;
}

Group* Group::rotateDancers(int by, Context* context) const {
	Group* d = cloneNonDancerData(context);
	for (int i = 0; i < _dancers.size(); i++) {
		d->insert(i < by ? _dancers[i]->cloneOffsetX(16) : _dancers[i]->clone());
	}
	d->done();
	return d;
}

Group* Group::cloneNonDancerData(Context* context) const {
	Group* d = context->stage()->newGroup(_homeGeometry);
	d->_rotation = _rotation;
	d->_base = _base;
	d->_geometry = _geometry;
	d->_transform = _transform->clone();
	d->_tiled = _tiled;
	return d;
}

Group* Group::cloneCoordinateSystem(Context* context) const {
	Group* d = context->stage()->newGroup(_homeGeometry);
	d->_rotation = _rotation;
	if (_base)
		d->_base = _base->cloneCoordinateSystem(context);
	else
		d->_base = null;
	d->_geometry = _geometry;
	d->_transform = _transform->clone();
	return d;
}

Group* Group::minus(const Group* y, Context* context) const {
	const Dancer* map[MAX_DANCERS];

	buildDancerArray(map);
	for (int i = 0; i < y->dancerCount(); i++)
		map[y->dancer(i)->dancerIndex()] = null;
	Group* d = context->stage()->newGroup(this);
	for (int i = 0; i < MAX_DANCERS; i++)
		if (map[i])
			d->_dancers.push_back(map[i]->clone());
	d->_dancers.sort();
	return d;
}

unsigned Group::dancerMask() const {
	unsigned mask = 0;

	for (int i = 0; i < _dancers.size(); i++)
		mask |= _dancers[i]->dancerMask();
	return mask;
}

Group* Group::extract(unsigned mask, Context* context) const {
	timing::Timer t("Group::extract");
	Group* d = context->stage()->newGroup(this);
	for (int i = 0; i < _dancers.size(); i++) {
		if (mask & _dancers[i]->dancerMask())
			d->_dancers.push_back(_dancers[i]->clone());
	}
	return d;
}

Group* Group::extractIn(const Group* coordinateSystem, unsigned mask, Context* context) const {
	Group* d = context->stage()->newGroup(coordinateSystem);
	for (int i = 0; i < _dancers.size(); i++) {
		if (mask & _dancers[i]->dancerMask())
			d->_dancers.push_back(_dancers[i]->cloneToCoordinates(this, coordinateSystem));
	}
	return d;
}

Group* Group::trim(unsigned mask, Context* context) const {
	Group* d = context->stage()->newGroup(this);
	for (int i = 0; i < _dancers.size(); i++) {
		if (mask & _dancers[i]->dancerMask())
			d->_dancers.push_back(_dancers[i]->clone());
	}
	return d;
}

Group* Group::normalizeRingCoordinates(Context* context) const {
	// The dancers are not in a normalized ring, normalize it and then match again
	Group* temp = cloneNonDancerData(context);
	for (int j = 0; j < _dancers.size(); j++) {
		const Dancer* dj = _dancers[j];
		int xNormal = dj->x;
		while (xNormal < 0)
			xNormal += 16;
		while (xNormal > 15)
			xNormal -= 16;
		temp->insert(new Dancer(xNormal, dj->y, dj->facing, dj->gender, dj->couple, dj->dancerIndex()));
	}
	temp->done();
	return temp;
}

bool Group::inSequence(const Anydirection* direction) const {
	bool forward = direction->direction() == D_FORWARD;
	const Dancer* boy1 = dancerByIndex(dancerIdx(1, BOY));
	const Transform* rotator;

	if (_geometry == RING) {
		if (boy1->y != 3)
			return false;
		if (forward) {
			if (boy1->facing != LEFT_FACING)
				return false;
		} else {
			if (boy1->facing != RIGHT_FACING)
				return false;
		}
		for (int i = 2; i <= 4; i++) {
			const Dancer* boyN = dancerByIndex(dancerIdx(i, BOY));
			if (boyN->y != 3)
				return false;
			if (forward) {
				if (boyN->facing != LEFT_FACING)
					return false;
			} else {
				if (boyN->facing != RIGHT_FACING)
					return false;
			}
			int deltaX = boyN->x - boy1->x;
			if (deltaX < 0)
				deltaX += 16;
			if (deltaX != 12)
				return false;
			boy1 = boyN;
		}
	} else {
		rotator = &Transform::rotate90;
		int boyDist;
		if (forward)
			boyDist = 2;
		else
			boyDist = 4;
		if (boy1->x == 0) {
			if (abs(boy1->y) != boyDist)
				return false;
		} else if (boy1->y == 0) {
			if (abs(boy1->x) != boyDist)
				return false;
		} else
			return false;
		for (int i = 2; i <= 4; i++) {
			int x = boy1->x;
			int y = boy1->y;
			const Dancer* boyN = dancerByIndex(dancerIdx(i, BOY));
			rotator->apply(&x, &y, null);
			if (boyN->x != x ||
				boyN->y != y)
				return false;
			boy1 = boyN;
		}
	}
	return true;
}

bool Group::atHome() const {
	return equals(home);
}

void Group::computeCenter(int* x, int* y) const {
	float fx = 0;
	float fy = 0;
	this->convertFromAbsolute(&fx, &fy);
	*x = fx;
	*y = fy;
}

bool Group::computeSplitCenter(int* x, int* y) const {
	Rectangle r;
	boundingBox(&r);

	int width = r.width();
	int height = r.height();

	if (width == height)
		return false;
	if (width > height) {
		*y = 0;
		if (*x == 0)
			return false;
		if (*x < 0)
			*x = r.left / 2;
		else
			*x = r.right / 2;
	} else {
		*x = 0;
		if (*y == 0)
			return false;
		if (*y < 0)
			*y = r.bottom / 2;
		else
			*y = r.top / 2;
	}
	return true;
}

void Group::include(const Group* input, unsigned mask) {
	for (int i = 0; i < input->_dancers.size(); i++)
		if (mask & input->_dancers[i]->dancerMask())
			_dancers.push_back(input->_dancers[i]->clone());
}

void Group::insert(const Dancer* dancer) {
	_dancers.push_back(dancer);
}

void Group::intersection(const Group* x, unsigned mask) {
	for (int i = 0; i < x->_dancers.size(); i++)
		if (mask & x->_dancers[i]->dancerMask())
			_dancers.push_back(x->_dancers[i]->clone());
}

void Group::subtraction(const Group* x, unsigned mask) {
	for (int i = 0; i < x->_dancers.size(); i++)
		if ((mask & x->_dancers[i]->dancerMask()) == 0)
			_dancers.push_back(x->_dancers[i]->clone());
}

void Group::done() {
	_dancers.sort();
}

void Group::clear() {
	_dancers.clear();
}

void Group::buildDancerArray(const Dancer** output) const {
	memset(output, 0, sizeof (const Dancer*) * MAX_DANCERS);
	for (int i = 0; i < _dancers.size(); i++)
		output[_dancers[i]->dancerIndex()] = _dancers[i];
}

void Group::print(int indent) const {
	printDetails(indent, false);
}

void Group::printDetails(int indent, bool includeDetails) const {
	int min_x = INT_MAX;

	for (int i = 0; i < _dancers.size(); i++)
		if (_dancers[i]->x < min_x)
			min_x = _dancers[i]->x;
	int y;
	if (_dancers.size())
		y = _dancers[0]->y;
	int x = min_x;
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	if (includeDetails)
		printf("(%p)", this);
	switch (_rotation) {
	case	UNROTATED:
		printf("\n");
		if (indent)
			printf("%*.*c", indent, indent, ' ');
		break;

	default:
		printf("  Rotated %f degrees (%d)\n", _rotation * 11.25, _rotation);
		if (indent)
			printf("%*.*c", indent, indent, ' ');
		break;
	}
	for (int dancer = 0; dancer < _dancers.size(); ) {
		if (y < _dancers[dancer]->y) {
			printf("\n");
			if (indent)
				printf("%*.*c", indent, indent, ' ');
			printf("*** Y order: %d vs. %d ***\n", y, _dancers[dancer]->y);
			if (indent)
				printf("%*.*c", indent, indent, ' ');
			x = min_x;
			y = _dancers[dancer]->y;
		} else {
			while (y > _dancers[dancer]->y) {
				printf("\n");
				if (indent)
					printf("%*.*c", indent, indent, ' ');
				y--;
				x = min_x;
			}
		}
		while (x < _dancers[dancer]->x) {
			printf(".   ");
			if (includeDetails)
				printf("       ");
			x++;
		}
		if (_dancers[dancer]->gender == BOY)
			printf("B");
		else if (_dancers[dancer]->gender == GIRL)
			printf("G");
		else							// UNSPECIFIED
			printf("X");
		if (_dancers[dancer]->couple)
			printf("%d%c", _dancers[dancer]->couple, facingGlyphs[_dancers[dancer]->facing]);
		else
			printf("%c ", facingGlyphs[_dancers[dancer]->facing]);
		if (includeDetails)
			printf("(%d)[%2d,%2d]", _dancers[dancer]->dancerIndex(), _dancers[dancer]->x, _dancers[dancer]->y);
		printf(" ");
		x++;
		dancer++;
	}
	if (_geometry == RING)
		printf(" in ring");
	printf("\n\n");
	if (includeDetails && _transform)
		_transform->print(indent + 4);
	if (includeDetails && _base) {
		if (indent)
			printf("%*.*c", indent, indent, ' ');
		printf("Based on (%p):\n", _base);
		_base->printDetails(indent + 4, true);
	}
}

int Group::compare(const Group* other) const {
	for (int i = 0; i < _dancers.size(); i++) {
		if (i < other->_dancers.size()) {
			int r = _dancers[i]->compare(other->_dancers[i]);
			if (r != 0)
				return r;
		} else
			return 1;
	}
	if (_dancers.size() < other->_dancers.size())
		return -1;
	else
		return 0;
}

void Group::boundingBox(Rectangle* box) const {
	if (_geometry == RING) {
		box->top = 4;
		box->left = -4;
		box->right = 4;
		box->bottom = -4;
		return;
	}
	box->top = INT_MIN;
	box->bottom = INT_MAX;
	box->left = INT_MAX;
	box->right = INT_MIN;
	for (int i = 0; i < _dancers.size(); i++) {
		if (_dancers[i]->x <= box->left)
			box->left = _dancers[i]->x - 1;
		if (_dancers[i]->x >= box->right)
			box->right = _dancers[i]->x + 1;
		if (_dancers[i]->y <= box->bottom)
			box->bottom = _dancers[i]->y - 1;
		if (_dancers[i]->y >= box->top)
			box->top = _dancers[i]->y + 1;
	}
}

Gender Group::gender() const {
	if (_dancers.size() == 0)
		return UNSPECIFIED_GENDER;
	Gender g = _dancers[0]->gender;
	for (int i = 1; i < _dancers.size(); i++)
		if (_dancers[i]->gender != g)
			return UNSPECIFIED_GENDER;
	return g;
}

int Group::couple() const {
	if (_dancers.size() == 0)
		return 0;
	int couple = _dancers[0]->couple;
	for (int i = 1; i < _dancers.size(); i++)
		if (_dancers[i]->couple != couple) {
			if (heads[D_4COUPLE] & _dancers[i]->dancerMask()) {
				if (heads[D_4COUPLE] & dancerMsk(couple, BOY))
					couple = 7;
				else
					return 0;
			} else if (sides[D_4COUPLE] & _dancers[i]->dancerMask()) {
				if (sides[D_4COUPLE] & dancerMsk(couple, BOY))
					couple = 8;
				else
					return 0;
			} else
				return 0;		// Not sure how we could have a couple neither head nor side
		}
	return couple;
}

Group* Group::findTile(const vector<Rectangle>& boundingBoxes, Context* context) const {
	Group* t = context->stage()->newGroup(this);
	if (_dancers.size() != 0) {
		const Dancer* d = _dancers[0];
		t->insert(d->clone());
		for (int i = 1; i < _dancers.size(); i++) {
			if (_dancers[i]->adjacentY(d)) {
				if (isBetween(boundingBoxes, d, _dancers[i]))
					continue;
				t->insert(_dancers[i]->clone());
				i++;
				for (; i < _dancers.size(); i++) {
					if (_dancers[i]->adjacentY(d)) {
							if (isBetween(boundingBoxes, d, _dancers[i]))
								continue;
							t->insert(_dancers[i]->clone());
					}
				}
				break;
			}
			if (_dancers[i]->adjacentX(d)) {
				if (isBetween(boundingBoxes, d, _dancers[i]))
					continue;
				t->insert(_dancers[i]->clone());
				i++;
				for (; i < _dancers.size(); i++) {
					if (_dancers[i]->adjacentX(d)) {
							if (isBetween(boundingBoxes, d, _dancers[i]))
								continue;
							t->insert(_dancers[i]->clone());
					}
				}
				break;
			}
		}
		t->done();
	}
	return t;
}

bool Group::singleTile(const vector<Rectangle>& boundingBoxes) const {
	if (_dancers.size() == 1)
		return true;
	const Dancer* d = _dancers[0];
	if (_dancers[1]->adjacentY(d)) {
		if (isBetween(boundingBoxes, d, _dancers[1]))
			return false;
		for (int i = 2; i < _dancers.size(); i++) {
			if (_dancers[i]->adjacentY(d)) {
				if (isBetween(boundingBoxes, d, _dancers[i]))
					return false;
			} else
				return false;
		}
	} else if (_dancers[1]->adjacentX(d)) {
		if (isBetween(boundingBoxes, d, _dancers[1]))
			return false;
		for (int i = 2; i < _dancers.size(); i++) {
			if (_dancers[i]->adjacentX(d)) {
				if (isBetween(boundingBoxes, d, _dancers[i]))
					return false;
			} else
				return false;
		}
	} else
		return false;
	return true;
}

int Group::realDancerCount() const {
	int realCount = 0;
	for (int i = 0; i < _dancers.size(); i++) {
		if (!_dancers[i]->isPhantom())
			realCount++;
	}
	return realCount;
}

const Dancer* Group::dancerByIndex(int dancerIndex) const {
	for (int i = 0; i < _dancers.size(); i++) {
		if (dancerIndex == _dancers[i]->dancerIndex())
			return _dancers[i];
	}
	return null;
}

const Dancer* Group::dancerByLocation(int x, int y, bool inBase) const {
	const Dancer* d;
	if (inBase) {
		if (_base == null)
			return null;
		int xBase = x;
		int yBase = y;
		if (_transform)
			_transform->revert(&xBase, &yBase, null);
		d = _base->dancerByLocation(xBase, yBase, false);
		if (d == null) {
			d = _base->dancerByLocation(xBase, yBase, true);
			if (d == null)
				return null;
		}
		// d could be temp, so be careful...
		static Dancer temp;

		temp._dancerIndex = d->_dancerIndex;
		temp.facing = d->facing;
		temp.couple = d->couple;
		temp.gender = d->gender;

		if (_transform)
			_transform->apply(&xBase, &yBase, &temp.facing);

		temp.x = x;
		temp.y = y;
		return &temp;
	} else {
		for (int i = 0; i < _dancers.size(); i++) {
			d = _dancers[i];
			if (d->x == x &&
				d->y == y)
				return d;
		}
		return null;
	}
}

void assignAfterCoordinates(vector<Plane*>& planes) {
	int centerLine = INT_MAX;

	for (int i = 0; i < planes.size(); i++)
		if (planes[i]->centerLine) {
			centerLine = i;
			break;
		}
	int coord = 0;
	for (int i = centerLine - 1; i >= 0; i --) {
		if (planes[i]->lesserEdge) {		// A center
			int gtrCoord = planes[i]->greaterEdge->after;
			if (gtrCoord != INT_MAX) {
				int halfWidth = planes[i]->afterWidth / 2;
				planes[i]->after = gtrCoord - halfWidth;
				continue;
			} else if (planes[i]->now < 0)
				coord = planes[i]->now;
		} else if (planes[i]->lesser) {		// A lesser edge
			int ctrCoord = planes[i]->tileCenter->after;
			if (ctrCoord != INT_MAX) {
				int currentWidth = ctrCoord - coord;
				int halfWidth = planes[i]->tileCenter->afterWidth / 2;

				if (currentWidth < halfWidth)
					coord -= halfWidth - currentWidth;
			} else
				coord = planes[i]->now;
		}									// A greater edge just gets the current coord
		planes[i]->after = coord;
	}
	coord = 0;
	for (int i = centerLine + 1; i < planes.size(); i++) {
		if (planes[i]->lesserEdge) {		// A center
			int lssCoord = planes[i]->lesserEdge->after;
			int halfWidth = planes[i]->afterWidth / 2;
			planes[i]->after = lssCoord + halfWidth;
			continue;
		} else if (!planes[i]->lesser) {		// A greater edge
			int ctrCoord = planes[i]->tileCenter->after;
			int currentWidth = coord - ctrCoord;
			int halfWidth = planes[i]->tileCenter->afterWidth / 2;

			if (currentWidth < halfWidth)
				coord += halfWidth - currentWidth;
		}									// A lesser edge just gets the current coord
		planes[i]->after = coord;
	}
}

Rotation rotateBy(Rotation r, int n) {
	int x = r + n;
	while (x < 0)
		x += 8;
	return Rotation(x & 7);
}

Motion* lastCurve(const Dancer* d, const Interval* interval, Context* context) {
	Motion* m = interval->lastCurve(d, true);
	if (m)
		return m;
	Sequence* sequence = context->sequence();
	if (sequence == null)
		return null;
	for (int i = 0; i < sequence->stages().size(); i++) {
		if (context->stage() == sequence->stages()[i]) {
			while (i > 0) {
				const Stage* s = sequence->stages()[i - 1];
				return s->motions()->lastCurve(d->dancerIndex(), true);
			}
		}
	}
	return null;
}

static bool isBetween(const vector<Rectangle>& boundingBoxes, const Dancer* a, const Dancer* b) {
	for (int i = 0; i < boundingBoxes.size(); i++)
		if (boundingBoxes[i].isBetween(a, b))
			return true;
	return false;
}

static bool anyOverlapping(const vector<Rectangle>& boundingBoxes) {
	for (int i = 0; i < boundingBoxes.size(); i++) {
		for (int j = i + 1; j < boundingBoxes.size(); j++) {
			if (boundingBoxes[i].overlaps(boundingBoxes[j]))
				return true;
		}
	}
	return false;
}

}  // namespace dance
