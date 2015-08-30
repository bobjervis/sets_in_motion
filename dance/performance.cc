#include "../common/platform.h"
#include "dance_ui.h"

#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/scrollbar.h"
#include "../display/window.h"
#include "call.h"

namespace dance {

	// Performance Fonts

display::Font levelFont("serif", 20, true, false, false, null, null);;
display::Font performanceCallFont("sansSerif", 36, true, false, false, null, null);;
display::Font performanceOtherFont("sansSerif", 36, false, false, false, null, null);;

Performance::Performance(DanceFrame* frame, Dance* parent, vector<PlayList*>& playLists, vector<Sequence*>& sequences) {
	_frame = frame;
	if (!loadPerformanceFonts())
		_frame->setStatus("Couldn't load performance fonts");
	_parent = parent;
	for (int i = 0; i < playLists.size(); i++)
		_performanceTabs.push_back(new PerformanceTab(frame, playLists[i]->label(), playLists[i]));
	if (sequences.size() > 0) {
		PlayList* p = new PlayList(parent);
		p->include(sequences);
		_performanceTabs.push_back(new PerformanceTab(frame, "Miscellaneous Sequences", p));
		_allocated = p;
	} else
		_allocated = null;
}

Performance::~Performance() {
	delete _allocated;
	_window->hide();
	_performanceTabs.deleteAll();
	delete _window;
}

void Performance::show() {
	_window = new display::Window();
	if (_performanceTabs.size() > 1) {
		_group = new display::TabbedGroup();
		for (int i = 0; i < _performanceTabs.size(); i++)
			_group->select(_performanceTabs[i]);
		_window->append(_group);
	} else if (_performanceTabs.size() == 1) {
		_window->append(_performanceTabs[0]->tabBody());
		_performanceTabs[0]->selected.fire();
		_group = null;
	} else
		return;
	_window->show();
	_window->close.addHandler(this, &Performance::onClose);
}

void Performance::onClose() {
	_frame->dismiss(this);
}

PerformanceTab::PerformanceTab(DanceFrame* frame, const string& label, PlayList* playList) {
	selected.addHandler(this, &PerformanceTab::onSelected);
	_frame = frame;
	_label = label;
	_playList = playList;
	_body = null;
	_index = -1;
}

PerformanceTab::~PerformanceTab() {
	if (_body) {
		_body->prune();
		delete _body;
		delete _callAreaHandler;
	}
}

void PerformanceTab::display(int index) {
	if (index != _index) {
		_index = index;
		_call = 0;
		for (int i = 0; i < _callMap.size(); i++) {
			_callMap[i]->set_value("");
			_callMap[i]->setBackground(null);
		}
		_description->set_value("");
		if (index < 0 || index >= _sequences.size())
			return;
		Sequence* s = _sequences[index];
		_level->set_value(levels[s->level()]);
		_description->set_value(s->comment);
		for (int i = 0; i < s->text().size(); i++) {
			_callMap[i]->set_value(s->text()[i]);
			if (i == _call) {
				_callMap[i]->set_font(&performanceCallFont);
				_callMap[i]->set_textColor(&display::white);
				_callMap[i]->setBackground(&display::blackBackground);
			} else {
				_callMap[i]->set_font(&performanceOtherFont);
				_callMap[i]->set_textColor(&display::black);
				_callMap[i]->setBackground(null);
			}
		}
	}
}

void PerformanceTab::advance() {
	Sequence* s = _sequences[_index];
	if (_call == s->text().size() - 1)
		return;
	_callMap[_call]->set_font(&performanceOtherFont);
	_callMap[_call]->set_textColor(&display::black);
	_callMap[_call]->setBackground(null);
	_call++;
	_callMap[_call]->set_font(&performanceCallFont);
	_callMap[_call]->set_textColor(&display::white);
	_callMap[_call]->setBackground(&display::blackBackground);
	display::rectangle r = _callMap[_call]->bounds;
	r.opposite.y += r.height();							// Make sure we expose some below
	_callArea->expose(r);
}

void PerformanceTab::backOneSequence() {
	if (_index > 0)
		display(_index - 1);
}

void PerformanceTab::forwardOneSequence() {
	if (_index < _sequences.size() - 1)
		display(_index + 1);
}

void PerformanceTab::onSelected() {
	_body->rootCanvas()->setKeyboardFocus(_body);
	display(_index);
}

const char* PerformanceTab::tabLabel() {
	return _label.c_str();
}

display::Canvas* PerformanceTab::tabBody() {
	if (_body == null) {
		display::Grid* infoArea = new display::Grid();
			infoArea->cell(new display::Label("Level:"));
			_level = new display::Label("");
			_level->set_font(&levelFont);
			infoArea->cell(_level);
			infoArea->cell(true);
			infoArea->cell(new display::Label("Comment: "));
			_description = new display::Label("");
			infoArea->cell(new display::Spacer(5, 0, 0, 0, _description));
		infoArea->complete();
		vector<Sequence*> sequences;
		_playList->fetchSequences(&sequences);
		for (int i = 0; i < sequences.size(); i++)
			_sequences.push_back(sequences[i]);
		int height = 1;
		for (int i = 0; i < _sequences.size(); i++)
			if (_sequences[i]->text().size() > height)
				height = _sequences[i]->text().size();

		display::Grid* callGrid = new display::Grid();
			for (int i = 0; i < height; i++) {
				if (i > 0)
					callGrid->row();
				display::Label* call = new display::Label("");
				call->set_font(&performanceOtherFont);
				_callMap.push_back(call);
				callGrid->cell(call);
			}
			callGrid->row();
			callGrid->cell();
		callGrid->complete();
		_callArea = new display::ScrollableCanvas();
		_callAreaHandler = new display::ScrollableCanvasHandler(_callArea);
		_callArea->append(callGrid);
		display::Grid* g = new display::Grid();
			display::Spacer* s = new display::Spacer(5, infoArea);
			s->setBackground(&display::buttonFaceBackground);
			g->cell(new display::Border(0, 0, 0, 2, s));
			g->row();
			g->cell(_callArea);
		g->complete();
		_callArea->setBackground(&display::editableBackground);
		_body = g;
		_body->character.addHandler(this, &PerformanceTab::onCharacter);
		_body->functionKey.addHandler(this, &PerformanceTab::onFunctionKey);
		display(0);
	}
	return _body;
}

void PerformanceTab::onCharacter(char c) {
	switch (c) {
	case	'-':
		// Log an error
		backOneSequence();
		break;

	case	'+':
		forwardOneSequence();
		break;

	case	' ':
		advance();
		break;
	}
}

void PerformanceTab::onFunctionKey(display::FunctionKey fk, display::ShiftState ss) {
	switch (fk) {
	case	display::FK_F1:
		_frame->onHelp(fk, ss);
		break;

	case	display::FK_UP:
	case	display::FK_LEFT:
		// Log an error
		backOneSequence();
		break;

	case	display::FK_DOWN:
	case	display::FK_RIGHT:
	case	display::FK_RETURN:
		forwardOneSequence();
		break;

	case	display::FK_ESCAPE:
		// Log an error
		forwardOneSequence();
		break;
	}
}

}  // namespace dance
