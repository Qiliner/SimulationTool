#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#if defined(_WIN32)
#   ifdef TASKSERVER_EXPORTS
#       define TASKSERVER_LIB __declspec(dllexport)
#   else
#       define TASKSERVER_LIB __declspec(dllimport)
#   endif
#elif defined(__GNUC__)  && (__GNUC__ >= 4)
#   ifdef TASKSERVER_EXPORTS
#       define TASKSERVER_LIB __attribute__((visibility("default")))
#   else
#       define TASKSERVER_LIB __attribute__((visibility("default")))
#   endif
#else
#   define TASKSERVER_LIB
#endif // _WIN32

#ifndef EXTERN_C
#  ifdef __cplusplus
#    define EXTERN_C    extern "C"
#  else
#    define EXTERN_C    extern
#  endif
#endif

#if !defined(ZR_DECL_EQ_DELETE)
#  if defined(_WIN32)
#    if _MSC_VER >= 1800
#      define ZR_DECL_EQ_DELETE = delete
#    else
#      define ZR_DECL_EQ_DELETE
#    endif
#  else
#    if __cplusplus >= 201103L
#      define ZR_DECL_EQ_DELETE = delete
#    else
#      define ZR_DECL_EQ_DELETE
#    endif
#  endif
#endif

#ifndef ZR_DISABLE_COPY
#define ZR_DISABLE_COPY(Class) \
    Class(const Class &) ZR_DECL_EQ_DELETE;\
    Class &operator=(const Class &) ZR_DECL_EQ_DELETE;
#endif

#if !defined(NOEXCEPT)
#if defined(_MSC_VER)
#  if _MSC_VER >= 1900
#    define NOEXCEPT  noexcept
#  else
#    define NOEXCEPT  throw()
#  endif
#else // !defined(_MSC_VER)
#  if (__cplusplus >= 201103L)
#    define NOEXCEPT  noexcept
#  else
#    define NOEXCEPT  throw()
#  endif
#endif 
#endif // !defined(NOEXCEPT)

#ifndef _WIN32
#  ifndef __int64
#    define __int8  char
#    define __int16 short int
#    define __int32 int
#    define __int64 long long int
#  endif // __int64
#endif // _WIN32

#ifndef _sizetypes_T
#define _sizetypes_T
typedef __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#endif // _sizetypes_T

#ifndef _realtypes_T
#define _realtypes_T
typedef float  real32, real;
typedef double real64;
#endif // _realtypes_T


namespace evo
{

#ifndef UNIQUE_KEY_DEFINED
#define UNIQUE_KEY_DEFINED
/// @brief 唯一key
union UniqueKey
{
    struct
    {
        /** @brief IPv4地址 */
        uint32 address;
        /** @brief 进程号 */
        uint32 processId;
    } u;

