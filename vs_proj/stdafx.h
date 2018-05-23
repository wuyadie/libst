// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#pragma warning(disable : 4503)			//任务链很容易类型名过长，所以，禁止这个警告

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <string>
#include <stack>
#include <chrono>
#include <tuple>

#include "task.h"
#include "task_context.h"

using namespace std::literals;
