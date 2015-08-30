#pragma once
#include "stored_data.h"

namespace dance {

class Interval;
class Motion;
class Plan;
class Step;
class Straight;
class Tile;

#define EPSILON 0.01

const int	NOT_AN_ANGLE = 1000000;

class FlowState {
public:
	int	turnState;			// 0 = neutral, < 0 = turning right, > 0 = turning left (magnitude >= 8 is possibly bad)
							// each 1/4 turn with radius = 1 (1/2 position), such as an arm turn increases magnitude by 2
	double	lastMotion;		// angle of last motion. (NOT_AN_ANGLE if dancer was not moving)

	FlowState() {
		turnState = 0;
		lastMotion = NOT_AN_ANGLE;
	}

	void idle(int dancerIndex, beats duration, Stage* stage);

	void turn(int dancerIndex, double angle, beats duration, double radius, Stage* stage);

	void straight(int dancerIndex, double angle, beats duration, Stage* stage);		// motion angle
};

const int EXCESSIVE_TURN = 12;		// 12 = 6 quarter turns in a row
const int OVER_TURN = 8;			// 8 = 4 quarter turns in a row, these should be followed by reverse turns to 'unwind'

class MotionSet {
public:
	MotionSet(bool startTogether);

	~MotionSet();

	bool validate(const Group* start, const Group* final) const;

	void checkFlow(FlowState* flowState, Stage* stage) const;

	Motion* lastCurve(int index, bool lastMotionOnly) const;

	void remember(MotionSet* ms);

	void remember(int dancerIndex, Motion* m);

	void replace(int dancerIndex, Motion* m);

	void also(int dancerIndex, Straight* m);

	void setRotation(int rotation) { _rotation = rotation; }

	beats duration() const;

	Motion* motion(int i) const;

	void print() const { printCommon(0); }

	int dancerCount() const { return _motions.size(); }

	bool startTogether() const { return _startTogether; }

	MotionSet* enclosing() const { return _enclosing; }

	bool remembered() const { return _remembered; }

protected:
	void printCommon(int indent) const;

	MotionSet* _enclosing;

	const vector<Straight*>& alsos() const { return _alsos; }

	vector<Motion*>& motions() { return _motions; }

private:
	vector<Straight*> _alsos;
	vector<Motion*> _motions;
	bool _startTogether;
	bool _remembered;
	int _rotation;
};
/*
 *	Interval
 *
 *	This identifies a set of motions by dancers.  The motions of each dancer is independent
 *	of the others and as soon as a given motion is completed, that dancer starts its next
 *	motion without waiting.
 *
 *	Each interval, however, must be finished before the next interval can start.
 */
class Interval : public MotionSet {
public:
	Interval(MotionSet* enclosing, bool startTogether) : MotionSet(startTogether) {
		_enclosing = enclosing;
	}

	~Interval() {
	}

	void print(int indent) const;

	void currentDancers(const Group* dancers) {
		_currentDancers = dancers;
	}

	void arc(const Dancer* dancer, Point center, double radius, double motionAngle, int noseQuarterTurns, beats duration, Context* context);

	void constructArc(const Dancer* dancer, const Point& center, const Point& start, const Point& end, double radius, double motionAngle, double startNose, double noseMotion, beats duration, Context* context);

	void forwardVeer(const Dancer* dancer, Point delta, float startYAdjust, double motionAngleAdjust, int rightQuarterTurns, beats duration, Context* context);

	void ringToSet(const Dancer* dancer, const Group* set, const Dancer* setDancer, beats duration, Context* context);

	void setToRing(const Dancer* dancer, const Group* ring, const Dancer* ringDancer, int ringX, int ringY, beats duration, Context* context);

	void trim(unsigned mask);

	void fillGaps(const Group* dancers, Context* context);

	void fillGaps(Motion* m, const Dancer* d, beats duration, Context* context);

	void also(const Dancer* dancer, int deltaX, int deltaY, Context* context);

	void displace(const Dancer* dancer, float startX, float startY, float deltaX, float deltaY, int rightQuarterTurns, double motionAngleAdjust, beats duration, Context* context);

	Motion* pause(Motion* previous, const Dancer* dancer, beats duration, Context* context);

	void perform(int dancerIndex, Motion* m);
	/*
	 *	lastCurve
	 *
	 *	lastMotionOnly:
	 *		false		Skip any straight motions to look for the last Curve available
	 *		true		Use the last motion of the dancer, Straight or Curve.
	 */
	Motion* lastCurve(const Dancer* dancer, bool lastMotionOnly) const;

	void matchTo(const Dancer* dancer, Context* context);

private:
	const Group* _currentDancers;
};

class Point {
public:
	Point(float x, float y) {
		this->x = x;
		this->y = y;
	}