    uint64 quad;
};

inline UniqueKey makeUniqueKey(uint32 addr, uint32 pid) NOEXCEPT
{
    UniqueKey k;
    k.u.address = addr;
    k.u.processId = pid;
    return k;
}

inline bool operator<(const UniqueKey& l, const UniqueKey& r) NOEXCEPT
{
    return l.quad < r.quad;
}

inline bool operator==(const UniqueKey& l, const UniqueKey& r) NOEXCEPT
{
    return l.quad == r.quad;
}

inline bool operator!=(const UniqueKey& l, const UniqueKey& r) NOEXCEPT
{
    return l.quad != r.quad;
}
#endif // UNIQUE_KEY_DEFINED

#ifndef TASK_COMMAND_DEFINED
#define TASK_COMMAND_DEFINED
/// @brief 任务命令
enum TaskCommand
{
    TASK_COMMAND_STOP,           // 停止
    TASK_COMMAND_START,          // 启动
    TASK_COMMAND_PAUSE,          // 暂停
    TASK_COMMAND_RESUME,         // 继续
    TASK_COMMAND_STEP,           // 单步
    TASK_COMMAND_RESET,          // 复位
    TASK_COMMAND_INIT = 0x100,   // 初始化，包含互联设计和初始化参数。
};
#endif // TASK_COMMAND_DEFINED


#ifndef TASK_STATE_DEFINED
#define TASK_STATE_DEFINED
/// @brief 任务状态
enum TaskState
{
    TASK_STATE_STOP,             // 停止
    TASK_STATE_INIT_DESIGN,      // 初始化设计
    TASK_STATE_INIT_PARAM,       // 初始化参数
    TASK_STATE_RUNNING,          // 运行中
    TASK_STATE_PAUSE,            // 暂停
    TASK_STATE_RESUME,           // 继续
    TASK_STATE_STEP,             // 单步
    TASK_STATE_RESET,            // 复位
    TASK_STATE_EVENT,            // 事件触发
};
#endif // TASK_STATE_DEFINED

#ifndef EXECUTE_STATE_DEFINED
#define EXECUTE_STATE_DEFINED
/// @brief 桥接器服务执行状态
enum ExceuteState
{
    EXCEUTE_STATE_INVALID,
    EXCEUTE_STATE_ONLINE,
    EXCEUTE_STATE_OFFLINE,
    EXCEUTE_STATE_INIT_DESIGN,
    EXCEUTE_STATE_INIT_PARAM,
    EXCEUTE_STATE_START,
    EXCEUTE_STATE_PAUSE,
    EXCEUTE_STATE_STOP,
    EXCEUTE_STATE_STEP,
    EXCEUTE_STATE_RESET,
    EXCEUTE_STATE_DISPATCH_EVENT,
};
#endif // EXECUTE_STATE_DEFINED

#ifndef EXECUTE_RESULT_DEFINED
#define EXECUTE_RESULT_DEFINED
/// @brief 事件/命令执行结果
struct ExecuteResult
{
    int64           sequenceNo;     // 执行序号
    int             retcode;        // 执行结果
    std::string     errorString;    // 错误信息
};
#endif // EXECUTE_RESULT_DEFINED

#ifndef MODEL_ENTITY_DEFINED
#define MODEL_ENTITY_DEFINED
/// @brief 类型信息定义
struct TASKSERVER_LIB ModelTypeInfo
{
    std::string name;
    std::string uuid;
    std::string note;
    std::string designVersion;
    std::string alias;
    std::string unicode;
};

/// @brief 模型属性定义
struct TASKSERVER_LIB ModelProperty
{
    std::string name;
    std::string typeUuid;
};

/// @brief 模型类型（用于IO）函数定义
struct TASKSERVER_LIB ModelTypeFunction
{
    std::string name;
    std::string typeUuid;
};

/// @brief 模型具体配置定义
struct TASKSERVER_LIB ModelEntity
{
    typedef ModelTypeInfo TypeInfo;
    typedef ModelProperty Property;
    typedef ModelTypeFunction TypeFunction;

    std::string uuid;       // 模型类型的UUID
    std::string name;       // 模型类型的名称
    std::string instanceName; // 模型注册名称
    std::string note;       // 描述信息
    std::string designVersion; // 版本信息
    std::string buildVersion; // 程序版本

    std::string design;     // 模型的设计信息

    std::vector<std::string> events; // 事件列表
    std::vector<TypeInfo> types; // 注册的类型信息
    std::vector<Property> properties; // 属性成员
    std::vector<TypeFunction> inputs; // 输入对象
    std::vector<TypeFunction> outputs;// 输出对象
    std::vector<std::string> timers; // 定时器函数
};
#endif

#ifndef MODEL_INIT_DESIGN_RESULT_DEFINED
#define MODEL_INIT_DESIGN_RESULT_DEFINED
/// @brief 表示输入输出接口的信息结构体。
struct ModelInitDesignIOInfo
{
    std::string interfaceName; // 接口名称
    std::string topicName; // 主题名称
    std::string fullTopicName; // 完整的主题名称
};

/// @brief 模型初始化结果信息
struct  TASKSERVER_LIB ModelInitDesignResult
{
    typedef ModelInitDesignIOInfo IO;

    std::vector<IO> inputs;
    std::vector<IO> outputs;
    std::vector<std::string> threads;
};
#endif // MODEL_INIT_DESIGN_RESULT_DEFINED

#ifndef MODEL_RUN_ROUND_DEFINED
#define MODEL_RUN_ROUND_DEFINED
struct TASKSERVER_LIB RunRound
{
    std::string name;  // 定时器名称
    int64 round;      // 定时器运行次数
};

/// @brief 模型当前已执行周期数
struct TASKSERVER_LIB ModelRunRound
{
    int64  number;           // 上报的次数
    std::vector<RunRound> context; // 所有定时器运行次数
};
#endif // MODEL_RUN_ROUND_DEFINED

#ifndef STATUS_STATISTICS_DEFINED
#define STATUS_STATISTICS_DEFINED
/// @brief IO状态信息
struct TASKSERVER_LIB IOStatus
{
    std::string     interfaceName;  // 模型接口名称
    std::string     topicName;      // 主题名称
    uint64          counter;        // 发送/接收的总包数
    int64           lastTime;       // 最近发送/接收的时间
    int64           sourceTime;     // 最近接收的包的发送时间

