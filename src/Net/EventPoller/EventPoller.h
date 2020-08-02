#ifndef CPPTOOLKITS_EVENTPOLL_H
#define CPPTOOLKITS_EVENTPOLL_H


class EventPoller
{
    /**
     * 获取EventPollerPool单例中的第一个EventPoller实例，
     * 保留该接口是为了兼容老代码
     * @return 单例
     */
    static EventPoller &Instance();
};


#endif //CPPTOOLKITS_EVENTPOLL_H
