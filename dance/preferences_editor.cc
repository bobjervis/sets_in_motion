#include "../common/platform.h"
#include "dance_ui.h"

#include "../display/device.h"
#include "../display/font_selector.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/root_canvas.h"
#include "../display/scrollbar.h"

namespace dance {

static vector<string> unitNames;

class PageLayout : public display::Canvas {
public:
	PageLayout() {
		if (unitNames.size() == 0) {
			unitNames.push_back("pixels");
			unitNames.push_back("inches");
			unitNames.push_back("mms");
			unitNames.push_back("cms");
			unitNames.push_back("pts");
		}
		display::Grid* topMargin = marginGrid("Top margin: ", &dance::topMargin);
		display::Grid* headerMargin = marginGrid("Header margin: ", &dance::headerMargin);
		display::Grid* leftMargin = marginGrid("Left margin: ", &dance::leftMargin);
		display::Grid* rightMargin = marginGrid("Right margin: ", &dance::rightMargin);
		display::Grid* bottomMargin = marginGrid("Bottom margin: ", &dance::bottomMargin);
		display::Grid* g = new display::Grid();
			// top margin
			g->cell();
			g->cell(topMargin);
			g->cell();
			// header
			g->row();
			g->cell();
			display::Spacer* s = new display::Spacer(10);
			s->setBackground(&display::buttonFaceBackground);
			g->cell(s);
			g->cell();
			// header margin
			g->row();
			g->cell();
			g->cell(headerMargin);
			g->cell();
			// body
			g->row();
			g->cell(new display::Filler(leftMargin));
			s = new display::Spacer(0, 40, 0, 0);
			s->setBackground(&display::buttonFaceBackground);
			g->cell(s);
			g->cell(new display::Filler(rightMargin));
			// footer
			g->row();
			g->cell();
			g->cell(bottomMargin);
			g->cell();
		g->complete();
		g->setBackground(&display::editableBackground);
		_grid = g;
		append(new display::Border(1, g));
	}

	virtual display::dimension measure() {
		return child->preferredSize();
	}

	virtual void paint(display::Device* device) {
	}

	display::Field* firstField() {
		display::RootCanvas* root = rootCanvas();
		display::Field* f = null;
		for (int i = 0; i < _labels.size(); i++) {
			display::Field* thisField = root->field(_labels[i]);
			if (f == null)
				f = thisField;
		}
		return f;
	}

private:
	display::Grid* marginGrid(const char* label, display::Measure* margin) {
		display::Grid* g = new display::Grid();
			g->cell(new display::Label(label));
			display::Label* marginSize = new display::Label(string(margin->value()));
			display::DropDown* marginUnit = new display::DropDown(margin->unit(), unitNames);
			g->cell(new display::Bevel(2, true, marginSize));
			g->cell(new display::Spacer(5, 0, 0, 0, new display::Border(1, new display::Spacer(2, marginUnit))));
			marginSize->valueCommitted.addHandler(this, &PageLayout::sizeChange, marginSize, margin);
			marginUnit->valueChanged.addHandler(this, &PageLayout::unitChange, marginUnit, margin);
			_labels.push_back(marginSize);
			_labels.push_back(marginUnit);
		g->complete();
		return g;
	} 

	void sizeChange(display::Label* size, display::Measure* margin) {
		margin->set_value(size->value().toDouble());
	}

	void unitChange(display::DropDown* unit, display::Measure* margin) {
		margin->set_unit((display::Units)unit->value());
	}

