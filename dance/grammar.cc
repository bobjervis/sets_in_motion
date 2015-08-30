#include "../common/platform.h"
#include "dance.h"

#include <ctype.h>
#include "../common/file_system.h"
#include "../common/locale.h"
#include "../common/timing.h"
#include "call.h"
#include "motion.h"
#include "stored_data.h"

namespace dance {

const char* tokenNames[] = {
	"ERROR_TOKEN",
	"UNKNOWN_WORD",
	"WORD",
	"INTEGER",
	"ORDINAL",
	"COUPLE_NUMBER",
	"SLASH",
	"DASH",
	"PLUS",
	"ASTERISK",
	"NOT",
	"NOT_EQ",
	"EQ_EQ",
	"GTR",
	"GTR_EQ",
	"LSS",
	"LSS_EQ",
	"OR",
	"AND",
	"XOR",
	"REMAINDER",
	"LPAREN",
	"RPAREN",
	"COMMA",
	"EQ",
	"END_OF_STRING",

	"R_L",
	"P_C",
	"ANYONE",
	"ANYTHING",
	"ANYCALL",
	"ANYDIRECTION",
	"FORMATION",
	"FRACTION",
	"VARIABLE",
	"DANCER_NAME",
	"PRIMITIVE",
};

const char* pivotNames[] = {
	"$center",
	"$box_center",
	"$split_center",
	"$line_center",
	"$inside_hand",
	"$outside_hand",
	"$last_hand",
	"$left_hand",
	"$inside_dancer",
	"$outside_dancer",
	"$left_dancer",
	"$right_hand",
	"$right_dancer",
	"$hand",
	"$self",
	"$nose",
	"$tail",
	"$inside_shoulder",
	"$left_2_dancers",
};

const char* directionNames[] = {
	"$as_you_are",
	"left",
	"right",
	"forward",
	"back",
	"in",
	"out",
	"promenade",
	"$reverse_promenade",
	"partner",
	"corner",
	"$original_partner",
	"$original_corner",
	"$last",
	"$away_from_partner",
};

const char* dancerNames[] = {
	"$dancerA",
	"$dancerB",
	"$dancerC",
	"$dancerD",
	"$dancerE",
	"$dancerF",
	"$dancerG",
	"$dancerH",
	"$dancerI",
	"$dancerJ",
	"$dancerK",
	"$dancerL",
};

string Definition::_emptyLabel("<new>");

static bool validWordContent(char c, bool inDefinition);

class DefinitionsContext {
public:
	DefinitionsContext() {
		definition = null;
	}

	int		line;
	string	text;
	const string*	filename;
	Definition*		definition;
};

class CallParser {
public:
	CallParser(const string& input, const Grammar* grammar, const Plan* variantPlan, bool inDefinition, Token* finalPartial) {
		_inputs.push_back(Input(input));
		_inDefinition = inDefinition;
		_finalPartial = finalPartial;
		_grammar = grammar;
		_variantPlan = variantPlan;
		if (finalPartial)
			finalPartial->type = END_OF_STRING;
		_tokens = null;
	}

	CallParser(const vector<Token>& tokens, int startAt) {
		_tokens = &tokens;
		_at = startAt;
		_finalPartial = null;
	}

	bool verifyCall() {
		scan();
		for (;;) {
			int wh, num, denom;

			const Term* t;
			switch(_token.type) {
			case	UNKNOWN_WORD:
			case	WORD:
				break;

			case	INTEGER:
				wh = _token.value;
				scan();
				if (_token.type == WORD &&
					_token.term == _grammar->and()) {
					scan();
					if (_token.type == INTEGER) {
						num = _token.value;
						scan();
						if (_token.type != SLASH)
							return false;
						scan();
						if (_token.type != INTEGER)
							return false;
						if (_token.value == 0)
							return false;
						break;
					} else {
						// fall through for most recent token
					}
				} else if (_token.type == SLASH) {
					scan();
					if (_token.type != INTEGER)
						return false;
					if (_token.value == 0)
						return false;
					break;
				} else {
					// continue to skip another call to 'scan()'
					continue;
				}
				break;

			default:
				break;

			case	ERROR_TOKEN:
				return false;

			case	END_OF_STRING:
				return true;
			}
			scan();
		}
	}

	const Term* parseExpression(const Anything* call, Context* context) {
		const Term* f = parseExpression(0, call, context);
		if (_token.type == END_OF_STRING)
			return f;
		else
			return null;
	}

	const Term* parseExpressionPrefix(const Anything* call, Context* context) {
		return parseExpression(0, call, context);
	}

