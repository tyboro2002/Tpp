#include "Parser.h"

int operatorToPrecedence(TokenType tokenType) { // return -1 if no bin operator else return precedence level
	if (tokenType == TokenType::addition) return 0;
	if (tokenType == TokenType::subtraction) return 0;
	if (tokenType == TokenType::multiplication) return 1;
	if (tokenType == TokenType::division) return 1;
	return -1;
}

bool isBinOp(TokenType tokenType) { // return true if binop else return false
	return !(operatorToPrecedence(tokenType) == -1);
}

std::optional<Token> Parser::peak(int ahead) const {
	if (m_index + ahead > m_tokens.size()) return {};
	else return m_tokens.at(m_index + ahead - 1);
}

Token Parser::consume() { return m_tokens.at(m_index++);}

bool Parser::isToken(TokenType type, int ahead) {
	return peak(ahead).has_value() && peak(ahead).value().type == type;
}

bool Parser::isNotToken(TokenType type, int ahead) {
	return !peak(ahead).has_value() || peak(ahead).value().type != type;
}

void Parser::sayError(char ch) {
	if (peak().has_value()) {
		std::cerr << "expected " << ch << " and found " << peak().value() << " after " << peak(0).value() << " at index " << m_index << std::endl;
	}
	else {
		std::cerr << "expected " << ch << " and found nothing" << std::endl;
	}
}

std::optional<NodeTerm> Parser::parse_term() {
	NodeTerm nodeTerm;
	if (isToken(TokenType::tppcount)) {
		nodeTerm.term_part = tryConsume(TokenType::tppcount, "this tppCount disapeared");
	}else if (isToken(TokenType::int_lit)) {
		nodeTerm.term_part = tryConsume(TokenType::int_lit, "this int_lit disapeared");
	}else if (isToken(TokenType::open_Quote)) {
		tryConsume(TokenType::open_Quote, "expected open quote");
		nodeTerm.term_part = tryConsume(TokenType::string_lit, "this string_lit disapeared");
		tryConsume(TokenType::closed_Quote, "expected closing quote");
	}else if (isToken(TokenType::identifier)) {
		nodeTerm.term_part = tryConsume(TokenType::identifier, "this identifier disapeared");
	}else if (isToken(TokenType::tppinp)) {
		nodeTerm.term_part = parseTppInp();
	}else if (isToken(TokenType::open_Paren)) {
		parseOpenParen();
		auto expr = parse_expr();
		if (!expr.has_value()) {
			std::cerr << "expected expression" << std::endl;
			exit(EXIT_FAILURE);
		}
		parseCloseParen();
		NodeTermParen nodeTermParen = NodeTermParen{.parenthesed_expr = expr.value()};
		nodeTerm.term_part = nodeTermParen;
	}
	else {
		return {};
	}
	return nodeTerm;
}