	display::Grid*	_grid;
	vector<display::Label*> _labels;
};

PreferencesEditor::PreferencesEditor(DanceFrame* frame) {
	selected.addHandler(this, &PreferencesEditor::onSelected);
	unselected.addHandler(this, &PreferencesEditor::onUnselected);
	_body = null;
	_frame = frame;
	_firstField = null;
	_scrollerHandler = null;
	_defsToggleHandler = null;
	_myDefsToggleHandler = null;
}

PreferencesEditor::~PreferencesEditor() {
	delete _scrollerHandler;
	delete _defsToggleHandler;
	delete _myDefsToggleHandler;
	delete _body;
}

bool PreferencesEditor::deleteTab() {
	return false;
}

const char* PreferencesEditor::tabLabel() {
	return "Preferences";
}

display::Canvas* PreferencesEditor::tabBody() {
	if (_body == null) {
		freshenPreferences();
		display::Toggle* defsToggle = new display::Toggle(&_definitionsLock);
		_defsToggleHandler = new display::ToggleHandler(defsToggle);
		display::Toggle* myDefsToggle = new display::Toggle(&_myDefinitionsLock);
		_myDefsToggleHandler = new display::ToggleHandler(myDefsToggle);
		display::Grid* g = new display::Grid();
			g->cell(new display::Label("Animation Speed: "));
			g->cell(speedSelector(&_speed));
			g->row();
			g->cell(new display::Label("Allow editing of Definitions: "));
			g->cell(defsToggle);
			g->row();
			g->cell(new display::Label("Allow editing of My Definitions: "));
			g->cell(myDefsToggle);
			g->row();
			display::Label* heading1 = new display::Label("Performance Settings");
			heading1->set_format(DT_CENTER|DT_VCENTER);
			g->cell(display::dimension(2, 1), new display::Spacer(0, 20, 0, 20, heading1));
			g->row();
			g->cell(new display::Label("Current Call Font: "));
			_performanceCallFont = new display::FontSelector(&performanceCallFont);
			g->cell(_performanceCallFont);
			g->row();
			g->cell(new display::Label("Other Calls Font: "));
			_performanceOtherFont = new display::FontSelector(&performanceOtherFont);
			g->cell(_performanceOtherFont);
			g->row();
			display::Label* heading2 = new display::Label("Printer Settings");
			heading2->set_format(DT_CENTER|DT_VCENTER);
			g->cell(display::dimension(2, 1), new display::Spacer(0, 20, 0, 20, heading2));
			g->row();
			//  visual order has to match field order below
			g->cell(new display::Label("Call Font: "));
			_printerCallFont = new display::FontSelector(&printerCallFont);
			g->cell(_printerCallFont);
			g->row();
			g->cell(new display::Label("Comment Font: "));
			_printerCommentFont = new display::FontSelector(&printerCommentFont);
			g->cell(_printerCommentFont);
			g->row();
			g->cell(new display::Label("Version Font: "));
			_printerVersionFont = new display::FontSelector(&printerVersionFont);
			g->cell(_printerVersionFont);
			g->row();
			g->cell(new display::Label("Level Font: "));
			_printerLevelFont = new display::FontSelector(&printerLevelFont);
			g->cell(_printerLevelFont);
			g->row();
			g->cell(new display::Label("Date Font: "));
			_printerDateFont = new display::FontSelector(&printerDateFont);
			g->cell(_printerDateFont);
			g->row();
			g->cell();
			_pageLayout = new PageLayout();
			g->cell(new display::Filler(new display::Spacer(0, 10, 0, 0, _pageLayout)));
		g->complete();
		display::Spacer* s = new display::Spacer(20, 40, 0, 0, g);
		s->setBackground(&display::buttonFaceBackground);
		_speed.changed.addHandler(this, &PreferencesEditor::onSpeedChanged);
		_definitionsLock.changed.addHandler(this, &PreferencesEditor::onLockChanged, &_definitionsLock, _frame->defaultEditor());
		_myDefinitionsLock.changed.addHandler(this, &PreferencesEditor::onLockChanged, &_myDefinitionsLock, _frame->myDefinitionsEditor());
		display::ScrollableCanvas* scroller = new display::ScrollableCanvas();
		scroller->append(s);
		scroller->setBackground(&display::buttonFaceBackground);
		_scrollerHandler = new display::ScrollableCanvasHandler(scroller);
		_body = scroller;
	}
	return _body;
}

void PreferencesEditor::onSelected() {
	if (_firstField == null) {
		// Order has to match visual order
		_firstField = _performanceCallFont->firstField();
		_performanceOtherFont->firstField();
		_printerCallFont->firstField();
		_printerCommentFont->firstField();
		_printerVersionFont->firstField();
		_printerLevelFont->firstField();
		_printerDateFont->firstField();
		_pageLayout->firstField();
	}
	display::RootCanvas* c = _body->rootCanvas();
	if (c)
		c->setKeyboardFocus(_firstField->label());
	freshenPreferences();
}

void PreferencesEditor::onUnselected() {
	display::RootCanvas* c = _body->rootCanvas();
	if (c)
		c->setKeyboardFocus(null);
}

bool PreferencesEditor::needsSave() {
	return false;
}

bool PreferencesEditor::save() {
	tabModified();
	return true;
}

void PreferencesEditor::onSpeedChanged() {
	setPreference("animationSpeed", _speed.value());
}

void PreferencesEditor::onLockChanged(data::Boolean* variable, GrammarEditor* editor) {
	string v = getPreference(editor->lockPreference());
	string n(variable->value() ? "false" : "true");			// Note: the truth value of the check box is the opposite of the preference
	if (v != n) {
		setPreference(editor->lockPreference(), n);
		_frame->refreshSubEditors(editor);
	}
}

void PreferencesEditor::freshenPreferences() {
	string speedPref = getPreference("animationSpeed");
	int speed = 3;
	if (speedPref.size()) {
		speed = speedPref.toInt();
		if (speed == 0)
			speed = 3;
	}
	_speed.set_value(speed);
	string lock = getPreference(definitionsLock);
	_definitionsLock.set_value(lock == "false");
	string myLock = getPreference(myDefinitionsLock);
	_myDefinitionsLock.set_value(myLock == "false");
	loadPrinterFonts();
	loadPerformanceFonts();
}

}  // namespace dance
