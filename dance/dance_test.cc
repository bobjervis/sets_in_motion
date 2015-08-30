#include "../common/platform.h"
#include "dance.h"

#include "../common/atom.h"
#include "../common/file_system.h"
#include "../common/parser.h"
#include "../engine/global.h"
#include "../sd/sd.h"
#include "../test/test.h"
#include "call.h"

namespace dance {

class TransformObject : public script::Object {
public:
	static script::Object* factory() {
		return new TransformObject();
	}

	Transform* t() const { return _t; }

	virtual bool validate(script::Parser* parser) {
		return true;
	}

	virtual bool run() {
		int x0 = 0, x1 = 0, x2 = 0, y0 = 0, y1 = 0, y2 = 0;
		Atom* a = get("x0");
		if (a)
			x0 = a->toString().toInt();
		a = get("x1");
		if (a)
			x1 = a->toString().toInt();
		a = get("x2");
		if (a)
			x2 = a->toString().toInt();
		a = get("y0");
		if (a)
			y0 = a->toString().toInt();
		a = get("y1");
		if (a)
			y1 = a->toString().toInt();
		a = get("y2");
		if (a)
			y2 = a->toString().toInt();
		_t = new Transform(x0, x1, x2, y0, y1, y2);
		return runAnyContent();
	}

private:
	TransformObject() {
		_t = null;
	}

	~TransformObject() {
		delete _t;
	}

	Transform* _t;
};

static const char* facingNames[] = {
	"right",	//	RIGHT_FACING,	// > caller's right side of hall
	"back",		//	BACK_FACING,	// ^ back of hall
	"left",		//	LEFT_FACING,	// < caller's left side of hall
	"front",	//	FRONT_FACING,	// v front of hall
	"head",		//	HEAD_FACING,	// | front or back
	"side",		//	SIDE_FACING,	// - left or right
	"any",		//	ANY_FACING,		// ? any value
};

const char* facingName(Facing facing) {
	static char buffer[256];

	if (facing >= RIGHT_FACING &&
		facing <= ANY_FACING)
		return facingNames[facing];
	else {
		sprintf(buffer, "*** Unknwon (%d) ***", facing);
		return buffer;
	}
}

class ApplyObject : public script::Object {
public:
	static script::Object* factory() {
		return new ApplyObject();
	}

private:
	ApplyObject() {}

	virtual bool validate(script::Parser* parser) {
		return true;
	}

	virtual bool run() {
		TransformObject* to;
		if (containedBy(&to)) {
			Transform* t = to->t();
			int x = 0, y = 0, expected_x, expected_y;
			Facing expected_facing = ANY_FACING;
			Facing facing = ANY_FACING;
			Gender gender = UNSPECIFIED_GENDER;
			int couple = 0;
			Atom* a = get("x");
			if (a)
				x = a->toString().toInt();
			a = get("y");
			if (a)
				y = a->toString().toInt();
			a = get("facing");
			if (a) {
				string s = a->toString();
				if (s == "right")
					facing = RIGHT_FACING;
				else if (s == "left")
					facing = LEFT_FACING;
				else if (s == "back")
					facing = BACK_FACING;
				else if (s == "front")
					facing = FRONT_FACING;
				else if (s == "head")
					facing = HEAD_FACING;
				else if (s == "side")
					facing = SIDE_FACING;
				else if (s == "any")
					facing = ANY_FACING;
				else {
					printf("Unknown facing: %s\n", s.c_str());
					return false;
				}
			}
			Dancer* d = new Dancer(x, y, facing, gender, couple, dancerIdx(couple, gender));
			const Dancer* out = t->apply(d);
			bool result = true;
			if (get("check_inverse")) {
				const Dancer* inv = t->revert(out);
				if (d->x != inv->x ||
					d->y != inv->y) {
					t->print(4);
					printf("orig x = %3d trans x = %3d final x = %3d\n", d->x, out->x, inv->x);
					printf("orig y = %3d trans y = %3d final y = %3d\n", d->y, out->y, inv->y);
					printf("Inverse not equal to original location\n");
					result = false;
				} else if (d->facing != inv->facing) {
					t->print(4);
					printf("orig facing = %s", facingName(d->facing));
					printf(" trans = %s", facingName(out->facing));
					printf(" final %s\n", facingName(inv->facing));
					printf("Inverse not equal to original facing\n");
					result = false;
				}
				delete inv;
			} else {
				Atom* a = get("expected_x");
				if (a)
					expected_x = a->toString().toInt();
				a = get("expected_y");
				if (a)
					expected_y = a->toString().toInt();
				if (out->x != expected_x ||
					out->y != expected_y) {
					t->print(4);
					printf("orig x = %3d trans x = %3d expected x = %3d\n", d->x, out->x, expected_x);
					printf("orig y = %3d trans y = %3d expected y = %3d\n", d->y, out->y, expected_y);
					printf("Not transformed to expected location\n");
					result = false;
				}
			}
			delete d;
			delete out;
			return result;
		} else {
			printf("Not contained in a transform object\n");
			return false;
		}
		return runAnyContent();
	}

private:
};

class RevertObject : public script::Object {
public:
	static script::Object* factory() {
		return new RevertObject();
	}

private:
	RevertObject() {}