	void scan() {
		if (_tokens) {
			if (_at >= _tokens->size())
				_token.type = END_OF_STRING;
			else
				_token = (*_tokens)[_at++];
			return;
		}
		int start;
		const Term* t;

		while (_inputs.size() > 0) {
			while (textRemaining()) {
				char c = nextChar();
				switch (c) {
				case	'0':
				case	'1':
				case	'2':
				case	'3':
				case	'4':
				case	'5':
				case	'6':
				case	'7':
				case	'8':
				case	'9':
					start = cursor();
					do
						skipChar();
						while (textRemaining() &&
							   isdigit(nextChar()));
					contents(start);
					_token.value = _token.text.toInt();
					_token.type = INTEGER;
					return;

				case	'!':
					skipChar();
					if (_inDefinition) {
						_token.type = NOT;
						if (textRemaining()) {
							if (nextChar() == '=') {
								skipChar();
								_token.type = NOT_EQ;
							}
						}
					} else {
						_token.text = "!";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'=':
					skipChar();
					if (_inDefinition) {
						if (textRemaining()) {
							if (nextChar() == '=') {
								skipChar();
								_token.type = EQ_EQ;
								return;
							}
						}
					}
					_token.type = EQ;
					return;

				case	'<':
					skipChar();
					if (_inDefinition) {
						_token.type = LSS;
						if (textRemaining()) {
							if (nextChar() == '=') {
								skipChar();
								_token.type = LSS_EQ;
							}
						}
					} else {
						_token.text = "<";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'>':
					skipChar();
					if (_inDefinition) {
						_token.type = GTR;
						if (textRemaining()) {
							if (nextChar() == '=') {
								skipChar();
								_token.type = GTR_EQ;
							}
						}
					} else {
						_token.text = ">";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'|':
					skipChar();
					if (_inDefinition)
						_token.type = OR;
					else {
						_token.text = "|";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'&':
					skipChar();
					if (_inDefinition)
						_token.type = AND;
					else {
						_token.text = "&";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'^':
					skipChar();
					if (_inDefinition)
						_token.type = XOR;
					else {
						_token.text = "^";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'/':
					skipChar();
					_token.type = SLASH;
					return;

				case	'%':
					skipChar();
					if (_inDefinition)
						_token.type = REMAINDER;
					else {
						_token.text = "%";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'-':
					skipChar();
					if (_inDefinition)
						_token.type = DASH;
					else {
						_token.text = "-";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'+':
					skipChar();
					if (_inDefinition)
						_token.type = PLUS;
					else {
						_token.text = "+";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'*':
					skipChar();
					if (_inDefinition)
						_token.type = ASTERISK;
					else {
						_token.text = "*";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	'(':
					skipChar();
					if (_inDefinition)
						_token.type = LPAREN;
					else {
						_token.text = "(";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	')':
					skipChar();
					if (_inDefinition)
						_token.type = RPAREN;
					else {
						_token.text = ")";
						_token.type = ERROR_TOKEN;
					}
					return;

				case	',':
					skipChar();
					if (_inDefinition) {
						_token.type = COMMA;
						return;
					}
					break;

				case	'[':
					skipChar();
					if (_inDefinition) {
						_token.text = "[";
						_token.type = ERROR_TOKEN;
						return;
					}
					break;

				case	']':
					skipChar();
					if (_inDefinition) {
						_token.text = "]";
						_token.type = ERROR_TOKEN;
						return;
					}
					break;

				case	' ':
				case	'\t':
				case	'\n':
				case	'\r':
				case	':':
				case	';':
				case	'.':
				case	'?':
					skipChar();
					break;

				case	'$':
					if (_inDefinition) {
						const char* p = peekChar(1);
						if (p && isdigit(*p)) {
							_token.type = VARIABLE;
							_token.value = *p - '0';
							if (_token.value >= 0) {
								skipChar();
								skipChar();
								return;
							}
						}
					}
					// fall through to be treated as an error

				default:
					if (validWordContent(c, _inDefinition)) {
						start = cursor();
						do
							skipChar();
							while (textRemaining() &&
								   validWordContent(nextChar(), _inDefinition));
						contents(start);
						if (_finalPartial && !textRemaining() && _inputs.size() == 1) {
							_token.type = FINAL_PARTIAL;
							_token.term = null;
							return;
						}
						if (_grammar) {
							const Synonym* s = _grammar->synonym(_token.text);
							if (s) {
								_inputs.push_back(Input(s->value()));
								break;
							}
							t = _grammar->lookup(_token.text);
						} else
							t = null;
						if (_variantPlan) {
							if (t) {
								const Anyone* v = _variantPlan->get(t);
								if (v != null) {
									_token.type = WORD;
									_token.term = v;
									return;
								}
							}
						}
						if (t == null) {
							_token.type = UNKNOWN_WORD;
							return;
						}
						t->token(&_token);
						return;
					}
					_token.text = string(&nextChar(), 1);
					skipChar();
					_token.type = ERROR_TOKEN;
					return;
				}
			}
			_inputs.resize(_inputs.size() - 1);
		}
		_token.type = END_OF_STRING;
		_token.text.clear();
	}

	const Token& token() const { return _token; }

	int at() const { return _at; }

private:
	class Input {
	public:
		Input(const string& input) {
			_input = &input;
			_cursor = 0;
		}

		Input() {
		}

		bool textRemaining() const {
			return _cursor < _input->size();
		}

		const char& nextChar() const {
			return (*_input)[_cursor];
		}

		const char* peekChar(int i) const {
			if (_cursor < _input->size() - i)
				return &(*_input)[_cursor + i];
			else
				return null;
		}

		int cursor() const { return _cursor; }

		void skipChar() { _cursor++; }

		void contents(int start, string* output) const {
			*output = _input->substr(start, _cursor - start);
		}

	private:
		const string* _input;
		int _cursor;
	};

	const Term* parseExpression(int old_precedence, const Anything* call, Context* context) {
		const Term* x;
		const Term* y;

		x = parseTerm(call, context);
		if (x == null)
			return null;
		scan();
		for (;;) {
			static int precedence[] = {
				-1,	//	ERROR_TOKEN,
				-1,	//	UNKNOWN_WORD,
				-1,	//	WORD,
				-1,	//	INTEGER,
				-1,	//	ORIDNAL
				-1,	//	COUPLE_NUMBER
				5,	//	SLASH,
				4,	//	DASH,
				4,	//	PLUS,
				5,	//	ASTERISK,
				-1,	//	NOT,
				3,	//	NOT_EQ,
				3,	//	EQ_EQ,
				2,	//	GTR,
				2,	//	GTR_EQ,
				2,	//	LSS,
				2,	//	LSS_EQ,
				1,	//	OR,
				1,	//	AND,
				1,	//	XOR,
				5,	//	REMAINDER,
				-1,	//	LPAREN,
				0,	//	RPAREN,
				0,	//  COMMA
				0,	//	EQ
				0,	//	END_OF_STRING,
			};
			TokenType op;

			op = _token.type;
			if (precedence[op] < 0)
				return null;
			if (precedence[op] <= old_precedence)
				return x;
			y = parseExpression(precedence[op], call, context);
			if (y == null)
				return null;
			int compareResult;
			switch (op) {
			case	DASH:
				x = x->subtract(y, context);
				break;

			case	PLUS:
				x = x->add(y, context);
				break;

			case	ASTERISK:
				x = x->multiply(y, context);
				break;

			case	SLASH:
				x = x->divide(y, context);
				break;

			case	AND:
				x = x->and(y, context);
				break;

			case	OR:
				x = x->or(y, context);
				break;

			case	XOR:
				x = x->xor(y, context);
				break;

			case	EQ_EQ:
				if (!x->compare(y, &compareResult))
					return null;
				x = context->stage()->newInteger(compareResult == 0);
				break;

			case	GTR:
				if (!x->compare(y, &compareResult))
					return null;
				if (compareResult == INT_MAX)
					return null;
				x = context->stage()->newInteger(compareResult > 0);
				break;

			case	LSS:
				if (!x->compare(y, &compareResult))
					return null;
				if (compareResult == INT_MAX)
					return null;
				x = context->stage()->newInteger(compareResult < 0);
				break;

			case	GTR_EQ:
				if (!x->compare(y, &compareResult))
					return null;
				if (compareResult == INT_MAX)
					return null;
				x = context->stage()->newInteger(compareResult >= 0);
				break;

			case	LSS_EQ:
				if (!x->compare(y, &compareResult))
					return null;
				if (compareResult == INT_MAX)
					return null;
				x = context->stage()->newInteger(compareResult <= 0);
				break;

			case	NOT_EQ:
				if (!x->compare(y, &compareResult))
					return null;
				x = context->stage()->newInteger(compareResult != 0);
				break;
			}
			if (x == null)
				return null;
		}
	}

	const Term* parseTerm(const Anything* call, Context* context) {
		const Term* x;
		const Term* t;
		const Fraction* f;

		scan();
		switch (_token.type) {
		case	DASH:
			x = parseTerm(call, context);
			if (x == null)
				return null;
			return x->negate(context);

		case	NOT:
			x = parseTerm(call, context);
			if (x == null)
				return null;
			return x->not(context);

		case	PLUS:
			x = parseTerm(call, context);
			if (x == null)
				return null;
			return x->positive(context);

		case	INTEGER:
			return context->stage()->newFraction(_token.value, 0, 1);

		case	FRACTION:
			return _token.term;

		case	VARIABLE:
			if (call == null)
				return null;
			if (_token.value > call->variables().size())
				return null;
			if (_token.value == 0)		// $0 - is not a number, so it makes no sense in expressions
				return null;
			x = call->variables()[_token.value - 1];
			if (typeid(*x) == typeid(Integer)) {
				const Integer* i = (const Integer*)x;

				return context->stage()->newFraction(i->value(), 0, 1);
			}
			return x;

		case	LPAREN:
			x = parseExpression(0, call, context);
			if (x == null)
				return null;
			if (_token.type == RPAREN)
				return x;
			return null;

		case	WORD:
			return _token.term;

		default:
			return null;
		}
	}

	bool textRemaining() const {
		return _inputs[_inputs.size() - 1].textRemaining();
	}

	const char& nextChar() const {
		return _inputs[_inputs.size() - 1].nextChar();
	}

	const char* peekChar(int i) const {
		return _inputs[_inputs.size() - 1].peekChar(i);
	}

	int cursor() const {
		return _inputs[_inputs.size() - 1].cursor();
	}

	void skipChar() {
		_inputs[_inputs.size() - 1].skipChar();
	}

	void contents(int start) {
		_inputs[_inputs.size() - 1].contents(start, &_token.text);
	}

	vector<Input> _inputs;
	const Grammar*	_grammar;
	const Plan* _variantPlan;		// Plan that contains any local variables
	bool _inDefinition;				// true if parsing a definition (production or action) string, not a call
	Token* _finalPartial;			// non-null if parsing partial user input, so final (partial) token will be stored here
	Token	_token;					// Last scanned token
	const vector<Token>* _tokens;
	int _at;						// if _tokens != null, _at is the next token to scan from _tokens.
};

Grammar::Grammar() {
	_danceType = D_4COUPLE;
	_error = false;
	_backupGrammar = null;
	_couple = null;
	_changeHandler = null;

	_termStorage = new Stage(null, null);

	_and = keyword("and");

	_words.insert("$as_you_are", new Anydirection(D_AS_YOU_ARE));
	_words.insert("left", new Anydirection(D_LEFT));
	_words.insert("right", new Anydirection(D_RIGHT));
	_words.insert("forward", new Anydirection(D_FORWARD));
	_words.insert("back", new Anydirection(D_BACK));
	_words.insert("in", new Anydirection(D_IN));
	_words.insert("out", new Anydirection(D_OUT));
	_words.insert("partner", new Anydirection(D_PARTNER));
	_words.insert("corner", new Anydirection(D_CORNER));
	_words.insert("promenade", new Anydirection(D_PROMENADE));
	_words.insert("$reverse_promenade", new Anydirection(D_REVERSE_PROMENADE));
	_words.insert("$original_partner", new Anydirection(D_ORIGINAL_PARTNER));
	_words.insert("$original_corner", new Anydirection(D_ORIGINAL_CORNER));
	_words.insert("$last", new Anydirection(D_LAST));
	_words.insert("$away_from_partner", new Anydirection(D_AWAY_FROM_PARTNER));

	_words.insert("$center", new Anypivot(P_CENTER));
	_words.insert("$box_center", new Anypivot(P_BOX_CENTER));
	_words.insert("$split_center", new Anypivot(P_SPLIT_CENTER));
	_words.insert("$line_center", new Anypivot(P_LINE_CENTER));
	_words.insert("$inside_hand", new Anypivot(P_INSIDE_HAND));
	_words.insert("$outside_hand", new Anypivot(P_OUTSIDE_HAND));
	_words.insert("$last_hand", new Anypivot(P_LAST_HAND));
	_words.insert("$left_hand", new Anypivot(P_LEFT_HAND));
	_words.insert("$inside_dancer", new Anypivot(P_INSIDE_DANCER));
	_words.insert("$outside_dancer", new Anypivot(P_OUTSIDE_DANCER));
	_words.insert("$left_dancer", new Anypivot(P_LEFT_DANCER));
	_words.insert("$left_two_dancers", new Anypivot(P_LEFT_TWO_DANCERS));
	_words.insert("$right_hand", new Anypivot(P_RIGHT_HAND));
	_words.insert("$right_dancer", new Anypivot(P_RIGHT_DANCER));
	_words.insert("$hand", new Anypivot(P_HAND));
	_words.insert("$self", new Anypivot(P_SELF));
	_words.insert("$nose", new Anypivot(P_NOSE));
	_words.insert("$tail", new Anypivot(P_TAIL));
	_words.insert("$inside_shoulder", new Anypivot(P_INSIDE_SHOULDER));

	_words.insert("$until_home", Fraction::untilHome());

	_words.insert("r_l", new BuiltIn(R_L));
	_words.insert("p_c", new BuiltIn(P_C));
	_words.insert("anyone", new BuiltIn(ANYONE));
	_words.insert("anything", new BuiltIn(ANYTHING));
	_words.insert("anycall", new BuiltIn(ANYCALL));
	_words.insert("anydirection", new BuiltIn(ANYDIRECTION));
	_words.insert("integer", new BuiltIn(INTEGER));
	_words.insert("fraction", new BuiltIn(FRACTION));
	_words.insert("couple_number", new BuiltIn(COUPLE_NUMBER));
	_words.insert("ordinal", new BuiltIn(ORDINAL));
	_words.insert("$primitive", new BuiltIn(PRIMITIVE));
	_words.insert("$dancer_name", new BuiltIn(DANCER_NAME));

	_words.insert("$activate", new Primitive(P_ACTIVATE));
	_words.insert("$any_who_can", new Primitive(P_ANY_WHO_CAN));
	_words.insert("$arc", new Primitive(P_ARC));
	_words.insert("$arc_face", new Primitive(P_ARC_FACE));
	_words.insert("$back_out", new Primitive(P_BACK_OUT));
	_words.insert("$breathe", new Primitive(P_BREATHE));
	_words.insert("$can_roll", new Primitive(P_CAN_ROLL));
	_words.insert("$can_start", new Primitive(P_CAN_START));
	_words.insert("$check_sequence", new Primitive(P_CHECK_SEQUENCE));
	_words.insert("$circle", new Primitive(P_CIRCLE));
	_words.insert("$circle_fraction", new Primitive(P_CIRCLE_FRACTION));
	_words.insert("$circle_home", new Primitive(P_CIRCLE_HOME));
	_words.insert("$closer_to_center", new Primitive(P_CLOSER_TO_CENTER));
	_words.insert("$conjure_phantom", new Primitive(P_CONJURE_PHANTOM));
	_words.insert("$definition", new Primitive(P_DEFINITION));
	_words.insert("$displace", new Primitive(P_DISPLACE));
	_words.insert("$dont_breathe", new Primitive(P_DONT_BREATHE));
	_words.insert("$face", new Primitive(P_FACE));
	_words.insert("$form_promenade", new Primitive(P_FORM_PROMENADE));
	_words.insert("$form_ring", new Primitive(P_FORM_RING));
	_words.insert("$form_thar", new Primitive(P_FORM_THAR));
	_words.insert("$form_set", new Primitive(P_FORM_SET));
	_words.insert("$forward", new Primitive(P_FORWARD));
	_words.insert("$forward_and_back", new Primitive(P_FORWARD_AND_BACK));
	_words.insert("$forward_peel", new Primitive(P_FORWARD_PEEL));
	_words.insert("$forward_veer", new Primitive(P_FORWARD_VEER));
	_words.insert("$forward_veer_face", new Primitive(P_FORWARD_VEER_FACE));
	_words.insert("$fractionalize", new Primitive(P_FRACTIONALIZE));
	_words.insert("$has_lateral_flow", new Primitive(P_HAS_LATERAL_FLOW));
	_words.insert("$if", new Primitive(P_IF));
	_words.insert("$in", new Primitive(P_IN));
	_words.insert("$mirror", new Primitive(P_MIRROR));
	_words.insert("$move_in", new Primitive(P_MOVE_IN));
	_words.insert("$normalize", new Primitive(P_NORMALIZE));
	_words.insert("$nothing",  new Primitive(P_NOTHING));
	_words.insert("$pull_by", new Primitive(P_PULL_BY));
	_words.insert("$reduce", new Primitive(P_REDUCE));
	_words.insert("$roll", new Primitive(P_ROLL));
	_words.insert("$rotate", new Primitive(P_ROTATE));
	_words.insert("$run", new Primitive(P_RUN));
	_words.insert("$run_to", new Primitive(P_RUN_TO));
//	_words.insert("$start_together", new Primitive(P_START_TOGETHER));
	_words.insert("$stretch", new Primitive(P_STRETCH));
	_words.insert("$those_who_can", new Primitive(P_THOSE_WHO_CAN));
	_words.insert("$veer", new Primitive(P_VEER));
	_words.insert("$phantom", new Primitive(P_PHANTOM));

	Level adv1 = NO_LEVEL;
	for (int i = 0; i < levels.size(); i++)
		if (levels[i] == "Advanced-1") {
			adv1 = i;
			break;
		}
	anyone("$last_active", LAST_ACTIVE);
	anyone("boys", BOYS);
	anyone("girls", GIRLS);
	anyone("heads", HEADS);
	anyone("sides", SIDES);
	anyone("others", OTHERS);
	anyone("centers", CENTERS);
	anyone("ends", ENDS);
	anyone("leaders", LEADERS);
	anyone("trailers", TRAILERS);
	anyone("beaus", BEAUS, adv1);
	anyone("belles", BELLES, adv1);
	anyone("$very_centers", VERY_CENTERS);
	anyone("$very_ends", VERY_ENDS);
	anyone("$facing_across", FACING_ACROSS);
	anyone("$facing_along", FACING_ALONG);
	anyone("$in_facing", IN_FACING);
	anyone("$out_facing", OUT_FACING);

	_words.insert("$dancera", new DancerName(0));
	_words.insert("$dancerb", new DancerName(1));
	_words.insert("$dancerc", new DancerName(2));
	_words.insert("$dancerd", new DancerName(3));
	_words.insert("$dancere", new DancerName(4));
	_words.insert("$dancerf", new DancerName(5));
	_words.insert("$dancerg", new DancerName(6));
	_words.insert("$dancerh", new DancerName(7));
	_words.insert("$danceri", new DancerName(8));
	_words.insert("$dancerj", new DancerName(9));
	_words.insert("$dancerk", new DancerName(10));
	_words.insert("$dancerl", new DancerName(11));
}

Grammar::~Grammar() {
	delete _termStorage;
	_words.deleteAll();
	_synonyms.deleteAll();
	_definitions.deleteAll();
	_designators.deleteAll();
	_formations.deleteAll();
	if (_changeHandler)
		_backupGrammar->changed.removeHandler(_changeHandler);
	for (int i = 0; i < _leadersTrailers.size(); i++)
		delete _leadersTrailers[i].pattern;
	for (int i = 0; i < _centersEnds.size(); i++)
		delete _centersEnds[i];
	for (int i = 0; i < _partners.size(); i++)
		if (_partners[i].pattern != _couple)
			delete _partners[i].pattern;
	delete _couple;
}

void Grammar::anyone(const string& word, DancerSet value, Level level) {
	Anyone* a = new Anyone(value, 0, null, null, level);
	_words.insert(word, a);
}

bool Grammar::read(const string& filename) {
	FILE* fp = fileSystem::openTextFile(filename);
	if (fp == null) {
		_error = true;
		return false;
	}
	_filename = fileSystem::absolutePath(filename);
	DefinitionsContext ctx;

	bool result = fileSystem::readAll(fp, &ctx.text);
	_lastChanged = fileSystem::lastModified(fp);
	fclose(fp);
	if (result) {
		ctx.filename = &filename;
		processText(ctx);
		for (int i = 0; i < _definitions.size(); i++)
			_definitions[i]->verify(this);
	} else {
		_error = true;
		return false;
	}
	return true;
}

void Grammar::setFilename(const string& filename) {
	_filename = fileSystem::absolutePath(filename);
}

void Grammar::touch() {
	_lastChanged.touch();
	_parseStates.clear();
	delete _termStorage;
	_termStorage = new Stage(null, null);
	changed.fire();
}

void Grammar::compact() {
	for (int i = 0; i < _formations.size(); i++)
		_formations[i]->compact();
}

bool Grammar::write(const string& filename) const {
	FILE* fp = fileSystem::createTextFile(filename);

	if (fp == null)
//		printf("Could not create file %s\n", filename);
		return false;
	writeContents(fp);
	fclose(fp);
	return ferror(fp) == 0;
}

void Grammar::print() {
//	printf("Synonyms:\n");
//	for (int i = 0; i < _synFileOrder.size(); i++)
//		_synFileOrder[i]->print();
//	printf("Definitions:\n");
//	for (int i = 0; i < _definitions.size(); i++)
//		_definitions[i]->print();
//	printf("Diagrams:\n");
//	for (int i = 0; i < _formFileOrder.size(); i++)
//		_formFileOrder[i]->print();
//	printf("Definitions file round trips as:\n");
	writeContents(stdout);
}

void Grammar::printStateMachines() const {
	for (int i = 0; i < _parseStates.size(); i++)
		_parseStates[i].printed = false;
	if (_initialState.size() == 0)
		printf("No initial states\n");
	for (int i = 0; i < _initialState.size(); i++) {
		if (_initialState[i] == NULL_STATE)
			continue;
		printf("%s: %d\n", tokenNames[i], _initialState[i]);
		recurseState(_initialState[i], 0);
		if (_suffixes[i] != NULL_STATE) {
			printf("  suffixes:\n");
			recurseState(_suffixes[i], 4);
		}
	}
	for (int i = 0; i < _parseStates.size(); i++)
		if (!_parseStates[i].printed) {
			printf("Dis-connected state %d:\n", i);
			printState(i, 4);
		}
}

void Grammar::printStateMachineStats() const {
	printf("Total initial states: %5d\n", _initialState.size());
	printf("Total states:         %5d\n", _parseStates.size());
	printf("Total reductions:     %5d\n", _reductions.size());
}

void Grammar::recurseState(int i, int indent) const {
	if (i == NULL_STATE)
		return;
	printState(i, indent + 4);
	while (i != NULL_STATE) {
		if (_parseStates[i].term) {
			recurseState(_parseStates[i].matchState, indent + 4);
			i = _parseStates[i].missState;
		} else
			break;
	}
}

void Grammar::printState(int i, int indent) const {
	if (i == NULL_STATE) {
		printf("\n");
		return;
	}
	if (_parseStates[i].printed) {
		printf("State %d already printed\n", i);
		return;
	}
	_parseStates[i].printed = true;
	printf("%*.*c%4d: ", indent, indent, ' ', i);
	if (_parseStates[i].term) {
		printf("[match->%4d] ", _parseStates[i].matchState);
		_parseStates[i].term->print(0);
		printState(_parseStates[i].missState, indent);
	} else if (_parseStates[i].missState == REDUCE_TOS)
		printf("reduce TOS\n\n");
	else {
		printf("reduce(%d)[%d] -> ", _parseStates[i].matchState, _parseStates[i].missState);
		if (_parseStates[i].missState >= 0) {
			const Reduction& r = _reductions[_parseStates[i].missState];
			switch (r.type) {
			case	ANYTHING:
			case	ANYCALL:
				if (r.meaning)
					printf("%s", r.meaning->label().c_str());
				else
					printf("<null>");
				break;

			case	ANYONE:
				if (r.meaning)
					printf("Anyone{ %s }", r.meaning->label().c_str());
				else
					printf("<null>");
				break;

			default:
				printf("<error>");
			}
			if (r.production)
				printf(" = '%s'", r.production->c_str());
			if (r.definitionsOnly)
				printf(" (definitions only)");
		}
		printf("\n\n");
	}
}

void Grammar::addDefinition(Definition* definition) {
	_definitions.push_back(definition);
}

void Grammar::removeDefinition(Definition* definition) {
	for (int i = 0; i < _definitions.size(); i++) {
		if (_definitions[i] == definition) {
			_definitions.remove(i);
			return;
		}
	}
}

void Grammar::addFormation(Formation* formation) {
	_formations.push_back(formation);
	if (formation->name().size()) {
		Formation** f = _formationDictionary.get(formation->name());
		if (*f == null)
			_formationDictionary.put(formation->name(), formation);
	}
}

void Grammar::removeFormation(Formation* formation) {
	for (int i = 0; i < _formations.size(); i++) {
		if (_formations[i] == formation) {
			_formations.remove(i);
			changeFormationName(formation, "");
			return;
		}
	}
}

void Grammar::addDesignator(Designator* designator) {
	_designators.push_back(designator);
}

void Grammar::removeDesignator(Designator* designator) {
	for (int i = 0; i < _designators.size(); i++) {
		if (_designators[i] == designator) {
			_designators.remove(i);
			return;
		}
	}
}

void Grammar::changeFormationName(Formation* formation, const string& newName) {
	// If we have a name, and the dictionary points to this formation as
	// that name, we should forget that name.
	if (formation->name().size()) {
		Formation** f = _formationDictionary.get(formation->name());
		if (*f == formation) {
			_formationDictionary.put(formation->name(), null);
			// Check for any possible duplicate that we should map in as needed.
			for (int j = 0; j < _formations.size(); j++) {
				if (_formations[j]->name() == formation->name()) {
					_formationDictionary.put(formation->name(), _formations[j]);
					break;
				}
			}
		}
	}
	if (newName.size()) {
		Formation** f = _formationDictionary.get(newName);
		if (*f == null)
			_formationDictionary.put(newName, formation);
	}
}

void Grammar::compileStateMachines() const {
	_initialState.clear();
	_suffixes.clear();
	_reductions.clear();
	_parseStates.clear();

	static string anything("ANYTHING");
	static string primitive("$primitive");
	static string dancerName("$dancer_name");

	includeProduction(ANYCALL, anything, (Definition*)null);	// This is the default ANYCALL production.
	includeProduction(ANYTHING, primitive, (Definition*)null);	// This is the default $primitive production for primitives.
	includeProduction(ANYONE, dancerName, (Definition*)null);	// This is the default $dancer_mask production for primitives.
	includeBackDefinitions(this);
}

void Grammar::includeBackDefinitions(const Grammar* master) const {
	if (_backupGrammar)
		_backupGrammar->includeBackDefinitions(master);
	for (int i = 0; i < _definitions.size(); i++) {
		const Definition* def = _definitions[i];
		for (int j = 0; j < def->productions().size(); j++)
			master->includeProduction(ANYTHING, def->productions()[j], def);
	}
	for (int i = 0; i < _designators.size(); i++) {
		const Designator* des = _designators[i];
		for (int j = 0; j < des->phrases().size(); j++)
			master->includeProduction(des->phrases()[j], des);
	}
}

void Grammar::includeProduction(TokenType initialStateProduction, const string& production, const Definition* def) const {
	bool definitionsOnly;
	int matchState = buildProductionTables(&initialStateProduction, production, &definitionsOnly);
	if (matchState == -1)
		return;
	if (production[0] == '$')
		definitionsOnly = true;
	const ParseState& ps = _parseStates[matchState];
	if (ps.missState < _reductions.size()) {
		Reduction& r = _reductions[ps.missState];
		if (r.type != ANYTHING &&
			r.type != ANYCALL) {
			printf("Confusion of ANYONE and ANYTHING reductions (expecting ANYTHING): '%s'\n", production.c_str());
			return;
		}
		// There was a reduction already registered for this, it should have come
		// from another grammar (otherwise these are duplicated productions in a
		// single set and the 'last' one will win).
		if (r.meaning->grammar() == def->grammar())
			printf("Duplicate production '%s'\n", production.c_str());
		r.meaning = def;
	} else {
		Reduction r;
		r.type = initialStateProduction;
		r.meaning = def;
		r.production = &production;
		r.definitionsOnly = definitionsOnly;
		_reductions.push_back(r);
	}
}

void Grammar::includeProduction(const string& production, const Designator* des) const {
	bool definitionsOnly;
	TokenType initialStateProduction = ANYONE;
	int matchState = buildProductionTables(&initialStateProduction, production, &definitionsOnly);
	if (matchState == -1)
		return;
	const ParseState& ps = _parseStates[matchState];
	if (ps.missState < _reductions.size()) {
		Reduction& r = _reductions[ps.missState];
		if (r.type != ANYONE) {
			printf("Confusion of ANYONE and ANYTHING reductions (expecting ANYONE): '%s'\n", production.c_str());
			return;
		}
		// There was a reduction already registered for this, it should have come
		// from another grammar (otherwise these are duplicated productions in a
		// single set and the 'last' one will win).
		if (r.meaning->grammar() == des->grammar())
			printf("Duplicate production '%s'\n", production.c_str());
		r.meaning = des;
	} else {
		Reduction r;
		r.type = ANYONE;
		r.meaning = des;
		r.production = &production;
		r.definitionsOnly = definitionsOnly;
		_reductions.push_back(r);
	}
}

int Grammar::buildProductionTables(TokenType* initialStateProduction, const string& production, bool* definitionsOnly) const {
	Context context(null, this);
	context.startStage(_termStorage);
	vector<Token> tokens;
	if (!tokenize(null, production, true, null, &context, null, tokens, null)) {
		printf("Could not tokenize '%s'\n", production.c_str());
		return -1;
	}
	if (tokens.size() == 0)
		return -1;
	*definitionsOnly = false;
	int last = tokens.size() - 1;
	if (*initialStateProduction == ANYTHING &&
		tokens[last].type == WORD &&
		typeid(*tokens[last].term) == typeid(BuiltIn) &&
		((const BuiltIn*)tokens[last].term)->index() == ANYCALL)
		*initialStateProduction = ANYCALL;
	while (_initialState.size() <= *initialStateProduction) {
		_initialState.push_back(NULL_STATE);
		_suffixes.push_back(NULL_STATE);
	}
	int* previousLink = &_initialState[*initialStateProduction];
	int nonTerminalCount = 0;
	int i = 0;
	if (tokens[0].type == WORD &&
		typeid(*tokens[0].term) == typeid(BuiltIn) &&
		((const BuiltIn*)tokens[0].term)->index() == *initialStateProduction) {
		// ditch this: it is a trivial reduction:
		//		P ::= P
		if (tokens.size() == 1)
			return -1;
		// for now ditch these since they are simple suffixes of the form:
		//		P ::= P <tokens> ...
		previousLink = &_suffixes[*initialStateProduction];
		if (_suffixes[*initialStateProduction] == NULL_STATE) {
			ParseState ps;
			ps.matchState = 1;
			ps.missState = REDUCE_TOS;				// Indicates top non-terminal is production to reduce.
			ps.term = null;
			_parseStates.push_back(ps);
			_suffixes[*initialStateProduction] = _parseStates.size() - 1;
		}
		i = 1;
		nonTerminalCount = 1;
	}
	for (; i < tokens.size(); i++) {
		switch(tokens[i].type) {
		case	INTEGER:
			tokens[i].type = WORD;
			tokens[i].term = _termStorage->newInteger(tokens[i].value);
			break;

		case	FRACTION:
			break;

		case	WORD:
			if (typeid(*tokens[i].term) == typeid(BuiltIn))
				nonTerminalCount++;
			else {
				for (int j = 0; j < tokens[i].text.size(); j++)
					if (!validWordContent(tokens[i].text[j], false)) {
						*definitionsOnly = true;
						break;
					}
			}
			break;

		case	COMMA:
			continue;

		default:
			printf("Could not process '%s' because of token %d: ", production.c_str(), i);
			tokens[i].print();
			return - 1;
		}

		int newState = _parseStates.size();
		ParseState ps;
		ps.matchState = NULL_STATE;
		ps.term = tokens[i].term;
		int newSortIndex = ps.term->sortIndex();
		int j;
		for (;;) {
			j = *previousLink;
			if (j == NULL_STATE ||
				_parseStates[j].term == null ||
				_parseStates[j].term->sortIndex() > newSortIndex) {
				ps.missState = j;
				*previousLink = newState;
				_parseStates.push_back(ps);
				j = newState;
				break;
			}
			int result;
			if (_parseStates[j].term->compare(tokens[i].term, &result) && result == 0)
				break;
			previousLink = &_parseStates[j].missState;
		}
		previousLink = &_parseStates[j].matchState;
	}

	int newState = _parseStates.size();

	for (;;) {
		int j = *previousLink;
		if (j == NULL_STATE)
			break;
		// Already in use...
		if (_parseStates[j].term == null)
			return j;
		previousLink = &_parseStates[j].missState;
	}
	*previousLink = newState;

	ParseState ps;
	ps.matchState = nonTerminalCount;
	ps.missState = _reductions.size();
	ps.term = null;
	_parseStates.push_back(ps);
	return newState;
}

void Grammar::define(const string& key, const Term* term) {
	_words.insert(key, term);
}

bool Grammar::parsePartial(TokenType goalSymbol, const string& sentence, Level level, vector<string>* output) const {
	switch (goalSymbol) {
	case	R_L:
		output->push_back("left");
		output->push_back("right");
		break;

	case	P_C:
		output->push_back("partner");
		output->push_back("corner");
		break;

	case	FRACTION:
		output->push_back("INTEGER");
		output->push_back("INTEGER/INTEGER");
		output->push_back("INTEGER and INTEGER/INTEGER");
		break;

	case	INTEGER:
		output->push_back("1");
		output->push_back("2");
		output->push_back("3");
		output->push_back("4");
		output->push_back("5");
		output->push_back("6");
		output->push_back("7");
		output->push_back("8");
		output->push_back("9");
		output->push_back("10");
		break;

	case	COUPLE_NUMBER:
		output->push_back("#1");
		output->push_back("#2");
		output->push_back("#3");
		output->push_back("#4");
		break;

	case	ORDINAL:
		output->push_back("1st");
		output->push_back("2nd");
		output->push_back("3rd");
		output->push_back("4th");
		output->push_back("5th");
		output->push_back("6th");
		output->push_back("7th");
		output->push_back("8th");
		output->push_back("9th");
		output->push_back("10th");
		break;

	case	ANYDIRECTION:
		output->push_back("left");
		output->push_back("right");
		output->push_back("in");
		output->push_back("out");
		output->push_back("as you are");
		break;

	default: {
		Sequence seq(null);
		Context context(&seq, this);
		seq.setLevel(level);
		Stage tempStorage(&seq, null);
		context.startStage(&tempStorage);

		vector<Token> tokens;
		Token finalPartial;
		if (tokenize(null, sentence, false, null, &context, null, tokens, &finalPartial)) {
			int matched;
			vector<int> partialStates;
			const Anything* a = matchAnycall(false, tokens, 0, true, &matched, &partialStates, &context);
			for (int i = 0; i < partialStates.size(); i++) {
				if (verboseParsing)
					printf("partialStates[%d] = %d\n", i, partialStates[i]);
				collectReductions(partialStates[i], finalPartial, &context, output);
			}
		} else
			collectReductionsAnon(_initialState[ANYCALL], output, &context);
		for (int i = 0; i < output->size(); i++) {
			if ((*output)[i].tolower() == "anything") {
				output->remove(i);
				collectReductionsAnon(_initialState[ANYTHING], output, &context);
				break;
			}
		}
	}
	}
	return true;
}

const Anything* Grammar::parse(const Group* dancers, const string& text, bool inDefinition, const Anything* call, Context* context, const Plan* variantPlan) const {
	timing::Timer t("Grammar::parse");
	vector<Token> tokens;
	if (!tokenize(dancers, text, inDefinition, &call->variables(), context, variantPlan, tokens, null))
		return null;
	int matched;
	if (verboseParsing) {
		printf("Parse: inDefinition=%s\n", inDefinition ? "true" : " false");
		for (int i = 0; i < tokens.size(); i++) {
			printf("    [%2d] ", i);
			tokens[i].print();
		}
	}
	const Anything* result = matchAnycall(inDefinition, tokens, 0, true, &matched, null, context);
	if (tokens.size() == matched)
		return result;
	else
		return null;
}

const Anyone* Grammar::parseAnyone(const Group* dancers, const string& text, const Anything* call, Context* context, const Plan* variantPlan, const Term** local) const {
	timing::Timer t("Grammar::parseAnyone");
	vector<Token> tokens;
	if (!tokenize(dancers, text, true, &call->variables(), context, variantPlan, tokens, null))
		return null;
	int matched;
	if (verboseParsing) {
		printf("ParseAnyone:\n");
		for (int i = 0; i < tokens.size(); i++) {
			printf("    [%2d] ", i);
			tokens[i].print();
		}
	}
	int tIndex = 0;
	if (tokens.size() > 2 &&
		tokens[1].type == EQ &&
		tokens[0].type == WORD) {
		*local = tokens[0].term;
		tIndex += 2;
	}
	const Term* result = stateMachine(ANYONE, true, tokens, tIndex, true, &matched, null, context);
	if (verboseParsing) {
		printf("matched=%d\n", matched);
		if (result)
			result->print(4);
		else
			printf("    <null>\n");
	}
	if (tokens.size() == matched + tIndex)
		return (const Anyone*)result;
	else
		return null;
}

const Anyone* Grammar::parseDesignatingPattern(const Group* dancers, const string& text, bool inDefinition, const Anything* call, Context* context) const {
	vector<Token> tokens;
	if (!tokenize(dancers, text, inDefinition, &call->variables(), context, null, tokens, null))
		return null;
	Anything* instance = context->stage()->newAnything(inDefinition, null);
	int matched = matchPrimitiveParameters(instance, tokens, 0, context);
	if (tokens.size() == matched) {
		if (instance->variables().size() != 1)
			return null;
		const Term* t = instance->variables()[0];
		if (typeid(*t) == typeid(Anyone))
			return (const Anyone*)t;
		else
			return null;
	} else
		return null;
}

const Anyone* Grammar::parseDesignatorExpression(const string& expression, const vector<const Term*>* variables, Context* context) const {
	vector<Token> tokens;
	if (!tokenize(null, expression, true, variables, context, null, tokens, null))
		return null;
	CallParser parser(tokens, 0);
	const Term* t = parser.parseExpression(null, context);
	if (t == null)
		return null;
	if (typeid(*t) != typeid(Anyone))
		return null;
	return (const Anyone*)t;
}

bool Grammar::tokenize(const Group* dancers, const string& text, bool inDefinition, const vector<const Term*>* variables, Context* context, const Plan* variantPlan, vector<Token>& tokens, Token* finalPartial) const {
	timing::Timer t("Grammar::tokenize");
		string lower = text.tolower();
	CallParser parser(lower, this, variantPlan, inDefinition, finalPartial);
	for (parser.scan();;) {
		TokenType t = parser.token().type;
		Token tok;

		switch (t) {
		case	END_OF_STRING:
			return tokens.size() > 0;

		case	FINAL_PARTIAL:
			*finalPartial = parser.token();
			finalPartial->type = WORD;
			return true;

		case	UNKNOWN_WORD:
			if (inDefinition) {
				tok.type = WORD;
				tok.text = parser.token().text;
				tok.term = new Word(tok.text);
				_words.insert(tok.text, tok.term);
				tokens.push_back(tok);
				break;
			} else
				return false;

		case	DANCER_NAME:
			if (dancers == null)
				return false;
			if (parser.token().value <= dancers->dancerCount()) {
				const Dancer* d = dancers->dancer(parser.token().value);

				tok.type = WORD;
				tok.term = context->stage()->newAnyone(DANCER_MASK, d->dancerMask(), null, null, NO_LEVEL);
				tokens.push_back(tok);
			}
			break;

		case	VARIABLE:
			if (variables && parser.token().value <= variables->size()) {
				if (parser.token().value)
					(*variables)[parser.token().value - 1]->token(&tok);
				else {
					tok.type = WORD;
					tok.term = dancers;
				}
				tokens.push_back(tok);
				break;
			} else
				return false;

		case	INTEGER: {
			int whole = parser.token().value;
			parser.scan();
			t = parser.token().type;
			if (t == SLASH) {
				parser.scan();
				if (parser.token().type != INTEGER) {
					if (finalPartial && parser.token().type == END_OF_STRING) {
						finalPartial->type = WORD;
						finalPartial->text = string(whole) + "/";
						return true;
					}
					return false;
				}
				if (parser.token().value == 0)
					return false;
				tok.type = FRACTION;
				tok.term = context->stage()->newFraction(0, whole, parser.token().value);
			} else if (t == WORD &&
					   parser.token().term == _and) {
				parser.scan();
				if (parser.token().type != INTEGER) {
					tok.type = INTEGER;
					tok.value = whole;
					tokens.push_back(tok);
					tok.type = WORD;
					tok.term = _and;
					tokens.push_back(tok);
					continue;
				}
				int num = parser.token().value;
				parser.scan();
				if (parser.token().type != SLASH) {
					if (finalPartial && parser.token().type == END_OF_STRING) {
						tok.type = INTEGER;
						tok.value = whole;
						tokens.push_back(tok);
						tok.type = WORD;
						tok.term = _and;
						tokens.push_back(tok);
						tok.type = INTEGER;
						tok.value = num;
						tokens.push_back(tok);
						return true;
					}
					return false;
				}
				parser.scan();
				if (parser.token().type != INTEGER) {
					if (finalPartial && parser.token().type == END_OF_STRING) {
						finalPartial->type = WORD;
						finalPartial->text = string(whole) + " and " + num + "/";
						return true;
					}
					return false;
				}
				if (parser.token().value == 0)
					return false;
				tok.type = FRACTION;
				tok.term = context->stage()->newFraction(whole, num, parser.token().value);
			} else {
				tok.type = INTEGER;
				tok.value = whole;
				tokens.push_back(tok);
				continue;
			}
			tokens.push_back(tok);
			break;
		}

		case	ERROR_TOKEN:
			return false;

		default:
			tokens.push_back(parser.token());
		}
		parser.scan();
	}
}

int Grammar::matchR_L(const vector<Token>& tokens, int tIndex) const {
	if (tokens[tIndex].type != WORD)
		return -1;
	const Term* t = tokens[tIndex].term;
	if (typeid(*t) != typeid(Anydirection))
		return -1;
	const Anydirection* dir = (const Anydirection*)t;
	if (dir->direction() == D_LEFT)
		return 1;
	else if (dir->direction() == D_RIGHT)
		return 1;
	else
		return -1;
}

int Grammar::matchP_C(const vector<Token>& tokens, int tIndex) const {
	if (tokens[tIndex].type != WORD)
		return -1;
	const Term* t = tokens[tIndex].term;
	if (typeid(*t) == typeid(Anydirection)) {
		const Anydirection* dir = (const Anydirection*)t;

		if (dir->direction() == D_PARTNER)
			return 1;
		else if (dir->direction() == D_CORNER)
			return 1;
	}
	return -1;
}

const Anything* Grammar::matchPrimitive(const vector<Token>& tokens, int tIndex, int* matched, Context* context) const {
	*matched = -1;
	if (tokens[tIndex].type != WORD)
		return null;
	if (verboseParsing) {
		printf("matchPrimitive:\n");
		tokens[tIndex].term->print(4);
	}
	if (typeid(*tokens[tIndex].term) == typeid(Anything)) {
		*matched = 1;
		return (const Anything*)tokens[tIndex].term;
	}
	if (verboseParsing)
		printf("Not an ANYTHING object\n");
	if (typeid(*tokens[tIndex].term) != typeid(Primitive))
		return null;
	int i = tIndex + 1;
	if (i < tokens.size() && tokens[i].type != LPAREN)
		return null;
	i++;
	if (i >= tokens.size())
		return null;
	Anything* instance = context->stage()->newAnything((Primitive*)tokens[tIndex].term);
	int m = matchPrimitiveParameters(instance, tokens, i, context);
	if (m < 0)
		return null;
	*matched = 2 + m;
	return instance;
}

const Anything* Grammar::matchAnycall(bool inDefinition, const vector<Token>& tokens, int tIndex, bool fullMatch, int* matched, vector<int>* partialStates, Context* context) const {
	return (const Anything*)stateMachine(ANYCALL, inDefinition, tokens, tIndex, fullMatch, matched, partialStates, context);
}

const Term* Grammar::stateMachine(int initialState, bool inDefinition, const vector<Token>& tokens, int tIndex, bool fullMatch, int* matched, vector<int>* partialStates, Context* context) const {
	timing::Timer t("Grammar::stateMachine");
	if (_parseStates.size() == 0)
		compileStateMachines();
	int state = _initialState[initialState];
	vector<const Term*> nonTerminals;
	// These vectors grow/shrink together as stacks
	vector<int> alternatives;
	vector<int> alternativeTIndex;
	vector<int> alternativeNTDepth;
	vector<int> alternativeReduceState;
	vector<int> alternativeReduceStateDepth;
	// These vectors grow/shrink together as stacks 
	vector<int> reduceState;

	int startingTIndex = tIndex;
	for (;;) {
		if (fullMatch) {
			while (tIndex < tokens.size()) {
				if (tokens[tIndex].type == COMMA)
					tIndex++;
				else
					break;
			}
		}
		while (state == NULL_STATE) {
			if (alternatives.size()) {
				int altNTDepth = alternativeNTDepth.pop_back();
				if (verboseParsing) {
					printf("Trying alternatives\n");
					printf("nt depth=%d alternative depth=%d reduce state depth=%d\n", nonTerminals.size(), alternatives.size(), reduceState.size());
					printf("alt nt depth=%d\n", altNTDepth);
				}
				state = alternatives.pop_back();
				tIndex = alternativeTIndex.pop_back();
				int rsSize = alternativeReduceStateDepth.pop_back();
				int rsState = alternativeReduceState.pop_back();
				if (rsSize > reduceState.size())
					reduceState.push_back(rsState);
				else
					reduceState.resize(rsSize);
				// If this is a backtrack through a reduction, the nonTerminal TOS is
				// the Anything object created by the reduction.  We need to unwind the
				// reduction and restore the nt stack.
				if (tIndex == string::npos && rsSize) {
					if (nonTerminals.size() == 0)
						return null;
					const Term* t = nonTerminals.pop_back();
					if (typeid(*t) == typeid(Anything)) {
						const Anything* call = (const Anything*)t;
						if (verboseParsing) {
							printf("nt depth=%d\n", nonTerminals.size());
							printf("Backtrack restoring:\n");
							call->print(4);
						}
						for (int i = 0; i < call->variables().size(); i++)
							nonTerminals.push_back(call->variables()[i]);
					} else if (typeid(*t) == typeid(Anyone)) {
						const Anyone* anyone = (const Anyone*)t;
						if (verboseParsing) {
							printf("nt depth=%d\n", nonTerminals.size());
							printf("Backtrack restoring:\n");
							anyone->print(4);
						}
						if (anyone->left()) {
							nonTerminals.push_back(anyone->left());
							if (anyone->right())
								nonTerminals.push_back(anyone->right());
						}
					} else
						return null;
				} else
					nonTerminals.resize(altNTDepth);
			} else
				return null;
			if (verboseParsing) {
				printf("backtrack to state %d\n", state);
				printf("  tIndex = %d nt depth = %d rs depth = %d\n", tIndex, nonTerminals.size(), reduceState.size());
			}
		}
		const ParseState& ps = _parseStates[state];
		if (ps.term) {
			if (tIndex < tokens.size()) {
				state = ps.missState;
				int ntDepth = nonTerminals.size();
				// First, special case any non-terminals that use the state machine
				if (typeid(*ps.term) == typeid(BuiltIn)) {
					int index = ((const BuiltIn*)ps.term)->index();
					if (index == ANYTHING || index == ANYCALL || index == ANYONE) {
/*
						if (index == ANYONE && 
							tokens[tIndex].type == WORD && 
							typeid(*tokens[tIndex].term) == typeid(Anyone)) {
							if (verboseParsing) {
								printf("Reducing ANYONE immediate term:\n");
								tokens[tIndex].term->print(4);
							}
							nonTerminals.push_back(tokens[tIndex].term);
							alternatives.push_back(ps.missState);
							alternativeTIndex.push_back(tIndex);
							alternativeNTDepth.push_back(ntDepth);
							alternativeReduceStateDepth.push_back(reduceState.size());
							alternativeReduceState.push_back(NULL_STATE);
							tIndex++;
							if (_suffixes[ANYONE] != NULL_STATE) {
								if (verboseParsing)
									printf("Diverting to ANYONE suffix state: %d\n", _suffixes[ANYONE]);
								reduceState.push_back(ps.matchState);
								state = _suffixes[ANYONE];
							} else
								state = ps.matchState;
							continue;
						}
 */
						if (verboseParsing) {
							printf("Trying to match %s\n", tokenNames[index]);
							printf("  tIndex = %d nt depth = %d rs depth = %d\n", tIndex, nonTerminals.size(), reduceState.size());
							tokens[tIndex].print();
						}
						// The grammar has some production that loops indefinitely, kill the parse
						if (reduceState.size() > 50)
							return null;
						alternatives.push_back(ps.missState);
						alternativeTIndex.push_back(tIndex);
						alternativeNTDepth.push_back(ntDepth);
						alternativeReduceStateDepth.push_back(reduceState.size());
						alternativeReduceState.push_back(NULL_STATE);
						state = _initialState[index];
						reduceState.push_back(ps.matchState);
						continue;
					}
				}
				// Otherwise, use the hard-coded matching code.
				int result = ps.term->match(tokens, tIndex, &nonTerminals, context);
				if (result >= 0) {
					alternatives.push_back(ps.missState);
					alternativeTIndex.push_back(tIndex);
					alternativeNTDepth.push_back(ntDepth);
					alternativeReduceStateDepth.push_back(reduceState.size());
					alternativeReduceState.push_back(NULL_STATE);
					tIndex += result;
					state = ps.matchState;
				}
			} else if (partialStates) {
				// We have reached a partial match, the available full tokens match to this state,
				// so any reductions that follow from this state are potential completions.
				partialStates->push_back(state);
				// we should walk up the reduction tree to discover something about the other
				// open non-terminals, just knowing the state gives us no picture of the larger
				// parse tree.

				// now continue to explore more of the expansion tree
				state = ps.missState;
			} else
				state = ps.missState;
		} else {
			// ps.matchState = non-terminal count on variable stack
			// ps.missState = reduction number, or -1 for reducing TOS
			if (ps.missState != REDUCE_TOS) {
				const Reduction& r = _reductions[ps.missState];
				if (verboseParsing)
					printf("tIndex = %d nt depth = %d rs depth = %d reducing[%s] %s\n", tIndex, nonTerminals.size(), reduceState.size(), tokenNames[r.type], r.meaning ? r.meaning->label().c_str() : "<null>");
				switch (r.type) {
				case	ANYCALL:
				case	ANYTHING: {
					if (r.meaning) {
						Anything* call = context->stage()->newAnything(inDefinition, (const Definition*)r.meaning);
						if (call == null)
							return null;
						int variableBase = nonTerminals.size() - ps.matchState;
						for (int i = variableBase; i < nonTerminals.size(); i++)
							call->variable(nonTerminals[i]);
						nonTerminals.resize(variableBase);
						nonTerminals.push_back(call);
						if (verboseParsing)
							call->print(4);
					}
					break;
				}

				case	ANYONE: {
					if (r.meaning) {
						const Designator* des = (const Designator*)r.meaning;
						vector<const Term*> terms;
						int variableBase = nonTerminals.size() - ps.matchState;
						for (int i = variableBase; i < nonTerminals.size(); i++)
							terms.push_back(nonTerminals[i]);
						const Anyone* anyone = des->compile(terms, context);
						if (anyone == null)
							return null;
						nonTerminals.resize(variableBase);
						nonTerminals.push_back(anyone);
						if (verboseParsing)
							anyone->print(4);
					}
					break;
				}
				default:
					return null;
				}
				if (_suffixes[r.type] != NULL_STATE) {
					state = _suffixes[r.type];
					continue;
				}
			}
			int reduceStateSz = reduceState.size();
			if (reduceStateSz == 0) {
				if (!fullMatch ||
					tIndex >= tokens.size()) {
					*matched = tIndex - startingTIndex;
					return (const Anything*)nonTerminals.pop_back();
				}
				reduceState.push_back(NULL_STATE);
			}
			alternatives.push_back(NULL_STATE);
			alternativeTIndex.push_back(string::npos);
			alternativeNTDepth.push_back(nonTerminals.size());
			alternativeReduceStateDepth.push_back(reduceStateSz);
			state = reduceState.pop_back();
			if (verboseParsing)
				printf("Reducing to state %d\n", state);
			alternativeReduceState.push_back(state);
		}
	}
}

bool Grammar::collectReductions(int state, const Token& finalPartial, Context* context, vector<string>* output) const {
	if (state >= _parseStates.size())
		return false;
	const ParseState& ps = _parseStates[state];
	if (ps.term) {
		if (verboseParsing) {
			printf("collectReductions state %d:\n", state);
			ps.term->print(4);
		}
		if (finalPartial.type != END_OF_STRING &&
			!ps.term->partialMatch(context, finalPartial.text, output))
			return false;
		if (verboseParsing)
			printf("    Partially matched!\n");
		collectReductionsAnon(ps.matchState, output, context);
		return true;
	} else {
		const Reduction& r = _reductions[ps.missState];

		// Off-level means no production match

		if (context->sequence()->level() != NO_LEVEL &&
			r.meaning != null &&
			r.meaning->level() > context->sequence()->level())
			return false;
		if (r.production != null && !r.definitionsOnly)
			output->push_back(*r.production);
		return false;
	}
}

void Grammar::collectReductionsAnon(int state, vector<string>* output, Context* context) const {
	for (; state != NULL_STATE; state = _parseStates[state].missState)
		if (!collectReductions(state, Token::endOfString, context, output))
			break;
}

void Grammar::partialMatch(TokenType type, const string& text, Context* context, vector<string>* output) const {
	Token finalPartial;
	finalPartial.text = text;
	finalPartial.type = WORD;
	for (int state = _initialState[type]; state != NULL_STATE; state = _parseStates[state].missState)
		collectReductions(state, finalPartial, context, output);
}

void Grammar::setBackupGrammar(Grammar* g) {
	if (_backupGrammar) {
		_backupGrammar->changed.removeHandler(_changeHandler);
		_changeHandler = null;
	}
	_backupGrammar = g; 
	if (g)
		_changeHandler = g->changed.addHandler(this, &Grammar::touch);
}

const Synonym* Grammar::synonym(const string& key) const { 
	const Synonym* syn = *_synonymDictionary.get(key); 
	if (syn)
		return syn;
	if (_backupGrammar)
		return _backupGrammar->synonym(key);
	else
		return null;
}

const Formation* Grammar::formation(const string& key) const { 
	const Formation* f = *_formationDictionary.get(key);
	if (f)
		return f;
	if (_backupGrammar)
		return _backupGrammar->formation(key);
	else
		return null;
}

const vector<VariantTile>& Grammar::leadersTrailers() const {
	if (_leadersTrailers.size() == 0) {
		_leadersTrailers.push_back(VariantTile(null, new Pattern(formation("box"), null)));
		_leadersTrailers.push_back(VariantTile(null, new Pattern(formation("twosome"), null)));
	}
	return _leadersTrailers; 
}

const vector<const Pattern*>& Grammar::centersEnds() const { 
	if (_centersEnds.size() == 0) {
		dictionary<const Formation*> forms;
		for (const Grammar* g = this; g != null; g = g->_backupGrammar)
			for (int i = 0; i < g->_formations.size(); i++)
				forms.insert(g->_formations[i]->name(), g->_formations[i]);
		for (dictionary<const Formation*>::iterator i = forms.begin(); i.valid(); i.next()) {
			const Formation* f = *i;
			if (f->hasCentersOrEnds())
				_centersEnds.push_back(new Pattern(f, null));
		}
	}
	return _centersEnds; 
}

const vector<VariantTile>& Grammar::partners() const { 
	if (_partners.size() == 0) {
		if (_couple == null)
			_couple = new Pattern(formation("couple"), null);

		_partners.push_back(VariantTile(null, new Pattern(formation("box"), null)));
		_partners.push_back(VariantTile(null, _couple));
		_partners.push_back(VariantTile(null, new Pattern(formation("rh_mini_wave"), null)));
		_partners.push_back(VariantTile(null, new Pattern(formation("lh_mini_wave"), null)));
	}
	return _partners;
}

const vector<VariantTile>& Grammar::couples() const { 
	if (_couples.size() == 0) {
		if (_couple == null)
			_couple = new Pattern(formation("couple"), null);

		_couples.push_back(VariantTile(null, _couple));
	}
	return _couples; 
}

fileSystem::TimeStamp Grammar::lastChanged() const {
	if (_backupGrammar) {
		fileSystem::TimeStamp t = _backupGrammar->lastChanged();
		if (t > _lastChanged)
			return t;
	}
	return _lastChanged; 
}


void Grammar::processText(DefinitionsContext& ctx) {
	ctx.line = 1;
	int startLine = 0;
	for (int i = 0; i < ctx.text.size(); i++) {
		if (ctx.text[i] == '\n') {
			if (i == startLine + 2 &&
				ctx.text[startLine] == '@' &&
				ctx.text[startLine + 1] == '@') {
				ctx.line++;
				processDesignators(ctx, i + 1);
				return;
			}
			if (i == startLine + 2 &&
				ctx.text[startLine] == '%' &&
				ctx.text[startLine + 1] == '%') {
				ctx.line++;
				processDiagrams(ctx, i + 1);
				return;
			}
			if (i != startLine)
				processLine(ctx, startLine, i);
			startLine = i + 1;
			ctx.line++;
		}
	}
	processLine(ctx, startLine, ctx.text.size());
}

void Grammar::processLine(DefinitionsContext& ctx, int startLine, int endLine) {
	while (startLine < endLine && isspace(ctx.text[startLine]))
		startLine++;
	// Skip blank lines
	if (startLine >= endLine)
		return;
	// Skip comment lines
	if (ctx.text[startLine] == '/')
		return;
	if (ctx.text[startLine] == ':') {
		int eq = ctx.text.find('=', startLine + 1);
		if (eq == string::npos || eq > endLine) {
			printf("%s line %d: Synonym missing =\n", ctx.filename->c_str(), ctx.line);
			_error = true;
			return;
		}
		string synonym, value;

		synonym = ctx.text.substr(startLine + 1, eq - startLine - 1);
		value = ctx.text.substr(eq + 1, endLine - eq - 1);
		Synonym** syn = _synonymDictionary.get(synonym);
		if (*syn) {
			printf("%s line %d: Duplicate synonym\n", ctx.filename->c_str(), ctx.line);
			_error = true;
			return;
		}
		Synonym* s = new Synonym(synonym, value);
		_synonyms.push_back(s);
		_synonymDictionary.insert(synonym, s);
		return;
	}
	if (startLine + 2 <= endLine && ctx.text[startLine] == '-' && ctx.text[startLine + 1] == '-') {
		ctx.definition = new Definition(this);
		addDefinition(ctx.definition);
		if (startLine + 2 < endLine) {
			string levelName = ctx.text.substr(startLine + 2, endLine - startLine - 2).trim();
			ctx.definition->setLevel(levelName);
		}
		return;
	}
	if (ctx.definition == null) {
		printf("%s line %d: Unexpected call text\n", ctx.filename->c_str(), ctx.line);
		_error = true;
		return;
	}
	if (ctx.text[startLine] == '.') {
		ctx.definition->setName(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}
	if (ctx.text[startLine] == 'C') {
		ctx.definition->setCreated(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}
	if (ctx.text[startLine] == 'M') {
		ctx.definition->setModified(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}

	if (ctx.text[startLine] == '+') {
		ctx.definition->nextPart(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}
	if (ctx.text[startLine] == '|') {
		ctx.definition->nextVariant();
		return;
	}
	if (ctx.text[startLine] == '!') {
		ctx.definition->setVariantLevel(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}
	if (ctx.text[startLine] == '^') {
		ctx.definition->setVariantPrecedence(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}
	if (ctx.text[startLine] == '>') {
		ctx.definition->action(ctx.text.substr(startLine + 1, endLine - startLine - 1));
		return;
	}
	if (ctx.text[startLine] == '<') {
		ctx.definition->addCompound();
		return;
	}
	if (ctx.text[startLine] == '@') {
		int len = endLine - startLine - 1;
		if (len < 2) {
			printf("%s line %d: Compound who line too short\n", ctx.filename->c_str(), ctx.line);
			_error = true;
			return;
		}
		bool finishTogether;
		if (ctx.text[startLine + 1] == 'T')
			finishTogether = true;
		else if (ctx.text[startLine + 1] == 'F')
			finishTogether = false;
		else {
			printf("%s line %d: Compound who line needs T or F\n", ctx.filename->c_str(), ctx.line);
			_error = true;
			return;
		}
		ctx.definition->compoundWho(finishTogether, ctx.text.substr(startLine + 3, len - 2));
		return;
	}
	if (ctx.text[startLine] == '#') {
		int len = endLine - startLine - 1;
		if (len < 2) {
			printf("%s line %d: Compound what line too short\n", ctx.filename->c_str(), ctx.line);
			_error = true;
			return;
		}
		bool anyWhoCan;
		if (ctx.text[startLine + 1] == 'T')
			anyWhoCan = true;
		else if (ctx.text[startLine + 1] == 'F')
			anyWhoCan = false;
		else {
			printf("%s line %d: Compound what line needs T or F\n", ctx.filename->c_str(), ctx.line);
			_error = true;
			return;
		}
		ctx.definition->compoundWhat(anyWhoCan, ctx.text.substr(startLine + 3, len - 2));
		return;
	}
	if (ctx.text[startLine] == '*') {
		string s = ctx.text.substr(startLine + 1, endLine - startLine - 1).trim();
		if (s.size())
			ctx.definition->addPattern(s);
		return;
	}
	int prod = ctx.definition->addProduction();
	if (!ctx.definition->setProduction(prod, ctx.text.substr(startLine, endLine - startLine))) {
		printf("%s line %d: Unexpected text in production\n", ctx.filename->c_str(), ctx.line);
		_error = true;
	}
}

void Grammar::processDesignators(DefinitionsContext& ctx, int start) {
	while (start < ctx.text.size()) {
		if (start < ctx.text.size() - 2 &&
			ctx.text[start] == '%' &&
			ctx.text[start + 1] == '%' &&
			ctx.text[start + 2] == '\n') {
			ctx.line++;
			processDiagrams(ctx, start + 3);
			return;
		}
		if (start >= ctx.text.size() - 1 ||
			ctx.text[start] != '+' ||
			ctx.text[start + 1] != '+') {
			printf("%s line %d: Expecting ++ at beginning of designator\n", ctx.filename->c_str(), ctx.line);
			return;
		}
		start += 2;
		// skip any white space
		while (start < ctx.text.size() && (ctx.text[start] == ' ' || ctx.text[start] == '\t'))
			start++;
		int level = start;
		int i = ctx.text.find('\n', level);
		if (i == string::npos) {
			printf("%s line %d: Last designator is empty\n", ctx.filename->c_str(), ctx.line);
			return;
		}
		start = i + 1;
		Designator* des = new Designator(this);
		_designators.push_back(des);
		des->setLevel(ctx.text.substr(level, i - level));
		if (start >= ctx.text.size()) {
			printf("%s line %d: Last designator is empty\n", ctx.filename->c_str(), ctx.line);
			return;
		}
		while (des->processLine(ctx, &start))
			;
	}
}

void Grammar::processDiagrams(DefinitionsContext& ctx, int start) {
	while (start < ctx.text.size()) {
		if (ctx.text[start] != '=') {
			printf("%s line %d: Expecting = at beginning of diagram name\n", ctx.filename->c_str(), ctx.line);
			return;
		}
		start++;
		// skip any white space
		while (start < ctx.text.size() && (ctx.text[start] == ' ' || ctx.text[start] == '\t'))
			start++;
		int name = start;
		int i = ctx.text.find('\n', name);
		if (i == string::npos) {
			printf("%s line %d: Last diagram is empty\n", ctx.filename->c_str(), ctx.line);
			return;
		}
		start = i + 1;
		Geometry geometry = UNSPECIFIED_GEOMETRY;
		int j = ctx.text.find(' ', name);
		if (j != string::npos && j < i) {
			int eol = i;
			i = j;
			j = ctx.text.find('@', j);
			if (j == string::npos) {
				printf("%s line %d: Unexpected text after diagram name\n", ctx.filename->c_str(), ctx.line);
				return;
			}
			string geo = ctx.text.substr(j + 1, eol - j - 1);
			if (geo == "ring")
				geometry = RING;
			else if (geo == "grid")
				geometry = GRID;
			else if (geo == "hexagonal")
				geometry = HEXAGONAL;
			else
				printf("%s line %d: Unexpected geometry '%s'\n", ctx.filename->c_str(), ctx.line, geo);
		}
		Formation* f = new Formation(this, ctx.text.substr(name, i - name), geometry);
		_formations.push_back(f);
		Formation** ff = _formationDictionary.get(f->name());
		if (*ff)
			printf("%s line %d: Duplicate formation name '%s'\n", ctx.filename->c_str(), ctx.line, f->name().c_str());
		else
			_formationDictionary.insert(f->name(), f);
		ctx.line++;
		while (start < ctx.text.size()) {
			i = ctx.text.find('\n', start);
			if (i == string::npos)
				i = ctx.text.size();
			if (ctx.text[start] == '=')
				break;
			if (!f->row(ctx.text, start, i))
				printf("%s line %d: Diagram syntax error\n", ctx.filename->c_str(), ctx.line);
			start = i + 1;
			ctx.line++;
		}
	}
}

void Grammar::writeContents(FILE *fp) const {
	for (int i = 0; i < _synonyms.size(); i++)
		_synonyms[i]->write(fp);
	for (int i = 0; i < _definitions.size(); i++)
		_definitions[i]->write(fp);
	if (_designators.size() > 0) {
		fprintf(fp, "@@\n");
		for (int i = 0; i < _designators.size(); i++)
			_designators[i]->write(fp);
	}
	if (_formations.size() > 0) {
		fprintf(fp, "%%%%\n");
		for (int i = 0; i < _formations.size(); i++)
			_formations[i]->write(fp);
	}
}

const Term* Grammar::keyword(const string& word) {
	const Word* w = new Word(word);
	_words.insert(word, w);
	return w;
}

int Grammar::matchPrimitiveParameters(Anything* instance, const vector<Token>& tokens, int i, Context* context) const {
	if (tokens[i].type == RPAREN)
		return 1;
	int start = i - 1;
	int whole;
	const Fraction* f;

	while (i < tokens.size()) {
		CallParser parser(tokens, i);

		const Term* t = parser.parseExpressionPrefix(null, context);
		if (t && typeid(*t) != typeid(Word)) {
			i = parser.at() - 1;
			instance->variable(t);
		} else {
			int m = 0;
			const Anything* parameter = matchAnycall(true, tokens, i, false, &m, null, context);
			if (m > 0) {
				i += m;
				instance->variable(parameter);
			} else {
				switch (tokens[i].type) {
				case	WORD:
					instance->variable(tokens[i].term);
					break;

				case	UNKNOWN_WORD:
					instance->variable(new Word(tokens[i].text));
					break;

				default:
					return -1;
				}
				i++;
			}
		}
		if (i >= tokens.size())
			return -1;
		if (tokens[i].type == RPAREN)
			return i - start;
		else if (tokens[i].type == COMMA)
			i++;
		else
			break;
	}
	return -1;
}

void Token::print() const {
	printf("  Token %s", tokenNames[type]);
	switch (type) {
	case	INTEGER:
		printf(" value %d", value);
		break;

	case	UNKNOWN_WORD:
	case	WORD:
	case	ERROR_TOKEN:
		printf(" text %s", text.c_str());
	}
	printf("\n");
}

Variant::~Variant() {
	_parts.deleteAll();
	_recognizers.deleteAll();
}

void Variant::setPattern(int index, const string& text) {
	if (text.size() == 0 && index == _patterns.size() - 1) {
		_patterns.resize(index);
		_recognizers.resize(index);
	} else {
		if (_patterns.size() == index) {
			_patterns.push_back(text);
			_recognizers.push_back(null);
		}
		_patterns[index] = text;
		_recognizers[index] = Pattern::parse(_definition->grammar(), text);
	}
	_definition->clearTiles();
}

int Variant::compare(const Variant* variant) const {
	if (_patterns.size() == 0) {
		if (variant->_recognizers.size() == 0)
			return 0;
		else
			return -1;
	} else if (variant->_recognizers.size() == 0)
		return 1;
	return _recognizers[0]->formation()->dancerCount() - variant->_recognizers[0]->formation()->dancerCount();
}

void Variant::action(const string& text) {
	if (_parts.size() == 0)
		_parts.push_back(new Part(this, ""));
	_parts[_parts.size() - 1]->addAction(text);
}

void Variant::addCompound() {
	if (_parts.size() == 0)
		_parts.push_back(new Part(this, ""));
	_parts[_parts.size() - 1]->addAction(new CompoundAction(_parts[_parts.size() - 1]));
}

bool Variant::compoundWho(bool finishTogether, const string& text) {
	if (_parts.size() == 0)
		_parts.push_back(new Part(this, ""));
	return _parts[_parts.size() - 1]->compoundWho(finishTogether, text);
}

bool Variant::compoundWhat(bool anyWhoCan, const string& text) {
	if (_parts.size() == 0)
		_parts.push_back(new Part(this, ""));
	return _parts[_parts.size() - 1]->compoundWhat(anyWhoCan, text);
}

void Variant::insertPart(int index, Part* p) {
	if (index >= _parts.size())
		_parts.push_back(p);
	else
		_parts.insert(index, p);
}

void Variant::deletePart(int index) {
	_parts.remove(index);
}

Part* Variant::nextPart(const string& text) {
	Part* p = new Part(this, text);
	_parts.push_back(p);
	return p;
}

void Variant::setPrecedence(const string& precedence) {
	for (int i = 0; i < precedences.size(); i++)
		if (precedences[i] == precedence) {
			_precedence = i;
			break;
		}
}

void Variant::setLevel(const string& text) {
	_levelName = text.trim();
	_level = *levelValues.get(_levelName);
}

void Variant::verify(const Grammar* grammar) {
	for (int i = 0; i < _patterns.size(); i++) {
		if (_recognizers[i] == null)
			setPattern(i, _patterns[i]);				// Force a parse of the pattern string
		if (verboseOutput && _recognizers[i] == null) {
			printf("Unknown formation name: %s\n", _patterns[i].c_str());
		}
	}
	// Make sure that when we parse in the file, each definition has at least one part.
	// This makes editing easier to get started.
	if (_parts.size() == 0)
		nextPart("");
}

void Variant::write(FILE* fp) {
	for (int i = 0; i < _patterns.size(); i++)
		fprintf(fp, "\t*%s\n", _patterns[i].c_str());
	if (_levelName.size())
		fprintf(fp, "\t!%s\n", _levelName.c_str());
	if (_precedence > 0)
		fprintf(fp, "\t^%s\n", precedences[_precedence].c_str());
	for (int i = 0; i < _parts.size(); i++) {
		if (i > 0 || _parts[i]->repeat().size()) {
			fprintf(fp, "\t+");
			if (_parts[i]->repeat().size())
				fprintf(fp, " %s", _parts[i]->repeat().c_str());
			fprintf(fp, "\n");
		}
		_parts[i]->write(fp);
	}
}

bool Variant::testAnyFormations(const Group* d, Context* context, const Pattern** matched, const Group** orientedGroup, const Anything* call, const Step* step) const {
	timing::Timer t("Variant::testAnyFormations");
	if (_recognizers.size() == 0) {
		*orientedGroup = d;
		return true;
	}
	if (verboseMatching) {
		printf("testAnyFormations:\n");
		d->printDetails(4, true);
	}
	for (int i = 0; i < _recognizers.size(); i++) {
		if (_recognizers[i] == null)
			continue;
		PatternClosure closure(_recognizers[i], call, step, context);
		*orientedGroup = d->match(_recognizers[i], context, &closure);
		if (*orientedGroup != null) {
			*matched = _recognizers[i];
			return true;
		}
	}
	return false;
}

bool Variant::testAnyPhantomFormations(const Group* d, Context* context, const Pattern** matched, const Group** orientedGroup, const Anything* call, const Step* step) const {
	timing::Timer t("Variant::testAnyPhantomFormations");
	if (_recognizers.size() == 0) {
		*orientedGroup = d;
		return true;
	}
	if (verboseMatching) {
		printf("testAnyPhantomFormations:\n");
		d->printDetails(4, true);
	}
	for (int i = 0; i < _recognizers.size(); i++) {
		if (_recognizers[i] == null)
			continue;
		PatternClosure closure(_recognizers[i], call, step, context);
		*orientedGroup = d->matchSome(0, _recognizers[i], context, &closure, TILE_WITH_PHANTOMS);
		if (*orientedGroup != null &&
			(*orientedGroup)->realDancerCount() == d->dancerCount()) {
			*matched = _recognizers[i];
			return true;
		}
	}
	return false;
}

bool Variant::construct(Plan* p, const Anything* call, Context* context, TileAction tileAction) const {
	timing::Timer t("Variant::construct");
	for (int j = 0; j < _parts.size(); j++) {
		const Part* part = _parts[j];
		int repeat = part->repeatCount(p, call, context);
		if (p->failed())
			return false;
		for (int r = 0; r < repeat; r++)
			if (!p->constructStep(part, context, tileAction))
				return false;
	}
	return true;
}

Level Variant::effectiveLevel() const {
	if (_level != NO_LEVEL)
		return _level;
	else
		return _definition->level();
}

Part::~Part() {
	_actions.deleteAll();
}

void Part::addAction(const string& text) {
	_actions.push_back(new SimpleAction(this, text));
}

void Part::addAction(Action* action) {
	_actions.push_back(action);
}

bool Part::compoundWho(bool finishTogether, const string& text) {
	if (_actions.size() == 0)
		return false;
	Action* a = _actions[_actions.size() - 1];
	if (typeid(*a) != typeid(CompoundAction))
		return false;
	CompoundAction* ca = (CompoundAction*)a;
	Track* t = ca->addTrack();
	t->finishTogether = finishTogether;
	t->who = text;
	return true;
}

bool Part::compoundWhat(bool anyWhoCan, const string& text) {
	if (_actions.size() == 0)
		return false;
	Action* a = _actions[_actions.size() - 1];
	if (typeid(*a) != typeid(CompoundAction))
		return false;
	CompoundAction* ca = (CompoundAction*)a;
	if (ca->tracks().size() == 0)
		return false;
	Track* t = ca->tracks()[ca->tracks().size() - 1];
	t->what = text;
	t->anyWhoCan = anyWhoCan;
	return true;
}

void Part::setAction(int i, const string& text) {
	if (i == _actions.size() && text.size() != 0) {
		_actions.push_back(new SimpleAction(this, text));
	} else if (text.size() == 0 && i == _actions.size() - 1)
		_actions.resize(i);
	else if (i >= 0 && i < _actions.size() && typeid(*_actions[i]) == typeid (SimpleAction))
		((SimpleAction*)_actions[i])->set_action(text);
}

void Part::setAction(int i, Action* action) {
	if (i == _actions.size() && action != null && !action->noop()) {
		_actions.push_back(action);
	} else if ((action == null || action->noop()) && i == _actions.size() - 1)
		_actions.resize(i);
	else if (i >= 0 && i < _actions.size())
		_actions[i] = action;
}

void Part::setRepeat(const string& text) {
	_repeat = text.trim();
}

int Part::repeatCount(Plan* p, const Anything* call, Context* context) const {
	if (_repeat.size() == 0)
		return 1;
	else {
		CallParser parser(_repeat, context->grammar(), null, true, false);
		const Term* t = parser.parseExpression(call, context);
		if (t == null) {
			p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Could not parse repeat expression"));
			if (verboseOutput) {
				printf("%s\n", _repeat.c_str());
				printf("   *** Syntax error in repeat expression.\n");
			}
			return 0;
		}
		if (typeid(*t) != typeid(Fraction)) {
			p->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Repeat expression is not numeric"));
			if (verboseOutput) {
				printf("%s\n", _repeat.c_str());
				printf("   *** Not a Fraction in repeat expression.\n");
			}
			return 0;
		}
		const Fraction* f = (const Fraction*)t;
		int v;
		if (!f->improperNumerator(1, null, &v)) {
			p->fail(context->stage()->newExplanation(USER_ERROR, "Repeat expression is not a whole number"));
			if (verboseOutput) {
				printf("%s\n", _repeat.c_str());
				printf("   *** Not a whole number in repeat expression.\n");
			}
			return 0;
		}
		return v;
	}
}

void Part::write(FILE* fp) {
	for (int i = 0; i < _actions.size(); i++)
		_actions[i]->write(fp);
}

Step* SimpleAction::construct(PartStep* step, Context* context, TileAction tileAction) const {
	const Anything* c = context->grammar()->parse(step->plan()->orientedStart(), 
												  _action, 
												  true, 
												  step->plan()->call(), context, step->plan());
	if (c)
		return step->tiles()[0]->plan()->constructStep(c, context, tileAction);
	else {
		if (anyVerbose()) {
			printf("        *** Unrecognized text ***\n");
			printf("        *** Variant failed ***\n");
		}
		step->fail(context->stage()->newExplanation(DEFINITION_ERROR, "Unrecognized text: " + _action));
		return null;
	}
}

bool SimpleAction::noop() const {
	return _action.size() == 0;
}

void SimpleAction::write(FILE* fp) const {
	if (_action.size())
		fprintf(fp, "\t\t>%s\n", _action.c_str());
}

void SimpleAction::print(int indent) const {
	printf("%*.*c'%s'\n", indent, indent, ' ', _action.c_str());
}

CompoundAction::~CompoundAction() {
	_tracks.deleteAll();
}

Track* CompoundAction::addTrack() {
	Track* t = new Track();
	_tracks.push_back(t);
	return t;
}

void CompoundAction::setWho(int track, const string& who) {
	while (track >= _tracks.size())
		addTrack();
	_tracks[track]->who = who;
	if (who.size() == 0) {
		if (noop(track)) {
			if (track == _tracks.size() - 1)
				_tracks.resize(track);
			return;
		}
	}
}

void CompoundAction::setWhat(int track, const string& what) {
	while (track >= _tracks.size())
		addTrack();
	_tracks[track]->what = what;
	if (what.size() == 0) {
		if (noop(track)) {
			if (track == _tracks.size() - 1)
				_tracks.resize(track);
			return;
		}
	}
}

void CompoundAction::setFinishTogether(int track, bool finishTogether) {
	while (track >= _tracks.size())
		addTrack();
	_tracks[track]->finishTogether = finishTogether;
}

void CompoundAction::setAnyWhoCan(int track, bool anyWhoCan) {
	while (track >= _tracks.size())
		addTrack();
	_tracks[track]->anyWhoCan = anyWhoCan;
}

bool CompoundAction::noop(int track) {
	if (track >= _tracks.size())
		return true;
	Track* t = _tracks[track];
	return t->who.size() == 0 && t->what.size() == 0;
}

Step* CompoundAction::construct(PartStep* step, Context* context, TileAction tileAction) const {
	return step->tiles()[0]->plan()->constructStep(this, context, tileAction);
}

bool CompoundAction::noop() const {
	for (int i = 0; i < _tracks.size(); i++)
		if (_tracks[i]->who.size() > 0 ||
			_tracks[i]->what.size() > 0)
			return false;
	return true;
}

void CompoundAction::write(FILE* fp) const {
	if (noop())
		return;
	fprintf(fp, "\t\t<\n");
	for (int i = 0; i < _tracks.size(); i++) {
		Track* t = _tracks[i];
		if (t->noop() && !t->anyWhoCan && !t->finishTogether)
			continue;
		fprintf(fp, "\t\t\t@%s %s\n", t->finishTogether ? "T" : "F", t->who.c_str());
		fprintf(fp, "\t\t\t#%s %s\n", t->anyWhoCan ? "T" : "F", t->what.c_str());
	}
}

void CompoundAction::print(int indent) const {
	printf("%*.*cStart together:\n", indent, indent, ' ');
	for (int i = 0; i < _tracks.size(); i++) {
		Track* t = _tracks[i];
		printf("%*.*c    %s / %s  %s\n", indent, indent, ' ', t->who.c_str(), t->what.c_str(), t->finishTogether ? "Finish together" : "");
	}
}

bool Track::noop() const {
	return who.size() == 0 && what.size() == 0;
}

int VariantTile::compare(const VariantTile* other) const {
	return pattern->formation()->dancerCount() - other->pattern->formation()->dancerCount();
}

void PhraseMeaning::setCreated(const string& value) {
	_created = parseLongLong(value, 0);
}

void PhraseMeaning::setModified(const string& value) {
	_modified = parseLongLong(value, 0);
}

void PhraseMeaning::setModified(time_t value) {
	_modified = value;
}

Definition::~Definition() {
	_variants.deleteAll();
}

void Definition::setLevel(const string& levelName) {
	_levelName = levelName;
	_level = *levelValues.get(_levelName);
}

int Definition::compare(const Definition* other) const {
	const string* p1;
	const string* p2;

	string empty;

	if (_productions.size())
		p1 = &_productions[0];
	else
		p1 = &empty;
	if (other->_productions.size())
		p2 = &other->_productions[0];
	else
		p2 = &empty;
	return p1->compare(p2);
}

int Definition::addProduction() {
	int i = _productions.size();
	_productions.push_back("");
	return i;
}

bool Definition::setProduction(int index, const string& text) {
	if (text.size() == 0 && index == _productions.size() - 1) {
		_productions.resize(index);
		return true;
	} else {
		_productions[index] = text;
		CallParser parser(_productions[index], grammar(), null, true, false);
		return parser.verifyCall();
	}
}

void Definition::addPattern(const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	_variants[_variants.size() - 1]->addPattern(text);
}

void Definition::action(const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	_variants[_variants.size() - 1]->action(text);
}

void Definition::addCompound() {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	_variants[_variants.size() - 1]->addCompound();
}

bool Definition::compoundWho(bool finishTogether, const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	return _variants[_variants.size() - 1]->compoundWho(finishTogether, text);
}

bool Definition::compoundWhat(bool anyWhoCan, const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	return _variants[_variants.size() - 1]->compoundWhat(anyWhoCan, text);
}

void Definition::nextPart(const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	_variants[_variants.size() - 1]->nextPart(text);
}

void Definition::setVariantLevel(const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	_variants[_variants.size() - 1]->setLevel(text);
}

void Definition::setVariantPrecedence(const string& text) {
	if (_variants.size() == 0)
		_variants.push_back(new Variant(this));
	_variants[_variants.size() - 1]->setPrecedence(text);
}

void Definition::nextVariant() {
	_variants.push_back(new Variant(this));
}

void Definition::insertVariant(int index, Variant* v) {
	if (index >= _variants.size())
		_variants.push_back(v);
	else
		_variants.insert(index, v);
	_tilesBuilt = false;
}

void Definition::deleteVariant(int index) {
	_variants.remove(index);
	_tilesBuilt = false;
}

void Definition::removeLastVariant() {
	_variants.resize(_variants.size() - 1);
	_tilesBuilt = false;
}

void Definition::setName(const string& name) {
	_name = name;
}

void Definition::verify(const Grammar* grammar) {
	for (int i = 0; i < _variants.size(); i++)
		_variants[i]->verify(grammar);
}

const vector<VariantTile>& Definition::tiles() const {
	if (!_tilesBuilt) {
		_tilesBuilt = true;
		_tiles.clear();
		vector<VariantTile> x;
		for (int i = 0; i < _variants.size(); i++) {
			const vector<const Pattern*>& p = _variants[i]->recognizers();
			VariantTile vt;
			vt.variant = _variants[i];
			for (int j = 0; j < p.size(); j++) {
				if (p[j]) {
					vt.pattern = p[j];
					x.push_back(vt);
				}
			}
		}
		// we can only sort pointers to T, not T's themselves.
		vector<VariantTile*> xp;
		for (int j = 0; j < x.size(); j++)
			xp.push_back(&x[j]);
		xp.sort(false);
		for (int i = 0; i < xp.size(); i++)
			_tiles.push_back(*xp[i]);
	}
	return _tiles; 
}

Level Definition::level() const	{ 
	return _level;
}

const string& Definition::label() const {
	if (_productions.size())
		return _productions[0];
	else
		return _emptyLabel; 
}

string Definition::callText() const {
	return label();
}

void Definition::write(FILE* fp) {
	fprintf(fp, "--");
	if (_levelName.size())
		fprintf(fp, " %s", _levelName.c_str());
	fprintf(fp, "\n");
	if (created())
		fprintf(fp, "C%I64d\n", created());
	if (modified())
		fprintf(fp, "M%I64d\n", modified());
	if (_name.size())
		fprintf(fp, ".%s\n", _name.c_str());
	for (int i = 0; i < _productions.size(); i++)
		fprintf(fp, "%s\n", _productions[i].c_str());
	for (int i = 0; i < _variants.size(); i++) {
		_variants[i]->write(fp);
		if (i < _variants.size() - 1)
			fprintf(fp, "|\n");
	}
}

void Definition::print() const {
	printf("--\n");
	if (_name.size())
		printf("Name: %s\n", _name.c_str());
	printf("Production%s:\n", _productions.size() > 1 ? "s" : "");
	for (int i = 0; i < _productions.size(); i++) {
		printf("    ");
		if (_productions.size() > 1)
			printf("%d: ", i + 1);
		printf("%s\n", _productions[i].c_str());
	}
}

Designator::Designator(Grammar* grammar, const Anyone* a, const string& word, Level level) : PhraseMeaning(grammar) {
	_level = level;
	_levelString = levels[level];
	_expression = word;
	_phrases.push_back(word);
}

void Designator::setLevel(const string& level) {
	_level = *levelValues.get(level);
}

void Designator::setExpression(const string& expression) {
	_expression = expression;
}

Level Designator::level() const {
	return _level; 
}
 
void Designator::write(FILE* fp) const {
	fprintf(fp, "++");
	if (_level != NO_LEVEL)
		fprintf(fp, " %s", levels[_level].c_str());
	fprintf(fp, "\n");
	if (created())
		fprintf(fp, "C%I64d\n", created());
	if (modified())
		fprintf(fp, "M%I64d\n", modified());
	fprintf(fp, ".%s\n", _expression.c_str());
	for (int i = 0; i < _phrases.size(); i++)
		fprintf(fp, "%s\n", _phrases[i].c_str());
}

bool Designator::processLine(DefinitionsContext& ctx, int* startP) {
	int start = *startP;
	int end = ctx.text.find('\n', start);
	if (end == string::npos) {
		printf("%s line %d: Last designator ends without a newline\n", ctx.filename->c_str(), ctx.line);
		*startP = ctx.text.size();
		return false;
	}
	switch (ctx.text[start]) {
	case	'.':
		setExpression(ctx.text.substr(start + 1, end - start - 1));
		start = end + 1;
		// Now we have the productions
		while (start < ctx.text.size() &&
			   ctx.text[start] != '%' &&
			   ctx.text[start] != '+') {
			end = ctx.text.find('\n', start);
			if (end == string::npos) {
				printf("%s line %d: Last designator ends without a newline\n", ctx.filename->c_str(), ctx.line);
				*startP = ctx.text.size();
				return false;
			}
			int i = addPhrase();
			setPhrase(i, ctx.text.substr(start, end - start));
			start = end + 1;
		}
		*startP = start;
		return false;

	case	'C':
		setCreated(ctx.text.substr(start + 1, end - start - 1));
		start = end + 1;
		break;

	case	'M':
		setModified(ctx.text.substr(start + 1, end - start - 1));
		start = end + 1;
		break;

	default:
		printf("%s line %d: Designator has unexpected line start\n", ctx.filename->c_str(), ctx.line);
		*startP = ctx.text.size();
		return false;
	}
	*startP = start;
	return true;
}

const string& Designator::label() const {
	if (_phrases.size())
		return _phrases[0];
	else
		return _expression;
}

int Designator::compare(const Designator* other) const {
	return _expression.compare(&other->_expression);
}

int Designator::addPhrase() {
	int index = _phrases.size();
	_phrases.push_back(string());
	return index;
}

void Designator::setPhrase(int index, const string& text) {
	if (text.size() == 0 && index == _phrases.size() - 1)
		_phrases.resize(index);
	else
		_phrases[index] = text;
}

const Anyone* Designator::compile(vector<const Term*>& terms, Context* context) const {
	return grammar()->parseDesignatorExpression(_expression, &terms, context);
}

void Synonym::write(FILE* fp) {
	fprintf(fp, ":%s=%s\n", _synonym.c_str(), _value.c_str());
}

void Synonym::print() {
	printf("  %s = %s\n", _synonym.c_str(), _value.c_str());
}

const Term* Term::negate(Context *context) const {
	return null;
}

const Term* Term::not(Context *context) const {
	return null;
}

const Term* Term::positive(Context *context) const {
	return null;
}

const Term* Term::add(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::subtract(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::multiply(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::divide(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::remainder(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::and(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::or(const Term* operand, Context *context) const {
	return null;
}

const Term* Term::xor(const Term* operand, Context *context) const {
	return null;
}

bool Term::compare(const Term* operand, int* compareResult) const {
	if (operand == this) {
		*compareResult = 0;
		return true;
	}
	if (operand == null)
		return false;
	if (typeid(*this) == typeid(*operand)) {
		*compareResult = INT_MAX;				// valid, but unordered != comparison
		return true;
	}
	return false;
}

void Term::token(Token* output) const {
	output->type = WORD;
	output->term = this;
}

int Term::tokenWeight() const {
	return 2;
}

int Term::match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const {
	if (tokens[tIndex].type == WORD &&
		this == tokens[tIndex].term)
		return 1;
	else
		return -1;
}

bool Term::partialMatch(Context* context, const string& text, vector<string>* output) const {
	if (text.size() > _spelling.size())
		return false;
	return memcmp(text.c_str(), _spelling.c_str(), text.size()) == 0;
}

int Term::sortIndex() const {
	return -1;
}

void Anypivot::print(int indent) const {
	printf("%*.*c%s\n", indent, indent, ' ', pivotNames[_pivot]);
}

void Anydirection::print(int indent) const {
	printf("%*.*c%s\n", indent, indent, ' ', directionNames[_direction]);
}

int Anydirection::match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const {
	if (tokens[tIndex].type == WORD &&
		this == tokens[tIndex].term)
		return 1;
	else
		return -1;
}

Level Anything::designatorLevel() const {
	timing::Timer t("Anything::designatorLevel");
	Level max = NO_LEVEL;
	for (int i = 0; i < _variables.size(); i++) {
		const Term* var = _variables[i];
		if (typeid(*var) == typeid(Anything)) {
			const Anything* subCall = (const Anything*)var;
			Level x = subCall->designatorLevel();
			if (x > max)
				max = x;
		} else if (typeid(*var) == typeid(Anyone)) {
			const Anyone* a = (const Anyone*)var;
			if (a->level() > max)
				max = a->level();
		}
	}
	return max;
}

void Anything::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	if (_primitive)
		printf("%s\n", primitiveNames[_primitive->index()]);
	else if (_definition)
		printf("%s\n", _definition->label().c_str());
	else
		printf("<no primitive or definition>\n");
	for (int i = 0; i < _variables.size(); i++) {
		_variables[i]->print(indent + 4);
	}
}

void Integer::token(Token* output) const {
	output->type = INTEGER;
	output->value = _value;
}

void Integer::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("Integer %d\n", _value);
}

int Integer::match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const {
	if (tokens[tIndex].type == INTEGER &&
		tokens[tIndex].value == _value)
		return 1;
	else
		return -1;
}

bool Integer::compare(const Term* operand, int* compareResult) const {
	if (operand == null)
		return false;
	if (typeid(*operand) == typeid(Integer)) {
		const Integer* integer = (const Integer*)operand;

		if (_value > integer->_value)
			*compareResult = 1;
		else if (_value < integer->_value)
			*compareResult = -1;
		else
			*compareResult = 0;
	} else if (typeid(*operand) == typeid(Fraction)) {
		const Fraction* f = (const Fraction*)operand;

		if (f->denominator() == 0)
			return false;
		if (_value < f->whole()) {
			*compareResult = -1;
			return true;
		}
		if (_value == f->whole() && f->numerator() == 0)
			*compareResult = 0;
		else
			*compareResult = 1;
	} else
		return false;
	return true;
}

void Word::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("'%s'\n", spelling().c_str());
}

bool Fraction::improperNumerator(int denominator, const Fraction* fraction, int* result) const {
	if (_denominator == 0 || denominator == 0)
		return false;
	int num = _numerator;
	int denom = _denominator;
	int whole = _whole;
	if (fraction) {
		if (fraction->whole() || fraction->denominator() == 0)
			return false;
		num *= fraction->numerator();
		num += whole % fraction->denominator();
		denom *= fraction->denominator();
		whole /= fraction->denominator();
	}
	int p = num * denominator;
	// Doesn't factor by denominator evenly
	if (p % denom)
		return false;
	*result = whole * denominator + p / denom;
	return true;
}

const Term* Fraction::negate(Context* context) const {
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0)
		return null;
	Fraction* f = context->stage()->newFraction(-_whole, -_numerator, _denominator);
	f->normalize(context);
	return f;
}

const Term* Fraction::not(Context* context) const {
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0)
		return null;
	if (_numerator != 0)
		return null;
	return new Fraction(!_whole, 0, 1);
}

const Term* Fraction::positive(Context* context) const {
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0)
		return null;
	return this;
}

const Term* Fraction::add(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	Fraction* nf = new Fraction(_whole + f->_whole, 
							   _numerator * f->_denominator + f->_numerator * _denominator, 
							   _denominator * f->_denominator);
	nf->normalize(context);
	return nf;
}

const Term* Fraction::subtract(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	Fraction* nf = context->stage()->newFraction(_whole - f->_whole, 
								_numerator * f->_denominator - f->_numerator * _denominator, 
								_denominator * f->_denominator);
	nf->normalize(context);
	return nf;
}

const Term* Fraction::multiply(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	int imp0, imp1;

	imp0 = _whole * _denominator + _numerator;
	imp1 = f->_whole * f->_denominator + f->_numerator;
	Fraction* nf = context->stage()->newFraction(0, 
							    imp0 * imp1, 
								_denominator * f->_denominator);
	nf->normalize(context);
	return nf;
}

const Term* Fraction::divide(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	int imp0, imp1;

	imp0 = _whole * _denominator + _numerator;
	imp1 = f->_whole * f->_denominator + f->_numerator;
	Fraction* nf = context->stage()->newFraction(0, 
							    imp0 * f->_denominator, 
								_denominator * imp1);
	nf->normalize(context);
	return nf;
}

const Term* Fraction::remainder(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	// You can only do remainder on whole numbers
	if (_numerator != 0 || f->_numerator != 0)
		return null;
	return context->stage()->newFraction(_whole % f->_whole, 0, 1);
}

const Term* Fraction::or(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	// You can only do boolean operations on whole numbers
	if (_numerator != 0 || f->_numerator != 0)
		return null;
	return context->stage()->newFraction(_whole | f->_whole, 0, 1);
}

const Term* Fraction::and(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	// You can only do boolean operations on whole numbers
	if (_numerator != 0 || f->_numerator != 0)
		return null;
	return context->stage()->newFraction(_whole & f->_whole, 0, 1);
}

const Term* Fraction::xor(const Term* operand, Context* context) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return null;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return null;
	// You can only do boolean operations on whole numbers
	if (_numerator != 0 || f->_numerator != 0)
		return null;
	return context->stage()->newFraction(_whole ^ f->_whole, 0, 1);
}

bool Fraction::compare(const Term* operand, int* compareResult) const {
	if (operand == null || typeid(*operand) != typeid(Fraction))
		return false;
	const Fraction* f = (const Fraction*)operand;
	// 0 denominator means a 'magic' fraction that is not really numeric
	if (_denominator == 0 || f->_denominator == 0)
		return Term::compare(operand, compareResult);
	int imp0, imp1;

	imp0 = (_whole * _denominator + _numerator) * f->_denominator;
	imp1 = (f->_whole * f->_denominator + f->_numerator) * _denominator;

	if (imp0 > imp1)
		*compareResult = 1;
	else if (imp0 == imp1)
		*compareResult = 0;
	else
		*compareResult = -1;
	return true;
}

const Fraction* Fraction::normalize(Context* context) const {
	Fraction* f = context->stage()->newFraction(_whole, _numerator, _denominator);
	if (_denominator) {
		if (_denominator < 0) {
			f->_denominator = -_denominator;
			f->_numerator = -_numerator;
		}
		if (f->_numerator > f->_denominator) {
			f->_whole += f->_numerator / f->_denominator;
			f->_numerator %= f->_denominator;
		}
		if (f->_numerator < 0) {
			f->_numerator += f->_denominator;
			f->_whole--;
		}
	}
	return f;
}

void Fraction::token(Token *output) const {
	output->type = FRACTION;
	output->term = this;
}

void Fraction::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	if (_denominator == 0) {
		switch (_numerator) {
		case	1:
			printf("$until_home\n");
			return;
		}
	}
	printf("Fraction ");
	if (_whole)
		printf("%d&", _whole);
	printf("%d/%d\n", _numerator, _denominator);
}

int Fraction::match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const {
	if (tokens[tIndex].type == FRACTION) {
		const Fraction* f = (const Fraction*)tokens[tIndex].term;
		if (_denominator == 0) {
			if (f->_denominator != 0)
				return -1;
			if (_numerator != f->_numerator)
				return -1;
			if (_whole != f->_whole)
				return -1;
		} else if (f->_denominator == 0)
			return -1;
		int ad = (_whole * _denominator + _numerator) * f->_denominator;
		int bc = (f->_whole * f->_denominator + f->_numerator) * _denominator;
		if (ad != bc)
			return -1;
		else
			return 1;
	}
	if (tIndex + 3 > tokens.size())
		return -1;
	if (tokens[tIndex].type != INTEGER)
		return -1;
	if (tokens[tIndex + 1].type == SLASH) {
		if (tokens[tIndex + 2].type == INTEGER) {
			if (_whole != 0)
				return -1;
			if (_numerator != tokens[tIndex].value)
				return -1;
			if (_denominator != tokens[tIndex + 2].value)
				return -1;
			return 3;
		} else
			return -1;
	} else if (tokens[tIndex + 1].type != WORD ||
			   tokens[tIndex + 1].term != context->grammar()->and())
		return -1;
	if (tIndex + 5 > tokens.size())
		return -1;
	if (tokens[tIndex + 2].type == INTEGER &&
		tokens[tIndex + 3].type == SLASH &&
		tokens[tIndex + 4].type == INTEGER) {
		if (_whole != tokens[tIndex].value)
			return -1;
		if (_numerator != tokens[tIndex + 2].value)
			return -1;
		if (_denominator != tokens[tIndex + 4].value)
			return -1;
		return 5;
	} else
		return -1;
}

int BuiltIn::tokenWeight() const {
	return 1;
}

void BuiltIn::print(int indent) const {
	if (indent)
		printf("%*.*c", indent, indent, ' ');
	printf("%s\n", tokenNames[_index]);
}

int BuiltIn::match(const vector<Token>& tokens, int tIndex, vector<const Term*>* nonTerminals, Context* context) const {
	const Term* t;
	const Anything* call;
	const Anyone* anyone;
	int x;

	switch (_index) {
	case	R_L:
		x = context->grammar()->matchR_L(tokens, tIndex);
		if (x > 0)
			nonTerminals->push_back(tokens[tIndex].term);
		return x;

	case	P_C:
		x = context->grammar()->matchP_C(tokens, tIndex);
		if (x > 0)
			nonTerminals->push_back(tokens[tIndex].term);
		return x;

	case	ANYONE:
	case	ANYTHING:
	case	ANYCALL:
		return -1;

	case	DANCER_NAME:
		if (tokens[tIndex].type != WORD)
			return -1;
		t = tokens[tIndex].term;
		if (typeid(*t) == typeid(Anyone)) {
			nonTerminals->push_back(t);
			return 1;
		}
		return -1;

	case	PRIMITIVE:
		call = context->grammar()->matchPrimitive(tokens, tIndex, &x, context);
		if (x > 0)
			nonTerminals->push_back(call);
		return x;

	case	INTEGER:
		if (tokens[tIndex].type == INTEGER) {
			nonTerminals->push_back(context->stage()->newInteger(tokens[tIndex].value));
			return 1;
		}
		if (tokens[tIndex].type == LPAREN) {
			CallParser parser(tokens, tIndex + 1);
			const Term* term = parser.parseExpressionPrefix(null, context);
			if (term == null)
				return -1;
			int i = parser.at() - 1;
			if (i >= tokens.size())
				return -1;
			if (tokens[i].type != RPAREN)
				return -1;
			if (typeid(*term) == typeid(Fraction)) {
				const Fraction* f = (const Fraction*)term;
				if (f->denominator() == 0)
					return -1;
				int value;
				if (!f->improperNumerator(1, null, &value))
					return -1;
				nonTerminals->push_back(context->stage()->newInteger(value));
				return i + 1 - tIndex;
			} else
				return -1;
		}
		return -1;

	case	FRACTION:
		if (tokens[tIndex].type == FRACTION) {
			nonTerminals->push_back(tokens[tIndex].term);
			return 1;
		}
		if (tokens[tIndex].type == LPAREN) {
			CallParser parser(tokens, tIndex + 1);
			const Term* term = parser.parseExpressionPrefix(null, context);
			if (term == null)
				return -1;
			int i = parser.at() - 1;
			if (i >= tokens.size())
				return -1;
			if (tokens[i].type != RPAREN)
				return -1;
			if (typeid(*term) == typeid(Fraction)) {
				nonTerminals->push_back(term);
				return i + 1 - tIndex;
			} else
				return -1;
		}
		if (tIndex + 3 > tokens.size())
			return -1;
		if (tokens[tIndex].type != INTEGER)
			return -1;
		if (tokens[tIndex + 1].type == SLASH) {
			if (tokens[tIndex + 2].type == INTEGER) {
				if (tokens[tIndex + 2].value == 0)
					return -1;
				nonTerminals->push_back(context->stage()->newFraction(0, tokens[tIndex].value, tokens[tIndex + 2].value));
				return 3;
			} else
				return -1;
		} else if (tokens[tIndex + 1].type != WORD ||
				   tokens[tIndex + 1].term != context->grammar()->and())
			return -1;
		if (tIndex + 5 > tokens.size())
			return -1;
		if (tokens[tIndex + 2].type == INTEGER &&
			tokens[tIndex + 3].type == SLASH &&
			tokens[tIndex + 4].type == INTEGER) {
			if (tokens[tIndex + 4].value == 0)
				return -1;
			nonTerminals->push_back(context->stage()->newFraction(tokens[tIndex].value, tokens[tIndex + 2].value, tokens[tIndex + 4].value));
			return 5;
		} else
			return -1;
	}
	return -1;
}

bool BuiltIn::partialMatch(Context* context, const string& text, vector<string>* output) const {
	vector<Token> tokens;
	Token finalPartial;
	switch (_index) {
	case	R_L:
		if (locale::startsWithIgnoreCase("left", text))
			return true;
		if (locale::startsWithIgnoreCase("right", text))
			return true;
		return false;

	case	P_C:
		if (locale::startsWithIgnoreCase("partner", text))
			return true;
		if (locale::startsWithIgnoreCase("corner", text))
			return true;
		return false;

	case	ANYONE:
	case	ANYTHING:
	case	ANYCALL:
		context->grammar()->partialMatch(_index, text, context, output);
		return false;

	case	DANCER_NAME:
		return locale::startsWithIgnoreCase("$dancer", text);

	case	PRIMITIVE:
		for (int i = 0; i < P_PRIMITIVE_COUNT; i++)
			if (locale::startsWithIgnoreCase(primitiveNames[i], text))
				return true;
		return false;

	case	INTEGER:
		return false;

	case	FRACTION:
		if (!context->grammar()->tokenize(null, text, false, null, context, null, tokens, &finalPartial) ||
			tokens.size() != 0 ||
			finalPartial.type == END_OF_STRING)
			return false;
		return finalPartial.text != text;
	}
	return false;
}

int BuiltIn::sortIndex() const {
	switch (_index) {
	case	INTEGER:
		return 0;

	case	FRACTION:
		return 1;

	default:
		return 2;
	}
}

static bool validWordContent(char c, bool inDefinition) {
	switch (c) {
	case	'_':
	case	'$':
		return inDefinition;

	case	'a':
	case	'b':
	case	'c':
	case	'd':
	case	'e':
	case	'f':
	case	'g':
	case	'h':
	case	'i':
	case	'j':
	case	'k':
	case	'l':
	case	'm':
	case	'n':
	case	'o':
	case	'p':
	case	'q':
	case	'r':
	case	's':
	case	't':
	case	'u':
	case	'v':
	case	'w':
	case	'x':
	case	'y':
	case	'z':
	case	'A':
	case	'B':
	case	'C':
	case	'D':
	case	'E':
	case	'F':
	case	'G':
	case	'H':
	case	'I':
	case	'J':
	case	'K':
	case	'L':
	case	'M':
	case	'N':
	case	'O':
	case	'P':
	case	'Q':
	case	'R':
	case	'S':
	case	'T':
	case	'U':
	case	'V':
	case	'W':
	case	'X':
	case	'Y':
	case	'Z':
	case	'\'':
	case	'-':
		return true;

	default:
		return false;
	}
}

}  // namespace dance