std::optional<NodeExpr*> Parser::parse_expr(int min_prec) { //TODO https://www.youtube.com/watch?v=6nl5HTGgvnk&t=1s 40:00 ->
	std::optional<NodeTerm> term_lhs = parse_term();
	if (!term_lhs.has_value()) {
		return {};
	}
	auto expr_lhs = m_allocator.alloc<NodeExpr>();
	expr_lhs->exprPart = term_lhs.value();

	while (true) {
		std::optional<Token> curr_tok = peak();
		int prec;
		if (curr_tok.has_value()){
			TokenType tokenType = curr_tok->type;
			prec = operatorToPrecedence(tokenType);
			if (!isBinOp(tokenType) || prec < min_prec) { //TODO test
				break;
			}
		}
		Token op = consume();
		int next_min_prec = prec + 1;
		auto expr_rhs = parse_expr(next_min_prec);
		if (!expr_rhs.has_value()) {
			std::cerr << "Unable to parse expression" << std::endl;
			exit(EXIT_FAILURE);
		}

		auto expr = m_allocator.alloc<NodeBinExpr>();
		auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
		if (op.type == TokenType::addition) {
			auto add = m_allocator.alloc<NodeBinExprAdd>();
			expr_lhs2->exprPart = expr_lhs->exprPart;
			add->left = expr_lhs2;
			add->right = expr_rhs.value();
			expr->expr = add;
		}
		else if (op.type == TokenType::subtraction) {
			auto sub = m_allocator.alloc<NodeBinExprSub>();
			expr_lhs2->exprPart = expr_lhs->exprPart;
			sub->left = expr_lhs2;
			sub->right = expr_rhs.value();
			expr->expr = sub;
		}
		else if (op.type == TokenType::division) {
			auto divi = m_allocator.alloc<NodeBinExprDiv>();
			expr_lhs2->exprPart = expr_lhs->exprPart;
			divi->left = expr_lhs2;
			divi->right = expr_rhs.value();
			expr->expr = divi;
		}
		else if (op.type == TokenType::multiplication) {
			auto multi = m_allocator.alloc<NodeBinExprMult>();
			expr_lhs2->exprPart = expr_lhs->exprPart;
			multi->left = expr_lhs2;
			multi->right = expr_rhs.value();
			expr->expr = multi;
		}
		expr_lhs->exprPart = expr;
	}
	return expr_lhs;
}

void Parser::parseOpenParen() {
	if (isNotToken(TokenType::open_Paren)) {
		sayError(OPEN_PAREN);
		exit(EXIT_FAILURE);
	}
	consume();
}

void Parser::parseCloseParen() {
	if (isNotToken(TokenType::closed_Paren)) {
		sayError(CLOSED_PAREN);
		exit(EXIT_FAILURE);
	}
	consume();
}

void Parser::parseOpenQuote() {
	if (isNotToken(TokenType::open_Quote)) {
		sayError(QUOTE);
		std::cerr << "please open your string quotes." << NewLine << peak().value() << std::endl;
		exit(EXIT_FAILURE);
	}
	consume();
}

void Parser::parseCloseQuote() {
	if (isNotToken(TokenType::closed_Quote)) {
		sayError(QUOTE);
		std::cerr << "please close your string quote" << std::endl;
		exit(EXIT_FAILURE);
	}
	consume();
}

void Parser::parseOpenCurly() {
	if (isNotToken(TokenType::open_curly)) {
		sayError(OPEN_CURLY);
		exit(EXIT_FAILURE);
	}
	consume();
}

void Parser::parseCloseCurly() {
	if (isNotToken(TokenType::closed_curly)) {
		sayError(CLOSED_CURLY);
		exit(EXIT_FAILURE);
	}
	consume();
}

void Parser::parseEquals() {
	if (isNotToken(TokenType::equals)) {
		sayError(EQUAL);
		exit(EXIT_FAILURE);
	}
	consume(); //consume the equals
}

void Parser::parseSemi() {
	if (isNotToken(TokenType::semi)) {
		sayError(SEMI);
		exit(EXIT_FAILURE);
	}
	consume(); //consume the semicoln
}

Token Parser::tryConsume(TokenType tokenType, std::string errorMessage) {
	if (isNotToken(tokenType)) {
		std::cerr << errorMessage << std::endl;
		exit(EXIT_FAILURE);
	}
	return consume();
}

NodeReturn Parser::parseReturn(){
	NodeReturn return_node;
	tryConsume(TokenType::_return, "expected return"); //consume the return node
	parseOpenParen();
	if (auto node_expr = parse_expr()) {
		return_node = NodeReturn{ .retVal = node_expr.value() };
	}
	else {
		std::cerr << "invalid expresion encouterd in return!" << std::endl;
		exit(EXIT_FAILURE);
	}
	parseCloseParen();
	parseSemi();
	return return_node;
}