	virtual bool validate(script::Parser* parser) {
		return true;
	}

	virtual bool run() {
		TransformObject* to;
		if (containedBy(&to)) {
			Transform* t = to->t();
		} else {
			printf("Not contained in a transform object\n");
			return false;
		}
		return runAnyContent();
	}

private:
};

class GrammarObject : public script::Object {
public:
	static script::Object* factory() {
		return new GrammarObject();
	}

	Grammar* grammar() const { return _grammar; }

private:
	GrammarObject() {
		_grammar = null;
	}

	~GrammarObject() {
		delete _grammar;
	}

	virtual bool validate(script::Parser* parser) {
		script::Atom* a = get("filename");
		if (a == null)
			_path = global::dataFolder + "/dance/calls.cdf";
		else
			_path = fileSystem::pathRelativeTo(a->toString(), parser->filename());
		return true;
	}

	virtual bool run() {
		_grammar = new Grammar();
		if (!_grammar->read(_path)) {
			printf("Could not load default definitions\n");
			return false;
		}
		_grammar->compileStateMachines();
		vector<int> variants;
		int productions = 0;
		for (int i = 0; i < levels.size(); i++)
			variants.push_back(0);
		for (int i = 0; i < _grammar->definitions().size(); i++) {
			const Definition* def = _grammar->definitions()[i];
			productions += def->productions().size();
			for (int j = 0; j < def->variants().size(); j++) {
				const Variant* v = def->variants()[j];
				int level = v->effectiveLevel();
				if (level == ERROR_LEVEL) {
					printf("*** Variant in definition for %s is in error ***\n", def->label().c_str());
				}
				if (v->patterns().size() == 0)
					variants[level]++;
				else
					variants[level] += v->patterns().size();
			}
		}
		if (verboseParsing)
			_grammar->printStateMachines();
		_grammar->printStateMachineStats();
		dictionary<const Term*>::iterator i = _grammar->_words.begin();
		dictionary<int> counts;
		dictionary<int> reservedCounts;
		int reservedWords = 0;
		while (i.valid()) {
			const Term* t = *i;
			int* v = counts.get(typeid(*t).name());
			const string& s = i.key();
			for (int k = 0; k < s.size(); k++)
				if (!isalpha(s[k])) {
					int* x = reservedCounts.get(typeid(*t).name());
					if (*x == 0)
						reservedCounts.insert(typeid(*t).name(), 1);
					else
						(*x)++;
					reservedWords++;
					break;
				}
			if (*v == 0)
				counts.insert(typeid(*t).name(), 1);
			else
				(*v)++;
			i.next();
		}
		dictionary<int>::iterator c = counts.begin();
		int tot = 0;
		printf("%40s  %5s/%5s\n\n", "Class", "total","plain");
		while (c.valid()) {
			tot += *c;
			int* v = reservedCounts.get(c.key());
			printf("%40s: %5d/%5d\n", c.key().c_str(), *c, *c - *v);
			c.next();
		}
		printf("%40s  ----- -----\n%40s  %5d/%5d\n", "", "Total", tot, tot - reservedWords);
		printf("Total definitions %d\n", _grammar->definitions().size());
		printf("Total formations %d\n", _grammar->formations().size());
		printf("Total productions %d\n", productions);
		for (int i = 0; i < variants.size(); i++)
			printf("   Total variants for level %-20s %5d\n", levels[i].c_str(), variants[i]);
		return runAllContent();
	}

