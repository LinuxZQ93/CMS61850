// main模板
#include <thread>
#include <map>
#include <string>
#include <cassert>

#include <signal.h>
#include "Print/Print.h"

#include "Runtime/Runtime.h"
#include "Log/Log.h"

using namespace std;

static bool s_bRunning = true;

void start()
{
    /// 注册所有组件，必须调用
    base::CRuntime::instance()->registerAllComponent();

    /// 组件初始化
    base::CRuntime::instance()->init();
    /// 组件启动
    base::CRuntime::instance()->start();
}

int main()
{
    setPrintLevel("all", INFO);
    /// 初始化日志模块，若不需要日志，可以删除
    if (!base::CLog::instance()->init())
    {
        errorf("init log failed\n");
        return -1;
    }
    infof("PncStart begin.............................\n");
    infof("Start local log success\n");
    start();
    infof("PncStart all ok.............................\n");
    while(s_bRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        debugf("running\n");
    }
    infof("PncStart end success\n");
    return 0;
}