NodeIdentifier Parser::parseIdentifier() {
	NodeIdentifier identifier_node;
	Token nameToken = tryConsume(TokenType::identifier, "expected identifier");
	if (isToken(TokenType::compound_add)) {
		identifier_node.compound = tryConsume(TokenType::compound_add, "expected +=");
	}else if (isToken(TokenType::compound_sub)) {
		identifier_node.compound = tryConsume(TokenType::compound_sub, "expected -=");
	}else if (isToken(TokenType::compound_div)) {
		identifier_node.compound = tryConsume(TokenType::compound_div, "expected /=");
	}else if (isToken(TokenType::compound_mult)) {
		identifier_node.compound = tryConsume(TokenType::compound_mult, "expected *=");
	}else if (isToken(TokenType::compound_modulus)) {
		identifier_node.compound = tryConsume(TokenType::compound_modulus, "expected %=");
	}else if (isToken(TokenType::compound_bitwise_and)) {
		identifier_node.compound = tryConsume(TokenType::compound_bitwise_and, "expected &=");
	}else if (isToken(TokenType::compound_bitwise_or)) {
		identifier_node.compound = tryConsume(TokenType::compound_bitwise_or, "expected |=");
	}else if (isToken(TokenType::compound_bitwise_xor)) {
		identifier_node.compound = tryConsume(TokenType::compound_bitwise_xor, "expected ^=");
	}else if (isToken(TokenType::compound_left_shift)) {
		identifier_node.compound = tryConsume(TokenType::compound_left_shift, "expected <<=");
	}else if (isToken(TokenType::compound_right_shift)) {
		identifier_node.compound = tryConsume(TokenType::compound_right_shift, "expected >>=");
	}else {
		parseEquals();
	}

	if (auto node_expr = parse_expr()) {
		identifier_node.name = nameToken.value.value();
		identifier_node.expr = node_expr.value();
	}
	else {
		std::cerr << "invalid expresion encounterd in identifier!" << std::endl;
		exit(EXIT_FAILURE);
	}
	parseSemi();
	return identifier_node;
}

NodeSay Parser::parseSay(){
	NodeSay print_node;
	tryConsume(TokenType::say, "expected say");
	parseOpenParen();
	if (isToken(TokenType::identifier)) {
		print_node = NodeSay{ .string_lit_identifier = tryConsume(TokenType::identifier, "this identifier disapeared") };
	}
	else {
		parseOpenQuote();
		if (isToken(TokenType::string_lit)) {
			print_node = NodeSay{ .string_lit_identifier = tryConsume(TokenType::string_lit, "this string_lit disapeared") };
		}
		parseCloseQuote();
	}
	parseCloseParen();
	parseSemi();
	return print_node;
}


NodeShout Parser::parseShout() {
	NodeShout print_node;
	tryConsume(TokenType::shout, "expected shout");
	parseOpenParen();
	if (isToken(TokenType::identifier)) {
		print_node = NodeShout{ .string_lit_identifier = tryConsume(TokenType::identifier, "this identifier disapeared") };
	}
	else {
		parseOpenQuote();
		if (isToken(TokenType::string_lit)) {
			print_node = NodeShout{ .string_lit_identifier = tryConsume(TokenType::string_lit, "this string_lit disapeared") };
		}
		parseCloseQuote();
	}
	parseCloseParen();
	parseSemi();
	return print_node;
}

NodeExit Parser::parseExit() {
	NodeExit exit_node;
	tryConsume(TokenType::_exit, "expected exit"); //consume the exit node
	parseOpenParen();
	if (auto node_expr = parse_expr()) {
		exit_node = NodeExit{ .expr = node_expr.value() };
	}
	else {
		std::cerr << "invalid expresion encounterd in exit!" << std::endl;
		exit(EXIT_FAILURE);
	}
	parseCloseParen();
	parseSemi();
	return exit_node;
}


std::optional<NodeElse> Parser::parseOptionalElse() {
	NodeElse elseNode;
	if (isNotToken(TokenType::_else)) {
		return {};
	}
	tryConsume(TokenType::_else, "expected else"); // consume the else token
	parseOpenCurly(); // consume the open curly
	elseNode = NodeElse{ .scope = parseProgram() };
	parseCloseCurly(); // consume the closed curly
	return elseNode;
}

