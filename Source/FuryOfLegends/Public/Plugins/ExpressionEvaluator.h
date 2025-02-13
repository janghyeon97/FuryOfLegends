// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <stack>

/**
 * 
 */
class FURYOFLEGENDS_API ExpressionEvaluator
{
public:
	ExpressionEvaluator();
	~ExpressionEvaluator();

public:
    // ������ ���ϴ� ���� �Լ�. ����� result�� ����Ǹ�, ���� �� true, ���� �� false ��ȯ
    bool Evaluate(const std::string& expression, const std::unordered_map<std::string, double>& variables, double& result);

private:
    // ������ ���ڿ� ��ū���� ��ȯ
    bool Tokenize(const std::string& expression, std::istringstream& tokens);

    // �������� �켱������ ��ȯ
    int Precedence(char op) const;

    // �����ڸ� �����Ͽ� �ǿ����� ���ÿ��� ���� ���
    bool ApplyOperator(std::stack<double>& values, char op);

    // ������ �� ��ū�� ó���ϴ� �Լ�
    bool ProcessToken(std::istringstream& tokens, std::stack<double>& values, std::stack<char>& operators, const std::unordered_map<std::string, double>& variables);
};
