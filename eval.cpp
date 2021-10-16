#include <iostream>
#include <vector>
#include <unordered_set>
#include <stack>
#include <assert.h>

enum class Type
{
    BINARY_OPERATOR,
    UNARY_OPERATOR,
    INTEGER,
    DECIMAL,
    L_PAREN,
    R_PAREN,
};

struct Token
{
    union
    {
        char op;
        char paren;
        int integer;
        float decimal;

    } data;

    Type type;
};

void LOG(const Token &token)
{

    switch (token.type)
    {
    case Type::INTEGER:
        std::cout << "Type:[INTEGER] Value[" << token.data.integer << "]\n";
        break;
    case Type::DECIMAL:
        std::cout << "Type:[DECIMAL] Value[" << token.data.decimal << "]\n";
        break;
    case Type::BINARY_OPERATOR:
        std::cout << "Type:[BINARY OPERATOR] Value[" << token.data.op << "]\n";
        break;
    case Type::UNARY_OPERATOR:
        std::cout << "Type[UNARY OPERATOR] Value[" << token.data.op << "]\n";
        break;
    case Type::L_PAREN:
        std::cout << "Type[L_PAREN] Value[" << token.data.paren << "]\n";
        break;
    case Type::R_PAREN:
        std::cout << "Type[R_PAREN] Value[" << token.data.paren << "]\n";
        break;
    default:
        assert(false);
    }
}

class Lexer
{
private:
    static bool isInt(const std::string &s)
    {
        for (const char &x : s)
        {
            if (!(x >= '0' && x <= '9'))
                return false;
        }
        return true;
    }

    static bool isFloat(const std::string &s)
    {
        bool dot = false;
        for (const char &x : s)
        {
            if (x == '.')
            {
                if (dot)
                {
                    return false;
                }
                else
                {
                    dot = true;
                }
            }
            else if (!(x >= '0' && x <= '9'))
            {
                return false;
            }
        }
        return dot;
    }

    static bool isOperator(const char &x)
    {
        static const std::unordered_set<char> op = {'+', '-', '/', '*', '^'};
        return op.count(x);
    }

    static Token GenerateToken(const std::string &lexeme)
    {
        Token token;
        if (isInt(lexeme))
        {
            token.data.integer = std::stoi(lexeme);
            token.type = Type::INTEGER;
        }

        if (isFloat(lexeme))
        {
            token.data.decimal = std::stof(lexeme);
            token.type = Type::DECIMAL;
        }

        return token;
    }

    static Token GenerateToken(const char &lexeme, const std::string &exp, int index)
    {
        Token token;
        if (isOperator(lexeme))
        {
            token.data.op = lexeme;
            if (index == 0 || isOperator(exp[index - 1]) || exp[index - 1] == '(')
            {
                //TODO::lexeme should be + or - else return error
                token.type = Type::UNARY_OPERATOR;
            }
            else
            {
                token.type = Type::BINARY_OPERATOR;
            }
        }
        else
        {
            //TODO::lexeme must be parenthisis else return error
            token.data.paren = lexeme;
            token.type = lexeme == '(' ? Type::L_PAREN : Type::R_PAREN;
        }

        return token;
    }

    static std::string Trim(const std::string &expression)
    {
        std::string trimmedExp;
        for (const char &x : expression)
        {
            trimmedExp.push_back(x);
        }
        return trimmedExp;
    }

public:
    static std::vector<Token> Lex(const std::string &expression)
    {
        const std::string &exp = Trim(expression);
        static const std::unordered_set<int> sep = {'+', '-', '/', '*', '^', '(', ')'};
        std::string lexeme;
        std::vector<Token> tokens;
        for (int i = 0; i < exp.length(); ++i)
        {
            if (sep.count(exp[i]))
            {
                if (!lexeme.empty())
                {
                    tokens.push_back(GenerateToken(lexeme));
                    lexeme.clear();
                }
                tokens.push_back(GenerateToken(exp[i], exp, i));
            }
            else
            {
                lexeme.push_back(exp[i]);
            }
        }
        if (!lexeme.empty())
            tokens.push_back(GenerateToken(lexeme));

        return tokens;
    }
};

class Evaluator
{
    static int PrecedenceOf(const Token &token)
    {
        //TODO::token can be either op or paren(Left)
        const char op = token.data.op;
        if (token.type == Type::UNARY_OPERATOR)
            return 4;
        if (op == '^')
            return 3;
        if (op == '/' || op == '*')
            return 2;
        if (op == '+' || op == '-')
            return 1;
        return -1;
    }