	string _path;
	Grammar* _grammar;
};

class ParseObject : public script::Object {
public:
	static script::Object* factory() {
		return new ParseObject();
	}

	bool expect(const string& s) {
		script::Atom* a = get("finalPartial");
		if (a) {
			_productions.push_back(s);
			return true;
		}
		for (int i = 0; i < _productions.size(); i++) {
			if (_productions[i] == s) {
				_productions.remove(i);
				return true;
			}
		}
		printf("Could not find expected production '%s'\n", s.c_str());
		return false;
	}
private:
	ParseObject() {}

	virtual bool validate(script::Parser* parser) {
		script::Atom* a;

		a = get("sentence");
		if (a == null) {
			printf("Need a 'sentence' to parse\n");
			return false;
		}
		_sentence = a->toString();
		a = get("level");
		if (a == null)
			_level = levels.size() - 1;
		else {
			string v = a->toString();
			for (_level = 0; _level < levels.size(); _level++)
				if (v == levels[_level])
					break;
			if (_level >= levels.size()) {
				printf("Unknown level name\n");
				return false;
			}
		}
		return true;
	}

	virtual bool run() {
		GrammarObject* go;
		if (containedBy(&go)) {
			Grammar* g = go->grammar();

			script::Atom* a = get("goal");
			if (a == null)
				_goalSymbol = ANYCALL;
			else {
				string lower = a->toString().tolower();
				const Term* t = g->lookup(lower);

				if (t == null) {
					printf("Unknown goal symbol: %s\n", a->toString().c_str());
					return false;
				}
				if (typeid(*t) != typeid(BuiltIn)) {
					printf("Goal symbol '%s' not a non-Terminal\n", a->toString().c_str());
					return false;
				}
				_goalSymbol = ((const BuiltIn*)t)->index();
			}
			a = get("finalPartial");
			if (a != null) {
				string expectedFinalPartial = a->toString();
				Context context(null, g);
				Stage tempStorage(null, null);
				context.startStage(&tempStorage);
				vector<Token> tokens;
				Token finalPartial;
				if (g->tokenize(null, _sentence, false, null, &context, null, tokens, &finalPartial)) {
					printf("'%s' Tokenized as:\n", _sentence.c_str());
					for (int i = 0; i < tokens.size(); i++) {
						printf("    [%2d] ", i);
						tokens[i].print();
					}
					printf("    finalPartial: ");
					finalPartial.print();
				} else {
					printf("Tokenize('%s') failed\n", _sentence.c_str());
					return false;
				}
				a = get("match");
				if (a) {
					int matched;
					vector<int> partialStates;
					const Anything* a = g->matchAnycall(false, tokens, 0, true, &matched, &partialStates, &context);
					if (a) {
						printf("matched %d tokens\n", matched);
						printf("matchAnycall returned:\n");
						a->print(4);
					} else
						printf("no call matched.\n");
					for (int i = 0; i < partialStates.size(); i++) {
						printf("matched to state %d\n", partialStates[i]);
					}
				}
				bool result = true;
				a = get("content");
				if (a) {
					if (finalPartial.type == END_OF_STRING) {
						if (expectedFinalPartial.size() != 0) {
							printf("*** No finalPartial found, expecting '%s'\n", expectedFinalPartial.c_str());
							result = false;
						}
					} else if (expectedFinalPartial.size() == 0) {
						if (finalPartial.type != END_OF_STRING) {
							printf("*** Unexpected finalPartial\n");
							printf("Expected: <none>\n");
							printf("Actual:   '%s'\n", finalPartial.text.c_str());
							result = false;
						}
					} else if (finalPartial.text != expectedFinalPartial) {
						printf("*** Unexpected finalPartial\n");
						printf("Expected: '%s'\n", expectedFinalPartial.c_str());
						printf("Actual:   '%s'\n", finalPartial.text);
						result = false;
					}
					result &= runAnyContent();
					if (result) {
						if (_productions.size() != tokens.size()) {
							printf("***Unexpected number of tokens\n");
							printf("Expected: %d\n", _productions.size());
							printf("Actual:   %d\n", tokens.size());
							result = false;
						}
						for (int i = 0; i < _productions.size() || i < tokens.size(); i++) {
							if (i < _productions.size())
								printf("Expected [%2d]: '%s'\n", i, _productions[i].c_str());
							else
								printf("Expected [%2d]: <none>\n", i);
							if (i < tokens.size()) {
								printf("Actual:  [%2d]: ", i);
								tokens[i].print();
							} else
								printf("Actual:  [%2d]: <none>\n", i);
							if (i < _productions.size() && i < tokens.size()) {
								if (tokens[i].type == WORD ||
									tokens[i].type == UNKNOWN_WORD) {
									if (tokens[i].text != _productions[i]) {
										printf("*** Unexpected value\n");
										result = false;
									}
								}
							}
						}
					}
				}
				return result;
			}
			bool result = g->parsePartial(_goalSymbol, _sentence, _level, &_productions);

			if (!result)
				printf("No productions matched sentence '%s' at level %s\n", _sentence.c_str(), levels[_level].c_str());
			a = get("content");
			if (a) {
				result &= runAnyContent();
				if (result) {
					if (_productions.size() > 0) {
						printf("Sentence '%s' has unexpected productions in output:\n", _sentence.c_str());
						for (int i = 0; i < _productions.size(); i++)
							printf("> %s\n", _productions[i].c_str());
						return false;
					}
				}
			} else {
				if (result)
					printf("Sentence '%s' can match %s productions at level %s:\n", _sentence.c_str(), tokenNames[_goalSymbol], levels[_level].c_str());
				for (int i = 0; i < _productions.size(); i++) {
					printf("[%d] %s\n", i + 1, _productions[i].c_str());
				}
			}
			return result;
		} else {
			printf("Not contained in a grammar object\n");
			return false;
		}
	}

