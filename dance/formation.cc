#include "../common/platform.h"
#include "stored_data.h"

#include "../common/timing.h"
#include "dance.h"
#include "call.h"

namespace dance {

static char positionChars[] = {
	0,				//	NO_POSITION,			// not found in any pattern
	'a',			//	ACTIVE,					// a
	'b',			//	ACTIVE_BOY,				// b
	'g',			//	ACTIVE_GIRL,			// g
	'd',			//	ACTIVE_DESIGNATED,		// d
	'n',			//	ACTIVE_NONDESIGNATED,	// n
	'c',			//	CENTER,					// c
	'e',			//	END,					// e
	'C',			//	VERY_CENTER,			// C
	'E',			//	VERY_END,				// E
	'i',			//	INACTIVE,				// i

		// Syntactic tokens

	0,				//	END_OF_ROW,				// end of string

		// Not dancers, but location and/or relationship markers

	'.',			//	EMPTY,					// .
	'-',			//	SAME_ROW,				// -
	'|',			//	SAME_COLUMN,			// |
	'<',			//	TO_THE_LEFT,			// <
	'>',			//	TO_THE_BACK,			// ^
	'\\',			//	WRAP,					// wraps to beginning of ring
};

Spot Spot::empty(EMPTY, ANY_FACING);

class DiagramParser {
public:
	DiagramParser(const string& row) {
		_row = row;
		_cursor = 0;
	}

	bool scan() {
		spot.facing = ANY_FACING;
		while (_cursor < _row.size()) {
			switch (_row[_cursor++]) {
			case	'i':
				spot.position = INACTIVE;
				return scanFacing();

			case	'a':
				spot.position = ACTIVE;
				return scanFacing();

			case	'b':
				spot.position = ACTIVE_BOY;
				return scanFacing();

			case	'g':
				spot.position = ACTIVE_GIRL;
				return scanFacing();

			case	'd':
				spot.position = ACTIVE_DESIGNATED;
				return scanFacing();

			case	'n':
				spot.position = ACTIVE_NONDESIGNATED;
				return scanFacing();

			case	'c':
				spot.position = CENTER;
				return scanFacing();

			case	'e':
				spot.position = END;
				return scanFacing();

			case	'C':
				spot.position = VERY_CENTER;
				return scanFacing();

			case	'E':
				spot.position = VERY_END;
				return scanFacing();

			case	' ':
			case	'\t':
				break;

			case	'.':
				spot.position = EMPTY;
				return true;

			case	'-':
				spot.position = SAME_ROW;
				return true;

			case	'|':
				spot.position = SAME_COLUMN;
				return true;

			case	'<':
				spot.position = TO_THE_LEFT;
				return true;

			case	'^':
				spot.position = TO_THE_BACK;
				return true;

			case	'\\':
				spot.position = WRAP;
				return true;

			default:
				return false;
			}
		}
		spot.position = END_OF_ROW;
		return true;
	}

	Spot	spot;

private:
	bool scanFacing() {
		switch (_row[_cursor++]) {
		case	'>':
			spot.facing = RIGHT_FACING;
			return true;

		case	'<':
			spot.facing = LEFT_FACING;
			return true;

		case	'^':
			spot.facing = BACK_FACING;
			return true;

		case	'v':
			spot.facing = FRONT_FACING;
			return true;

		case	'|':
			spot.facing = HEAD_FACING;
			return true;

		case	'-':
			spot.facing = SIDE_FACING;
			return true;

		case	'?':
			spot.facing = ANY_FACING;
			return true;

		default:
			return false;
		}
	}

