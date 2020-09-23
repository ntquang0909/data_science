#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <sstream>
#include <stack>

using namespace std;

namespace calculator
{

  /// calculator::eval() throws a calculator::error if it fails
  /// to evaluate the expression string.
  ///
  class error : public std::runtime_error
  {
  public:
    error(const std::string &expr, const std::string &message)
        : std::runtime_error(message),
          expr_(expr)
    {
    }
#if __cplusplus < 201103L
    ~error() throw()
    {
    }
#endif
    std::string expression() const
    {
      return expr_;
    }

  private:
    std::string expr_;
  };

  template <typename T>
  class ExpressionParser
  {
  public:
    /// Evaluate an integer arithmetic expression and return its result.
    /// @throw error if parsing fails.
    ///
    T eval(const std::string &expr)
    {
      T result = 0;
      index_ = 0;
      expr_ = expr;
      try
      {
        result = parseExpr();
        if (!isEnd())
          unexpected();
      }
      catch (const calculator::error &)
      {
        while (!stack_.empty())
          stack_.pop();
        throw;
      }
      return result;
    }

  private:
    enum
    {
      OPERATOR_NULL,
      OPERATOR_ADDITION,       /// +
      OPERATOR_SUBTRACTION,    /// -
      OPERATOR_MULTIPLICATION, /// *
      OPERATOR_DIVISION,       /// /
    };

    struct Operator
    {
      /// Operator, one of the OPERATOR_* enum definitions
      int op;
      int precedence;
      /// 'L' = left or 'R' = right
      int associativity;
      Operator(int opr, int prec, int assoc) : op(opr),
                                               precedence(prec),
                                               associativity(assoc)
      {
      }
    };

    struct OperatorValue
    {
      Operator op;
      T value;
      OperatorValue(const Operator &opr, T val) : op(opr),
                                                  value(val)
      {
      }
      int getPrecedence() const
      {
        return op.precedence;
      }
      bool isNull() const
      {
        return op.op == OPERATOR_NULL;
      }
    };

    /// Expression string
    std::string expr_;
    /// Current expression index, incremented whilst parsing
    std::size_t index_;
    /// The current operator and its left value
    /// are pushed onto the stack if the operator on
    /// top of the stack has lower precedence.
    std::stack<OperatorValue> stack_;

    T checkZero(T value) const
    {
      if (value == 0)
      {
        std::string divOperators("/%");
        std::size_t division = expr_.find_last_of(divOperators, index_ - 2);
        std::ostringstream msg;
        msg << "Parser error: division by 0";
        if (division != std::string::npos)
          msg << " (error token is \""
              << expr_.substr(division, expr_.size() - division)
              << "\")";
        throw calculator::error(expr_, msg.str());
      }
      return value;
    }

    T calculate(T v1, T v2, const Operator &op) const
    {
      switch (op.op)
      {
      case OPERATOR_ADDITION:
        return v1 + v2;
      case OPERATOR_SUBTRACTION:
        return v1 - v2;
      case OPERATOR_MULTIPLICATION:
        return v1 * v2;
      case OPERATOR_DIVISION:
        return v1 / checkZero(v2);
      default:
        return 0;
      }
    }

    bool isEnd() const
    {
      return index_ >= expr_.size();
    }

    /// Returns the character at the current expression index or
    /// 0 if the end of the expression is reached.
    ///
    char getCharacter() const
    {
      if (!isEnd())
        return expr_[index_];
      return 0;
    }

    /// Parse str at the current expression index.
    /// @throw error if parsing fails.
    ///
    void unexpected() const
    {
      std::ostringstream msg;
      msg << "Syntax error: unexpected token \""
          << expr_.substr(index_, expr_.size() - index_)
          << "\" at index "
          << index_;
      throw calculator::error(expr_, msg.str());
    }

    /// Eat all white space characters at the
    /// current expression index.
    ///
    void eatSpaces()
    {
      while (std::isspace(getCharacter()) != 0)
        index_++;
    }

