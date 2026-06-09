#pragma once
#include <string>

struct Quadruple {
	std::string op;      // 操作符：j> / j< / j= / j / =
	std::string arg1;    // 第一操作数
	std::string arg2;    // 第二操作数
	std::string result;  // 结果或跳转标签
};

/*
* 四元式语义约定：
*场景	        产生式位置	            N 中 = 的语义	                生成的四元式示例
*条件判断	E → id1 N id2	                判等（==）	           (j=, x, y, L1) — 若 x==y 跳转至 L1
*赋值语句	P → id N NUM（N 为 =）	        赋值（=）	         (=, 5, _, z) — 将 5 赋给 z
*条件跳转	P → id N NUM（N 为 > 或 <）	比较跳转	         (j>, a, 3, L2) — 若 a>3 跳转至 L2
*无条件跳转	n/a	                            n/a	                 (j, _, _, L_else) —  无条件跳转至 L_else
*/