	string _row;
	int _cursor;
};

const bool isDancer(PositionType position) {
	if (position <= VERY_END)
		return true;
	else
		return false;
}

const bool isSignificantSpot(PositionType position) {
	if (position <= INACTIVE)
		return true;
	else
		return false;
}

int Formation::compare(const Formation* other) const {
	return _name.compare(&other->_name);
}

unsigned Formation::extract(const Group* inFormation, const Group* dancers, PositionType pos, PositionType altPos) const {
//	const Dancer* basis[MAX_DANCERS];
	unsigned mask = 0;

//	dancers->buildDancerArray(basis);
	int dancerI = 0;
	for (int i = 0; i < _spotRows.size(); i++)
		for (int j = 0; j < _spotRows[i].size(); j++) {
			PositionType pt = _spotRows[i][j].position;
			if (isDancer(pt)) {
				if (pt == pos || pt == altPos)
					mask |= inFormation->dancer(dancerI)->dancerMask();
				if (pt != INACTIVE)
					dancerI++;
			}
		}
	return mask;
}
/*
 *	match
 *
 *	This method tries to match the given formation
 *	against the dancers set.  This returns true if the
 *	formation matches all of the dancers in the 'dancers'
 *	set.
 */
bool Formation::match(const Group* dancers, const PatternClosure* closure) const {
	return matchSome(dancers, 0, closure) == dancers->dancerMask();
}

/*
 *	matchSome
 *
 *	This method tries to match the given formation
 *	against the dancers set.  If a match is obtained,
 *	then the designated 'active' dancers are copied out
 *	and translated to a 'normalized' coordinate system.
 *
 *	Returns a bit mask with 1's at each matching dancer.
 *	A zero return is a miss.
 */
unsigned Formation::matchSome(const Group* dancers, int startWith, const PatternClosure* closure) const {
	int dancerCount = dancers->dancerCount();

	if (startWith >= dancerCount)
		return 0;

	// If not enough dancers remain in the set, also give up
	if (dancerCount - startWith < _dancerCount)
		return 0;

	int row = 0;
	int column = _firstSignificantColumn;
	int x_offset;
	int y_offset;
	const Dancer* d;
	unsigned mask;
	if (verboseMatching)
		printf("'%s'::matchSome(..., %d, ...) first column=%d\n", _name.c_str(), startWith, _firstDancerColumn);
	switch (_geometry) {
	case	GRID:
	case	RING:
		// This is a 'grid' formation, only match against grid geometry
		if (dancers->geometry() != _geometry)
			return 0;

	case	UNSPECIFIED_GEOMETRY:
		d = dancers->dancer(startWith);

		// x and y coordinates are treated differently here because formation rows progress in the negative y direction.
		// so, for spot [column, row] the coords in dancers is [column - x_offset, y_offset - row]
		x_offset = _firstDancerColumn - d->x;
		y_offset = d->y + _firstDancerRow;
		if (_firstDancerRow < 0)
			return 0;
		if (!d->matches(_spotRows[_firstDancerRow][_firstDancerColumn], closure))
			return 0;
		if (verboseMatching)
			printf("Hit %d: offset [%d,%d] row=%d column=%d\n", startWith, x_offset, y_offset, row, column);
		mask = d->dancerMask();
		for (int spot = 0; spot < _significantCount; spot++) {
			if (row >= _spotRows.size())
				return 0;
			int i;
			int endPoint;
			int x1 = column - x_offset;
			int y1 = y_offset - row;
			if (verboseMatching)
				printf("Next spot at row=%d column=%d\n", row, column);
			const Spot& s = _spotRows[row][column];
			switch (s.position) {
			case	INACTIVE:
				if (dancers->base() == null)
					return 0;
				d = dancers->dancerByLocation(x1, y1, false);
				if (d)
					return 0;					// The spot is occupied by an active dancer
				d = dancers->dancerByLocation(x1, y1, true);
				if (d == null)
					return 0;					// The spot is unoccupied
				if (!d->matches(s, closure))
					return 0;					// Some facing constraint is not satisfied
				break;

			default:
				if (row == _firstDancerRow && column == _firstDancerColumn) {
					d = dancers->dancer(startWith);
					break;
				}

				d = dancers->dancerByLocation(x1, y1, false);
				if (d == null)
					return 0;					// The spot is not occupied by an active dancer
				if (!d->matches(s, closure))
					return 0;					// Some facing constraint is not satisfied
				mask |= d->dancerMask();
			}
			if (verboseMatching) {
				printf("Found dancer:\n");
				d->print(4);
			}
			nextSignificantSpot(&row, &column);
		}
		return mask;

	case	HEXAGONAL:
		if (dancers->geometry() != _geometry)
			return 0;
		break;
	}
	return 0;
}

Group* Formation::matchWithPhantoms(const Group* dancers, const PatternClosure* closure) const {
	int dancerCount = dancers->dancerCount();

	vector<PhantomMatch> matching;

	if (verboseMatching)
		printf("'%s'::matchWithPhantoms(...)\n", _name.c_str());
	int row = 0;
	int column = _firstSignificantColumn;
	int x_offset;
	int y_offset;
	Rectangle r;
	const Dancer* d;
	int matchedDancers = 0;
	Group* g = closure->context()->stage()->newGroup(dancers);
	switch (_geometry) {
	case	GRID:
	case	RING:
		// This is a 'grid' formation, only match against grid geometry
		if (dancers->geometry() != _geometry)
			return null;

	case	UNSPECIFIED_GEOMETRY:
		if (_firstDancerRow < 0)
			return null;

		boundingBox(&r);
		x_offset = r.right / 2;
		y_offset = r.top / 2;
		for (int spot = 0; spot < _significantCount; spot++) {
			if (row >= _spotRows.size())
				return null;
			int i;
			int endPoint;
			int x1 = column - x_offset;
			int y1 = y_offset - row;
			if (verboseMatching)
				printf("Next spot at row=%d column=%d\n", row, column);
			const Spot& s = _spotRows[row][column];
			switch (s.position) {
			case	INACTIVE:			// We don't care what might be at this location.
										// It either is one of the active dancers, in which
										// case they won't be included in the matched set,
										// or it isn't, in which case it could be a phantom.
				break;

			default:
				d = dancers->dancerByLocation(x1, y1, false);
				if (d != null &&
					d->matches(s, closure)) {
					if (verboseMatching) {
						printf("Found dancer:\n");
						d->print(4);
					}
					matchedDancers++;
					g->insert(d->clone());
				} else
					g->insert(new Dancer(x1, y1, s.facing, gender(s.position), 0, closure->step()->plan()->nextPhantom()));
			}
			nextSignificantSpot(&row, &column);
		}
		if (matchedDancers > 0) {
			g->done();
			return g;
		} else
			return null;

	case	HEXAGONAL:
		if (dancers->geometry() != _geometry)
			return null;
		break;
	}
	return null;
}

bool Formation::row(const string& text, int start, int end) {
	if (text.size() > start && text[start] == '*') {
		if (text.size() == start + 1)
			return false;
		switch (text[start + 1]) {
		case	'C':
			setCreated(text.substr(start + 2, end - (start + 2)));
			break;

		case	'M':
			setModified(text.substr(start + 2, end - (start + 2)));
			break;

		default:
			return false;
		}
		return true;
	}
	int i = _rows.size();
	_rows.push_back(text.substr(start, end - start));
	_spotRows.resize(i + 1);

	DiagramParser d(_rows[i]);

	int positions = 0;
	while (d.scan()) {
		if (d.spot.position == END_OF_ROW) {
			// The top row of a diagram must have at least one dancer in it.
			if (i == 0 && _dancerCount == 0)
				return false;
			if (positions > _maxPositions)
				_maxPositions = positions;
			return true;
		}
		if (isSignificantSpot(d.spot.position)) {
			if (_firstSignificantColumn < 0)
				_firstSignificantColumn = _spotRows[i].size();
			_significantCount++;
		}
		if (isDancer(d.spot.position)) {
			if (_firstDancerColumn < 0) {
				_firstDancerColumn = _spotRows[i].size();
				_firstDancerRow = i;
			}
			_dancerCount++;
			_dancerTypes[d.spot.position]++;
		}
		_spotRows[i].push_back(d.spot);
		positions++;
	}
	return false;
}

int Formation::rotationalSymmetry() const {
	if (_rotationalSymmetry == 0) {
		calculateSymmetry();
		if (verboseMatching)
			printf("Symmetry of %s = %d\n", _name.c_str(), _rotationalSymmetry);
	}
	return _rotationalSymmetry;
}

void Formation::setName(const string& name) {
	_grammar->changeFormationName(this, name);
	_name = name;
}


void Formation::setCreated(const string& value) {
	_created = parseLongLong(value, 0);
}

void Formation::setModified(const string& value) {
	_modified = parseLongLong(value, 0);
}

void Formation::setModified(time_t value) {
	_modified = value;
}

void Formation::setGeometry(Geometry value) {
	_geometry = value;
}

void Formation::boundingBox(Rectangle* result) const {
	result->top = _spotRows.size();
	result->bottom = 0;
	result->left = 0;
	result->right = 0;
	for (int i = 0; i < _spotRows.size(); i++) {
		int rowLen = _spotRows[i].size();
		if (rowLen >= result->right)
			result->right = rowLen;
	}
}

void Formation::compact() {
	int highestNonEmpty;
	int minRowStart = 0;
	// First trim rows that end in empty spots
	for (int i = 0; i < _spotRows.size(); i++) {
		vector<Spot>& row = _spotRows[i];
		highestNonEmpty = -1;
		for (int j = row.size() - 1; j >= 0; j--) {
			if (row[j].position != EMPTY) {
				highestNonEmpty = j;
				break;
			}
		}
		row.resize(highestNonEmpty + 1);
		if (minRowStart < highestNonEmpty)
			minRowStart = highestNonEmpty;
	}
	// Now trim off trailing empty lines
	highestNonEmpty = -1;
	for (int i = 0; i < _spotRows.size(); i++) {
		if (_spotRows[i].size() > 0)
			highestNonEmpty = i;
	}
	_spotRows.resize(highestNonEmpty + 1);
	// Now trim off leading empty lines
	for (int i = 0; i < _spotRows.size(); i++) {
		if (_spotRows[i].size() > 0) {
			if (i > 0) {
				_spotRows.remove(0, i);
				_biasY += i;
			}
			break;
		}
	}
	// Now trim off leading empty columns, first find how much to trim
	for (int i = 0; i < _spotRows.size(); i++) {
		vector<Spot>& row = _spotRows[i];
		for (int j = 0; j < row.size() && j < minRowStart; j++) {
			if (row[j].position != EMPTY) {
				minRowStart = j;
				break;
			}
		}
	}
	if (minRowStart > 0) {
		// Now trim the empty prefixes.  We rely on remove doing the right thing for short lines.
		for (int i = 0; i < _spotRows.size(); i++) {
			vector<Spot>& row = _spotRows[i];
			row.remove(0, minRowStart);
		}
		_biasX += minRowStart;
	}
}

void Formation::write(FILE* fp) const {
	fprintf(fp, "=%s", _name.c_str());
	switch (_geometry) {
	case	GRID:
		fprintf(fp, " @grid");
		break;

	case	HEXAGONAL:
		fprintf(fp, " @hexagonal");
		break;

	case	RING:
		fprintf(fp, " @ring");
		break;
	}
	fprintf(fp, "\n");
	if (_created)
		fprintf(fp, "*C%I64d\n", _created);
	if (_modified)
		fprintf(fp, "*M%I64d\n", _modified);
	for (int i = 0; i < _spotRows.size(); i++) {
		const vector<Spot>& row = _spotRows[i];
		for (int j = 0; j < row.size(); j++) {
			if (j > 0)
				fputc(' ', fp);
			const Spot& s = row[j];
			static char facingChars[] = {
				'>',	//	RIGHT_FACING,	// > caller's right side of hall
				'^',	//	BACK_FACING,	// ^ back of hall
				'<',	//	LEFT_FACING,	// < caller's left side of hall
				'v',	//	FRONT_FACING,	// v front of hall
				'|',	//	HEAD_FACING,	// | front or back
				'-',	//	SIDE_FACING,	// - left or right
				'?',	//	ANY_FACING,		// ? any value
			};
			fputc(positionChars[s.position], fp);
			switch (s.position) {
			default:
				fputc(facingChars[s.facing], fp);
				break;

			case	EMPTY:
			case	SAME_ROW:
			case	SAME_COLUMN:
			case	TO_THE_LEFT:
			case	TO_THE_BACK:
			case	WRAP:
				break;
			}
		}
		fprintf(fp, "\n");
	}
}

void Formation::print() {
	printf("Formation %s", _name.c_str());
	switch (_geometry) {
	case	GRID:
		printf(" grid");
		break;

	case	HEXAGONAL:
		printf(" hexagonal");
		break;

	case	RING:
		printf(" ring");
		break;
	}
	printf("\n");
	printf(" _dancerCount=%d _significantCount=%d ", _dancerCount, _significantCount);
	printf(" INACTIVE=%d", _dancerTypes[INACTIVE]);
	printf(" ACTIVE=%d", _dancerTypes[ACTIVE]);
	printf(" ACTIVE_BOY=%d", _dancerTypes[ACTIVE_BOY]);
	printf(" ACTIVE_GIRL=%d", _dancerTypes[ACTIVE_GIRL]);
	printf(" ACTIVE_DESIGNATED=%d", _dancerTypes[ACTIVE_DESIGNATED]);
	printf(" ACTIVE_NONDESIGNATED=%d", _dancerTypes[ACTIVE_NONDESIGNATED]);
	printf(" CENTER=%d", _dancerTypes[CENTER]);
	printf(" END=%d", _dancerTypes[END]);
	printf(" VERY_CENTER=%d", _dancerTypes[VERY_CENTER]);
	printf(" VERY_END=%d", _dancerTypes[VERY_END]);
	printf("\n");
}

bool Formation::hasCentersOrEnds() const {
	for (int i = 0; i < _spotRows.size(); i++) {
		for (int j = 0; j < _spotRows[i].size(); j++) {
			const Spot& spot = _spotRows[i][j];
			switch(spot.position) {
			case	CENTER:
			case	END:
			case	VERY_CENTER:
			case	VERY_END:
				return true;
			}
		}
	}
	return false;
}

const Group* Formation::recenter(Group* d, Context* context) const {
	if (d->dancerCount() > 0) {
		if (d->geometry() != RING ||
			d->dancerCount() != 8) {
			const Dancer* first = d->dancer(0);
			int offsetX = _firstDancerColumn - (_maxPositions >> 1) - first->x;
			int offsetY = (_spotRows.size() >> 1) - first->y;
			if (offsetX != 0 || offsetY != 0)
				d = d->apply(Transform::translate(offsetX, offsetY), context);
		}
	}
	d->setTiled();
	return d;
}

const Spot& Formation::spot(int x, int y) const {
	x -= _biasX;
	y -= _biasY;
	if (y >= 0 && y < _spotRows.size()) {
		const vector<Spot>& row = _spotRows[y];
		if (x >= 0 && x < row.size())
			return row[x];
	}
	return Spot::empty;
}

void Formation::setSpot(int x, int y, const Spot& s) {
	if (s.position == EMPTY) {
		if (x < _biasX || y < _biasY)
			return;						// This is a no-op
	} else {
		if (x < _biasX) {
			for (int i = 0; i < _spotRows.size(); i++) {
				vector<Spot>& row = _spotRows[i];
				for (int j = 0; j < _biasX - x; j++) {
					row.insert(0, Spot::empty);
				}
			}
			_biasX = x;
		}
		while (y < _biasY) {
			vector<Spot> v;
			_spotRows.insert(0, v);
			_biasY--;
		}
	}
	x -= _biasX;
	y -= _biasY;
	while (y >= _spotRows.size()) {
		vector<Spot> v;
		_spotRows.push_back(v);
	}
	vector<Spot>& row = _spotRows[y];
	while (x >= row.size())
		row.push_back(Spot::empty);
	_spotRows[y][x] = s;
}

bool Formation::blocked(int x, int y) const {
	const Spot& left = spot(x - 1, y);
	if (left.position != EMPTY)
		return true;
	const Spot& right = spot(x + 1, y);
	if (right.position != EMPTY)
		return true;
	const Spot& back = spot(x, y + 1);
	if (back.position != EMPTY)
		return true;
	const Spot& front = spot(x, y - 1);
	if (front.position != EMPTY)
		return true;
	return false;
}

void Formation::calculateSymmetry() const {
	if (has4FoldSymmetry()) {
		_rotationalSymmetry = 4;
		return;
	}
	_rotationalSymmetry = 1;					// be pessimistic in case we bail out early
	// ok, check for 2-fold symmetry
	int checkRows = (_spotRows.size() + 1) / 2;
	for (int i = 0; i < checkRows; i++) {
		for (int j = 0; j < _spotRows[i].size(); j++) {
			const Spot& spot = _spotRows[i][j];
			int rotatedRow = _spotRows.size() - i - 1;
			int rotatedColumn = _spotRows[i].size() - j - 1;
			// You can elide trailing empty spots
			if (_spotRows[rotatedRow].size() <= rotatedColumn) {
				if (spot.position != EMPTY)
					return;
				else
					continue;
			}
			const Spot& rotatedSpot = _spotRows[rotatedRow][rotatedColumn];
			switch (spot.position) {
			case	INACTIVE:
			case	ACTIVE:
			case	ACTIVE_BOY:
			case	ACTIVE_GIRL:
			case	ACTIVE_DESIGNATED:
			case	ACTIVE_NONDESIGNATED:
			case	CENTER:
			case	END:
			case	VERY_CENTER:
			case	VERY_END:
			case	EMPTY:
			case	SAME_ROW:
			case	SAME_COLUMN:
				if (spot.position != rotatedSpot.position)
					return;
				break;

			case	WRAP:			// only for RING geometry - so it can't be 4-fold symmetric
			case	TO_THE_LEFT:
			case	TO_THE_BACK:
				return;
			}
			static Facing rotatedFacing[] = {
				LEFT_FACING,		// RIGHT
				FRONT_FACING,		// BACK
				RIGHT_FACING,		// LEFT
				BACK_FACING,		// FRONT
				HEAD_FACING,		// HEAD
				SIDE_FACING,		// SIDE
				ANY_FACING,			// ANY
			};

			if (rotatedFacing[spot.facing] != rotatedSpot.facing)
				return;
		}
	}
	_rotationalSymmetry = 2;
}

bool Formation::has4FoldSymmetry() const {
	// If the formation is square, it can have 4-fold symmetry
	if (_maxPositions == _spotRows.size()) {
		for (int i = 0; i < _spotRows.size(); i++) {
			for (int j = 0; j < _spotRows[i].size(); j++) {
				const Spot& spot = _spotRows[i][j];
				int rotatedRow = j;
				int rotatedColumn = _spotRows.size() - i - 1;;
				// You can elide trailing empty spots
				if (_spotRows[rotatedRow].size() <= rotatedColumn) {
					if (spot.position != EMPTY)
						return false;
					else
						continue;
				}
				const Spot& rotatedSpot = _spotRows[rotatedRow][rotatedColumn];
				switch (spot.position) {
				case	INACTIVE:
				case	ACTIVE:
				case	ACTIVE_BOY:
				case	ACTIVE_GIRL:
				case	ACTIVE_DESIGNATED:
				case	ACTIVE_NONDESIGNATED:
				case	CENTER:
				case	END:
				case	VERY_CENTER:
				case	VERY_END:
				case	EMPTY:
					if (spot.position != rotatedSpot.position)
						return false;
					break;

				case	SAME_ROW:
					if (rotatedSpot.position != SAME_COLUMN)
						return false;
					break;

				case	SAME_COLUMN:
					if (rotatedSpot.position != SAME_ROW)
						return false;
					break;

				case	WRAP:			// only for RING geometry - so it can't be 4-fold symmetric
				case	TO_THE_LEFT:
				case	TO_THE_BACK:
					return false;
				}
				static Facing rotatedFacing[] = {
					FRONT_FACING,		// RIGHT
					RIGHT_FACING,		// BACK
					BACK_FACING,		// LEFT
					LEFT_FACING,		// FRONT
					SIDE_FACING,		// HEAD
					HEAD_FACING,		// SIDE
					ANY_FACING,			// ANY
				};

				if (rotatedFacing[spot.facing] != rotatedSpot.facing)
					return false;
			}
		}
		// All spots are symmetric
		return true;
	} else
		return false;
}

void Formation::nextSignificantSpot(int* row, int* column) const {
	if (verboseMatching)
		printf("nextSignificantSpot(&%d, &%d)\n", *row, *column);
	for (;;) {
		(*column)++;
		while (*column >= _spotRows[*row].size()) {
			(*row)++;
			if (*row >= _spotRows.size())
				return;
			*column = 0;
		}
		if (verboseMatching)
			printf("Trying row=%d column=%d position = %c\n", *row, *column, positionChars[_spotRows[*row][*column].position]);
		if (isSignificantSpot(_spotRows[*row][*column].position))
			return;
	}
}

Group* Formation::matchNext(vector<PhantomMatch>& matching, int spot, int row, int column, const Group* dancers, int startWith, const PatternClosure* closure) const {
	if (spot >= _significantCount)
		return null;
	if (row >= _spotRows.size())
		return null;
	if (startWith >= dancers->dancerCount())
		return null;

	int x_offset;
	int y_offset;
	const Dancer* d;

	d = dancers->dancer(startWith);

	// x and y coordinates are treated differently here because formation rows progress in the negative y direction.
	// so, for spot [column, row] the coords in dancers is [column - x_offset, y_offset - row]
	x_offset = _firstDancerColumn - d->x;
	y_offset = d->y + _firstDancerRow;

	if (verboseMatching)
		printf("Hit %d: offset [%d,%d] row=%d column=%d\n", startWith, x_offset, y_offset, row, column);
	int i;
	int endPoint;
	int x1 = column - x_offset;
	int y1 = y_offset - row;
	if (verboseMatching)
		printf("Next spot at row=%d column=%d\n", row, column);
	const Spot& s = _spotRows[row][column];
	PhantomMatch m;
	switch (s.position) {
	case	INACTIVE:
		if (dancers->base() == null)
			return null;
		// So the only way this pattern matches is if the current (startWith)
		// dancer matches the rest of the pattern (the the current position is
		// an 'inactive' phantom).
		nextSignificantSpot(&row, &column);
		return matchNext(matching, spot + 1, row, column, dancers, startWith, closure);

	default:
		m.x = x1;
		m.y = y1;
		m.dancer = null;
		d = dancers->dancerByLocation(x1, y1, false);
		if (d != null && d->matches(s, closure)) {
			m.dancer = d;
			if (verboseMatching) {
				printf("Found non-phantom dancer:\n");
				d->print(4);
			}
		}
		int top = matching.size();
		matching.push_back(m);
		if (matching.size() == _dancerCount) {
			for (int i = 0; i < matching.size(); i++)
				if (matching[i].dancer != null) {
					Group* g = closure->context()->stage()->newGroup(dancers);
					for (i = 0; i < matching.size(); i++) {
						const Dancer* n;

						if (matching[i].dancer != null)
							n = matching[i].dancer->clone();
						else
							n = new Dancer(matching[i].x, matching[i].y, ANY_FACING, UNSPECIFIED_GENDER, 0, 0);
						g->insert(n);
					}
					g->done();
					return g;
				}
		}
		nextSignificantSpot(&row, &column);
		if (m.dancer != null) {
			Group* result = matchNext(matching, spot + 1, row, column, dancers, startWith + 1, closure);
			if (result)
				return result;
			// Using a real dancer failed, try this position as a phantom.
			matching[top].dancer = null;
		}
		return matchNext(matching, spot + 1, row, column, dancers, startWith, closure);
	}
}

}  // namespace dance