std::vector<NodeElif> Parser::parseElifs() {
	std::vector<NodeElif> elifs;
	NodeElif elifNode;// = m_allocator.alloc<NodeElif>();
	while (isToken(TokenType::_elif)) {
		tryConsume(TokenType::_elif, "expected elif"); // consume the elif
		parseOpenParen();
		if (auto node_expr = parse_expr()) {
			NodeExpr* exprNodeLeft =node_expr.value();
			if (auto node_test = parseTest(exprNodeLeft)) {
				elifNode.expr = node_test.value();
			}
			else {
				elifNode.expr = exprNodeLeft;
			}
			parseCloseParen();
			parseOpenCurly();
			elifNode.scope.codeLines = parseProgram();
			parseCloseCurly();
		}
		else {
			std::cerr << "invalid expresion encounterd in elif!" << std::endl;
			exit(EXIT_FAILURE);
		}
		elifs.push_back(elifNode);
	}
	return elifs;
}

NodeIf Parser::parseIf() {
	NodeIf ifNode;
	tryConsume(TokenType::_if, "expected if"); //consume the if token
	parseOpenParen();
	if (auto node_expr = parse_expr()) {
		NodeExpr* exprNodeLeft = node_expr.value();
		if (auto node_test = parseTest(exprNodeLeft)) {
			parseCloseParen();
			parseOpenCurly();
			ifNode = NodeIf{ .expr = node_test.value(), .scope = parseProgram()};
		}
		else {
			parseCloseParen();
			parseOpenCurly();
			ifNode = NodeIf{ .expr = exprNodeLeft, .scope = parseProgram() };
		}
		parseCloseCurly();
		if (peak().has_value() && peak().value().type == TokenType::_elif) {
			ifNode.elifs = parseElifs();
		}
		ifNode.elsePart = parseOptionalElse();
	}
	else{
		std::cerr << "invalid expresion encounterd in if!" << std::endl;
		exit(EXIT_FAILURE);
	}
	return ifNode;
}


NodeInput Parser::parseInput() {
	NodeInput nodeInp;
	tryConsume(TokenType::request, "expected request");
	parseOpenParen();
	nodeInp.identifier = tryConsume(TokenType::identifier, "expected an identifier in the request");
	parseCloseParen();
	parseSemi();
	return nodeInp;
}

NodeVarDump Parser::parseVarDump() {
	NodeVarDump nodeVarDump;
	tryConsume(TokenType::var_dump, "expected var_dump");
	parseOpenParen();
	parseOpenQuote();
	nodeVarDump.str_lit = tryConsume(TokenType::string_lit, "expected a string lit");
	parseCloseQuote();
	parseCloseParen();
	parseSemi();
	return nodeVarDump;
}


NodeTppInp Parser::parseTppInp() {
	NodeTppInp nodeTppInp;
	tryConsume(TokenType::tppinp, "expected tppInp");
	parseOpenParen();
	nodeTppInp.number = std::stoi(tryConsume(TokenType::int_lit, "expected a int lit").value.value());
	parseCloseParen();
	return nodeTppInp;
}



