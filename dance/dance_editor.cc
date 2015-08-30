#include "../common/platform.h"
#include "dance_ui.h"

#include "../display/background.h"
#include "../display/context_menu.h"
#include "../display/device.h"
#include "../display/grid.h"
#include "../display/label.h"
#include "../display/scrollbar.h"
#include "../display/window.h"
#include "call.h"

namespace dance {

display::Font titleCardFont("Times New Roman", 14, true, false, false, null, null);

const char* PlayListEditor::tabLabel() {
	if (_playList->label().size() > 0)
		return _playList->label().c_str();
	else
		return "<unnamed>";
}

display::Canvas* PlayListEditor::tabBody() {
	if (_body == null) {
		display::Grid* infoArea = new display::Grid();
			infoArea->cell(new display::Spacer(0, 5, 0, 0, new display::Label("Description: ")));
			display::Label* playListName = new display::Label(_playList->label());
			playListName->setBackground(&display::editableBackground);
			infoArea->cell(new display::Spacer(5, new display::Bevel(2, true, playListName)));
			infoArea->row();
			infoArea->cell(new display::Label("Created:"));
			string cDate, mDate;
			cDate.localTime(_playList->created, "%a %b %d, %Y %I:%M:%S %p");
			infoArea->cell(new display::Label(cDate));
			infoArea->row();
			infoArea->cell(new display::Label("Modified:"));
			if (_playList->modified != 0)
				mDate.localTime(_playList->modified, "%a %b %d, %Y %I:%M:%S %p");
			infoArea->cell(new display::Label(mDate));
		infoArea->complete();
		infoArea->setBackground(&display::buttonFaceBackground);
		display::Grid* g = new display::Grid();
			g->cell(infoArea);
			g->row();
			g->cell(commonTabBody(false));
		g->complete();
		_body = g;
		_body->functionKey.addHandler((DanceEditor*)this, &DanceEditor::onFunctionKey);
	}
	return _body;
}

const char* LibraryEditor::tabLabel() {
	return "My Library";
}

display::Canvas* LibraryEditor::tabBody() {
	if (_body == null) {
		_body = commonTabBody(true);
		_body->functionKey.addHandler((DanceEditor*)this, &DanceEditor::onFunctionKey);
	}
	return _body;
}

DanceFileEditor::DanceFileEditor(Dance* dance, DanceFrame* frame) : DanceEditor(dance, frame) {
	_label = fileSystem::basename(dance->filename());
	_nameLabel = null;
}

DanceFileEditor::~DanceFileEditor() {
	delete dance();
}

const char* DanceFileEditor::tabLabel() {
	return _label.c_str();
}

display::Canvas* DanceFileEditor::tabBody() {
	if (_body == null) {
		_nameLabel = new display::Label(dance()->filename());
		display::Grid* infoArea = new display::Grid();
			infoArea->cell(new display::Spacer(0, 5, 0, 5, new display::Label("Filename: ")));
			infoArea->cell(new display::Spacer(5, _nameLabel));
		infoArea->complete();
		infoArea->setBackground(&display::buttonFaceBackground);
		display::Grid* g = new display::Grid();
			g->cell(infoArea);
			g->row();
			g->cell(commonTabBody(true));
		g->complete();

		_nameLabel->setBackground(&display::editableBackground);
		_body = g;
		_body->functionKey.addHandler((DanceEditor*)this, &DanceEditor::onFunctionKey);
	}
	return _body;
}

DanceEditor::DanceEditor(Dance* dance, DanceFrame* frame) {
	selected.addHandler(this, &DanceEditor::onSelected);
	_dance = dance;
	_frame = frame;
	_body = null;
	_commonBody = null;
	_printJob = null;
	_listening = false;
	_scrollerHandler = null;
}

DanceEditor::~DanceEditor() {
	delete _scrollerHandler;
	for (int i = 0; i < _sequenceMap.size(); i++) {
		delete _sequenceMap[i].checked;
		delete _sequenceMap[i].checkedHandler;
	}
	delete _body;
	_undoStack.clear();
}

bool DanceEditor::deleteTab() {
	return false;
}

void DanceEditor::onSelected() {
	display::RootCanvas* c = _body->rootCanvas();
	if (c)
		c->setKeyboardFocus(_body);
}

bool DanceEditor::needsSave() {
	return !_undoStack.atSavedState();
}

bool DanceEditor::save() {
	if (_dance->filename().size() == 0) {
		OPENFILENAME o;
		char buffer[1024];

		memset(&o, 0, sizeof o);
		buffer[0] = 0;
		o.lStructSize = sizeof o;
		o.lpstrDefExt = "dnc";
		display::RootCanvas* rc = _body->rootCanvas();
		if (rc == null)
			rc = _frame->rootCanvas();
		o.hwndOwner = rc->handle();
		o.lpstrFile = buffer;
		o.nMaxFile = sizeof buffer;
		if (GetSaveFileName(&o)) {
			string ext = fileSystem::extension(buffer);
			if (ext != ".dnc")
				_frame->setStatus("Saving dance file with non-standard extension");
			_dance->setFilename(buffer);
		}
	}
	if (!fileSystem::createBackupFile(_dance->filename())) {
		_frame->setStatus("Could not create backup for " + _dance->filename());
		return false;
	}
	if (_dance->save()) {
		_frame->setStatus(_dance->label() + " saved to " + _dance->filename());
		tabModified();
		_undoStack.markSavedState();
		return true;
	} else {
		_frame->setStatus("FAILED: " + _dance->label() + " save to " + _dance->filename());
		return false;
	}
}

bool DanceEditor::calculateOneSequence() {
	const vector<Sequence*>& sequences = _dance->sequences();
	for (int i = 0; i < sequences.size(); i++) {
		Sequence* s = sequences[i];
		if (s->updateStatus(myDefinitions)) {
			touch(s);
			return true;
		}
	}
	return false;
}

display::Canvas* DanceEditor::commonTabBody(bool editPlayLists) {
	if (_body == null) {
		editPlayLists = false;
		defaultDefinitions->changed.addHandler(this, &DanceEditor::grammarChanged);
		myDefinitions->changed.addHandler(this, &DanceEditor::grammarChanged);
		display::Grid* playListHeaders;
		display::Grid* playListAreaGrid;
		if (editPlayLists) {
			playListHeaders = new display::Grid();
				display::Label* titleCard = new display::Label("Play Lists");
				titleCard->setBackground(&display::buttonFaceBackground);
				titleCard->set_format(DT_CENTER);
				titleCard->set_font(&titleCardFont);
				playListHeaders->cell(display::dimension(4, 1), titleCard);
				playListHeaders->row();
				playListHeaders->cell(new display::Label("",2));
				playListHeaders->cell(new display::Label("  Level", 6));
				playListHeaders->cell(new display::Label("Date Created", 8));
				playListHeaders->cell(new display::Label("Description"));
			playListHeaders->complete();
			playListHeaders->setBackground(&display::buttonFaceBackground);
			_playListArea = new display::Grid();
				for (int i = 0; i < _dance->playLists().size(); i++) {
					PlayList* playList = _dance->playLists()[i];
					if (i != 0)
						_playListArea->row();
					showPlayListInfo(i, playList);
				}
			_playListArea->complete();
			_playListArea->mouseCursor = display::standardCursor(display::HAND);
			_playListArea->click.addHandler(this, &DanceEditor::onPlayListClick);
			_playListArea->openContextMenu.addHandler(this, &DanceEditor::onOpenPlayListContextMenu);
			display::Label* morePlayList = new display::Label("Add another play list...");
			morePlayList->mouseCursor = display::standardCursor(display::HAND);
			morePlayList->click.addHandler(this, &DanceEditor::onAddPlayList);
			playListAreaGrid = new display::Grid();
				playListAreaGrid->row();
				playListAreaGrid->cell(_playListArea);
				playListAreaGrid->row();
				playListAreaGrid->cell(morePlayList);
			playListAreaGrid->complete();
		}
		display::Grid* sequenceHeaders = new display::Grid();
			if (editPlayLists) {
				display::Label* titleCard = new display::Label("Sequences");
				titleCard->setBackground(&display::buttonFaceBackground);
				titleCard->set_format(DT_CENTER);
				titleCard->set_font(&titleCardFont);
				sequenceHeaders->cell(display::dimension(6, 1), titleCard);
				sequenceHeaders->row();
			}
			sequenceHeaders->cell(new display::Label("ID",4));
			sequenceHeaders->cell(new display::Label("",2));
			sequenceHeaders->cell(new display::Label("Status",7));
			sequenceHeaders->cell(new display::Label("  Level", 6));
			sequenceHeaders->cell(new display::Label("Date Created", 8));
			sequenceHeaders->cell(new display::Label("Comment"));
		sequenceHeaders->complete();
		sequenceHeaders->setBackground(&display::buttonFaceBackground);
		_sequenceArea = new display::Grid();
			for (int i = 0; i < _dance->sequences().size(); i++) {
				Sequence* sequence = _dance->sequences()[i];
				if (i != 0)
					_sequenceArea->row();
				showSequenceInfo(i, sequence);
			}
		_sequenceArea->complete();
		_sequenceArea->mouseCursor = display::standardCursor(display::HAND);
		_sequenceArea->click.addHandler(this, &DanceEditor::onClick);
		_sequenceArea->openContextMenu.addHandler(this, &DanceEditor::onOpenContextMenu);
		display::Label* moreSeq = new display::Label("Add another sequence...");
		moreSeq->mouseCursor = display::standardCursor(display::HAND);
		moreSeq->click.addHandler(this, &DanceEditor::onAddSequence);
		display::Grid* seqAreaGrid = new display::Grid();
			seqAreaGrid->cell(_sequenceArea);
			seqAreaGrid->row();
			seqAreaGrid->cell(new display::Filler(moreSeq));
		seqAreaGrid->complete();
		display::Grid* g = new display::Grid();
			if (editPlayLists) {
				g->cell(playListHeaders);
				g->row();
				g->cell(playListAreaGrid);
				g->row();
			}
			g->cell(sequenceHeaders);
			g->row();
			g->cell(seqAreaGrid);
		g->complete();

		display::ScrollableCanvas* scroller = new display::ScrollableCanvas();
		scroller->setBackground(&display::editableBackground);
		scroller->append(g);
		_scrollerHandler = new display::ScrollableCanvasHandler(scroller);
		_body = scroller;
	}
	return _body;
}

bool DanceEditor::startPrinting() {
	_printJob = danceWindow->openPrintDialog();
	if (_printJob) {
		_printJob->startPrinting();
		return true;
	} else
		return false;
}

display::Font printerCallFont("Times New Roman", 16, false, false, false, null, null);
display::Font printerCommentFont("Times New Roman", 12, false, false, false, null, null);
display::Font printerVersionFont("Times New Roman", 12, false, false, false, null, null);
display::Font printerDateFont("Times New Roman", 12, false, false, false, null, null);
display::Font printerLevelFont("Times New Roman", 12, false, false, false, null, null);
display::Measure topMargin(0.5, display::INCH);
display::Measure headerMargin(0.25, display::INCH);
display::Measure leftMargin(0.5, display::INCH);
display::Measure rightMargin(0.5, display::INCH);
display::Measure bottomMargin(0.5, display::INCH);

void DanceEditor::print(Sequence* sequence) {
	if (_printJob == null)
		return;
	if (!loadPrinterFonts()) {
		_frame->setStatus("Could not load printer fonts");
		return;
	}
//	sequence->updateStages(myDefinitions);
	string cDate;
	cDate.localTime(sequence->modified, "%a %b %d, %Y %I:%M:%S %p");
	display::Label* creationDate = new display::Label(cDate, &printerDateFont);
	display::Grid* infoArea = new display::Grid();
		infoArea->cell(creationDate);
		infoArea->cell();
		display::Label* level = new display::Label(levels[sequence->level()], &printerLevelFont);
		level->set_format(DT_RIGHT);
		infoArea->cell(level);
		infoArea->row();
		infoArea->cell(new display::Label(sequence->createdWith, &printerVersionFont));
		infoArea->cell();
		display::Label* comment = new display::Label(sequence->comment, &printerCommentFont);
		comment->set_format(DT_RIGHT);
		infoArea->cell(comment);
	infoArea->complete();
	infoArea->resizeColumn(1);
	display::Grid* callArea = new display::Grid();
		for (int i = 0; i < sequence->text().size(); i++) {
			callArea->row();
			display::Label* call = new display::Label(sequence->text()[i], &printerCallFont);
			callArea->cell(new display::Spacer(0, 10, 0, 0, call));
//			Picture* p = new Picture(sequence->stages()[i]->final(), 16);
//			callArea->cell(p);
		}
	callArea->complete();
	display::Grid* g = new display::Grid();
		g->cell(new display::Spacer(leftMargin.pixels(_printJob), topMargin.pixels(_printJob), 0, 0));
		g->cell();
		g->cell(new display::Spacer(rightMargin.pixels(_printJob), 0, 0, 0));
		g->row();
		g->cell();
		g->cell(infoArea);
		g->cell();
		g->row();
		g->cell(new display::Spacer(0, headerMargin.pixels(_printJob), 0, 0));
		g->row();
		g->cell();
		g->cell(callArea);
		g->cell();
		g->row();
		g->cell(new display::Spacer(0, bottomMargin.pixels(_printJob), 0, 0));
	g->complete();
	g->resizeColumn(1);
	g->resizeRow(3);
	_printJob->append(g);
	_printJob->startPage();
	_printJob->paint();
	_printJob->endPage();
/*
	display::Spacer* s = new display::Spacer(0, g);
	s->setBackground(&display::editableBackground);
	display::Window* w = new display::Window();
	w->append(s);
	w->show();
	warningMessage("When done click OK");
	w->hide();
	delete w;
 */
}

void DanceEditor::donePrinting() {
	if (_printJob) {
		_printJob->close();
		delete _printJob;
		_printJob = null;
	}
}

void DanceEditor::touch(Sequence* sequence) {
	for (int i = 0; i < _sequenceMap.size(); i++)
		if (_sequenceMap[i].sequence == sequence) {
			_sequenceMap[i].comment->set_value(sequence->comment);
			_sequenceMap[i].level->set_value(levels[sequence->level()]);
			setSequenceStatus(_sequenceMap[i].status, sequence);
			break;
		}
	tabModified();
}

void DanceEditor::onFunctionKey(display::FunctionKey fk, display::ShiftState ss) {
	switch (fk) {
	case	display::FK_Y:
		if (ss == display::SS_CONTROL) {
			if (_undoStack.nextRedo())
				_undoStack.redo();
			else
				_frame->setStatus("No more redo");
		}
		break;

	case	display::FK_Z:
		if (ss == display::SS_CONTROL) {
			if (_undoStack.nextUndo())
				_undoStack.undo();
			else
				_frame->setStatus("No more undo");
		}
		break;

	case	display::FK_A:
		if (_body->rootCanvas() &&
			_body->rootCanvas()->keyFocus() == _body) {
			for (int i = 0; i < _playListMap.size(); i++)
				_playListMap[i].checked->set_value(true);
			for (int i = 0; i < _sequenceMap.size(); i++)
				_sequenceMap[i].checked->set_value(true);
		}
		break;
	}
}

void DanceEditor::showSequenceInfo(int i, Sequence* sequence) {
	SequenceMapEntry sme;

	sme.sequence = sequence;
	display::Label* x;
	sme.checked = new data::Boolean(false);
	display::Toggle* check = new display::Toggle(sme.checked);
	sme.checkedHandler = new display::ToggleHandler(check);
	sme.checked->changed.addHandler(this, &DanceEditor::onChecked, sme.checked, i);
	_sequenceArea->cell(new display::Label(string(i + 1), 4));
	_sequenceArea->cell(new display::Spacer(0, 1, 10, 1, check));
	x = new display::Label("", 7);
//	sequence->updateStages(myDefinitions);
	setSequenceStatus(x, sequence);
	sme.status = x;
	_sequenceArea->cell(x);
	sme.level = new display::Label(levels[sequence->level()], 6);
	_sequenceArea->cell(sme.level);
	string date;
	date.localTime(sequence->created, "%m/%d/%Y");
	_sequenceArea->cell(new display::Label(date, 8));
	sme.comment = new display::Label(sequence->comment);
	_sequenceArea->cell(sme.comment);
	_sequenceMap.push_back(sme);
}

void DanceEditor::showPlayListInfo(int i, PlayList* playList) {
	PlayListMapEntry pme;

	pme.playList = playList;
	display::Label* x;
	pme.checked = new data::Boolean(false);
	display::Toggle* check = new display::Toggle(pme.checked);
	new display::ToggleHandler(check);
	pme.checked->changed.addHandler(this, &DanceEditor::onPlayListChecked, pme.checked, i);
	_playListArea->cell(new display::Spacer(0, 1, 10, 1, check));
	pme.level = new display::Label(levels[playList->level()], 6);
	_playListArea->cell(pme.level);
	string date;
	date.localTime(playList->created, "%m/%d/%Y");
	_playListArea->cell(new display::Label(date, 8));
	pme.comment = new display::Label(playList->comment);
	_playListArea->cell(pme.comment);
	_playListMap.push_back(pme);
}

void DanceEditor::onClick(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	editSequence(p, target);
}

void DanceEditor::onAddSequence(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	Sequence* sequence = _dance->newSequence();
	string levelName = getPreference("defaultLevel");
	sequence->setLevelName(levelName);
	_sequenceArea->reopen(-1, -1);
		showSequenceInfo(_sequenceMap.size(), sequence);
	_sequenceArea->complete();
	SequenceEditor* se = _frame->edit(this, _dance->sequences().size(), sequence);
	_frame->rootCanvas()->run(se, &SequenceEditor::select, 0);
}

void DanceEditor::onOpenContextMenu(display::point p, display::Canvas* target) {
	display::ContextMenu* c = new display::ContextMenu(_sequenceArea->rootCanvas(), p, target);
	int hitRow = hitSequence(p);
	const vector<Sequence*>& sequences = _dance->sequences();
	Sequence* sequence = sequences[hitRow];
	c->choice("Edit this sequence")->click.addHandler(this, &DanceEditor::editSequence);
	c->choice("Perform this sequence")->click.addHandler(this, &DanceEditor::performSequence, sequence);
	c->choice("Print this sequence")->click.addHandler(this, &DanceEditor::printSequence, sequence);

	if (sequence->lastChecked() < defaultDefinitions->lastChanged() ||
		sequence->lastChecked() < myDefinitions->lastChanged())
		c->choice("Recheck this sequence")->click.addHandler(this, &DanceEditor::recheckSequence);

	for (int i = 0; i < sequences.size(); i++) {
		Sequence* s = sequences[i];
		if (s->lastChecked() < defaultDefinitions->lastChanged() ||
			s->lastChecked() < myDefinitions->lastChanged()) {
			c->choice("Recheck all sequences")->click.addHandler(this, &DanceEditor::recheckAllSequences);
			break;
		}
	}
	const vector<PlayList*>& playLists = _dance->playLists();
	bool listSelected = false;
	for (int i = 0; i < playLists.size(); i++) {
		if (_playListMap[i].checked->value()) {
			listSelected = true;
			break;
		}
	}
	for (int i = 0; i < sequences.size(); i++) {
		if (_sequenceMap[i].checked->value()) {
			if (listSelected) {
				c->choice("Print selected play lists and sequences")->click.addHandler(this, &DanceEditor::printSelected);
				c->choice("Perform selected play lists and sequences")->click.addHandler(this, &DanceEditor::performSelected);
			} else {
				c->choice("Print selected sequences")->click.addHandler(this, &DanceEditor::printSelected);
				c->choice("Perform selected sequences")->click.addHandler(this, &DanceEditor::performSelected);
			}
			break;
		}
	}
	c->show();
}

void DanceEditor::onChecked(data::Boolean* b, int index) {
	_body->rootCanvas()->setKeyboardFocus(_body);
	if (b->value())
		_sequenceMap[index].status->setBackground(&display::buttonFaceBackground);
	else
		_sequenceMap[index].status->setBackground(&display::editableBackground);
}

void DanceEditor::onPlayListClick(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	editPlayList(p, target);
}

void DanceEditor::onAddPlayList(display::MouseKeys mKeys, display::point p, display::Canvas* target) {
	PlayList* playList = _dance->newPlayList();
	_playListArea->reopen(-1, -1);
		showPlayListInfo(_playListMap.size(), playList);
	_playListArea->complete();
}

void DanceEditor::onOpenPlayListContextMenu(display::point p, display::Canvas* target) {
	display::ContextMenu* c = new display::ContextMenu(_playListArea->rootCanvas(), p, target);
	int hitRow = hitPlayList(p);
	const vector<PlayList*>& playLists = _dance->playLists();
	PlayList* playList = playLists[hitRow];
	c->choice("Edit this play list")->click.addHandler(this, &DanceEditor::editPlayList);
	c->choice("Perform this play list")->click.addHandler(this, &DanceEditor::performPlayList, playList);
	c->choice("Print this play list's sequences")->click.addHandler(this, &DanceEditor::printPlayList, playList);
	if (playList->lastChecked() < defaultDefinitions->lastChanged() ||
		playList->lastChecked() < myDefinitions->lastChanged())
		c->choice("Recheck this play list")->click.addHandler(this, &DanceEditor::recheckPlayList);

	const vector<Sequence*>& sequences = _dance->sequences();
	bool seqSelected = false;
	for (int i = 0; i < sequences.size(); i++) {
		if (_sequenceMap[i].checked->value()) {
			seqSelected = true;
			break;
		}
	}
	for (int i = 0; i < playLists.size(); i++) {
		if (_playListMap[i].checked->value()) {
			if (seqSelected) {
				c->choice("Print selected play lists and sequences")->click.addHandler(this, &DanceEditor::printSelected);
				c->choice("Perform selected play lists and sequences")->click.addHandler(this, &DanceEditor::performSelected);
			} else {
				c->choice("Print selected play lists")->click.addHandler(this, &DanceEditor::printSelected);
				c->choice("Perform selected play lists")->click.addHandler(this, &DanceEditor::performSelected);
			}
			break;
		}
	}
	c->show();
}

void DanceEditor::onPlayListChecked(data::Boolean* b, int index) {
	_body->rootCanvas()->setKeyboardFocus(_body);
	if (b->value())
		_playListMap[index].comment->setBackground(&display::buttonFaceBackground);
	else
		_playListMap[index].comment->setBackground(&display::editableBackground);
}

display::Color unchecked(0xff8000);

void DanceEditor::grammarChanged() {
	for (int i = 0; i < _sequenceMap.size(); i++) {
		_sequenceMap[i].status->set_value("Unchecked");
		_sequenceMap[i].status->set_textColor(&unchecked);
	}
}

int DanceEditor::hitSequence(display::point p) {
	int deltaY = p.y - _sequenceArea->bounds.topLeft.y;
	int rows = _sequenceArea->rows();
	int hitRow;
	if (rows == 1)
		hitRow = 0;
	else {
		int rowHeight = _sequenceArea->y(1);
		hitRow = deltaY / rowHeight;
	}
	return hitRow;
}

int DanceEditor::hitPlayList(display::point p) {
	int deltaY = p.y - _playListArea->bounds.topLeft.y;
	int rows = _playListArea->rows();
	int hitRow;
	if (rows == 1)
		hitRow = 0;
	else {
		int rowHeight = _playListArea->y(1);
		hitRow = deltaY / rowHeight;
	}
	return hitRow;
}

void DanceEditor::editSequence(display::point p, display::Canvas* target) {
	if (!_listening) {
		_body->rootCanvas()->afterInputEvent.addHandler(&_undoStack, &display::UndoStack::rememberCurrentUndo);
		_listening = true;
	}
	int hitRow = hitSequence(p);
	Sequence* sequence = _dance->sequences()[hitRow];
	_frame->edit(this, hitRow, sequence);
}

void DanceEditor::editPlayList(display::point p, display::Canvas* target) {
	if (!_listening) {
		_body->rootCanvas()->afterInputEvent.addHandler(&_undoStack, &display::UndoStack::rememberCurrentUndo);
		_listening = true;
	}
	int hitRow = hitPlayList(p);
	PlayList* playList = _dance->playLists()[hitRow];
	_frame->edit(this, hitRow, playList);
}

void DanceEditor::printSequence(display::point p, display::Canvas* target, Sequence* sequence) {
	if (startPrinting()) {
		print(sequence);
		donePrinting();
	}
}

void DanceEditor::printPlayList(display::point p, display::Canvas* target, PlayList* playList) {
	vector<Sequence*> sequences;
	playList->fetchSequences(&sequences);
	if (sequences.size() > 0) {
		if (startPrinting()) {
			for (int i = 0; i < sequences.size(); i++)
				print(sequences[i]);
			donePrinting();
		}
	} else
		_frame->setStatus("No sequences to print");
}

void DanceEditor::performSequence(display::point p, display::Canvas* target, Sequence* sequence) {
	vector<PlayList*> playLists;
	vector<Sequence*> sequences;

	sequences.push_back(sequence);
	Performance* perf = new Performance(_frame, _dance, playLists, sequences);
	_frame->start(perf);
}

void DanceEditor::performPlayList(display::point p, display::Canvas* target, PlayList* playList) {
	vector<PlayList*> playLists;
	vector<Sequence*> sequences;

	playLists.push_back(playList);
	Performance* perf = new Performance(_frame, _dance, playLists, sequences);
	_frame->start(perf);
}

void DanceEditor::recheckSequence(display::point p, display::Canvas* target) {
	int hitRow = hitSequence(p);
	recheckSequence(_dance->sequences()[hitRow]);
}

void DanceEditor::recheckPlayList(display::point p, display::Canvas* target) {
	int hitRow = hitPlayList(p);
	PlayList* playList = _dance->playLists()[hitRow];
	vector<Sequence*> sequences;
	playList->fetchSequences(&sequences);
	if (sequences.size()) {
		for (int i = 0; i < sequences.size(); i++)
			recheckSequence(sequences[i]);
	} else
		_frame->setStatus("No sequences to recheck");
}

void DanceEditor::recheckAllSequences(display::point p, display::Canvas* target) {
	for (int i = 0; i < _sequenceMap.size(); i++) {
		Sequence* sequence = _sequenceMap[i].sequence;

		sequence->updateStatus(myDefinitions);
		setSequenceStatus(_sequenceMap[i].status, sequence);
	}
}

void DanceEditor::recheckSequence(Sequence* sequence) {
	sequence->updateStatus(myDefinitions);
	_frame->refreshSequenceEditor(sequence);
	for (int i = 0; i < _sequenceMap.size(); i++)
		if (_sequenceMap[i].sequence == sequence) {
			setSequenceStatus(_sequenceMap[i].status, sequence);
			break;
		}
}

void DanceEditor::printSelected(display::point p, display::Canvas* target) {
	if (startPrinting()) {
		for (int i = 0; i < _sequenceMap.size(); i++) {
			if (_sequenceMap[i].checked->value()) {
				Sequence* sequence = _sequenceMap[i].sequence;

				print(sequence);
			}
		}
		donePrinting();
	}
}

void DanceEditor::performSelected(display::point p, display::Canvas* target) {
	vector<PlayList*> playLists;
	vector<Sequence*> sequences;
	for (int i = 0; i < _playListMap.size(); i++)
		if (_playListMap[i].checked->value())
			playLists.push_back(_playListMap[i].playList);
	for (int i = 0; i < _sequenceMap.size(); i++)
		if (_sequenceMap[i].checked->value())
			sequences.push_back(_sequenceMap[i].sequence);
	Performance* p = new Performance(_frame, _dance, playLists, sequences);
	_frame->start(p);
}

void DanceEditor::setSequenceStatus(display::Label* label, Sequence* sequence) {
	switch (sequence->status) {
	case	SEQ_UNCHECKED:
		label->set_value("Unchecked");
		label->set_textColor(&unchecked);
		break;

	case	SEQ_FAILED:
		label->set_value("Failed");
		label->set_textColor(&display::red);
		break;

	case	SEQ_UNRESOLVED:
		label->set_value("Unresolved");
		label->set_textColor(&display::blue);
		break;

	case	SEQ_READY:
		label->set_value("Ready");
		label->set_textColor(&display::black);
		break;

	default:
		label->set_value("Bad value: " + int(sequence->status));
		label->set_textColor(&display::red);
	}
}

}  // namespace dance