	string	_sentence;
	Level	_level;
	TokenType _goalSymbol;
	vector<string> _productions;
};

class ExpectObject : public script::Object {
public:
	static script::Object* factory() {
		return new ExpectObject();
	}

private:
	ExpectObject() {}

	virtual bool validate(script::Parser* parser) {
		return true;
	}

	virtual bool run() {
		ParseObject* po;

		if (containedBy(&po)) {
			script::Atom* a = get("content");
			if (a == null) {
				printf("No content\n");
				return false;
			}
			return po->expect(a->toString());
		} else {
			printf("Not contained by a d_parse object\n");
			return false;
		}
	}
};

class DanceObject : public script::Object {
public:
	static script::Object* factory() {
		return new DanceObject();
	}

private:
	DanceObject() {
		_localGrammar = null;
	}

	~DanceObject() {
		delete _localGrammar;
	}

	virtual bool validate(script::Parser* parser) {
		script::Atom* a = get("filename");
		if (a == null) {
			printf("No filename attribute on d_dance tag\n");
			return false;
		}
		_path = fileSystem::pathRelativeTo(a->toString(), parser->filename());
		return true;
	}

	virtual bool run() {
		GrammarObject* go;
		Grammar* g;
		if (containedBy(&go))
			g = go->grammar();
		else {
			g = new Grammar();
			_localGrammar = g;
			if (!g->read(global::dataFolder + "/dance/calls.cdf")) {
				printf("Could not load default definitions\n");
				return false;
			}
		}
		bool result = true;
		int reportCoverLevel = -1;
		dictionary<int> variantMap;
		script::Atom* a = get("cover");
		if (a) {
			bool matchedOne = false;
			string c = a->toString();
			for (int i = 0; i < levels.size(); i++) {
				if (c == levels[i]) {
					reportCoverLevel = i;
					matchedOne = true;
					const vector<Definition*>& defs = g->definitions();
					for (int j = 0; j < defs.size(); j++) {
						Definition* def = defs[j];
						const vector<Variant*>& vars = def->variants();
						if (vars.size() == 0) {
							printf(" *** Definition %s has no variants.\n", def->label().c_str());
							result = false;
						}
						for (int k = 0; k < vars.size(); k++) {
							if (vars[k]->effectiveLevel() != i)
								continue;
							string s;
							void* buffer = s.buffer_(sizeof (VariantTile));
							if (vars[k]->patterns().size() == 0) {
								VariantTile vt(vars[k], null);
								memcpy(buffer, &vt, sizeof vt);
								variantMap.insert(s, 0);
							} else {
								Variant* v = vars[k];
								for (int m = 0; m < v->recognizers().size(); m++) {
									VariantTile vt(v, v->recognizers()[m]);
									memcpy(buffer, &vt, sizeof vt);
									vt.pattern = null;
									variantMap.insert(s, 0);
								}
							}
						}
					}
				}
			}
			if (!matchedOne) {
				printf("Unknown level: %s\n", c.c_str());
				return false;
			}
		}
		Dance* d;
		if (_path.tolower().endsWith(".dnc")) {
			d = new Dance(_path, _path);
			if (!d->read()) {
				printf("Errors in file, contents might be corrupted: %s\n", _path.c_str());
				return false;
			}
		} else
			d = parseSdFile(_path);
		if (d == null) {
			printf("Can't load file %s\n", _path.c_str());
			return false;
		}
		g->compileStateMachines();
		a = get("allowUnresolved");
		if (!d->runAll(a != null, g)) {
			printf("Some sequence failed to resolve\n");
			result = false;
		}
		int calls = 0;
		int failedCalls = 0;
		int resolvedSequences = 0;
		const vector<Sequence*>& seqs = d->sequences();
		for (int j = 0; j < seqs.size(); j++) {
			Sequence* seq = seqs[j];
			const vector<const Stage*>& stages = seq->stages();
			calls += stages.size();
			for (int k = 0; k < stages.size(); k++)
				if (stages[k]->failed())
					failedCalls++;
			if (seq->resolved())
				resolvedSequences++;
		}
		printf("%s\n   Total sequences %d (resolved %d %0.1f%%) / calls %d (passed %d %0.1f%%)\n", _path.c_str(), seqs.size(), resolvedSequences, (100.0 * resolvedSequences) / seqs.size(), calls, calls - failedCalls, (100.0 * (calls - failedCalls)) / calls);
		if (reportCoverLevel >= 0) {
			for (int j = 0; j < seqs.size(); j++) {
				Sequence* seq = seqs[j];
				const vector<const Stage*>& stages = seq->stages();
				for (int k = 0; k < stages.size(); k++) {
					const Stage* s = stages[k];
					if (s->failed())
						continue;
					if (verboseOutput)
						printf("Try seq %d / %d:\n", j, k);
					explore(stages[k], variantMap);
				}
			}
			int uncovered = 0;
			int covered = 0;
			dictionary<int>::iterator i = variantMap.begin();
			while (i.valid()) {
				const string& key = i.key();
				VariantTile* vt = (VariantTile*)&key[0];
				int value = *i;
				if (verboseOutput)
					printf("%5d: %s / %s\n", value, vt->variant->definition()->label().c_str(), vt->pattern ? vt->pattern->formation()->name().c_str() : "<default>");
				if (value == 0) {
					printf(" *** Uncovered: %s / %s%c\n", vt->variant->definition()->label().c_str(), vt->pattern ? vt->pattern->formation()->name().c_str() : (vt->variant->patterns().size() ? vt->variant->patterns()[0].c_str() : "<default>"),
						vt->pattern || vt->variant->patterns().size() == 0 ? ' ' : '*');
					uncovered++;
				} else
					covered++;
				i.next();
			}
			if (uncovered > 0) {
				printf("Total %d uncovered variants / %d covered (%d total %0.1f%%)\n", uncovered, covered, uncovered + covered, 100 * covered / (double)(covered + uncovered));
				result = false;
			} else
				printf("All %d variants covered (100%%)\n", covered);
		}
		for (int j = 0; j < seqs.size(); j++) {
			Sequence* seq = seqs[j];
			seq->clearStages();
		}
		delete d;
		if (!result)
			return false;
		return runAnyContent();
	}