    // 以下字段仅供接收端填写使用
    UniqueKey       uk;             // 发送端标识
    uint64          writeSeqNum;    // 发送序号
};

/// @brief  模型对象输入输出状态信息
struct TASKSERVER_LIB ModelIOStatus
{
    std::vector<IOStatus> inputStatus;   // 模型IO状态采集信息
    std::vector<IOStatus> outputStatus;   // 模型IO状态采集信息
};
#endif // STATUS_STATISTICS_DEFINED

#ifndef MODEL_STATE_DEFINED
#define MODEL_STATE_DEFINED
/// @brief 自定义状态
struct TASKSERVER_LIB ModelCustomState
{
    std::string name;  // 自定义状态名称
    std::string value; // 自定义状态值
};
#endif // MODEL_STATE_DEFINED

#ifndef TRANS_TIMEOUT_WARN_DEFINED
#define TRANS_TIMEOUT_WARN_DEFINED
/// @brief 数据推送超时告警
struct TASKSERVER_LIB TransTimeoutWaring
{
    std::string interfaceName;     // 模型对象中的接口名称
    int64 timeoutInterval;         // 相对于上次已超时间隔
};
#endif // TRANS_TIMEOUT_WARN_DEFINED

#ifndef LOG_CONTEXT_DEFINED
#define LOG_CONTEXT_DEFINED
/// @brief 日志等级
enum LogLevel
{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_FATAL = 1, // 致命
    LOG_LEVEL_ERRR = 2, // 错误
    LOG_LEVEL_WARN = 3, // 警告
    LOG_LEVEL_INFO = 4, // 信息
    LOG_LEVEL_DBUG = 5, // 调试
    LOG_LEVEL_VERB = 6  // 冗余
};

/// @brief 日志报文
struct TASKSERVER_LIB LogContext
{
    int64 timestamp;        // 日志输出时间
    int32 level;            // 日志输出等级。参考#LogLevel
    std::string file;       // 产生日志的文件名
    int32 line;             // 产生日志的行号
    std::string function;   // 产生日志的函数名称
    std::string category;   // 分类器
    int64 threadId;         // 输出时线程ID

    std::string text;       // 日志文本
};
#endif // LOG_CONTEXT_DEFINED


/// @brief 任务启动参数
struct StartupParam
{
    uint32 domainId;            // 任务运行时的域ID, -1则服务器自行分配，[1,232]
    std::string typeDesign;     // 数据类型设计，所有用到的类型的定义
    std::string initParam;      // 模型对象的配置参数（包括桥接器模型对象的参数）
    std::string systemDesign;   // 系统互联设计

    inline StartupParam(uint32 domainId = 101)
        : domainId(domainId)
    {
    }

    inline StartupParam(const StartupParam& rhs)
        : domainId(rhs.domainId)
        , typeDesign(rhs.typeDesign)
        , initParam(rhs.initParam)
        , systemDesign(rhs.systemDesign)
    {
    }

    inline StartupParam& operator=(const StartupParam& rhs)
    {
        if (this != &rhs)
        {
            domainId = rhs.domainId;
            typeDesign = rhs.typeDesign;
            initParam = rhs.initParam;
            systemDesign = rhs.systemDesign;
        }
        return *this;
    }

    inline StartupParam(StartupParam&& rhs) NOEXCEPT
        : domainId(rhs.domainId)
        , typeDesign(std::move(rhs.typeDesign))
        , initParam(std::move(rhs.initParam))
        , systemDesign(std::move(rhs.systemDesign))
    {
    }

    inline StartupParam& operator=(StartupParam&& rhs) NOEXCEPT
    {
        if (this != &rhs)
        {
            domainId = rhs.domainId;
            typeDesign = std::move(rhs.typeDesign);
            initParam = std::move(rhs.initParam);
            systemDesign = std::move(rhs.systemDesign);
        }
        return *this;
    }

    inline bool isValid() const
    {
        return (domainId > 0 && domainId <= 232) && !typeDesign.empty() && !initParam.empty() && !systemDesign.empty();
    }
};

#ifndef TIME_SPEC_DEFINED
#define TIME_SPEC_DEFINED
struct TimeSpec
{
    /** @brief 秒 - >= 0 */
    int64 sec;
    /** @brief 纳秒 - [0, 999999999] */
    uint64 nsec;
};
#endif // TIME_SPEC_DEFINED

/// @brief 时钟类型
enum ClockType
{
    CLOCK_NO_TYPE = 0,