std::vector<standAloneNode> Parser::parseProgram(){
	program prog;
	while (peak().has_value()) {
		if (isToken(TokenType::closed_curly)) {
			return prog.codeLines;
		}else if (isToken(TokenType::open_curly)) {
			tryConsume(TokenType::open_curly, "this open curly disapeared");
			prog.codeLines.push_back(NodeScope{ .codeLines = parseProgram() });
			parseCloseCurly();
		}else if (isToken(TokenType::request)) {
			prog.codeLines.push_back(parseInput()); // Add the input node to the program
		}else if (isToken(TokenType::_exit)) {
			prog.codeLines.push_back(parseExit()); // Add the exit node to the program
		}else if (isToken(TokenType::say)) {
			prog.codeLines.push_back(parseSay()); // Add the say node to the program
		}else if (isToken(TokenType::shout)) {
			prog.codeLines.push_back(parseShout()); // Add the shout node to the program
		}else if (isToken(TokenType::_return)) {
			prog.codeLines.push_back(parseReturn()); // Add the return node to the program
		}else if (isToken(TokenType::identifier)) {
			prog.codeLines.push_back(parseIdentifier()); // Add the identifier node to the program
		}else if (isToken(TokenType::_if)) {
			prog.codeLines.push_back(parseIf()); // Add the if node to the program
		}else if (isToken(TokenType::var_dump)) {
			prog.codeLines.push_back(parseVarDump()); // Add the varDump node to the program
		}else if (isToken(TokenType::_while)) {
			prog.codeLines.push_back(parseWhile()); // Add the while node to the program
		}
		else {
			std::cerr << "found a token i dont like here, namely: " << peak().value() << std::endl; //TODO add parsing for extra nodes
			exit(EXIT_FAILURE);
		}
	}
	return prog.codeLines;
}

Token Parser::parseStringLit() {
	Token str;
	parseOpenQuote();
	if (isToken(TokenType::string_lit)) {
		str = tryConsume(TokenType::string_lit, "this string_lit disapeared");
	}
	else {
		std::cerr << "invalid string!" << std::endl;
		exit(EXIT_FAILURE);
	}
	parseCloseQuote();
	return str;
}

std::optional<NodeTest> Parser::parseTest(NodeExpr* exprNodeLeft) {
	NodeTest nodeTest;
	nodeTest.left_expr = exprNodeLeft;
	if (isToken(TokenType::test_equal)) {
		consume();
		nodeTest.test_expr = Token{ .type = TokenType::test_equal };
	}else if (isToken(TokenType::test_not_equal)) {
		consume();
		nodeTest.test_expr = Token{ .type = TokenType::test_not_equal };
	}else if (isToken(TokenType::test_equal_greater)) {
		consume();
		nodeTest.test_expr = Token{ .type = TokenType::test_equal_greater };
	}else if (isToken(TokenType::test_equal_smaller)) {
		consume();
		nodeTest.test_expr = Token{ .type = TokenType::test_equal_smaller };
	}else if (isToken(TokenType::test_greater)) {
		consume();
		nodeTest.test_expr = Token{ .type = TokenType::test_greater };
	}else if (isToken(TokenType::test_smaller)) {
		consume();
		nodeTest.test_expr = Token{ .type = TokenType::test_smaller };
	}else {
		return {};
	}
	if (auto node_expr = parse_expr()) {
		nodeTest.right_expr = node_expr.value();
	}
	else {
		std::cerr << "expresion missing after: " << nodeTest.test_expr << std::endl;
		exit(EXIT_FAILURE);
	}
	return nodeTest;
}

NodeWhile Parser::parseWhile() {
	NodeWhile whileNode;
	tryConsume(TokenType::_while, "expected while");
	parseOpenParen();
	if (auto node_expr = parse_expr()) {
		//whileNode.expr = node_expr.value();
		NodeExpr* exprNodeLeft = node_expr.value();
		if (auto node_test = parseTest(exprNodeLeft)) {
			parseCloseParen();
			parseOpenCurly();
			whileNode = NodeWhile{ .expr = node_test.value(), .scope = parseProgram()};
		}
		else {
			parseCloseParen();
			parseOpenCurly();
			whileNode = NodeWhile{ .expr = exprNodeLeft, .scope = parseProgram() };
		}
	}
	else {
		std::cerr << "found no expresion in while" << std::endl;
		exit(EXIT_FAILURE);
	}
	parseCloseCurly();
	return whileNode;
}

std::optional<program> Parser::parse() {
	program prog;
	while (isToken(TokenType::_import)) {
		tryConsume(TokenType::_import, "this import disapeared"); // consume the import
		prog.imports.push_back(parseStringLit().value.value());
		parseSemi(); //consuming the semicoln
	}
	prog.codeLines = parseProgram();
	return prog;
}