	void explore(const Plan* p, dictionary<int>& variantMap) {
		if (!exploreCheck(p, variantMap))
			return;
		for (int i = 0; i < p->stepCount(); i++) {
			if (verboseOutput)
				printf("Step %d:\n", i);
			Step* step = p->step(i);
			for (int j = 0; j < step->collapsed().size(); j++)
				exploreCheck(step->collapsed()[j], variantMap);
			const vector<Tile*>& tiles = step->tiles();
			for (int j = 0; j < tiles.size(); j++) {
				if (verboseOutput)
					printf("Tile %d:\n", j);
				explore(tiles[j]->plan(), variantMap);
			}
		}
	}

	bool exploreCheck(const Plan* p, dictionary<int>& variantMap) {
		if (p->applied()) {
			const Variant* v = p->applied();
			const Pattern* pattern = p->matched();

			VariantTile vt(v, pattern);

			if (verboseOutput) {
				printf("probe %s ", v->definition()->label().c_str());
				if (v->patterns().size())
					printf("%s", v->patterns()[0].c_str());
				else
					printf("<none>");
				printf("/");
				if (pattern)
					printf("%s:%s", pattern->formation()->name().c_str(), pattern->parameterList().c_str());
				else
					printf("<null>");
				printf("\n");
			}
			string s;
			void* buffer = s.buffer_(sizeof (VariantTile));
			memcpy(buffer, &vt, sizeof vt);
			vt.pattern = null;
			if (variantMap.probe(s)) {
				if (verboseOutput)
					printf("hit!\n");
				int* entry = variantMap.get(s);
				(*entry)++;
			}
			const string& defName = v->definition()->name();
			// These 'connective' calls don't stop us from counting
			// their constituent parts.  Otherwise, concepts do such 
			// extensive modification of their operands that we can't
			// count a call under a concept as being 'covered'
			if (defName != "transparent_")
				return false;
		}
		return true;
	}

private:
	string _path;
	Grammar* _localGrammar;
};


void initTestObjects() {
	script::objectFactory("d_grammar", GrammarObject::factory);
	script::objectFactory("d_dance", DanceObject::factory);
	script::objectFactory("d_transform", TransformObject::factory);
	script::objectFactory("d_parse", ParseObject::factory);
	script::objectFactory("d_expect", ExpectObject::factory);
	script::objectFactory("t_apply", ApplyObject::factory);
	script::objectFactory("t_revert", RevertObject::factory);

	// This loads reference data
	loadLevels();
	initializePrecedences();
	atexit(clearMemory);
}

}  // namespace dance