    /// @brief 自由同步。各节点自己定时处理
    CLOCK_FREE_TYPE = 1,

    /// @brief 严格同步。控于主节点的时钟，所有节点严格按照主节点的时钟处理。
    CLOCK_FORCE_TYPE = 2,
};

/// @brief 时钟状态
enum ClockState
{
    /// @brief 无服务运行。
    CLOCK_STATE_INVALID = 0,

    /// @brief 时钟已准备好。
    CLOCK_STATE_READY = 1,

    /// @brief 时钟正在运行。
    CLOCK_STATE_RUNNING = 2,

    /// @brief 时钟已暂停。
    CLOCK_STATE_PAUSING = 3
};

/// @brief 时钟运行的状态信息
struct ClockStatus
{
    /// @brief 时钟ID
    uint64 id;

    /// @brief 时钟状态。
    ClockState state;

    /// @brief 推进的逻辑时间。
    TimeSpec galt;

    /// @brief 墙上时钟的时间信息。
    TimeSpec wall;

    /// @brief 时钟的类型。
    ClockType type;

    /// @brief 设置速度。
    double set_speed;

    /// @brief 实际速度。
    double real_speed;
};

/// @brief 定义字节数组
typedef std::vector<char> bytearray;

/// @brief 无效的唯一键
EXTERN_C TASKSERVER_LIB UniqueKey INVALID_UNIQUE_KEY;

/// @brief 任务运行回调对象类
class TASKSERVER_LIB TaskStateCallbacker
{
    ZR_DISABLE_COPY(TaskStateCallbacker)
public:
    TaskStateCallbacker() {}
    virtual ~TaskStateCallbacker() {}

public:
    /// @brief  任务运行系统日志回调接口
    /// @param   severity  日志等级，参考#LogLevel
    /// @param   text     日志内容
    virtual void systemLogCallback(int severity, const std::string& text) = 0;

    /// @brief 模型连接状态回调接口
    /// @param   uk      模型唯一键
    /// @param   data    模型实体数据
    /// @param   online   为true则模型上线，否则下线
    virtual void modelConnectStateChanged(const UniqueKey& uk, const ModelEntity& data, bool online) = 0;

    /// @brief 模型执行状态回调接口
    /// @param uk     模型唯一键
    /// @param state 当前执行状态
    virtual void modelExecuteStateChanged(const UniqueKey& uk, ExceuteState state) = 0;

    /// @brief 模型初始化设计结果返回
    /// @param  data  初始化成功的数据
    virtual void modelInitDesignResult(const UniqueKey& uk, const ModelInitDesignResult& data) = 0;

    /// @brief 模型执行命令的应答
    /// @param   uk     模型唯一键
    /// @param command 执行的命令
    /// @param sequenceNo 执行的命令的序号
    virtual void modelCommandExecuteReply(const UniqueKey& uk, TaskCommand command, int64 sequenceNo) = 0;

    /// @brief 模型执行命令的结果
    /// @param uk     模型唯一键
    /// @param data 执行的结果
    virtual void modelCommandExecuteResult(const UniqueKey& uk, const ExecuteResult& data) = 0;

    /// @brief 模型事件执行结果
    /// @param uk     模型唯一键
    /// @param data 执行结果数据
    virtual void modelEventExecuteResult(const UniqueKey& uk, const ExecuteResult& data) = 0;

    /// @brief 任务运行状态变更
    /// @param current  当前运行状态
    /// @param previous  之前运行状态
    virtual void taskStateChanged(TaskState current, TaskState previous) = 0;

    // 以下为监控状态
public:
    /// @brief 模型各thread运行的周期
    /// @param uk     模型唯一键
    /// @param data   状态数据
    virtual void modelRunRoundCallback(const UniqueKey& uk, const ModelRunRound& data) = 0;

    /// @brief 模型IO状态回调
    /// @param uk     模型唯一键
    /// @param data  状态信息
    virtual void modelIOStatusCallback(const UniqueKey& uk, const ModelIOStatus& data) = 0;

    /// @brief 模型上报的自定义状态
    /// @param uk     模型唯一键
    /// @param data  自定义状态信息
    virtual void modelCustomStateCallback(const UniqueKey& uk, const ModelCustomState&data) = 0;