    static std::vector<Token> InfixToPostifx(const std::vector<Token> &tokens)
    {
        std::stack<Token> st;
        std::vector<Token> postfix;

        for (const Token &token : tokens)
        {
            if (token.type == Type::INTEGER || token.type == Type::DECIMAL)
                postfix.push_back(token);
            else if (token.type == Type::L_PAREN)
                st.push(token);
            else if (token.type == Type::R_PAREN)
            {
                while (st.top().type != Type::L_PAREN)
                {
                    postfix.push_back(st.top());
                    st.pop();
                }
                st.pop();
            }
            else
            {
                while (!st.empty() && PrecedenceOf(token) <= PrecedenceOf(st.top()))
                {
                    postfix.push_back(st.top());
                    st.pop();
                }
                st.push(token);
            }
        }

        while (!st.empty())
        {
            postfix.push_back(st.top());
            st.pop();
        }
        return postfix;
    }

    static Token EvaluateUnary(const Token &operand, const Token &operation)
    {
        Token result;
        if (operand.type == Type::INTEGER)
        {
            result.data.integer = operation.data.op == '-' ? -operand.data.integer : operand.data.integer;
            result.type = Type::INTEGER;
        }
        else
        {
            result.data.decimal = operation.data.op == '-' ? -operand.data.decimal : operand.data.decimal;
            result.type = Type::DECIMAL;
        }
        return result;
    }

    static Token Power(Token base, int power)
    {
        Token result;
        result.type = base.type;
        if (result.type == Type::INTEGER)
            result.data.integer = 1;
        else
            result.data.decimal = 1.0f;

        while (power)
        {
            if (power & 1)
            {
                if (result.type == Type::INTEGER)
                    result.data.integer *= base.data.integer;
                else
                    result.data.decimal *= base.data.decimal;
            }

            if (base.type == Type::INTEGER)
                base.data.integer *= base.data.integer;
            else
                base.data.decimal *= base.data.decimal;
            power >>= 1;
        }
        return result;
    }

    static Token Power(const Token &lhs, const Token &rhs)
    {
        //TODO::Throw error on non integer power.
        Token result = Power(lhs, abs(rhs.data.integer));
        if (rhs.data.integer < 0)
        {
            result.data.decimal = 1.0f / (result.type == Type::INTEGER ? result.data.integer : result.data.decimal);
            result.type = Type::DECIMAL;
        }
        return result;
    }

    static Token EvaluateBinary(const Token &lhs, const Token &rhs, const Token &operation)
    {
        Token result;
        if (lhs.type == Type::DECIMAL || rhs.type == Type::DECIMAL)
            result.type = Type::DECIMAL;
        else
            result.type = Type::INTEGER;
        switch (operation.data.op)
        {
        case '+':
            if (result.type == Type::DECIMAL)
                result.data.decimal = (lhs.type == Type::DECIMAL ? lhs.data.decimal : lhs.data.integer) + (rhs.type == Type::DECIMAL ? rhs.data.decimal : rhs.data.integer);
            else
                result.data.integer = lhs.data.integer + rhs.data.integer;
            break;
        case '-':
            if (result.type == Type::DECIMAL)
                result.data.decimal = (lhs.type == Type::DECIMAL ? lhs.data.decimal : lhs.data.integer) - (rhs.type == Type::DECIMAL ? rhs.data.decimal : rhs.data.integer);
            else
                result.data.integer = lhs.data.integer - rhs.data.integer;
            break;
        case '/':
            if (result.type == Type::DECIMAL)
                result.data.decimal = (lhs.type == Type::DECIMAL ? lhs.data.decimal : lhs.data.integer) / (rhs.type == Type::DECIMAL ? rhs.data.decimal : rhs.data.integer);
            else
                result.data.integer = lhs.data.integer / rhs.data.integer;
            break;
        case '*':
            if (result.type == Type::DECIMAL)
                result.data.decimal = (lhs.type == Type::DECIMAL ? lhs.data.decimal : lhs.data.integer) * (rhs.type == Type::DECIMAL ? rhs.data.decimal : rhs.data.integer);
            else
                result.data.integer = lhs.data.integer * rhs.data.integer;
            break;
        case '^':
            return Power(lhs, rhs);
        default:
            assert(false);
            break;
        }
        return result;
    }

    static Token PostfixEvaluator(const std::vector<Token> &postfix)
    {
        std::stack<Token> st;
        for (const Token &token : postfix)
        {
            if (token.type == Type::UNARY_OPERATOR)
            {
                const Token operand = st.top();
                st.pop();
                st.push(EvaluateUnary(operand, token));
            }
            else if (token.type == Type::BINARY_OPERATOR)
            {
                const Token rhs = st.top();
                st.pop();
                const Token lhs = st.top();
                st.pop();
                st.push(EvaluateBinary(lhs, rhs, token));
            }
            else
            {
                st.push(token);
            }
        }
        return st.top();
    }

public:
    static Token Eval(const std::string &expression)
    {
        return PostfixEvaluator(InfixToPostifx(Lexer::Lex(expression)));
    }
};

int main()
{
    std::string expression;
    std::getline(std::cin, expression);
    Token result = Evaluator::Eval(expression);
    LOG(result);
    return 0;
}