#include "Plugins/ExpressionEvaluator.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <cctype>
#include <cstring>



ExpressionEvaluator::ExpressionEvaluator()
{
}

ExpressionEvaluator::~ExpressionEvaluator()
{
}


/**
 * @brief   �� �Լ��� �ǿ����� ���ÿ��� �� ���� ���� ���� �־��� ������ `op`�� �����Ͽ� ����� �����մϴ�.
 *          ���� ����� �ٽ� �ǿ����� ���ÿ� ����˴ϴ�. ���ϱ�, ����, ���ϱ�, ������, �ŵ����� ������ �����ϸ�,
 *          ������ ���� �� 0���� ������ ��찡 �߻��ϸ� `false`�� ��ȯ�Ͽ� ������ ó���մϴ�.
 *          ���������� ������ �Ϸ�Ǹ� `true`�� ��ȯ�մϴ�.
 *
 * @param   values �ǿ����ڸ� �����ϴ� ����.
 * @param   op ������ ������.
 * @return  ������ �����ϸ� true, ������ �߻��ϸ� false�� ��ȯ�մϴ�.
 */
bool ExpressionEvaluator::ApplyOperator(std::stack<double>& values, char op) {
    if (values.size() < 2) return false;  // �ǿ����ڰ� ������� Ȯ��

    double right = values.top(); values.pop();
    double left = values.top(); values.pop();

    switch (op) {
    case '+': values.push(left + right); break;
    case '-': values.push(left - right); break;
    case '*': values.push(left * right); break;
    case '/':
        if (right == 0) return false;  // 0���� ������ ���� ó��
        values.push(left / right);
        break;
    case '^': values.push(std::pow(left, right)); break;
    default: return false;  // �� �� ���� ������
    }
    return true;
}




/**
 * @brief   �־��� �������� �켱������ ��ȯ�մϴ�.
 *          ���ϱ�� ����� �켱���� 1, ���ϱ�� ������� 2, �ŵ������� 3�� ��ȯ�մϴ�.
 *          �� �� ���� �����ڰ� �ԷµǸ� 0�� ��ȯ�մϴ�.
 *
 * @param   op �켱������ Ȯ���� ������.
 * @return  �ش� �������� �켱������ ��Ÿ���� ���� ��.
 */
int ExpressionEvaluator::Precedence(char op) const {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;  // �� �� ���� �������� ���
}



/**
 * @brief   ���� ���ڿ��� ��ū ��Ʈ������ ��ȯ�մϴ�.
 *          �� ��ū ��Ʈ���� ���� ���� �� �������� ���˴ϴ�.
 *          ��ȿ�� �����̸� `true`, �׷��� ������ `false`�� ��ȯ�մϴ�.
 *
 * @param   expression �Էµ� ���� ���ڿ�.
 * @param   tokens ��ūȭ�� ����� ������ ��Ʈ��.
 * @return  ��ȿ�� �����̸� true, �ƴϸ� false�� ��ȯ�մϴ�.
 */
bool ExpressionEvaluator::Tokenize(const std::string& expression, std::istringstream& tokens) {
    tokens.str(expression);
    return !expression.empty();
}




/**
 * @brief   ������ �����ϴ� �� ��ū(����, ����, ������, ��ȣ ��)�� ó���մϴ�.
 *          ���ڴ� �ǿ����� ���ÿ�, �����ڴ� ������ ���ÿ� ����Ǹ�, ��ȣ�� �켱������ ���� ������ ó���մϴ�.
 *          ������ �־��� ���� �ʿ��� ���� �����Ͽ� ó���մϴ�.
 *          ���������� ó���Ǹ� `true`, ������ �߻��ϸ� `false`�� ��ȯ�մϴ�.
 *
 * @param   tokens ��ūȭ�� ���� ��Ʈ��.
 * @param   values �ǿ����ڸ� �����ϴ� ����.
 * @param   operators �����ڸ� �����ϴ� ����.
 * @param   variables ���� �̸��� ���� ������ ��.
 * @return  ó���� �����ϸ� true, ���� �߻� �� false�� ��ȯ�մϴ�.
 */
bool ExpressionEvaluator::ProcessToken(std::istringstream& tokens, std::stack<double>& values, std::stack<char>& operators, const std::unordered_map<std::string, double>& variables)
{
    char token;

    // ��ū�� �дµ� ������ ��� false�� ��ȯ�Ͽ� ������ ����
    if (!(tokens >> token))
    {
        return false;
    }

    // ���� ó��: ��ū�� ���ڷ� ��ȯ�Ͽ� ���ÿ� ����
    if (std::isdigit(token) || token == '.')
    {
        tokens.putback(token);
        double value;
        tokens >> value;
        values.push(value);
    }

    // ���� ó��: �������� �а� ���� �ʿ��� ���� ã�� ���ÿ� ����
    else if (std::isalpha(token))
    {
        tokens.putback(token);
        std::string var;
        tokens >> var;
        auto it = variables.find(var);
        if (it == variables.end()) return false;  // �� �� ���� ������ ��� ���� ó��
        values.push(it->second);
    }

    // ���� ��ȣ�� ���ÿ� ����
    else if (token == '(')
    {
        operators.push(token);
    }

    // �ݴ� ��ȣ�� ������ ���� ��ȣ�� ���� ������ ������ ����
    else if (token == ')')
    {
        while (!operators.empty() && operators.top() != '(')
        {
            if (!ApplyOperator(values, operators.top())) return false;
            operators.pop();
        }

        if (operators.empty()) return false;  // ��ȣ ����ġ ����
        operators.pop();  // ���� ��ȣ ����
    }

    // ������ ó��: �켱������ ���� ������ ����
    else if (std::strchr("+-*/^", token))
    {
        while (!operators.empty() && Precedence(operators.top()) >= Precedence(token))
        {
            if (!ApplyOperator(values, operators.top())) return false;
            operators.pop();
        }
        operators.push(token);
    }
    else
    {
        return false; // ��ȿ���� ���� ��ū�̸� false ��ȯ
    }

    return true;
}




/**
 * @brief   ������ ���ϴ� ���� �Լ��Դϴ�.
 *          ���� ������ ��ū���� ��ȯ�� ��, �� ��ū�� ó���Ͽ� �ǿ����ڿ� �����ڸ� ���ÿ� �����մϴ�.
 *          �����ڰ� ��� ó���Ǹ� ���� ����� �ǿ����� ���ÿ� ������, �� ���� result ������ �����մϴ�.
 *          ���� �������� ������ �߻��ϸ� `false`, ���� �� `true`�� ��ȯ�մϴ�.
 *
 * @param   expression ���� ����.
 * @param   variables ���� �̸��� �� ���� ������ ��.
 * @param   result ���� ����� ������ ���� ����.
 * @return  ��꿡 �����ϸ� true, �����ϸ� false�� ��ȯ�մϴ�.
 */
bool ExpressionEvaluator::Evaluate(const std::string& expression, const std::unordered_map<std::string, double>& variables, double& result) {
    std::istringstream tokens;
    if (!Tokenize(expression, tokens)) return false;

    std::stack<double> values;
    std::stack<char> operators;

    // ������ ��ū ������ ó��
    while (ProcessToken(tokens, values, operators, variables));

    // ���� ������ ó��
    while (!operators.empty()) 
    {
        if (!ApplyOperator(values, operators.top())) return false;
        operators.pop();
    }

    if (values.size() != 1) return false;  // ����� ���� ���� �ƴϸ� ����
    result = values.top();

    return true;
}
