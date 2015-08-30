#include "../common/platform.h"
#include "call.h"

namespace dance {

static void cuts(int x, int y, const vector<const Rectangle*>& boxes, bool* xCuts, bool* yCuts);
static int hBucket(const vector<int>& hCuts, int y);
static int vBucket(const vector<int>& vCuts, int x);

static const char* breatheActionNames[] = {
	"Undecided",
	"Don't Breathe",
	"Skip Breathe",
	"Breathe",
	"Normalize",
	"Form Grid"
};

void Resolution::copy(const Resolution& r) {
	_final = r._final;
	_beforeBreathing = r._beforeBreathing;
	_outcome.clear();
	for (int i = 0; i < r._outcome.size(); i++)
		_outcome.push_back(r._outcome[i]);
	_breatheAction = DONT_BREATHE;
}

void Resolution::constructOutcomes(Step* step) {
	for (int i = 0; i < step->tiles().size(); i++) {
		Plan* p = step->tiles()[i]->plan();
		_outcome.push_back(p->start());
	}
}

void Resolution::collectOutcomes(Step* step, Context* context) {
	collectOutcomes(~0, ~0, step, context);
}

void Resolution::collectOutcomes(unsigned mask0, unsigned mask1, Step* step, Context* context) {
	if (_resolved)
		return;
	_resolved = true;
	for (int i = 0; i < step->tiles().size(); i++) {
		Plan* p = step->tiles()[i]->plan();
		if (p->stepCount() == 0)
			continue;
		unsigned mask;
		if (i == 0)
			mask = mask0;
		else if (i == 1)
			mask = mask1;
		else
			mask = ~0;
		Step* last = p->step(p->stepCount() - 1);
		if (!last->resolution().resolved()) {
	//		last->combine(false, null, context);
	//		if (last->failed())
	//			return;
	//		if (last->final()->sameCoordinateSpace(p->start()))
	//			collectOutcomes(last, context);
	//		else
			last->collectOutcomes(context);
			if (!p->start()->sameCoordinateSpace(last->resolution().outcome(0)))
				last->resolution().resolveStep(last, null, context);
		}
		collectOutcomes(mask, last->resolution(), context);
	}
}

void Resolution::collectOutcomes(unsigned mask, const Resolution& r, Context* context) {
	if (mask == ~0) {
		if (r._final != null)
			_outcome.push_back(r._final);
		else {
			for (int i = 0; i < r._outcome.size(); i++)
				_outcome.push_back(r._outcome[i]);
		}
	} else {
		if (r._final != null)
			_outcome.push_back(r._final->extract(mask, context));
		else {
			for (int i = 0; i < r._outcome.size(); i++)
				_outcome.push_back(r._outcome[i]->extract(mask, context));
		}
	}
}

void Resolution::resolvePlan(Plan* p, Context* context) {
	_resolved = true;
	Step* last = p->step(p->stepCount() - 1);
	last->combine(true, null, context);
	copy(last->resolution());
}

void Resolution::exclusiveOutcome(const Group* g) {
	_resolved = true;
	_outcome.clear();
	addOutcome(g);
}

void Resolution::resolveStep(Step* step, const Group* abstracted, Context* context) {
	if (!_resolved)
		collectOutcomes(step, context);
	_resolved = true;
	if (_final != null) {
		_breatheAction = SKIP_BREATHE;
		return;
	}
	unsigned affectedMask = 0;
	for (int i = 0; i < _outcome.size(); i++) {
		const Group* g = _outcome[i];
		unsigned m = g->dancerMask();
		if (m & affectedMask) {
			step->fail(context->stage()->newExplanation(USER_ERROR, "Some dancers are given conflicting instructions"));
			return;
		}
		if (g->sameCoordinateSpace(step->start())) {
			g = g->toCommonCoordinates(step->start(), context);
			if (g == null) {
				if (verboseBreathing) {
					printf("Can't find common coordinates at tile %d\n", i);
					printf("  Step start:\n");
					step->start()->printDetails(4, true);
					for (int j = 0; j < _outcome.size(); j++) {
						printf("  Tile %d\n", j);
						_outcome[j]->printDetails(4, true);
					}
				}
				step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Can't find common coordinates"));
				return;
			}
		}
		_outcome[i] = g;
		affectedMask |= m;
	}
	// For the special case of a single tile, it can change the geometry without error.
	if (_outcome.size() == 1 &&
		!_outcome[0]->sameCoordinateSpace(step->start())) {
		if (_outcome[0]->dancerCount() == step->start()->dancerCount()) {
			_final = _outcome[0];
			_breatheAction = SKIP_BREATHE;
		} else {
			if (verboseBreathing) {
				printf("Start:\n");
				step->start()->printDetails(4, true);
				printf("Outcome:\n");
				_outcome[0]->printDetails(4, true);
			}
			step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Geometry change didn't affect all dancers"));
			return;
		}
	} else {
		decideBreatheAction(step, context);
		_beforeBreathing = step->start()->combineSubGroups(_outcome, context);
		if (verboseBreathing) {
			printf("Breathe Action: %s\n", breatheActionNames[_breatheAction]);
			step->print(4, false);
			printf("before breathing:\n");
			_beforeBreathing->printDetails(4, true);
			for (int i = 0; i < _outcome.size(); i++) {
				printf("Affected[%d]:\n", i);
				_outcome[i]->printDetails(4, true);
			}
		}
		switch (_breatheAction) {
		default:
			_final = step->start()->combine(_outcome, context);
			break;

		case	BREATHE:
			step->collectMotions(null, context);
			breathe(step, abstracted, true, context);
			break;

		case	FORM_GRID:
			_final = step->start()->formGrid(step->interval(), _outcome, context);
			break;

		case	NORMALIZE:
			step->collectMotions(null, context);
			normalize(step, context);
		}
	}

	if (_final == null)
		step->fail(context->stage()->newExplanation(PROGRAM_BUG, "Conflicting merge: different geometries"));
}

void Resolution::normalize(Step* step, Context* context) {
	vector<Rectangle> boundingBoxes;

	for (int i = 0; i < _outcome.size(); i++) {
		Rectangle r;
		_outcome[i]->boundingBox(&r);
		boundingBoxes.push_back(r);
	}

	if (verboseBreathing) {
		for (int i = 0; i < boundingBoxes.size(); i++)
			boundingBoxes[i].print(4);
	}
	for (int i = 0; i < _outcome.size(); i++) {
		if (_outcome[i]->dancerCount() == 0)
			continue;
		if (!_outcome[i]->singleTile(boundingBoxes)) {
			if (verboseBreathing) {
				printf("Affected tile %d not singleton\n", i);
			}
			const Group* g = _beforeBreathing;
			_outcome.clear();
			for (;;) {
				Group* t = g->findTile(boundingBoxes, context);
				_outcome.push_back(t);
				if (t->dancerCount() == g->dancerCount())
					break;
				Group* x = context->stage()->newGroup(step->start());
				x->subtraction(g, t->dancerMask());
				g = x;
			}
			break;
		}
	}
	breathe(step, null, false, context);
}

void Resolution::breathe(Step* enclosing, const Group* abstracted, bool preserveRelativePositions, Context* context) {
	const Group* start = enclosing->start();

	_final = null;

	// Merge the affected dancer sets into the 'before breathing' set

	const Group* combined = start->combine(_outcome, context);
	if (combined == null)
		return;
	if (_beforeBreathing->dancerCount() == start->dancerCount() && combined->shouldBeRing()) {
		_final = combined->formRing(enclosing->interval(), context);
		return;
	}
	if (start->dancerCount() == 1) {
		if (_outcome.size() > 0)
			_final = _outcome[0];
		else
			_final = start;
		return;
	}
	if (verboseBreathing) {
		printf(" ***** Breathing **** %s %s\n", breatheActionNames[_breatheAction], preserveRelativePositions ? "preserveRelativePositions" : "");
		printf("start:\n");
		start->printDetails(4, true);
		if (abstracted != null) {
			printf("Before (abstracted):\n");
			abstracted->printDetails(4, true);
		} else {
			printf("Before:\n");
			combined->printDetails(4, true);
		}
	}
	int actives = 0;
	unsigned activesMask = 0;
	vector<Rectangle> startingBoxes;
	vector<Rectangle> finalBoxes;
	for (int i = 0; i < _outcome.size(); i++) {
		const Group* g = _outcome[i];
		unsigned mask = g->dancerMask();
		Rectangle r;
		Rectangle finalBox;
		g->boundingBox(&finalBox);

		if (preserveRelativePositions) {
			if (abstracted != null) {
				const Dancer* d = abstracted->dancerByIndex(i);
				r.top = d->y + 1;
				r.bottom = d->y - 1;
				r.left = d->x - 1;
				r.right = d->x + 1;
			} else {
				r.top = INT_MIN;
				r.left = INT_MAX;
				r.right = INT_MIN;
				r.bottom = INT_MAX;

				for (int j = 0; j < start->dancerCount(); j++) {
					const Dancer* d = start->dancer(j);
					if (d->dancerMask() & mask) {
						if (r.top <= d->y)
							r.top = d->y + 1;
						if (r.bottom >= d->y)
							r.bottom = d->y - 1;
						if (r.right <= d->x)
							r.right = d->x + 1;
						if (r.left >= d->x)
							r.left = d->x - 1;
					}
				}
				if (!finalBox.coincident(r))
					r = finalBox;
			}
		} else {
			r = finalBox;
		}
		startingBoxes.push_back(r);
		finalBoxes.push_back(finalBox);
		// r is now the bounding box of this group's before state
		activesMask |= g->dancerMask();
		actives += g->dancerCount();
		if (verboseBreathing) {
			printf("Tile %d:\n", i);
			g->printDetails(4, true);
			printf("Starting Bounding box:\n");
			startingBoxes[i].print(4);
			printf("Final Bounding box:\n");
			finalBoxes[i].print(4);
		}
	}

		// Find the inactive tiles

	if (actives < start->dancerCount()) {
		Group* inactives = context->stage()->newGroup(start);
		inactives->subtraction(start, activesMask);
		for (;;) {
			Group* n = inactives->findTile(startingBoxes, context);
			if (verboseBreathing) {
				printf("Tile %d (inactive):\n", _outcome.size());
				n->printDetails(4, true);
			}
			_outcome.push_back(n);
			int b = startingBoxes.size();
			startingBoxes.resize(_outcome.size());
			finalBoxes.resize(_outcome.size());
			n->boundingBox(&startingBoxes[b]);
			finalBoxes[b] = startingBoxes[b];
			if (n->dancerCount() < inactives->dancerCount()) {
				Group* x = context->stage()->newGroup(start);
				x->subtraction(inactives, n->dancerMask());
				inactives = x;
			} else
				break;
		}
	}
	unsigned rootMask = 0;
	if (start->base() && !start->tiled()) {
		unsigned currentGroupMask = start->dancerMask();
		const Group* root = start->base();
		const Group* best = start;
		unsigned bestMask = 0;
		do {
			rootMask = root->dancerMask() & ~currentGroupMask;
			if ((rootMask & ~bestMask) != 0) {
				best = root;
				bestMask = rootMask;
			}
			if (root->tiled())
				break;
			root = root->base();
		} while (root);
		root = best;
		rootMask = bestMask;
		if (verboseBreathing) {
			printf("Root:\n");
			root->printDetails(4, true);
		}
		if (rootMask) {
			const Group* g = root->extractIn(start, rootMask, context);
			for (;;) {
				Group* n = g->findTile(finalBoxes, context);
				if (verboseBreathing) {
					printf("Tile %d (inactive):\n", _outcome.size());
					n->printDetails(4, true);
				}
				_outcome.push_back(n);
				int b = startingBoxes.size();
				startingBoxes.resize(_outcome.size());
				finalBoxes.resize(_outcome.size());
				n->boundingBox(&startingBoxes[b]);
				finalBoxes[b] = startingBoxes[b];
				if (n->dancerCount() < g->dancerCount()) {
					Group* x = context->stage()->newGroup(start);
					x->subtraction(g, n->dancerMask());
					g = x;
				} else
					break;
			}
		}
	}
	CompactifyInfo info(CENTER, 0, CENTER, 0);
	combined->boundingBox(&info.finalBox);
	start->boundingBox(&info.startBox);
	for (int i = 0; i < _outcome.size(); i++) {
		info.groupIds.push_back(i);
		info.startingBoxes.push_back(&startingBoxes[i]);
		info.finalBoxes.push_back(&finalBoxes[i]);
	}
	bool anyMotion = compactify(enclosing, &info, rootMask, context);

	const Group* d = start->combine(_outcome, context);
	if (d == null)
		return;

	// We now need to make one more adjustment.  This will make diamonds take proper
	// shape and also align so-called "C-1 phantom" formations.  Essentially,
	// one- and two-dancer groupings that find themselves near a hand-hold of the 
	// appropriate kind will adjust to align to that hand-hold.

	Group* out = context->stage()->newGroup(d);
	enclosing->interval()->currentDancers(d);
	for (int i = 0; i < d->dancerCount(); i++) {
		const Dancer* a = d->dancer(i);
		switch (a->facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			if (abs(a->x) == 1) {
				for (int j = 0; j < d->dancerCount(); j++) {
					const Dancer* b = d->dancer(j);
					if (b->x != a->x)
						continue;
					if (b->facing != BACK_FACING &&
						b->facing != FRONT_FACING)
						continue;
					for (int k = 0; k < d->dancerCount(); k++) {
						const Dancer* c = d->dancer(k);
						if (c->y != b->y)
							continue;
						if (c->x != b->x * 3)
							continue;
						if (c->facing != BACK_FACING &&
							c->facing != FRONT_FACING)
							continue;
						bool blockingDancer = false;
						for (int m = 0; m < d->dancerCount(); m++) {
							const Dancer* n = d->dancer(m);
							if (n->x == a->x * 3 &&
								n->y == a->y) {
								blockingDancer = true;
								break;
							}
						}
						if (blockingDancer)
							break;
						if (verboseBreathing)
							printf("Adjusting Tile %d (for diamond/phantom) by [%d,%d]\n", i, a->x, 0);
						// We have a hand-hold at the correct place, adjust dancer a
						out->insert(a->displace(a->x, 0, enclosing->interval(), context));
						anyMotion = true;
						break;
					}
				}
			}
			break;

		case	BACK_FACING:
		case	FRONT_FACING:
			if (abs(a->y) == 1) {
				for (int j = 0; j < d->dancerCount(); j++) {
					const Dancer* b = d->dancer(j);
					if (b->y != a->y)
						continue;
					if (b->facing != RIGHT_FACING &&
						b->facing != LEFT_FACING)
						continue;
					for (int k = 0; k < d->dancerCount(); k++) {
						const Dancer* c = d->dancer(k);
						if (c->x != b->x)
							continue;
						if (c->y != b->y * 3)
							continue;
						if (c->facing != RIGHT_FACING &&
							c->facing != LEFT_FACING)
							continue;
						bool blockingDancer = false;
						for (int m = 0; m < d->dancerCount(); m++) {
							const Dancer* n = d->dancer(m);
							if (n->y == a->y * 3 &&
								n->x == a->x) {
								blockingDancer = true;
								break;
							}
						}
						if (blockingDancer)
							break;
						if (verboseBreathing)
							printf("Adjusting Tile %d (for diamond/phantom) by [%d,%d]\n", i, 0, a->y);
						// We have a hand-hold at the correct place, adjust dancer a
						out->insert(a->displace(0, a->y, enclosing->interval(), context));
						anyMotion = true;
						break;
					}
				}
			}
			break;
		}
	}
	if (!anyMotion)
		_breatheAction = SKIP_BREATHE;
	_final = out->merge(context);
}

bool Resolution::compactify(Step* enclosing, CompactifyInfo* info, unsigned rootMask, Context* context) {
	if (verboseBreathing)
		info->print("compactify");
	if (info->groupIds.size() == 1)
		return info->displaceIfNeeded(this, enclosing, rootMask, context);


	// Calculate the cuts needed to partition the groups either vertically ot horizontally

	vector<int> vCuts;
	vector<int> hCuts;
	for (int i = 0; i < info->groupIds.size(); i++) {
		const Rectangle* r = info->startingBoxes[i];

		bool xCuts, yCuts;
		cuts(r->right, r->top, info->startingBoxes, &xCuts, &yCuts);
		if (verboseBreathing) {
			printf("xCuts = %s yCuts = %s right = %d top = %d\n", xCuts ? "true" : "false", yCuts ? "true" : "false", r->right, r->top);
		}
		if (xCuts) {
			for (int j = 0; ; j++) {
				if (j >= vCuts.size()) {
					vCuts.push_back(r->right);
					break;
				}
				if (r->right == vCuts[j])
					break;
				if (r->right < vCuts[j]) {
					vCuts.insert(j, r->right);
					break;
				}
			}
		}
		if (yCuts) {
			for (int j = 0; ; j++) {
				if (j >= hCuts.size()) {
					hCuts.push_back(r->top);
					break;
				}
				if (r->top == hCuts[j])
					break;
				if (r->top < hCuts[j]) {
					hCuts.insert(j, r->top);
					break;
				}
			}
		}
	}
	// Note: cut counts actually include one cut on the very right and very bottom margin,
	// so the number of buckets is the same and the number of cuts (and always > 0)
	if (hCuts.size() == 1) {
		if (vCuts.size() == 1) {
			if (verboseBreathing) {
				printf("Only one cut each way.\n");
				printf("<<compactify\n");
			}
			return info->displaceIfNeeded(this, enclosing, rootMask, context);
		}
	} else if (vCuts.size() != 1) {
		if (hCuts.size() < vCuts.size()) {
			vCuts[0] = vCuts[vCuts.size() - 1];
			vCuts.resize(1);
		}
		else if (hCuts.size() > vCuts.size()) {
			hCuts[0] = hCuts[hCuts.size() - 1];
			hCuts.resize(1);
		}
	}
	int totalBuckets = vCuts.size() * hCuts.size();
	vector<CompactifyInfo> subInfo(totalBuckets);
	for (int i = 0; i < info->groupIds.size(); i++) {
		int vB = vBucket(vCuts, info->startingBoxes[i]->right);
		int hB = hBucket(hCuts, info->startingBoxes[i]->top);
		int bucket = vCuts.size() * hB + vB;
		subInfo[bucket].groupIds.push_back(info->groupIds[i]);
		subInfo[bucket].startingBoxes.push_back(info->startingBoxes[i]);
		subInfo[bucket].finalBoxes.push_back(info->finalBoxes[i]);
	}
	if (verboseBreathing) {
		for (int j = 0; j < hCuts.size(); j++) {
			for (int i = 0; i < vCuts.size(); i++) {
				int bucket = vCuts.size() * j + i;
				printf("  [v %d, h %d]\n", i, j);
				for (int k = 0; k < subInfo[bucket].groupIds.size(); k++) {
					const Rectangle* s = subInfo[bucket].startingBoxes[k];
					const Rectangle* f = subInfo[bucket].finalBoxes[k];
					printf("             Tile %d s[l %d,t %d,r  %d,b %d] f[l %d,t %d,r  %d,b %d]\n", 
							   subInfo[bucket].groupIds[k],
							   s->left, s->top, s->right, s->bottom,
							   f->left, f->top, f->right, f->bottom);
				}
			}
		}
	}
	// Calculate the mid-points in each dimension and the
	// widths/heights of each segment.
	vector<int> heights(hCuts.size());
	heights.setAll(0);
	int middleHoriz = -1;
	bool middleHorizStraddles = false;
	for (int j = 0; j < hCuts.size(); j++) {
		for (int i = 0; i < vCuts.size(); i++) {
			int bucket = vCuts.size() * j + i;
			Rectangle r;
			Rectangle rf;
			if (subInfo[bucket].startingBoxes.size() == 0) {
				r.clear();
				rf.clear();
			} else {
				r = *subInfo[bucket].startingBoxes[0];
				for (int k = 1; k < subInfo[bucket].startingBoxes.size(); k++) {
					r.enclose(*subInfo[bucket].startingBoxes[k]);
				}
				rf = *subInfo[bucket].finalBoxes[0];
				for (int k = 1; k < subInfo[bucket].finalBoxes.size(); k++) {
					rf.enclose(*subInfo[bucket].finalBoxes[k]);
				}
				int h = rf.height();
				if (h > heights[j])
					heights[j] = h;
				if (r.bottom < 0) {
					middleHoriz = j;
					middleHorizStraddles = r.top > 0;
				}
			}
			subInfo[bucket].startBox = r;
			subInfo[bucket].finalBox = rf;
		}
	}
	vector<int> widths(vCuts.size());
	widths.setAll(0);
	int middleVert = -1;
	bool middleVertStraddles = false;
	for (int i = 0; i < vCuts.size(); i++) {
		for (int j = 0; j < hCuts.size(); j++) {
			int bucket = vCuts.size() * j + i;
			Rectangle r = subInfo[bucket].startBox;
			Rectangle rf = subInfo[bucket].finalBox;
			if (subInfo[bucket].startingBoxes.size() != 0) {
				int w = rf.width();
				if (w > widths[i])
					widths[i] = w;
				if (r.left < 0) {
					middleVert = i;
					middleVertStraddles = r.right > 0;
				}
			}
		}
	}
	int middleXLeft;
	int middleXRight;
	switch (info->vertical) {
	case	LEFT_OF_CENTER:
		middleXLeft = info->verticalPosition;
		middleVert = vCuts.size();
		break;

	case	CENTER:
		if (middleVert < 0) {
			middleXLeft = 0;
			middleXRight = 0;
			middleVert = vCuts.size();
		} else {
			middleXLeft = INT_MAX;
			middleXRight = INT_MIN;
			for (int j = 0; j < hCuts.size(); j++) {
				int bucket = vCuts.size() * j + middleVert;
				const Rectangle& r = subInfo[bucket].finalBox;
				if (middleXLeft > r.left)
					middleXLeft = r.left;
				if (middleXRight < r.right)
					middleXRight = r.right;
			}
			if (middleXRight < 0) {
				middleXLeft -= middleXRight;
				middleXRight = 0;
			} else if (middleXRight > 0) {
				if (!middleVertStraddles) {
					middleXLeft -= middleXRight;
					middleXRight = 0;
				}
			}
			if (middleXLeft > 0) {
				middleXRight -= middleXLeft;
				middleXLeft = 0;
			}
		}
		break;

	case	RIGHT_OF_CENTER:
		middleVert = -1;
		middleXRight = info->verticalPosition;
		break;
	}
	int middleYBottom;
	int middleYTop;
	switch (info->horizontal) {
	case	LEFT_OF_CENTER:
		middleYBottom = info->horizontalPosition;
		middleHoriz = hCuts.size();
		break;

	case	CENTER:
		if (middleHoriz < 0) {
			middleYBottom = 0;
			middleYTop = 0;
			middleHoriz = hCuts.size();
		} else {
			middleYBottom = INT_MAX;
			middleYTop = INT_MIN;
			for (int j = 0; j < vCuts.size(); j++) {
				int bucket = vCuts.size() * middleHoriz + j;
				const Rectangle& r = subInfo[bucket].finalBox;
				if (middleYBottom > r.bottom)
					middleYBottom = r.bottom;
				if (middleYTop < r.top)
					middleYTop = r.top;
			}
			if (middleYTop < 0) {
				middleYBottom -= middleYTop;
				middleYTop = 0;
			} else if (middleYTop > 0) {
				if (!middleHorizStraddles) {
					middleYBottom -= middleYTop;
					middleYTop = 0;
				}
			}
			if (middleYBottom > 0) {
				middleYTop -= middleYBottom;
				middleYBottom = 0;
			}
		}
		break;

	case	RIGHT_OF_CENTER:
		middleYTop = info->horizontalPosition;
		middleHoriz = -1;
		break;
	}
	if (verboseBreathing) {
		printf(    "V Cut Data:  cut  width\n");
		for (int i = 0; i < vCuts.size(); i++) {
			printf("[%3d]        %3d  %5d\n", i, vCuts[i], widths[i]);
		}
		printf("middleVert = %d MiddleXLeft = %d  MiddleXRight = %d straddles=%s\n", middleVert, middleXLeft, middleXRight, middleVertStraddles ? "true" : "false");
		printf(    "H Cut Data:  cut height\n");
		for (int i = 0; i < hCuts.size(); i++) {
			printf("[%3d]        %3d  %5d\n", i, hCuts[i], heights[i]);
		}
		printf("middleHoriz = %d MiddleYBottom = %d  MiddleYTop = %d straddles=%s\n", middleHoriz, middleYBottom, middleYTop, middleHorizStraddles ? "true" : "false");
		for (int i = 0; i < vCuts.size(); i++) {
			for (int j = 0; j < hCuts.size(); j++) {
				int bucket = vCuts.size() * j + i;
				const Rectangle& r = subInfo[bucket].finalBox;
				printf("[v%3d,h%3d] ", i, j);
				r.print(0);
			}
		}
	}
	for (int j = 0; j < hCuts.size(); j++) {
		for (int i = 0; i < vCuts.size(); i++) {
			int bucket = vCuts.size() * j + i;
			CompactifyInfo& ci = subInfo[bucket];

			if (i < middleVert) {
				ci.vertical = LEFT_OF_CENTER;
				ci.verticalPosition = middleXLeft;
				for (int k = i + 1; k < middleVert; k++)
					ci.verticalPosition -= widths[k];
			} else if (i == middleVert) {
				ci.vertical = CENTER;
				ci.verticalPosition = 0;
			} else {
				ci.vertical = RIGHT_OF_CENTER;
				ci.verticalPosition = middleXRight;
				for (int k = middleVert + 1; k < i; k++)
					ci.verticalPosition += widths[k];
			}
			if (j < middleHoriz) {
				ci.horizontal = LEFT_OF_CENTER;
				ci.horizontalPosition = middleYBottom;
				for (int k = j + 1; k < middleHoriz; k++)
					ci.horizontalPosition -= heights[k];
			} else if (j == middleHoriz) {
				ci.horizontal = CENTER;
				ci.horizontalPosition = 0;
			} else {
				ci.horizontal = RIGHT_OF_CENTER;
				ci.horizontalPosition = middleYTop;
				for (int k = middleHoriz + 1; k < j; k++)
					ci.horizontalPosition += heights[k];
			}
		}
	}
	bool anyMotion = false;
	for (int i = 0; i < totalBuckets; i++)
		anyMotion |= compactify(enclosing, &subInfo[i], rootMask, context);
	if (verboseBreathing)
		printf("<<compactify\n");
	return anyMotion;
}

void Resolution::resolveReduction() {
}

bool Resolution::toCommonCoordinates(const Group* start, Context* context) {
	if (_final->geometry() == start->geometry()) {
		const Group* d = _final->toCommonCoordinates(start, context);
		if (d == null) {
			if (verboseOutput) {
				printf("Final:\n");
				_final->printDetails(4, true);
				printf("Start:\n");
				start->printDetails(4, true);
			}
			return false;
		}
		_final = d;
/*
		if (verboseOutput) {
			printf("=== Plan start:\n");
			_start->printDetails(4, true);
			printf("--- Plan final:\n");
			_final->printDetails(4, true);
			printf("=== Plan\n");
		}
*/
	} else {
		const Group* fin = _final;
		while (_final->base() != null) {
			const Group* d = _final->unwind(context);
			if (d == null) {
				if (verboseOutput) {
					printf("Final:\n");
					fin->printDetails(4, true);
					printf("Start:\n");
					start->printDetails(4, true);
					printf("at:\n");
					_final->printDetails(4, true);
				}
				return false;
			}
			_final = d;
		}
/*
		if (verboseOutput) {
			printf("=== Plan start (different geometries):\n");
			_start->printDetails(4, true);
			printf("--- Plan final:\n");
			_final->printDetails(4, true);
			printf("=== Plan\n");
		}
*/
	}
	return true;
}

void Resolution::checkSamePositionRule(PartStep* step, Context* context) {
	Step* s = _final->checkSamePositionRule(step, context);
	if (s) {
		_final = s->combine(true, null, context);
		s->collectMotions(step->interval(), context);
		if (verboseBreathing) {
			printf("Some collisions (after breathing):\n");
			step->print(4, false);
		}
	}
}

void Resolution::decideBreatheAction(Step* step, Context* context) {
	if (_breatheAction != UNDECIDED)
		return;

	_breatheAction = SKIP_BREATHE;

	if (step->failed())
		return;

	// If all the dancers are active as a single unit, no breathing should be done.
	// the call that arranged the dancers is responsible for creating a properly
	// grouped dancer set.
	if (_outcome.size() == 1 &&
		lastActiveMask() == step->start()->dancerMask())
		return;

	// RING's don't breathe - except that when a ring ends a part with dancers off
	// the ring radius (3), that generally means that the dancers are formed into
	// a star or thar.  That should transform them back into a GRID, which formGrid
	// will accomplish.  (That resulting formation may breathe.)

	if (step->start()->geometry() == RING) {
		for (int i = 0; i < _outcome.size(); i++) {
			const Group* f = _outcome[i];
			if (f == null) {
				step->fail(context->stage()->newExplanation(PROGRAM_BUG, string("null outcome element: ") + i));
				return;
			}
			for (int j = 0; j < f->dancerCount(); j++) {
				const Dancer* d = f->dancer(j);
				if (d->y != 3) {
					_breatheAction = FORM_GRID;
					return;
				}
			}
		}
		return;
	} else if (typeid(*step) == typeid(DefinitionStep)) {
		for (int i = 0; i < step->tiles().size(); i++) {
			Plan* p = step->tiles()[i]->plan();
			if (p->stepCount() == 0)
				continue;
			Step* s = p->step(p->stepCount() - 1);
			if (s->breatheAction() != DONT_BREATHE) {
				_breatheAction = BREATHE;
				return;
			}
		}
	} else {
		_breatheAction = NORMALIZE;
	}
}

const char* Resolution::breatheActionLabel() const {
	return breatheActionNames[_breatheAction];
}

unsigned Resolution::finalDancerMask() const {
	if (_final != null)
		return _final->dancerMask();
	unsigned mask = 0;
	for (int i = 0; i < _outcome.size(); i++)
		mask |= _outcome[i]->dancerMask();
	return mask;       
}

unsigned Resolution::lastActiveMask() const {
	// Not sure that this is correct
	if (_final != null)
		return _final->dancerMask();
	unsigned mask = 0;
	for (int i = 0; i < _outcome.size(); i++)
		mask |= _outcome[i]->dancerMask();
	return mask;
}

int Resolution::dancerCount() const {
	if (_final != null)
		return _final->dancerCount();
	int count = 0;
	for (int i = 0; i < _outcome.size(); i++)
		count += _outcome[i]->dancerCount();
	return count;
}

void Resolution::print(int indent) const {
	printf("%*.*c%s", indent, indent, ' ', breatheActionNames[_breatheAction]);
	if (_final) {
		printf(" Final:\n");
		_final->print(indent);
	} else
		printf("\n");
}

bool Resolution::CompactifyInfo::displaceIfNeeded(Resolution* that, Step* step, unsigned rootMask, Context* context) {
	if (verboseBreathing)
		print("displaceIfNeeded");
	int deltaX = 0;
	switch (vertical) {
	case	LEFT_OF_CENTER:
		deltaX = verticalPosition - finalBox.right;
		break;

	case	CENTER:
		if (startBox.right <= 0)
			deltaX = -finalBox.right;
		else if (startBox.left >= 0)
			deltaX = -finalBox.left;
		break;

	case	RIGHT_OF_CENTER:
		deltaX = verticalPosition - finalBox.left;
		break;
	}
	int deltaY = 0;
	switch (horizontal) {
	case	LEFT_OF_CENTER:
		deltaY = horizontalPosition - finalBox.top;
		break;

	case	CENTER:
		if (startBox.top <= 0)
			deltaY = -finalBox.top;
		else if (startBox.bottom >= 0)
			deltaY = -finalBox.bottom;
		break;

	case	RIGHT_OF_CENTER:
		deltaY = horizontalPosition - finalBox.bottom;
		break;
	}
	bool anyMotion = false;
	if (deltaX != 0 || deltaY != 0) {
		for (int i = 0; i < groupIds.size(); i++) {
			const Group* g = that->outcome(groupIds[i]);
			if (g->dancerMask() & rootMask)
				continue;
			anyMotion = true;
			if (verboseBreathing)
				printf("Adjusting tile %d by [%d, %d]\n", groupIds[i], deltaX, deltaY);
			that->replaceOutcome(groupIds[i], g->displace(deltaX, deltaY, step->interval(), context));
		}
	}
	return anyMotion;
}

void Resolution::CompactifyInfo::print(const char* label) {
	static const char* names[] = { "LEFT_OF_CENTER", "CENTER", "RIGHT_OF_CENTER" };
	printf("%s {v <%s, %d>, h <%s, %d>}>>\n", label, names[vertical], verticalPosition, names[horizontal], horizontalPosition);
	printf("  Start box: ");
	startBox.print(0);
	printf("  Final box: ");
	finalBox.print(0);
	for (int i = 0; i < groupIds.size(); i++)
		printf("    Tile %d\n", groupIds[i]);
}

static int hBucket(const vector<int>& hCuts, int y) {
	int hB = -1;
	for (int j = 0; j < hCuts.size(); j++) {
		if (y > hCuts[j])
			continue;
		if (hB < 0)
			hB = j;
		else if (hCuts[hB] > hCuts[j])
			hB = j;
	}
	return hB;
}

static int vBucket(const vector<int>& vCuts, int x) {
	int vB = -1;
	for (int j = 0; j < vCuts.size(); j++) {
		if (x > vCuts[j])
			continue;
		if (vB < 0)
			vB = j;
		else if (vCuts[vB] > vCuts[j])
			vB = j;
	}
	return vB;
}

static void cuts(int x, int y, const vector<const Rectangle*>& boxes, bool* xCuts, bool* yCuts) {
	*xCuts = true;
	*yCuts = true;
	for (int i = 0; i < boxes.size(); i++) {
		if (boxes[i]->intersectsHorizontally(y)) {
			*yCuts = false;
			if (!*xCuts)
				return;
		}
		if (boxes[i]->intersectsVertically(x)) {
			*xCuts = false;
			if (!*yCuts)
				return;
		}
	}
}


}  // namespace dance