	Point() {
		x = 0;
		y = 0;
	}

	double distance() const;

	bool operator == (const Point& p2) const {
		if (x != p2.x)
			return false;
		return y == p2.y;
	}

	float		x;
	float		y;
};

class Motion {
public:
	void prepend(Motion* prev) {
		_previous = prev;
	}

	void combine(Motion* also) {
		_also = also;
	}

	void scheduleAfter(beats startAt);

	double effectiveNoseMotion() const;

	double effectiveStartNose() const;

	double effectiveStartX() const;

	double effectiveStartY() const;

	double cumulativeNoseMotion() const;

	double cumulativeStartNose() const;

	double cumulativeStartX() const;

	double cumulativeStartY() const;

	double cumulativeEndX() const;

	double cumulativeEndY() const;

	double cumulativeEndNose() const;

	double cumulativeRadius() const;

	bool validate(float* fx, float* fy, double* fNoseAngle) const;

	void checkFlow(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const;

	virtual void checkThis(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const = 0;

	virtual void accumulateAlsoMotion(int at, double* deltaX, double* deltaY) const = 0;

	virtual void location(double fraction, double* x, double* y) const = 0;

	virtual void print(int indent) const = 0;

	Motion* split(const Group* pair, Point start, Facing* facing, int forward, int lateral, Context* context) const;

	virtual Motion* splitThis(const Group* pair, Point start, int forward, int lateral, Context* context) const = 0;

	Motion* fillGaps(const Group* startingDancers, const Dancer* d, beats duration);

	void finalizeAlso();

	void convertAlsoToStraight();

	const Point& start() const { return _start; }

	const Point& end() const { return _end; }

	double startNose() const { return _startNose; }

	double noseMotion() const { return _noseMotion; }

	void setDuration(beats duration) { _duration = duration; }

	beats endAt() const { return _startAt + _duration; }

	beats startAt() const { return _startAt; }

	beats duration() const { return _duration; }

	Motion* previous() const { return _previous; }

	Motion* also() const { return _also; }

protected:
	Motion(Point start, Point end, double startNose, double noseMotion, beats duration) {
		_start = start;
		_end = end;
		_startNose = startNose;
		_noseMotion = noseMotion;
		_duration = duration;
		_startAt = 0;
		_previous = null;
		_also = null;
	}

	Motion(int deltaX, int deltaY, Motion* also, beats duration) {
		_end.x = deltaX;
		_end.y = deltaY;
		_duration = duration;
		_noseMotion = 0;
		_startNose = 0;
		_startAt = 0;
		_previous = null;
		_also = also;
	}

	void printCommon(int indent) const;

private:
	Motion() {}

	Point			_start;			// starting dancer position
	Point			_end;			// Ending dancer position
	double			_startNose;		// starting nose angle (in radians)
	double			_noseMotion;	// direction and magnitude (in radians) of motion
	beats			_startAt;		// # of beats after start of the stage to start this motion
	beats			_duration;		// time needed for motion
	Motion*			_previous;		// previous motion in time along this chain of motions.
	Motion*			_also;			// Motions to be combined with this one.
};

class Straight : public Motion {
	friend Stage;
public:
	virtual void location(double fraction, double* x, double* y) const;

	virtual Motion* splitThis(const Group* pair, Point start, int forward, int lateral, Context* context) const;

	virtual void checkThis(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const;

	virtual void accumulateAlsoMotion(int at, double* deltaX, double* deltaY) const;

	virtual void print(int indent) const;

private:
	Straight(Point start, Point end, double startNose, double noseMotion, beats duration)
		: Motion(start, end, startNose, noseMotion, duration) {
	}

};

class Curve : public Motion {
	friend Stage;
public:
	virtual void location(double fraction, double* x, double* y) const;

	virtual Motion* splitThis(const Group* pair, Point start, int forward, int lateral, Context* context) const;

	virtual void checkThis(int dancerIndex, FlowState& flowState, Stage* stage, Motion* top) const;

	virtual void accumulateAlsoMotion(int at, double* deltaX, double* deltaY) const;

	virtual void print(int indent) const;

	double radius() const { return _radius; }

private:
	Curve(Point center, double motionAngle, double radius, Point start, Point end, double startNose, double noseMotion, beats duration) 
		: Motion(start, end, startNose, noseMotion, duration) {
		_center = center;
		_motionAngle = motionAngle;
		_radius = radius;
	}
	Point			_center;		// center of any arcing motion
	double			_motionAngle;	// direction and magnitude (in radians) of motion
	double			_radius;		// radius of ending dancer position
};

}  // namespace dance
