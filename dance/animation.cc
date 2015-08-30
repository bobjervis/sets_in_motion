#include "../common/platform.h"
#include "dance_ui.h"

#include <math.h>
#include "../display/background.h"
#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/window.h"
#include "call.h"
#include "motion.h"

namespace dance {

const int MSEC_PER_TICK = 25;

#define PLAY				"\x34"
#define PAUSE				"\x3B"
#define	START_OVER			"\x39"
#define BACK_ONE_CALL		"\x37"
#define FORWARD_ONE_CALL	"\x38"

display::Font playerFont("+Webdings", 16, false, false, false, null, null);
display::Font unboundFont("Arial", 12, true, false, false, null, null);

Animator::Animator() {
	selected.addHandler(this, &Animator::onSelected);
	unselected.addHandler(this, &Animator::onUnselected);
	_body = null;
	_editor = null;
	_sequence = null;
	_tick = null;
	_playing = false;
	_index = 0;
	_speed = null;
	_startOverHandler = null;
	_backOneHandler = null;
	_forwardOneHandler = null;
	_playPauseHandler = null;
	deletingStage.addHandler(this, &Animator::onDeletingStage);
}

Animator::~Animator() {
	if (_tick != null)
		_tick->kill(null);
	delete _speed;
	delete _startOverHandler;
	delete _backOneHandler;
	delete _forwardOneHandler;
	delete _playPauseHandler;
	delete _body;
}

void Animator::setup(SequenceEditor* editor, int index, Sequence* sequence) {
	if (_editor == editor &&
		_index == index &&
		_sequence == sequence)
		return;
	_editor = editor;
	_index = index;
	if (_body) {
		if (_sequence != sequence) {
			if (playing())
				onPlayPauseClick(null);
			_meter->setup(sequence);
			_picture->setup(sequence);
		}
		_meter->setEditor(_editor);
		_meter->reset(index);
	}
	_sequence = sequence;
}

bool Animator::deleteTab() {
	return false;
}

const char* Animator::tabLabel() {
	return "Animation";
}

display::Canvas* Animator::tabBody() {
	if (_body == null) {
		_speed = new data::Integer();
		_picture = new Picture();
		_meter = new Meter(_picture);
		_playPauseLabel = new display::Label(PLAY, 3, &playerFont);
		_playPauseLabel->set_format(DT_CENTER);
		_startOverButton = new display::Bevel(2, new display::Label(START_OVER, &playerFont));
		_backOneCallButton = new display::Bevel(2, new display::Label(BACK_ONE_CALL, &playerFont));
		_playPauseButton = new display::Bevel(2, _playPauseLabel);
		_forwardOneCallButton = new display::Bevel(2, new display::Label(FORWARD_ONE_CALL, &playerFont));
		display::Grid* controls = new display::Grid();
			controls->cell(_startOverButton);
			controls->cell(_backOneCallButton);
			controls->cell(_playPauseButton);
			controls->cell(_forwardOneCallButton);
			controls->cell(_meter, true);
			controls->cell(speedSelector(_speed));
		controls->complete();
		controls->setBackground(&display::buttonFaceBackground);
		display::Grid* g = new display::Grid();
			g->cell(controls);
			g->row();
			g->cell(_picture);
		g->complete();
		_body = g;
		if (_sequence) {
			SequenceEditor* e = _editor;
			Sequence* s = _sequence;
			_editor = null;					// Have to do this to force the setup function to do some work
			_sequence = null;
			setup(e, _index, s);
		}

		_startOverHandler = new display::ButtonHandler(_startOverButton, 0);
		_startOverHandler->click.addHandler(this, &Animator::onStartOverClick);
		_backOneHandler = new display::ButtonHandler(_backOneCallButton, 0);
		_backOneHandler->click.addHandler(this, &Animator::onBackOneCall);
		_playPauseHandler = new display::ButtonHandler(_playPauseButton, 0);
		_playPauseHandler->click.addHandler(this, &Animator::onPlayPauseClick);
		_forwardOneHandler = new display::ButtonHandler(_forwardOneCallButton, 0);
		_forwardOneHandler->click.addHandler(this, &Animator::onForwardOneCall);
		_meter->drag.addHandler(this, &Animator::onMeterDrag);
		_meter->drop.addHandler(this, &Animator::onMeterDrop);
		_speed->changed.addHandler(this, &Animator::onSpeedChanged, _speed);
	}
	return _body;
}

void Animator::startTimer() {
	if (_tick == null) {
		_tick = _body->startTimer(MSEC_PER_TICK);
		_tick->tick.addHandler(this, &Animator::onTick);
	}
}

void Animator::stopTimer() {
	if (_tick != null) {
		_tick->kill(null);
		_tick = null;
	}
}

void Animator::onSelected() {
	string speedPref = getPreference("animationSpeed");
	int speed = 3;
	if (speedPref.size()) {
		speed = speedPref.toInt();
		if (speed == 0)
			speed = 3;
	}
	_meter->setSpeed(speed);
	_speed->set_value(speed);
	if (_editor)
		_meter->refreshEditor();
	if (_playing)
		startTimer();
}

void Animator::onUnselected() {
	if (_editor)
		_editor->select(-1);
	if (_playing)
		stopTimer();
}

void Animator::onDeletingStage(const Stage* stage) {
	if (_sequence == stage->sequence()) {
		if (_playing)
			onPlayPauseClick(null);
		_meter->setup(null);
		_picture->setup(null);
		_sequence = null;
	}
}

void Animator::onStartOverClick(display::Bevel* button) {
	_meter->reset(0);
}

void Animator::onBackOneCall(display::Bevel* button) {
	if (_sequence) {
		int index = _picture->index();
		double partial = _picture->partial();
		if (partial)
			_meter->reset(index);
		else if (index > 0)
			_meter->reset(index - 1);
	}
}

void Animator::onForwardOneCall(display::Bevel* button) {
	if (_sequence) {
		int index = _picture->index();

		if (index < _sequence->stages().size()) {
			_meter->reset(index + 1);
			if (_playing &&
				index + 1 == _sequence->stages().size())
				onPlayPauseClick(null);
		}
	}
}

void Animator::onPlayPauseClick(display::Bevel* button) {
	if (_sequence) {
		if (_playing) {
			_playPauseLabel->set_value(PLAY);
			_playing = false;
			stopTimer();
		} else if (_meter->timeRemaining()) {
			_playPauseLabel->set_value(PAUSE);
			_playing = true;
			_picture->startPlaying();
			_meter->invalidate();
			startTimer();
		}
	}
}
// each tick advances animation by 1/20 beat
void Animator::onTick() {
	if (_playing) {
		_picture->invalidate();
		if (!_meter->tick())
			onPlayPauseClick(null);
	}
}

void Animator::onSpeedChanged(data::Integer* variable) {
	setPreference("animationSpeed", variable->value());
	_meter->setSpeed(_speed->value());
}

void Animator::onMeterDrag(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	if (_sequence) {
	}
}

void Animator::onMeterDrop(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	if (_sequence) {
	}
}

void Meter::reset(int index) {
	_timeIndex = 0;
	for (int i = 0; i < index && i < _stageDurations.size(); i++)
		_timeIndex += _stageDurations[i];
	_timeIndex *= _ticksPerBeat;
	if (_editor)
		_editor->select(index);
	_picture->setIndex(index);
	invalidate();
}

bool Meter::tick() {
	invalidate();
	_timeIndex++;
	int cumulative = 0;
	int i;
	for (i = 0; i < _stageDurations.size(); i++) {
		int dur = _stageDurations[i] * _ticksPerBeat;
		if (cumulative + dur > _timeIndex)
			break;
		cumulative += dur;
	}
	if (_editor)
		_editor->select(i);
	if (cumulative == _timeIndex)
		_picture->setIndex(i);
	_picture->setPartial(double(_timeIndex - cumulative) / _ticksPerBeat);
	return _timeIndex < _duration * _ticksPerBeat;
}

void Meter::setup(Sequence *sequence) {
	_stageDurations.clear();
	_duration = 0;
	if (sequence) {
		const vector<const Stage*>& stages = sequence->stages();

		for (int i = 0; i < stages.size(); i++) {
			if (stages[i])
				_stageDurations.push_back(stages[i]->duration());
			else
				_stageDurations.push_back(0);
			_duration += _stageDurations[i];
		}
	}
}

void Meter::refreshEditor() {
	int t = _timeIndex;
	for (int i = 0; i < _stageDurations.size(); i++) {
		if (t < _stageDurations[i]) {
			_editor->select(i);
			return;
		}
		t -= _stageDurations[i];
	}
	_editor->select(_stageDurations.size());
}

static int speeds[] = {
	0,
	40,		// 60 BPM
	27,		// 90 BPM
	20,		// 120 BPM
	16,		// 150 BPM
	10,		// 240 BPM
};

void Meter::setSpeed(int speed) {
	if (_ticksPerBeat) {
		double beatIndex = _timeIndex / _ticksPerBeat;
		_ticksPerBeat = speeds[speed];
		_timeIndex = beatIndex * _ticksPerBeat;
	} else
		_ticksPerBeat = speeds[speed];
	invalidate();
}

display::dimension Meter::measure() {
	return display::dimension(16, 16);
}

static display::Pen* meterPen = display::createPen(PS_SOLID, 5, 0);
static display::Pen* sliderPen = display::createPen(PS_SOLID, 1, 0);
static display::Color sliderColor(0x00ff00);

static const int SLIDER_SIZE = 12;

void Meter::paint(display::Device* device) {
	int duration = _picture->duration() * _ticksPerBeat;

	if (duration == 0) {
		device->fillRect(bounds, &display::buttonFaceColor);
		return;
	}
	device->fillRect(bounds, &display::editableColor);
	int height = bounds.height();
	int width = bounds.width();

	display::point start, end;
	start.x = bounds.topLeft.x + 8;
	start.y = bounds.topLeft.y + height / 2 - 2;
	end.x = bounds.opposite.x - 7;
	end.y = start.y;
	device->set_pen(meterPen);
	device->line(start.x, start.y, end.x, end.y);
	display::rectangle r;
	r.topLeft.y = bounds.topLeft.y + height / 2 - SLIDER_SIZE / 2 - 1;
	r.opposite.y = r.topLeft.y + SLIDER_SIZE;
	int range = width - (SLIDER_SIZE + SLIDER_SIZE / 2);
	int center = range * _timeIndex / duration;
	r.topLeft.x = bounds.topLeft.x + center + SLIDER_SIZE / 2;
	r.opposite.x = r.topLeft.x + SLIDER_SIZE;
	device->set_pen(sliderPen);
	device->set_background(&sliderColor);
	device->ellipse(r);
}

bool Meter::bufferedDisplay(display::Device* device) {
	return false;
}

void Picture::drawDancers(const Group* dancers) {
	_start = dancers;
	if (_start)
		_start->boundingBox(&_boundingBox);
	else
		_boundingBox.clear();
	_ignoreRotation = true;
}

void Picture::setHalfSpot(int pixels) {
	_minimumHalfSpot = pixels;
	if (pixels) {
		if (_formation)
			_formation->boundingBox(&_boundingBox);
		else if (_start)
			_start->boundingBox(&_boundingBox);
		else
			_boundingBox = Rectangle(-8, 8, 8, -8);
	}
}

void Picture::setup(Sequence* sequence) {
	_index = 0;
	_sequence = sequence;
	if (sequence != null)
		_start = Group::home;
	else
		_start = null;
}

void Picture::setIndex(int index) {
	if (_sequence) {
		const vector<const Stage*>& stages = _sequence->stages();

		_start = Group::home;
		_index = index;
		while (index > 0) {
			if (index <= stages.size()) {
				if (!stages[index - 1]->failed()) {
					_start = stages[index - 1]->final();
					break;
				}
			}
			index--;
		}
	}
	_partial = 0;
	invalidate();
}

void Picture::setPartial(double beats) {
	_partial = beats;
}

void Picture::startPlaying() {
	if (_sequence) {
		const vector<const Stage*>& stages = _sequence->stages();

		while (stages[_index]->failed()) {
			_index++;
			if (_index >= stages.size()) {
				_index--;
				break;
			}
		}
		invalidate();
	}
}

beats Picture::duration() const {
	if (_sequence)
		return _sequence->duration();
	else
		return 0;
}

display::dimension Picture::measure() {
	display::dimension d(0, 0);
	if (_minimumHalfSpot) {
		d.width = _minimumHalfSpot * _boundingBox.width();
		d.height = _minimumHalfSpot * _boundingBox.height();
		if (_formation) {
			d.width += 8;
			d.height += 8;
		}
	}
	return d;
}

bool Picture::hitTest(display::point p, PictureHit *output) {
	int spotWidth = _boundingBox.width();
	int spotHeight = _boundingBox.height();

	int baseX = -((spotWidth - _formationBox.width()) / 2);
	int baseY = -((spotHeight - _formationBox.height()) / 2);
	int deltaX = p.x - bounds.topLeft.x;
	int deltaY = p.y - bounds.topLeft.y;
	output->x = deltaX / _halfSpot + baseX;
	output->y = deltaY / _halfSpot + baseY;
	deltaX %= _halfSpot;
	deltaY %= _halfSpot;
	int h = _halfSpot / 2;
	deltaX -= h;
	deltaY -= h;
	h /= 2;
	if (abs(deltaX) <= h && abs(deltaY) <= h)
		output->zone = FZ_CENTER;
	else if (deltaY < 0) {
		if (deltaX < 0) {
			if (deltaX < deltaY)
				output->zone = FZ_LEFT;
			else
				output->zone = FZ_BACK;
		} else if (deltaX > -deltaY)
			output->zone = FZ_RIGHT;
		else
			output->zone = FZ_BACK;
	} else if (deltaX < 0) {
		if (-deltaX > deltaY)
			output->zone = FZ_LEFT;
		else
			output->zone = FZ_FRONT;
	} else {
		if (deltaX > deltaY)
			output->zone = FZ_RIGHT;
		else
			output->zone = FZ_FRONT;
	}
	return true;
}

static display::Color inactiveColor(0xe00000
									);
static display::Color activeColor(0x00e000);
static display::Color designatedColor(0x00ff00);
static display::Color nondesignatedColor(0x00e000);
static display::Color boyColor(0xa0a0ff);
static display::Color girlColor(0xff8080);

void Picture::paint(display::Device* device) {
	int width = bounds.width();
	int height = bounds.height();
	if (_minimumHalfSpot && _formation) {
		width -= 8;
		height -= 8;
	}
	display::point topLeft = bounds.topLeft;

	int spotWidth = _boundingBox.width();
	int spotHeight = _boundingBox.height();

	if (spotWidth == 0 || spotHeight == 0)
		return;

	int pixelsPerSpotX = width / spotWidth;
	int pixelsPerSpotY = height / spotHeight;

	if (pixelsPerSpotX < pixelsPerSpotY)
		_halfSpot = pixelsPerSpotX;
	else
		_halfSpot = pixelsPerSpotY;
	// The center spot may not lie within the picture,
	// but the dancers inside the bounding box will be visible.
	_center.x = topLeft.x - _halfSpot * _boundingBox.left;
	_center.y = topLeft.y + _halfSpot * _boundingBox.top;
	device->fillRect(bounds, &display::editableColor);
	if (_formation) {
		int baseX = -((spotWidth - _formationBox.width()) / 2);
		int margin = _halfSpot / 2;
		if (_minimumHalfSpot)
			margin += 4;
		for (int x = topLeft.x + margin; x < bounds.opposite.x; x += _halfSpot, baseX++) {
			int baseY = -((spotHeight - _formationBox.height()) / 2);
			for (int y = topLeft.y + margin; y < bounds.opposite.y; y += _halfSpot, baseY++) {
				const Spot& s = _formation->spot(baseX, baseY);
				double n1Angle, n2Angle;
				facingToAngles(s.facing, &n1Angle, &n2Angle);
				display::Color* color = &display::white;
				string caption;
				switch (s.position) {
				case	EMPTY:
					device->set_pen(display::blackPen);
					device->line(x - 2, y, x + 3, y);
					device->line(x, y - 2, x, y + 3);
					continue;

				case	WRAP:
					device->set_pen(display::blackPen);
					device->line(x - 5, y - 5, x + 6, y + 6);
					continue;

				case	INACTIVE:
					color = &inactiveColor;
					caption = "I";
					break;

				case	ACTIVE:
					color = &activeColor;
					break;

				case	ACTIVE_BOY:
					color = &boyColor;
					caption = "B";
					break;

				case	ACTIVE_GIRL:
					color = &girlColor;
					caption = "G";
					break;

				case	ACTIVE_DESIGNATED:
					color = &designatedColor;
					caption = "D";
					break;

				case	ACTIVE_NONDESIGNATED:
					color = &nondesignatedColor;
					caption = "N";
					break;

				case	CENTER:
					color = &activeColor;
					caption = "C";
					break;

				case	END:
					color = &activeColor;
					caption = "E";
					break;

				case	VERY_CENTER:
					color = &activeColor;
					caption = "vC";
					break;

				case	VERY_END:
					color = &activeColor;
					caption = "vE";
					break;
				}
				drawFigure(device, false, color, caption, x, y, n1Angle, n2Angle);
			}
		}
	}
	if (_sequence) {
		if (_partial) {
			for (int i = 0; i < MAX_DANCERS; i++)
				_painted[i] = false;
			const Stage* stage;
			if (_index < _sequence->stages().size())
				stage = _sequence->stages()[_index];
			else
				stage = null;
			if (stage) {
				int dancerCount = stage->dancerCount();
				for (int i = 0; i < dancerCount; i++)
					_painted[i] = paintMotion(device, i, stage->motion(i), _partial);
			}
			for (int i = 0; i < MAX_DANCERS; i++)
				if (!_painted[i]) {
					const Dancer* d = _start->dancerByIndex(i);
					if (d)
						drawDancer(device, getBaseAngle(), d);
				}
			return;
		}
	}
	if (_start) {
		double baseAngle = getBaseAngle();
		for (int i = 0; i < _start->dancerCount(); i++) {
			const Dancer* d = _start->dancer(i);

			drawDancer(device, baseAngle, d);
		}
	}
}

bool Picture::paintMotion(display::Device* device, int dancerIndex, const Motion* m, double partial) {
	double x = 0;
	double y = 0;
	double noseAngle = 0;

	bool painted = false;
	for (;;) {
		m = combineMotion(m, partial, &x, &y, &noseAngle);
		if (m) {
			painted = true;
			m = m->also();
			if (m == null)
				break;
		} else
			break;
	}

	if (painted)
		drawDancer(device, Gender(dancerIndex & 1), (dancerIndex / 2) + 1, x, y, noseAngle, NO_NOSE);
	return painted;
}

const Motion* Picture::combineMotion(const Motion* m, double partial, double* x, double* y, double* noseAngle) {
	for (; m; m = m->previous()) {
		if (m->startAt() >= partial)
			continue;
		double fraction;
		if (m->endAt() > partial)
			fraction = (partial - m->startAt()) / m->duration();
		else
			fraction = 1;
		double localX, localY;
		m->location(fraction, &localX, &localY);

		*noseAngle += m->startNose() + fraction * m->noseMotion();
		*x += localX;
		*y += localY;
		return m;
	}
	return null;
}

void Picture::paintFinishedMotion(display::Device* device, int dancerIndex, const Motion* m) {
	paintMotion(device, dancerIndex, m, NO_NOSE);
}

bool Picture::bufferedDisplay(display::Device* device) {
	return true;
}

static display::Pen* circlePen = display::createPen(PS_SOLID, 1, 0);
static display::Pen* nosePen = display::createPen(PS_SOLID, 1, 0);

double Picture::getBaseAngle() const {
	if (_ignoreRotation)
		return 0;
	else
		return rotationAngle(_start->rotation());
}

void Picture::facingToAngles(Facing facing, double* noseAngle, double* secondNoseAngle) {
	*secondNoseAngle = NO_NOSE;
	switch (facing) {
	case	HEAD_FACING:
		*secondNoseAngle = -PI / 2;
	case	BACK_FACING:
		*noseAngle = PI / 2;
		break;

	case	SIDE_FACING:
		*secondNoseAngle = PI;
	case	RIGHT_FACING:
		*noseAngle = 0;
		break;

	case	LEFT_FACING:
		*noseAngle = PI;
		break;

	case	FRONT_FACING:
		*noseAngle = -PI / 2;
		break;

	case	ANY_FACING:
		*noseAngle = NO_NOSE;
	}
}

void Picture::drawDancer(display::Device* device, double baseAngle, const Dancer* dancer) {
	float x = dancer->x;
	float y = dancer->y;
	double noseAngle;
	double secondNoseAngle;

	if (baseAngle == 0 && _start->geometry() != RING)
		facingToAngles(dancer->facing, &noseAngle, &secondNoseAngle);
	else {
		Facing facing = dancer->facing;
		secondNoseAngle = NO_NOSE;
		_start->convertToAbsolute(&x, &y, &facing, &noseAngle);
	}
	drawDancer(device, dancer->gender, dancer->couple, x, y, noseAngle, secondNoseAngle);
}

void Picture::drawDancer(display::Device* device, Gender gender, int couple, double x, double y, 
						 double noseAngle, 
						 double secondNoseAngle) {
	display::rectangle r;
	int noseWidth = _halfSpot / 4;
	int dancerX = _center.x + x * _halfSpot;
	int dancerY = _center.y - y * _halfSpot;
	display::Color* c;
	switch (gender) {
	case	BOY:
		c = &boyColor;
		break;

	case	GIRL:
		c = &girlColor;
		break;

	default:
		c = &display::white;
	}
	string caption;
	switch (couple) {
	case	0:
		caption = "";
		break;

	case	7:
		caption = "H";
		break;

	case	8:
		caption = "S";
		break;

	default:
		caption = string(couple);
	}
	drawFigure(device, gender == BOY, c, caption, dancerX, dancerY, noseAngle, secondNoseAngle);
}

void Picture::drawFigure(display::Device* device, bool square, display::Color* color, const string& caption, double x, double y, 
						 double noseAngle, 
						 double secondNoseAngle) {
	display::rectangle r;
	int noseWidth = _halfSpot / 4;
	drawNose(device, x, y, noseAngle);
	drawNose(device, x, y, secondNoseAngle);
	device->set_pen(circlePen);
	device->set_background(color);
	if (square) {
		display::point a[4];
		double sqrt2over2 = sqrt(2.0) / 2;
		a[0].x = x + int(cos(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		a[0].y = y - int(sin(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		noseAngle += PI / 2;
		a[1].x = x + int(cos(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		a[1].y = y - int(sin(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		noseAngle += PI / 2;
		a[2].x = x + int(cos(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		a[2].y = y - int(sin(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		noseAngle += PI / 2;
		a[3].x = x + int(cos(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		a[3].y = y - int(sin(noseAngle + PI / 4) * _halfSpot * sqrt2over2 + 0.5);
		device->polygon(a, 4);
	} else {
		r.topLeft.x = x - _halfSpot / 2;
		r.topLeft.y = y - _halfSpot / 2;
		r.opposite.x = r.topLeft.x + _halfSpot;
		r.opposite.y = r.topLeft.y + _halfSpot;
		device->ellipse(r);
	}
	if (caption.size() && (_minimumHalfSpot == 0 || _minimumHalfSpot > 15)) {
		display::dimension size = device->textExtent(caption);
		if (_font == null)
			_font = unboundFont.currentFont(rootCanvas());
		device->set_font(_font);
		device->backMode(TRANSPARENT);
		device->text(x - size.width / 2, y - size.height / 2, caption);
	}
}

void Picture::drawNose(display::Device* device, int x, int y, double noseAngle) {
	display::rectangle r;

	if (noseAngle > 500000)
		return;
	device->set_pen(nosePen);
	device->set_background(&display::black);
	int noseX = x + cos(noseAngle) * _halfSpot / 2 + 0.5;
	int noseY = y - sin(noseAngle) * _halfSpot / 2 + 0.5;
	int noseSize = _halfSpot / 4;
	if (noseSize < 8)
		noseSize += 2;
	else if (noseSize < 3)
		noseSize = 3;
	r.topLeft.x = noseX - noseSize / 2;
	r.topLeft.y = noseY - noseSize / 2;
	r.opposite.x = r.topLeft.x + noseSize;
	r.opposite.y = r.topLeft.y + noseSize;
	device->ellipse(r);
}

SpeedControl::SpeedControl(data::Integer* value) {
	display::RadioButton* slowestButton = new display::RadioButton(value, 1);
	display::RadioButton* slowButton = new display::RadioButton(value, 2);
	display::RadioButton* normalButton = new display::RadioButton(value, 3);
	display::RadioButton* fastButton = new display::RadioButton(value, 4);
	display::RadioButton* fastestButton = new display::RadioButton(value, 5);

		cell(slowestButton);
		cell(new display::Label("Slowest"));
		cell(slowButton);
		cell(new display::Label("Slow"));
		cell(normalButton);
		cell(new display::Label("Normal"));
		cell(fastButton);
		cell(new display::Label("Fast"));
		cell(fastestButton);
		cell(new display::Label("Fastest"));
	complete();

	_speedRadioHandler = new display::RadioHandler(value);
	_speedRadioHandler->member(slowestButton);
	_speedRadioHandler->member(slowButton);
	_speedRadioHandler->member(normalButton);
	_speedRadioHandler->member(fastButton);
	_speedRadioHandler->member(fastestButton);
}

SpeedControl::~SpeedControl() {
	delete _speedRadioHandler;
}

SpeedControl* speedSelector(data::Integer* value) {
	return new SpeedControl(value);
}

}  // namespace dance
