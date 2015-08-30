#include "../common/platform.h"
#include "dance.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include "../common/file_system.h"
#include "../common/machine.h"
#include "../common/timing.h"
#include "../engine/global.h"
#include "call.h"
#include "dance_ui.h"
#include "motion.h"

namespace dance {

vector<string> levels;
vector<string> precedences;
dictionary<int> levelValues;

bool verboseOutput = false;
bool verboseBreathing = false;
bool verboseParsing = false;
bool verboseMatching = false;
bool showUI = true;

bool anyVerbose() {
	return verboseOutput || verboseBreathing || verboseParsing || verboseMatching;
}

Token Token::endOfString = { END_OF_STRING };

const char* genderNames[] = {
	"GIRL",
	"BOY",
	"any gender",
};

char facingGlyphs[] = {
		'>',	// RIGHT_FACING,	// > caller's right side of hall
		'^',	// BACK_FACING,	// ^ back of hall
		'<',	// LEFT_FACING,	// < caller's left side of hall
		'v',	// FRONT_FACING,	// v front of hall
		'|',	// HEAD_FACING,	// | front or back
		'-',	// SIDE_FACING,	// - left or right
		'?',	// ANY_FACING,		// ? any value
};

Primitive Primitive::nothing(P_NOTHING);

const Transform Transform::identity(1, 0, 0, 0, 1, 0);
	// _x0 = 0, _x1 = -N, _y0 = N, _y1 = 0 -> 90 degree   (left 1/4 / right 3/4)
	// _x0 = -N, _x1 = 0, _y0 = 0, _y1 = -N -> 180 degree (left 2/4 / right 2/4)
	// _x0 = 0, _x1 = N, _y0 = -N, _y1 = 0 -> 270 degree (left 3/4 / right 1/4)
const Transform Transform::rotate90(0, -1, 0, 1, 0, 0);
const Transform Transform::rotate180(-1, 0, 0, 0, -1, 0);
const Transform Transform::rotate270(0, 1, 0, -1, 0, 0);
const Transform Transform::mirror(-1, 0, 0, 0, 1, 0);

const Group* Group::home = Group::makeHome();

fileSystem::TimeStamp PlayList::lastChecked() const {
	vector<Sequence*> sequences;
	fetchSequences(&sequences);
	if (sequences.size() == 0)
		return fileSystem::TimeStamp(time(null));			// report everything as checked now, since there are none
	fileSystem::TimeStamp t = sequences[0]->lastChecked();
	for (int i = 1; i < sequences.size(); i++)
		if (t > sequences[i]->lastChecked())
			t = sequences[i]->lastChecked();
	return t;
}

void PlayList::include(vector<Sequence*>& sequences) {
	for (int i = 0; i < sequences.size(); i++)
		_sequences.push_back(sequences[i]);
}

void PlayList::fetchSequences(vector<Sequence*>* output) const {
	if (_sequences.size() > 0) {
		for (int i = 0; i < _sequences.size(); i++)
			output->push_back(_sequences[i]);
	} else {
	}
}

Sequence::Sequence(Dance* dance) {
	_dance = dance;
	created = time(null);
	createdWith = VERSION;
	modified = 0;
	_level = NO_LEVEL;				// A sequence defaults to no specified level
	status = SEQ_UNCHECKED;
	_danceType = D_4COUPLE;
}

Sequence::~Sequence() {
	clearStages();
}

void Sequence::setLevel(Level newLevel) {
	_lastChecked.clear();
	_level = newLevel;
}

void Sequence::setLevelName(const string& name) {
	_lastChecked.clear();
	_level = *levelValues.get(name);
}

void Sequence::setCall(int index, const string& call) {
	_lastChecked.clear();
	if (index == _text.size())
		_text.push_back(call);
	else
		_text[index] = call;
}

void Sequence::setLastChecked(fileSystem::TimeStamp t) {
	_lastChecked = t;
}

void Sequence::append(const string& text) {
	_text.push_back(text);
}

void Sequence::appendNotes(const string &text) {
	_notes.push_back(text);
}

bool Sequence::run(bool allowUnresolved, const Grammar* grammar) {
	updateStages(grammar);
	for (int i = 0; i < _stages.size(); i++)
		if (_stages[i]->failed())
			return false;
	if (!allowUnresolved) {
		if (!resolved()) {
			if (verboseOutput)
				printf("    *** Sequence not resolved ***\n");
			return false;
		}
	}
	return true;
}

bool Sequence::current(const Grammar* grammar) {
	return grammar->lastChanged() < _lastChecked;
}

bool Sequence::updateStatus(const Grammar* grammar) {
	if (grammar->lastChanged() > _lastChecked || status == SEQ_UNCHECKED) {
		updateStages(grammar);
		clearStages();
		return true;
	} else
		return false;
}

void Sequence::clearStages() {
	for (int i = 0; i < _stages.size(); i++)
		deletingStage.fire(_stages[i]);
	_stages.deleteAll();
}

bool Sequence::updateStages(const Grammar* grammar) {
	timing::Timer t("Sequence::updateStages");

	if (grammar->lastChanged() > _lastChecked || _stages.size() != _text.size()) {
		clearStages();

		const Group* stageStart;
		Context context(this, grammar);

		FlowState flowState[MAX_DANCERS];

		stageStart = Group::home;
		status = SEQ_UNCHECKED;
		for (int i = 0; i < _text.size(); i++) {
			Stage* stage = new Stage(this, stageStart);
			_stages.push_back(stage);
			context.startStage(stage);
			if (_text[i].size() == 0)
				stage->setFinal(stageStart);	// A no-op call (empty text) ends where it starts.
			else {
				const Anything* c = context.grammar()->parse(null, _text[i], false, null, &context, null);
				if (c) {
					Level x = c->designatorLevel();
					if (_level > NO_LEVEL &&
						x > _level) {
						stage->fail(stage->newExplanation(USER_ERROR, "Some dancer designation phrase is off-level (" + levels[x] + ")"));
						status = SEQ_FAILED;
						if (anyVerbose())
							printf("    *** Designator off-level ***\n\n");
						continue;
					}
					stage->setCall(c);
					if (anyVerbose())
						printf("  %4d:   %s\n", i + 1, _text[i].c_str());
					stage->perform(null, &context, TILE_ALL);
					stage->breathe(&context);
					context.endStage();

					if (!stage->failed()) {
						const Group* endOfStage = stage->final();
						if (endOfStage == null)
							stage->fail(stage->newExplanation(PROGRAM_BUG, "Unexpected null final value"));
						else {
							stage->collectMotions();
							stage->checkFlow(flowState);
							if (verboseOutput)
								stage->print();
							if (!stage->failed()) {
								stageStart = endOfStage;
								continue;
							}
						}
					}
					status = SEQ_FAILED;
					if (verboseOutput) {
						printf("    *** Call failed ***\n");
						stage->print();
					}
				} else {
					if (anyVerbose())
						printf("  %4d: ? %s\n", i + 1, _text[i].c_str());
					stage->fail(stage->newExplanation(USER_ERROR, "Call was not recognized"));
					status = SEQ_FAILED;
				}
				if (anyVerbose())
					printf("\n");
			}
		}
		if (status != SEQ_FAILED) {
			if (_stages.size() > 0 && 
				_stages[_stages.size() - 1]->final()->atHome())
				status = SEQ_READY;
			else
				status = SEQ_UNRESOLVED;
		}
		_lastChecked.touch();
		return true;
	} else
		return false;
}

void Sequence::print() {
	string s;

	s.localTime(created, "%A %m/%d/%y %I:%M:%S %p");
	printf("    ");
	if (_level)
		printf("Level %s ", levels[_level].c_str());
	else
		printf("Level unknown ");
	printf("Duration %d beats\n", duration());
	printf("    Created on %s\n", s.c_str());
	if (createdWith.size())
		printf("    Created with: %s\n", createdWith.c_str());
	if (comment.size())
		printf("    %s\n", comment.c_str());
	for (int i = 0; i < _text.size(); i++)
		printf("  %4d: %s\n", i + 1, _text[i].c_str());
	printf("\n");
}

void Sequence::write(FILE* fp) {
	fprintf(fp, "S%I64d\n", created);
	if (modified)
		fprintf(fp, "M%I64d\n", modified);
	fprintf(fp, "X%I64d\n", _lastChecked.value());
	fprintf(fp, "s%c\n", status + 'a');
	fprintf(fp, " %s\n", levels[_level].c_str());
	fprintf(fp, "/%s\n", comment.escapeC().c_str());
	fprintf(fp, "!%s\n", createdWith.c_str());
	for (int i = 0; i < _text.size(); i++)
		if (_text[i].size() || (i < _notes.size() && _notes[i].size()))
			fprintf(fp, ".%s\n", _text[i].c_str());
	for (int i = 0; i < _notes.size(); i++)
		if (_text[i].size() || _notes[i].size())
			fprintf(fp, "#%s\n", _notes[i].escapeC().c_str());
}

bool Sequence::resolved() const {
	if (_stages.size())
		return _stages[_stages.size() - 1]->resolved();
	else
		return false;
}

beats Sequence::durationThru(int index) const {
	if (index > _stages.size())
		index = _stages.size();
	beats dur = 0;
	for (int i = 0; i < index; i++) {
		if (_stages[i])
			dur += _stages[i]->duration();
	}
	return dur;
}

double rotationAngle(Rotation r) {
	return r * PI / 16;
}
/*
 *	angle
 *
 *	This calculates the angle theta, in radians, formed be the vector [x,y] with the X axis.
 *
 *  The angle lies between -PI and PI.
 */
double angle(double x, double y) {
	double hypot = sqrt(x * x + y * y);
	if (x > 0)
		return asin(y / hypot);
	else if (y > 0)
		return acos(x / hypot);
	else
		return acos(-x / hypot) - PI;
}

Facing mirrorFacing(Facing facing) {
	switch (facing) {
	case	LEFT_FACING:
		return RIGHT_FACING;

	case	RIGHT_FACING:
		return LEFT_FACING;

	default:
		return facing;
	}
}

Facing quarterLeft(Facing facing, int amount) {
	switch (facing) {
	case	HEAD_FACING:
		if (amount & 1)
			return SIDE_FACING;
		else
			return HEAD_FACING;

	case	SIDE_FACING:
		if (amount & 1)
			return HEAD_FACING;
		else
			return SIDE_FACING;

	case	ANY_FACING:
		return ANY_FACING;

	default:
		return Facing((facing + amount) & 3);
	}
}

Facing quarterRight(Facing facing, int amount) {
	return quarterLeft(facing, (4 - amount) & 3);
}

Facing reverse(Facing facing) {
	static Facing rev[] = { 
		LEFT_FACING,	// RIGHT_FACING,	// > caller's right side of hall
		FRONT_FACING,	// BACK_FACING,	// ^ back of hall
		RIGHT_FACING,	// LEFT_FACING,	// < caller's left side of hall
		BACK_FACING,	// FRONT_FACING,	// v front of hall
		HEAD_FACING,	// HEAD_FACING,	// | front or back
		SIDE_FACING,	// SIDE_FACING,	// - left or right
		ANY_FACING,		// ANY_FACING,		// ? any value
	};

	return rev[facing];
}

bool ambiguous(Facing facing) {
	switch (facing) {
	case	HEAD_FACING:
	case	SIDE_FACING:
	case	ANY_FACING:
		return true;

	default:
		return false;
	}
}

int oppositeCouple(const Sequence* sequence, int couple) {
	static int o2_and_4[] = { 0, 3, 4, 1, 2 };
	static int o6[] = { 0, 4, 5, 6, 1, 2, 3 };

	switch (sequence->danceType()) {
	default:
	case	D_2COUPLE:
	case	D_4COUPLE:
		return o2_and_4[couple];

	case	D_6COUPLE:
	case	D_HEXAGONAL:
		return o6[couple];
	}
}

Dance::Dance(const string& label, const string& filename) {
	_label = label;
	_filename = filename;
	_levelName = "*";					// Magic 'name' of the highest level
	_level = NO_LEVEL;					// Dances default to unspecified level
	_danceType = D_UNSPECIFIED;			// Dances default to unspecified type (2 couple, 4 cou0ple, 6 couple, hex, etc.)
}

Dance::~Dance() {
	_sequences.deleteAll();
}

bool Dance::read() {
	FILE* fp = fileSystem::openTextFile(_filename);
	if (fp == null)
		return false;
	string contents;
	bool result = fileSystem::readAll(fp, &contents);
	fclose(fp);
	if (!result)
		return result;
	vector<string> lines;
	contents.split('\n', &lines);
	Sequence* sequence = null;
	for (int i = 0; i < lines.size(); i++) {
		fileSystem::TimeStamp t;

		if (lines[i].size() == 0)
			continue;
		switch (lines[i][0]) {
		case	'=':
			_levelName = lines[i].substr(1);
			_level = *levelValues.get(_levelName);
			break;

		case	'S':
			sequence = newSequence();
			sequence->created = parseLongLong(lines[i], 1);
			break;

		case	'M':
			sequence->modified = parseLongLong(lines[i], 1);
			break;

		case	'X':
			t.setValue(parseLongLong(lines[i], 1));
			sequence->setLastChecked(t);
			break;

		case	's':
			sequence->status = (SequenceStatus)(lines[i][1] - 'a');
			break;

		case	' ':
			sequence->setLevelName(lines[i].substr(1));
			break;

		case	'/':
			if (!lines[i].substr(1).unescapeC(&sequence->comment))
				return false;
			break;

		case	'!':
			sequence->createdWith = lines[i].substr(1);
			break;

		case	'.':
			sequence->append(lines[i].substr(1));
			break;

		case	'#': {
			string notes;

			if (!lines[i].substr(1).unescapeC(&notes))
				return false;
			sequence->appendNotes(notes);
			break;
		}

		default:
			return false;
		}
	}
	return true;
}

Sequence* Dance::newSequence() {
	Sequence* s = new Sequence(this);
	_sequences.push_back(s);
	return s;
}

PlayList* Dance::newPlayList() {
	PlayList* p = new PlayList(this);
	_playLists.push_back(p);
	return p;
}

void Dance::append(Sequence* sequence) {
	_sequences.push_back(sequence);
}

bool Dance::runAll(bool allowUnresolved, const Grammar* grammar) {
	bool result = true;
	bool failed = false;
	for (int i = 0; i < _sequences.size(); i++) {
		Sequence* seq = _sequences[i];
		if (!seq->run(allowUnresolved, grammar)) {
			for (int j = 0; j < seq->stages().size(); j++) {
				const Stage* stage = seq->stages()[j];

				if (stage->failed()) {
					printf(" [%d:%d] %s\n", i + 1, j, stage->cause()->text().c_str());
					failed = true;
				}
			}
			if (!failed) {
				if (!seq->resolved())
					printf(" [%d] Not resolved\n", i + 1);
			} else
				printf("**** Sequence failed ****\n");
			result = false;
		}
	}
	if (verboseOutput) {
		const vector<const Pattern*>& ce = grammar->centersEnds();
		printf("Centers-ends formations:\n");
		for (int i = 0; i < ce.size(); i++)
			ce[i]->formation()->write(stdout);
	}
	return result;
}

bool Dance::save() {
	FILE* fp = fileSystem::createTextFile(_filename);
	if (fp == null)
		return false;
	fprintf(fp, "=%s\n", levels[_level].c_str());
	for (int i = 0; i < _sequences.size(); i++)
		_sequences[i]->write(fp);
	fclose(fp);
	return ferror(fp) == 0;
}

void Dance::print() {
	for (int i = 0; i < _sequences.size(); i++) {
		printf("Sequence %d:\n", i + 1);
		_sequences[i]->print();
	}
}

void Dance::setFilename(const string& filename) {
	_filename = filename;
	_label = fileSystem::basename(filename);
}

Pattern* Pattern::parse(const Grammar* grammar, const string& expression) {
	int pos = expression.find('(');
	if (pos == string::npos) {
		const Formation* f = grammar->formation(expression);
		if (f)
			return new Pattern(f, "");
	} else {
		if (expression[expression.size() - 1] == ')') {
			string form = expression.substr(0, pos);
			const Formation* f = grammar->formation(form);
			if (f)
				return new Pattern(f, expression.substr(pos + 1));
		}
	}
	return null;
}

bool PatternClosure::designates(const Dancer *dancer) const {
	// If the pattern did not include parameters, we do not need to
	// check anything further.
	if (_pattern->parameterList().size() == 0)
		return true;
	const Group* designeeReference;
	if (_step)
		designeeReference = _step->start();
	else
		designeeReference = _context->stage()->start();
	if (_designatorTerm == null) {
		_designatorTerm = _context->grammar()->parseDesignatingPattern(designeeReference, _pattern->parameterList(), true, _call, _context);
		if (_designatorTerm == null)
			return false;
	}
	unsigned mask = _designatorTerm->match(designeeReference, _step, _context);
	if (mask == 0)
		return false;
	if (mask & dancer->dancerMask())
		return true;
	else
		return false;
}

const Transform* Transform::translate(int offsetX, int offsetY) {
	Transform* t = new Transform(1, 0, offsetX, 0, 1, offsetY);
	t->_allocated = true;
	return t;
}

void Transform::apply(int *x, int *y, Facing* facing) const {
	if (x && y) {
		int nx = _x0 * *x + _x1 * *y + _x2;
		int ny = _y0 * *x + _y1 * *y + _y2;
		*x = nx;
		*y = ny;
	}
	if (facing) {
		if (isMirror())
			*facing = mirrorFacing(*facing);
		else
			*facing = quarterLeft(*facing, leftQuarterTurns());
	}
}

void Transform::apply(float *x, float *y, Facing* facing) const {
	if (x && y) {
		float nx = _x0 * *x + _x1 * *y + _x2;
		float ny = _y0 * *x + _y1 * *y + _y2;
		*x = nx;
		*y = ny;
	}
	if (facing) {
		if (isMirror())
			*facing = mirrorFacing(*facing);
		else
			*facing = quarterLeft(*facing, leftQuarterTurns());
	}
}

void Transform::revert(int *x, int *y, Facing* facing) const {
	int denom = _x0 * _y1 - _x1 * _y0;
	if (denom == 0) {
		*x = INT_MAX;
		*y = INT_MAX;
		if (facing)
			*facing = ANY_FACING;
	} else {
		int translate_x = *x - _x2;
		int translate_y = *y - _y2;
		if (translate_x < -30)
			translate_x = -30;
		if (translate_y < -30)
			translate_y = -30;
		*x = _y1 * translate_x / denom - _x1 * translate_y / denom;
		*y = -_y0 * translate_x / denom + _x0 * translate_y / denom;
		if (facing) {
			if (isMirror())
				*facing = mirrorFacing(*facing);
			else
				*facing = quarterRight(*facing, leftQuarterTurns());
		}
	}
}

void Transform::revert(float *x, float *y, Facing* facing) const {
	int denom = _x0 * _y1 - _x1 * _y0;
	if (denom == 0) {
		*x = INT_MAX;
		*y = INT_MAX;
		if (facing)
			*facing = ANY_FACING;
	} else {
		double translate_x = *x - _x2;
		double translate_y = *y - _y2;
		*x = _y1 / denom * translate_x - _x1 / denom * translate_y;
		*y = -_y0 / denom * translate_x + _x0 / denom * translate_y;
		if (facing) {
			if (isMirror())
				*facing = mirrorFacing(*facing);
			else
				*facing = quarterRight(*facing, leftQuarterTurns());
		}
	}
}

void Transform::revert(double* angle) const {
	if (isMirror())
		*angle = -*angle;
}

const Dancer* Transform::apply(const Dancer* source) const {
	Dancer* d = new Dancer(source->x, source->y, source->facing, source->gender, source->couple, source->dancerIndex());
	apply(&d->x, &d->y, &d->facing);
	return d;
}
// Note: Matrix inversion formula's taken from Wikipedia article
const Dancer* Transform::revert(const Dancer* source) const {
	Dancer* d = new Dancer(source->x, source->y, source->facing, source->gender, source->couple, source->dancerIndex());
	revert(&d->x, &d->y, &d->facing);
	return d;
}

const Transform* Transform::clone() const {
	if (this == null)
		return null;
	if (_allocated) {
		Transform* t = new Transform(_x0, _x1, _x2, _y0, _y1, _y2);
		t->_allocated = true;
		return t;
	} else
		return this;
}

void Transform::dispose() const {
	if (this != null) {
		if (_allocated)
			delete this;
	}
}

int Dancer::compare(const Dancer* other) const {
	if (y > other->y)
		return -1;
	if (y < other->y)
		return 1;
	if (x < other->x)
		return -1;
	if (x > other->x)
		return 1;
	return 0;
}

bool Dancer::identical(const Dancer* other) const {
	if (compare(other) != 0)
		return false;
	if (facing != other->facing ||
		gender != other->gender ||
		couple != other->couple ||
		_dancerIndex != other->_dancerIndex)
		return false;
	else
		return true;
}

bool Dancer::opposite(const Sequence* sequence, const Dancer* other) const {
	return x == -other->x &&
		   y == -other->y &&
		   facing == reverse(other->facing) &&
		   gender == other->gender &&
		   couple == oppositeCouple(sequence, other->couple);
}

bool Dancer::matches(const Spot& spot, const PatternClosure* closure) const {
	switch (spot.position) {
	case	ACTIVE_BOY:
		if (gender == GIRL)
			return false;
		break;

	case	ACTIVE_GIRL:
		if (gender == BOY)
			return false;
		break;

	case	ACTIVE_DESIGNATED:
		if (closure != null && closure->discriminates() && !closure->designates(this))
			return false;
		break;

	case	ACTIVE_NONDESIGNATED:
		if (closure != null && closure->discriminates() && closure->designates(this))
			return false;
		break;
	}
	switch (spot.facing) {
	case	ANY_FACING:
		return true;

	case	HEAD_FACING:
		if (facing == BACK_FACING ||
			facing == FRONT_FACING ||
			facing == HEAD_FACING ||
			facing == ANY_FACING)			// ANY_FACING applies to phantoms
			return true;
		break;

	case	SIDE_FACING:
		if (facing == LEFT_FACING ||
			facing == RIGHT_FACING ||
			facing == SIDE_FACING ||
			facing == ANY_FACING)			// ANY_FACING applies to phantoms
			return true;
		break;

	default:
		if (facing == spot.facing)
			return true;
	}
	return false;
}

double round(double x) {
	return floor(x + 0.5);
}

const Dancer* Dancer::arc(Geometry geometry, Point center, double radius, int rightSixteenthTurns, double angleAdjust, int noseQuarterTurns, Interval* interval, Context* context) const {
	double deltaX = x - center.x;
	double deltaY = y - center.y;
	double sweepAngle = -rightSixteenthTurns * PI / 8;
	int newX;
	int newY;
	if (abs(deltaX) < EPSILON && abs(deltaY) < EPSILON) {
		newX = x;
		newY = y;
	} else {
		double startAngle = angle(deltaX, deltaY);
		newX = round(center.x + radius * cos(startAngle + sweepAngle));
		newY = round(center.y + radius * sin(startAngle + sweepAngle));
	}
	int rightQuarterTurns;
	if (rightSixteenthTurns >= 0)
		rightQuarterTurns = (rightSixteenthTurns + 3) / 4;
	else
		rightQuarterTurns = -(-rightSixteenthTurns + 3) / 4;
	beats duration = abs(rightQuarterTurns);
	if (radius > 2)
		duration *= 2;
	if (duration == 0)
		duration = 1;
	interval->arc(this, center, radius, sweepAngle + angleAdjust, noseQuarterTurns, duration, context);
	return new Dancer(newX, newY, quarterRight(facing, rightQuarterTurns + noseQuarterTurns), gender, couple, _dancerIndex);
}

const Dancer* Dancer::forwardVeer(int amount, int veer, int rightQuarterTurns, Interval* interval, Context* context) const {
	int deltaX, deltaY;

	displace(amount, veer, &deltaX, &deltaY);
	int newX = x + deltaX;
	int newY = y + deltaY;
	beats dur = abs(amount);
	beats vdur = abs(veer);
	if (vdur > dur)
		dur = vdur;
	interval->forwardVeer(this, Point(deltaX, deltaY), 0, 0, rightQuarterTurns, dur, context); 
	return new Dancer(newX, newY, quarterRight(facing, rightQuarterTurns), gender, couple, _dancerIndex);
}

const Dancer* Dancer::face(int amount, Interval* interval, Context* context) const {
	Facing newFacing;

	if (amount > 0)
		newFacing = quarterLeft(facing, amount);
	else if (amount < 0)
		newFacing = quarterRight(facing, -amount);
	else
		newFacing = facing;
	interval->arc(this, Point(x, y), 0, amount * PI / 2, 0, 1, context);
	Dancer* d = new Dancer(x, y, newFacing, gender, couple, _dancerIndex);
	return d;
}

const Dancer* Dancer::displace(int deltaX, int deltaY, Interval* interval, Context* context) const {
	interval->also(this, deltaX, deltaY, context);
	return new Dancer(x + deltaX, y + deltaY, facing, gender, couple, _dancerIndex);
}

void Dancer::displace(int forward, int lateral, int* x, int* y) const {
	switch (facing) {
	case	RIGHT_FACING:
		*y = -lateral;
		*x = forward;
		break;

	case	BACK_FACING:
		*y = forward;
		*x = lateral;
		break;

	case	LEFT_FACING:
		*y = lateral;
		*x = -forward;
		break;

	case	FRONT_FACING:
		*y = -forward;
		*x = -lateral;
		break;
	}
}

bool Dancer::adjacentY(const Dancer* other) const {
	if (other->y != y)
		return false;
	switch (facing) {
	case	FRONT_FACING:
	case	BACK_FACING:
		switch (other->facing) {
		case	FRONT_FACING:
		case	BACK_FACING:
			return true;
		}
	}
	return false;
}

bool Dancer::adjacentX(const Dancer* other) const {
	if (other->x != x)
		return false;
	switch (facing) {
	case	RIGHT_FACING:
	case	LEFT_FACING:
		switch (other->facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			return true;
		}
	}
	return false;
}

int Dancer::quarterTurnsTo(const Anydirection* dir, int targetX, int targetY) const {
	switch (facing) {
	case	FRONT_FACING:
		if (dir->direction() == D_IN) {
			if (x < targetX)
				return 1;
			else if (x > targetX)
				return -1;
		} else {
			if (x < targetX)
				return -1;
			else if (x > targetX)
				return 1;
		}
		break;

	case	BACK_FACING:
		if (dir->direction() == D_IN) {
			if (x < targetX)
				return -1;
			else if (x > targetX)
				return 1;
		} else {
			if (x < targetX)
				return 1;
			else if (x > targetX)
				return -1;
		}
		break;

	case	LEFT_FACING:
		if (dir->direction() == D_IN) {
			if (y < targetY)
				return -1;
			else if (y > targetY)
				return 1;
		} else {
			if (y < targetY)
				return 1;
			else if (y > targetY)
				return -1;
		}
		break;

	case	RIGHT_FACING:
		if (dir->direction() == D_IN) {
			if (y < targetY)
				return 1;
			else if (y > targetY)
				return -1;
		} else {
			if (y < targetY)
				return -1;
			else if (y > targetY)
				return 1;
		}
		break;
	}
	return 0;
}

void Dancer::calculateArcParameters(const Group* container, 
									const Anypivot* center, 
									const Anydirection* direction, 
									Point* centerPoint, 
									double* radius, 
									Rotation newRotation, 
									int* rightSixteenthTurns, 
									double* angleAdjust,
									Interval* motionData) const {
	*radius = -1;
	int rad;
	switch (center->pivot()) {
	case	P_CENTER: {
		int specialAdjust;
		if (*rightSixteenthTurns > 0)
			specialAdjust = (*rightSixteenthTurns % 4);
		else
			specialAdjust = -((-*rightSixteenthTurns) % 4);
		switch (direction->direction()) {
		case	D_FORWARD:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		switch (facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			if (container->geometry() == RING) {
				if (facing == LEFT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else {
				if (y < 0) {
					if (facing == RIGHT_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
				} else if (y > 0) {
					if (facing == LEFT_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
				} else
					return;
			}
			break;

		case	BACK_FACING:
		case	FRONT_FACING:
			if (container->geometry() == RING) {
				// You can't arc around the center in RING geometry when you are facing FRONT or BACK (which are away from/toward center)
				return;
			} else {
				if (x < 0) {
					if (facing == FRONT_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
				} else if (x > 0) {
					if (facing == BACK_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
				} else
					return;
			}
			break;
		}

		// This allows for 1/8th and 1/16th turns around the center (compensated already with newRotation

		*rightSixteenthTurns += specialAdjust;
		*angleAdjust = rotationAngle(Rotation(newRotation - container->rotation()));
		centerPoint->x = 0;
		centerPoint->y = 0;
		container->convertFromAbsolute(&centerPoint->x, &centerPoint->y);
		double dx = x - centerPoint->x;
		double dy = y - centerPoint->y;
		*radius = sqrt(dx * dx + dy * dy);
		break;
	}

	case	P_SPLIT_CENTER: {
		if (container->rotation() != newRotation)
			return;
		if (container->geometry() == RING)
			return;
		int sx = x;
		int sy = y;
		if (!container->computeSplitCenter(&sx, &sy))
			return;
		switch (direction->direction()) {
		case	D_FORWARD:
		case	D_LEFT:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		switch (facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			if (y < sy) {
				if (facing == RIGHT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else if (y > sy) {
				if (facing == LEFT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else
				return;
			break;

		case	BACK_FACING:
		case	FRONT_FACING:
			if (x < sx) {
				if (facing == FRONT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else if (x > sx) {
				if (facing == BACK_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else
				return;
			break;
		}
		int dx = sx - x;
		int dy = sy - y;
		*radius = sqrt(double(dx * dx + dy * dy));
		break;
	}
	case	P_LINE_CENTER:
	case	P_BOX_CENTER: {
		if (container->rotation() != newRotation)
			return;
		if (container->geometry() == RING)
			return;
		centerPoint->x = 0;
		centerPoint->y = 0;
		switch (direction->direction()) {
		case	D_FORWARD:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		case	D_RIGHT:
			*rightSixteenthTurns = -*rightSixteenthTurns;
		case	D_LEFT:
			*radius = sqrt(double(x * x + y * y));
			return;

		default:
			return;
		}
		switch (facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			if (center->pivot() == P_LINE_CENTER) {
				centerPoint->x = x;
				*radius = abs(y);
			}
			if (y < 0) {
				if (facing == RIGHT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else if (y > 0) {
				if (facing == LEFT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else
				return;
			break;

		case	BACK_FACING:
		case	FRONT_FACING:
			if (center->pivot() == P_LINE_CENTER) {
				centerPoint->y = y;
				*radius = abs(x);
			}
			if (x < 0) {
				if (facing == FRONT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else if (x > 0) {
				if (facing == BACK_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
			} else
				return;
			break;
		}
		if (center->pivot() == P_BOX_CENTER)
			*radius = sqrt(double(x * x + y * y));
		break;
	}

	case	P_INSIDE_SHOULDER:
	case	P_INSIDE_HAND:
	case	P_INSIDE_DANCER: {
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_FORWARD:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		double rad;
		if (center->pivot() == P_INSIDE_HAND)
			rad = 1;
		else if (center->pivot() == P_INSIDE_SHOULDER)
			rad = 0.5;
		else
			rad = 2;
		switch (facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			centerPoint->x = x;
			if (y < 0) {
				if (facing == RIGHT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				centerPoint->y = y + rad;
			} else if (y > 0) {
				if (facing == LEFT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				centerPoint->y = y - rad;
			} else
				return;
			break;

		case	BACK_FACING:
		case	FRONT_FACING:
			centerPoint->y = y;
			if (x < 0) {
				if (facing == FRONT_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				centerPoint->x = x + rad;
			} else if (x > 0) {
				if (facing == BACK_FACING)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				centerPoint->x = x - rad;
			} else
				return;
			break;

		default:
			return;
		}
		*radius = rad;
		break;
	}
	case	P_OUTSIDE_HAND:
	case	P_OUTSIDE_DANCER:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_FORWARD:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		if (center->pivot() == P_OUTSIDE_HAND)
			rad = 1;
		else
			rad = 2;
		switch (facing) {
		case	RIGHT_FACING:
		case	LEFT_FACING:
			if (container->geometry() == RING) {
				return;
			} else {
				centerPoint->x = x;
				if (y < 0) {
					if (facing == LEFT_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
					centerPoint->y = y - rad;
				} else if (y > 0) {
					if (facing == RIGHT_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
					centerPoint->y = y + rad;
				} else {
					float cx = 0;
					float cy = 0;
					container->convertFromAbsolute(&cx, &cy);
					if (0 < cy) {
						if (facing == LEFT_FACING)
							*rightSixteenthTurns = -*rightSixteenthTurns;
						centerPoint->y = y - rad;
					} else if (0 > cy) {
						if (facing == RIGHT_FACING)
							*rightSixteenthTurns = -*rightSixteenthTurns;
						centerPoint->y = y + rad;
					} else
						return;
				}
			}
			break;

		case	BACK_FACING:
		case	FRONT_FACING:
			if (container->geometry() == RING) {
				return;
			} else {
				centerPoint->y = y;
				if (x < 0) {
					if (facing == BACK_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
					centerPoint->x = x - rad;
				} else if (x > 0) {
					if (facing == FRONT_FACING)
						*rightSixteenthTurns = -*rightSixteenthTurns;
					centerPoint->x = x + rad;
				} else {
					float cx = 0;
					float cy = 0;
					container->convertFromAbsolute(&cx, &cy);
					if (0 < cx) {
						if (facing == BACK_FACING)
							*rightSixteenthTurns = -*rightSixteenthTurns;
						centerPoint->x = x - rad;
					} else if (0 > cx) {
						if (facing == FRONT_FACING)
							*rightSixteenthTurns = -*rightSixteenthTurns;
						centerPoint->x = x + rad;
					} else
						return;
				}
			}
			break;

		default:
			return;
		}
		*radius = rad;
		break;

	case P_LAST_HAND: {
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_FORWARD:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		Motion* m = motionData->lastCurve(this, false);
		if (m == null)
			return;
		double d = m->cumulativeNoseMotion();
		container->convertFromAbsolute(&d);
		if (d == 0) {
			if (verboseOutput) {
				printf("lastCurve was 0:\n");
				motionData->print(4);
			}
			return;
		}
		if (d > 0)
			*rightSixteenthTurns = -*rightSixteenthTurns;		// Positive angle is a left turn
		switch (facing) {
		case	RIGHT_FACING:
			if (container->geometry() == RING) {
				return;
			} else {
				centerPoint->x = x;
				if (d > 0)
					centerPoint->y = y + 1;
				else
					centerPoint->y = y - 1;
			}
			break;

		case	LEFT_FACING:
			if (container->geometry() == RING) {
				return;
			} else {
				centerPoint->x = x;
				if (d > 0)
					centerPoint->y = y - 1;
				else
					centerPoint->y = y + 1;
			}
			break;

		case	BACK_FACING:
			if (container->geometry() == RING) {
				return;
			} else {
				centerPoint->y = y;
				if (d > 0)
					centerPoint->x = x - 1;
				else
					centerPoint->x = x + 1;
			}
			break;

		case	FRONT_FACING:
			if (container->geometry() == RING) {
				return;
			} else {
				centerPoint->y = y;
				if (d > 0)
					centerPoint->x = x + 1;
				else
					centerPoint->x = x - 1;
			}
			break;
		}
		*radius = 1;
		break;
	}

	case	P_HAND:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_LEFT:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		case	D_RIGHT:
			break;

		default:
			return;
		}
		switch (facing) {
		case	RIGHT_FACING:
			centerPoint->x = x;
			if (direction->direction() == D_LEFT)
				centerPoint->y = y + 1;
			else
				centerPoint->y = y - 1;
			break;

		case	LEFT_FACING:
			centerPoint->x = x;
			if (direction->direction() == D_LEFT)
				centerPoint->y = y - 1;
			else
				centerPoint->y = y + 1;
			break;

		case	BACK_FACING:
			centerPoint->y = y;
			if (direction->direction() == D_LEFT)
				centerPoint->x = x - 1;
			else
				centerPoint->x = x + 1;
			break;

		case	FRONT_FACING:
			centerPoint->y = y;
			if (direction->direction() == D_LEFT)
				centerPoint->x = x + 1;
			else
				centerPoint->x = x - 1;
			break;
		}
		*radius = 1;
		break;

	case	P_NOSE:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_LEFT:
			break;

		case	D_RIGHT:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		switch (facing) {
		case	RIGHT_FACING:
			centerPoint->y = y;
			centerPoint->x = x + 1;
			break;

		case	LEFT_FACING:
			centerPoint->y = y;
			centerPoint->x = x - 1;
			break;

		case	BACK_FACING:
			centerPoint->x = x;
			centerPoint->y = y + 1;
			break;

		case	FRONT_FACING:
			centerPoint->x = x;
			centerPoint->y = y - 1;
			break;
		}
		*radius = 1;
		break;

	case	P_TAIL:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_LEFT:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		case	D_RIGHT:
			break;

		default:
			return;
		}
		switch (facing) {
		case	RIGHT_FACING:
			centerPoint->y = y;
			centerPoint->x = x - 1;
			break;

		case	LEFT_FACING:
			centerPoint->y = y;
			centerPoint->x = x + 1;
			break;

		case	BACK_FACING:
			centerPoint->x = x;
			centerPoint->y = y - 1;
			break;

		case	FRONT_FACING:
			centerPoint->x = x;
			centerPoint->y = y + 1;
			break;
		}
		*radius = 1;
		break;

	case	P_LEFT_HAND:
	case	P_LEFT_DANCER:
	case	P_LEFT_TWO_DANCERS:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_FORWARD:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		case	D_BACK:
			break;

		default:
			return;
		}
		if (center->pivot() == P_LEFT_HAND)
			*radius = 1;
		else if (center->pivot() == P_LEFT_DANCER)
			*radius = 2;
		else
			*radius = 3;
		switch (facing) {
		case	RIGHT_FACING:
			centerPoint->x = x;
			centerPoint->y = y + *radius;
			break;

		case	LEFT_FACING:
			centerPoint->x = x;
			centerPoint->y = y - *radius;
			break;

		case	BACK_FACING:
			centerPoint->y = y;
			centerPoint->x = x - *radius;
			break;

		case	FRONT_FACING:
			centerPoint->y = y;
			centerPoint->x = x + *radius;
			break;
		}
		break;

	case	P_RIGHT_HAND:
	case	P_RIGHT_DANCER:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_FORWARD:
			break;

		case	D_BACK:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		default:
			return;
		}
		if (center->pivot() == P_RIGHT_HAND)
			*radius = 1;
		else
			*radius = 2;
		switch (facing) {
		case	RIGHT_FACING:
			centerPoint->x = x;
			centerPoint->y = y - *radius;
			break;

		case	LEFT_FACING:
			centerPoint->x = x;
			centerPoint->y = y + *radius;
			break;

		case	BACK_FACING:
			centerPoint->y = y;
			centerPoint->x = x + *radius;
			break;

		case	FRONT_FACING:
			centerPoint->y = y;
			centerPoint->x = x - *radius;
			break;
		}
		break;

	case	P_SELF:
		if (container->rotation() != newRotation)
			return;
		switch (direction->direction()) {
		case	D_LEFT:
			*rightSixteenthTurns = -*rightSixteenthTurns;
			break;

		case	D_RIGHT:
			break;

		case	D_IN:
			switch (facing) {
			case	RIGHT_FACING:
				if (y < 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;

			case	LEFT_FACING:
				if (y > 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;

			case	BACK_FACING:
				if (x > 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;

			case	FRONT_FACING:
				if (x < 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;
			}
			break;

		case	D_OUT:
			switch (facing) {
			case	RIGHT_FACING:
				if (y > 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;

			case	LEFT_FACING:
				if (y < 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;

			case	BACK_FACING:
				if (x < 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;

			case	FRONT_FACING:
				if (x > 0)
					*rightSixteenthTurns = -*rightSixteenthTurns;
				break;
			}
			break;

		default:
			return;
		}
		centerPoint->x = x;
		centerPoint->y = y;
		*radius = 0;
		break;

	default:
		break;
	}
}

const Dancer* Dancer::clone() const {
	return Transform::identity.apply(this);
}

const Dancer* Dancer::cloneToCoordinates(const Group* base, const Group* coordinateSystem) const {
	Dancer* d = new Dancer(x, y, facing, gender, couple, _dancerIndex);
	base->convertToLocal(coordinateSystem, d);
	return d;
}

const Dancer* Dancer::cloneOffsetX(int amount) const {
	return new Dancer(x + amount, y, facing, gender, couple, _dancerIndex);
}

int Dancer::leftTurnsNeededToFace(Geometry geometry, const Dancer* other) const {
	switch (relativeLocationOf(geometry, other)) {
	default:
		return INT_MAX;

	case	LEFT_OF:
		return 1;

	case	RIGHT_OF:
		return -1;

	case	IN_FRONT:
		return 0;

	case	BEHIND:
		return 2;
	}
}

SpatialRelation Dancer::relativeLocationOf(Geometry geometry, const Dancer* other) const {
	if (x == other->x) {
		int myY = y;
		int otherY = other->y;
		if (geometry == RING) {
			if (myY == 14 && otherY == 0)
				myY = -2;
			if (otherY == 14 && myY == 0)
				otherY = -2;
		}
		if (abs(myY - otherY) != 2)
			return OTHER;
		switch (facing) {
		case	RIGHT_FACING:
			if (myY < otherY)
				return LEFT_OF;
			else
				return RIGHT_OF;

		case	LEFT_FACING:
			if (myY < otherY)
				return RIGHT_OF;
			else
				return LEFT_OF;

		case	FRONT_FACING:
			if (myY < otherY)
				return BEHIND;
			else
				return IN_FRONT;

		case	BACK_FACING:
			if (myY < otherY)
				return IN_FRONT;
			else
				return BEHIND;
		}
	} else if (y == other->y) {
		int myX = x;
		int otherX = other->x;
		if (geometry == RING) {
			if (myX == 14 && otherX == 0)
				myX = -2;
			if (otherX == 14 && myX == 0)
				otherX = -2;
		}
		if (abs(myX - otherX) != 2) {
			return OTHER;
		}
		switch (facing) {
		case	RIGHT_FACING:
			if (myX < otherX)
				return IN_FRONT;
			else
				return BEHIND;

		case	LEFT_FACING:
			if (myX < otherX)
				return BEHIND;
			else
				return IN_FRONT;

		case	FRONT_FACING:
			if (myX < otherX)
				return LEFT_OF;
			else
				return RIGHT_OF;

		case	BACK_FACING:
			if (myX < otherX)
				return RIGHT_OF;
			else
				return LEFT_OF;
		}
	}
	return OTHER;
}

void Dancer::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("(%d) #%d %s [%d,%d]%c\n", _dancerIndex, couple, genderNames[gender], x, y, facingGlyphs[facing]);
}

bool Pattern::match(const Group* dancers, const PatternClosure* closure) const {
	return _formation->match(dancers, closure);
}

unsigned Pattern::matchSome(const Group* dancers, int startWith, const PatternClosure* closure) const {
	return _formation->matchSome(dancers, startWith, closure);
}

Group* Pattern::matchWithPhantoms(const Group* dancers, const PatternClosure* closure) const {
	return _formation->matchWithPhantoms(dancers, closure);
}

void initializePrecedences() {
	precedences.push_back("Normal");
	precedences.push_back("Preferred");
}

bool loadLevels() {
	string levelsFile = getPreference("levels");

	if (levelsFile.size() == 0)
		levelsFile = global::dataFolder + "/dance/levels.txt";
	FILE* f = fileSystem::openTextFile(levelsFile);
	char buffer[1024];

	if (f == null)
		return false;
	levels.clear();
	levelValues.clear();
	levels.push_back("Error");
	// Make the default 'no level specified' be 1.
	levels.push_back("");
	levelValues.insert("", 1);
	while (fgets(buffer, sizeof buffer, f) != null) {
		// Strip any trailing newline.
		char* eol = strchr(buffer, '\n');
		if (eol)
			*eol = 0;
		char* comment = strchr(buffer, '/');
		if (comment)
			*comment = 0;
		string s(buffer);
		s = s.trim();
		levelValues.insert(s, levels.size());		// level numbering is 1 based.
		levels.push_back(s);
	}
	fclose(f);
	return true;
}

// This assumes a->y >= b->y (which would be true if a and b were taken
// from a Group object in their index order).
bool Rectangle::isBetween(const Dancer* a, const Dancer* b) const {
	if (bottom > a->y || top < b->y)
		return false;
	if (right < a->x || left > b->x)
		return false;
	if (bottom >= b->y && top <= a->y)
		return true;
	if (left >= a->x && right <= b->x)
		return true;
	return false;
}

bool Rectangle::coincident(const Rectangle& other) const {
	if (left + right != other.left + other.right)
		return false;
	return top + bottom == other.top + other.bottom;
}

bool Rectangle::overlaps(const Rectangle& other) const {
	if (left >= other.right ||
		right <= other.left)
		return false;
	if (top <= other.bottom ||
		bottom >= other.top)
		return false;
	return true;
}

bool Rectangle::intersectsHorizontally(int y) const {
	if (top <= y)
		return false;
	return bottom < y;
}

bool Rectangle::intersectsVertically(int x) const {
	if (right <= x)
		return false;
	return left < x;
}

void Rectangle::enclose(const Rectangle& r) {
	if (width() == 0 || height() == 0) {
		*this = r;
		return;
	} else if (r.width() == 0 || r.height() == 0)
		return;
	if (r.top > top)
		top = r.top;
	if (r.bottom < bottom)
		bottom = r.bottom;
	if (r.left < left)
		left = r.left;
	if (r.right > right)
		right = r.right;
}

void Rectangle::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("[ top=%d left=%d bottom=%d right=%d ]\n", top, left, bottom, right);
}

int Plane::compare(const Plane* other) const {
	int diff = before - other->before;
//	int diff = now - other->now;
	if (diff)
		return diff;

	// Make sure the center line sorts before all lesser edges on the same line
	// and after all greater edges on the same line
	if (owner == null || lesserEdge) {
		// Centers are less then lesser edges, and tile centers are less than the set center
		if (other->lesser || other->owner == null)
			return -1;
		else
			return 1;
	} else if (other->owner == null || other->lesserEdge) {
		// A lesser edge is greater than a center.
		if (lesser)
			return 1;
		else
			return -1;
	} else if (lesser)
		return 1;			// a lesser edge is greater than a greater edge
	else if (other->lesser)
		return -1;			// we must be a greater edge, so it will be less than a lesser edge
	return 0;
}

void Plane::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("before %2d now %2d after %2d afterWidth %2d ", before, now, after, afterWidth);
	if (owner == null)
		printf("---- Center line\n");
	else {
		if (lesserEdge)
			printf(" Center ");
		else if (lesser)
			printf(" Lesser edge ");
		else
			printf(" Greater edge ");
		printf("of tile %d\n", tile);
	}
}

__int64 parseLongLong(const string& s, int offset) {
	__int64 value = 0;
	if (offset >= s.size())
		return 0;
	bool negate = false;
	if (s[offset] == '-') {
		negate = true;
		offset++;
	}
	for (int i = offset; i < s.size(); i++) {
		if (isdigit(s[i]))
			value = value * 10 + (s[i] - '0');
		else
			break;
	}
	if (negate)
		value = -value;
	return value;
}

Gender gender(PositionType position) {
	switch (position) {
	case	ACTIVE_BOY:
		return BOY;

	case	ACTIVE_GIRL:
		return GIRL;

	default:
		return UNSPECIFIED_GENDER;
	}
}

}  // namespace dance