    /// @brief 模型超时未发送数据告警
    /// @param uk     模型唯一键
    /// @param data  告警数据
    virtual void modelTransTimeoutWarning(const UniqueKey& uk, const TransTimeoutWaring& data) = 0;

    /// @brief 模型或者模型上报的日志
    /// @param  uk     模型唯一键
    /// @param data  日志内容
    virtual void modelCustomLogCallback(const UniqueKey& uk, const LogContext& data) = 0;
};

class TaskObjectPrivate;

/// @brief 任务对象异常
class TASKSERVER_LIB TaskObjectException
{
public:
    TaskObjectException(const char* formatString, ...);
    TaskObjectException(const std::string& errr);

    inline const char* what() const { return m_errMsg.c_str(); }

private:
    std::string m_errMsg;
};

/// @brief 任务运行服务
class TASKSERVER_LIB TaskObject
{
    ZR_DISABLE_COPY(TaskObject)
public:
    TaskObject(const std::string& taskName, const std::string& taskUuid, TaskStateCallbacker *callbacker);
    ~TaskObject();

    /// @brief 初始化任务
    /// @param para     任务参数
    /// @param timeoutMs  超时时间
    /// @return 初始化成功返回true，其他情况返回false
    bool initialize(const StartupParam &para, int timeoutMs = 120000);

    /// @brief 终止任务并释放所有资源
    void finalize();

    /// @brief 设置任务的时钟类型，对仿真时间推进存再影响
    /// @param type 时钟类型
    /// @return 成功返回true，其他情况返回false
    bool setClockType(ClockType type);

    /// @brief 设置任务的时间范围
    /// @note 该函数必须在任务开始之前调用
    /// @param start        起始时间
    /// @param end          结束时间
    /// @return 成功返回true，其他情况返回false
    void setClockRange(const TimeSpec& start, const TimeSpec& end);

    /// @brief 设置任务的时间速率
    /// @param rate         速率。<=0：全速；1：正常速度；大于1：仿真时间变化速度大于计算机时间速度；小于1：仿真时间变化速度小于计算机时间变化速度。
    /// @return 成功返回true，其他情况返回false
    void setClockRate(double rate);

    /// @brief 获取时钟状态
    /// @param status    时钟状态信息
    void getClockStatus(ClockStatus* status);

    /// @brief 控制任务
    /// @param cmd      控制命令
    /// @param data     命令携带数据
    /// @return 处理成功返回命令序号，否则返回0
    int64 control(TaskCommand cmd, const bytearray &data = bytearray());

    /// @brief 派发事件
    /// @param name     事件名称
    /// @param data     事件数据
    /// @return 处理成功返回事件序号，否则返回0
    int64 postEvent(const std::string& name, const bytearray& data);

    /// @brief 在任务运行过程中添加模型
    /// @param uk       模型唯一键
    /// @param designpara 模型设计参数
    /// @param attrpara 模型属性参数
    void runModel(const UniqueKey& uk, const std::string& designPara, const std::string& initPara);

    /// @brief 注入初始化参数
    /// @param initPara  初始化参数内容
    void injectInitPara(const std::string& initPara);

    /// @return 返回当前运行的任务状态
    TaskState state() const;

    /// @return 返回任务执行的下一个状态
    TaskState nextState() const;

    /// @return 返回当前任务是否已停止且被释放
    bool isFinished();

    /// @brief 查询模型的初始化设计
    /// @note  【异步接口】
    /// @param    modelUuid         模型类型UUID
    /// @param    objectName       模型名称
    /// @param  [out] data      模型配置数据
    /// @returns 存在结果返回true，其他情况返回false
    bool queryModelEntity(const std::string &modelUuid, const std::string & modelName, evo::ModelEntity &data);

    /// @brief 设置模型状态上报周期
    /// @param periodMs   上报周期，单位毫秒。<=0则停止上报
    /// @param uk         模型唯一键。如果未指定（uk==INVALID_UNIQUE_KEY），则表示设置所有模型
    void setModelStatusReportPeriod(int periodMs, const UniqueKey& uk = INVALID_UNIQUE_KEY);

    /// @brief 请求一次模型状态上报
    /// @param uk         模型唯一键。如果未指定（uk==INVALID_UNIQUE_KEY），则表示请求所有模型
    void requestReportModelStatusOnce(const UniqueKey& uk);

private:
    TaskObjectPrivate *d;

    friend class TaskManager;
};

} // namespace evo
