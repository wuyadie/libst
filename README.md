 lib_shark_task(st)
=============
基于C++14语法的任务链


# 主要特性

- 实现一个链式调用的任务链功能，来减少回调地狱的问题；
- 通过std: :future, 获取最后一个调用的结果。如果中途发生了异常，则std: :future: :get()会抛出异常；
- 支持将后续调用绑定到特定的“执行环境”上去调用，从而精确控制执行的时机和线程；
- 支持将一个回调函数包装成一个任务节点。回调参数即任务节点返回值；
- 需求环境:
	- 需要也只需要C++14；
	- Windows下，使用了VS的_Promise实现，故只支持VS2015/VS2017；
	- Android下，由于NDK附带的GCC版本只到4.9，不支持C++14。故需要clang 3.6；

# 接口说明

## make_task
```C++
	template<class _Ftype>
	task<> make_task(_Ftype && fn);
	//构建任务链的首节点。
	//_Ftype是一个函数(对象)

	task<> make_task();
	//构建任务链的首节点，这个首节点什么事情都不做

	template<class _Context, class _Ftype>
	task<> make_task(_Context & ctx, _Ftype && fn)
	//构建一个任务链，首节点什么事情都不做，第二个节点将fn放入ctx里运行。从而看起来将首任务投入到ctx里运行。
```

## marshal_task
```C++
	template<class _Fcb, class... _Args>
	task<> marshal_task(_Fcb&& fn, _Args&&... args)
	//构建任务链的首节点。
	//_Fcb是一个基于回调的函数(对象)。
	//_Fcb的返回值不再关心。并且_Fcb内部要么抛异常，要么就必须调用回调函数，并且不关心回调函数的返回值
```

## task
```C++
	template<class _FirstNode, class _LastNode>
	struct task
	//任务调用链。通过make_task/marshal_task生成，然后不停的then/marshal生成调用链。
```

## then
```C++
	template<class _Ftype>
	task<> task::then(_Ftype && fn)
	//在上一个链节点后继续运行fn
	
	template<class _Context, class _Ftype> 
	task<> task::then(_Context & ctx, _Ftype && fn)
	//在上一个链节点后，投递到指定的ctx里继续运行fn

	//注：
	//_Ftype的入参，是上一个调用链节点的返回值。
	//如果上一个调用链节点的返回值是std::tuple<...>类型，则fn的入参，是tuple解包后的参数。目前要求参数个数/类型精确匹配。
```

## marshal
```C++
	template<class _Fcb, class... _Types>
	task<> task::marshal(_Fcb&& fn, _Types&&... args)
	//在上一个链节点后继续运行fn。
	
	template<class _Context, class _Fcb, class... _Types> 
	task<> task::marshal(_Context & ctx, _Fcb&& fn, _Types&&... args)
	//在上一个链节点后，投递到指定的ctx里继续运行fn
	
	//注：
	//_Fcb是一个基于回调的函数(对象)。其回调函数参数使用内部构造的兼容对象，其他参数是上一个调用链节点的返回值。
	//回调函数的入参，作为本任务节点的返回值
```

## when_all
```C++
	#include "task_when_all.h"
	
	template<class _Task, class... _TaskRest>
	task<> when_all(_Task& tfirst, _TaskRest&... rest)
	//等待多个不同类型的任务完成。
	//多个任务的结果，放在一个拼合的tuple<>里。这个拼合的tuple<>，将作为下一个任务节点的入参，或者最后一个节点future<tuple<>>的结果。
	
	template<class _Iter>
	task<> when_all(_Iter begin, _Iter end)
	//等待一组相同类型的任务完成.
	//多个任务的结果类型肯定是一致的，数量运行时才能确定。故结果放在vector<>里。
	//如果每个任务返回的是单值，则为vector<T>；如果返回的多值，则为vector<tuple<T>>。
```

## when_any
```C++
	#include "task_when_any.h"
	
	template<class _Task, class... _TaskRest>
	task<> when_any(_Task& tfirst, _TaskRest&... rest)
	//等待多个不同类型的任务之一完成。
	
	template<class _Iter>
	task<> when_any(_Iter begin, _Iter end)
	//等待多个不同类型的任务之一完成.
	//多个任务的结果类型肯定是一致的，故返回值的第一个参数指示是哪一个任务完成，后续参数是结果
```

## get_future
```C++
	std::future<> task::get_future() 
	//返回最后一个链节点的返回值
```

## get_executor
```C++
	executor_sptr task::get_executor() 
	//返回task_executor<_FirstNode>，以便于在指定的task_context里运行
	
	template<class... _Args>
	void task::operator()(_Args&&... args) const 
	//执行整个调用链。args...参数必须跟第一个链节点需要的参数匹配
```

## task_node
```C++
	template<class _Rtype, class... _PrevArgs>
	struct task_node
	//包装任务链节点。通过内嵌promise实现，提供get_future()功能
```

## task_context
```C++
	concept task_context
	{
		void add(const executor_sptr & runner) const;
	};
	//代表任务运行环境/线程的虚类，需要外部实现某种运行方案
	
	extern immediate_task_context imm_context;
	//在当前线程，立即运行task_context的实现。注意，需要在某个地方定义imm_context。
	
	extern async_task_context async_context;
	//使用std::thread运行task_context的实现。注意，需要在某个地方定义async_context。
```

## executor
```C++
	template<class _State>
	struct task_executor : public executor
	//将task包装成执行器，以便于放入指定的task_context里运行
```