    /// Parse a binary operator at the current expression index.
    /// @return Operator with precedence and associativity.
    ///
    Operator parseOp()
    {
      eatSpaces();
      switch (getCharacter())
      {
      case '+':
        index_++;
        return Operator(OPERATOR_ADDITION, 10, 'L');
      case '-':
        index_++;
        return Operator(OPERATOR_SUBTRACTION, 10, 'L');
      case '/':
        index_++;
        return Operator(OPERATOR_DIVISION, 20, 'L');
      case '*':
        index_++;
        if (getCharacter() != '*')
          return Operator(OPERATOR_MULTIPLICATION, 20, 'L');
      default:
        return Operator(OPERATOR_NULL, 0, 'L');
      }
    }

    static T toInteger(char c)
    {
      if (c >= '0' && c <= '9')
        return c - '0';
      if (c >= 'a' && c <= 'f')
        return c - 'a' + 0xa;
      if (c >= 'A' && c <= 'F')
        return c - 'A' + 0xa;
      T noDigit = 0xf + 1;
      return noDigit;
    }

    T getInteger() const
    {
      return toInteger(getCharacter());
    }

    T parseDecimal()
    {
      T value = 0;
      for (T d; (d = getInteger()) <= 9; index_++)
        value = value * 10 + d;
      
      try
      {
        stoll(to_string(value)); // from -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
      }
      catch(const std::out_of_range& )
      {
        throw calculator::error(expr_, "an element in the expression is out of range");
      }
      
      return value;
    }

    T parseHex()
    {
      index_ = index_ + 2;
      T value = 0;
      for (T h; (h = getInteger()) <= 0xf; index_++)
        value = value * 0x10 + h;
      return value;
    }

    bool isHex() const
    {
      if (index_ + 2 < expr_.size())
      {
        char x = expr_[index_ + 1];
        char h = expr_[index_ + 2];
        return (std::tolower(x) == 'x' && toInteger(h) <= 0xf);
      }
      return false;
    }

    /// Parse an integer value at the current expression index.
    /// The unary `+', `-' operators and opening
    /// parentheses `(' cause recursion.
    ///
    T parseValue()
    {
      T val = 0;
      eatSpaces();
      switch (getCharacter())
      {
      case '0':
        if (isHex())
          val = parseHex();
        else
          val = parseDecimal();
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        val = parseDecimal();
        break;
      case '(':
        index_++;
        val = parseExpr();
        eatSpaces();
        if (getCharacter() != ')')
        {
          if (!isEnd())
            unexpected();
          throw calculator::error(expr_, "Syntax error: `)' expected at end of expression");
        }
        index_++;
        break;
      case '+':
        index_++;
        val = parseValue();
        break;
      case '-':
        index_++;
        val = parseValue() * static_cast<T>(-1);
        break;
      default:
        if (!isEnd())
          unexpected();
        throw calculator::error(expr_, "Syntax error: value expected at end of expression");
      }
      return val;
    }

    /// Parse all operations of the current parenthesis
    /// level and the levels above, when done
    /// return the result (value).
    ///
    T parseExpr()
    {
      stack_.push(OperatorValue(Operator(OPERATOR_NULL, 0, 'L'), 0));
      // first parse value on the left
      T value = parseValue();

      while (!stack_.empty())
      {
        // parse an operator (+, -, *, ...)
        Operator op(parseOp());
        while (op.precedence < stack_.top().getPrecedence() || (op.precedence == stack_.top().getPrecedence() &&
                                                                op.associativity == 'L'))
        {
          // end reached
          if (stack_.top().isNull())
          {
            stack_.pop();
            return value;
          }
          // do the calculation ("reduce"), producing a new value
          value = calculate(stack_.top().value, value, stack_.top().op);
          stack_.pop();
        }

        // store on stack_ and continue parsing ("shift")
        stack_.push(OperatorValue(op, value));
        // parse value on the right
        value = parseValue();
      }
      return 0;
    }
  };

  template <typename T>
  inline T eval(const std::string &expression)
  {
    ExpressionParser<T> parser;
    return parser.eval(expression);
  }
} // namespace calculator

#endif
