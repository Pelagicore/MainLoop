#pragma once

#include "ivi-main-loop-common.h"
#include "glib.h"

namespace ivi {

static constexpr int UNREGISTERED_SOURCE = -1;

class GLibEventDispatcher;

class GLibIdleEventSource :
    public IdleEventSource
{

public:
    GLibIdleEventSource(GLibEventDispatcher &mainLoop, CallBackFunction callBackFunction) :
        IdleEventSource(callBackFunction),
        m_mainLoop(mainLoop)
    {
        m_func = callBackFunction;
    }

    ~GLibIdleEventSource();

    void enable() override;

    void disable() override;

private:
    static gboolean onGLibCallback(gpointer data);

    GSource *m_source = nullptr;
    GLibEventDispatcher &m_mainLoop;

};


class GLibTimeOutEventSource :
    public TimeOutEventSource
{
public:
    GLibTimeOutEventSource(GLibEventDispatcher &mainLoop, CallBackFunction callBackFunction, DurationInMilliseconds duration) :
        TimeOutEventSource(duration, callBackFunction), m_mainLoop(mainLoop)
    {
    }

public:
    ~GLibTimeOutEventSource();

    void setDuration(DurationInMilliseconds duration) override;

    void enable() override;

    void disable() override;

private:
    static gboolean onTimerCallback(gpointer data);

    GLibEventDispatcher &m_mainLoop;
    GSource *m_source = nullptr;
};

class GLibChannelWatchEventSource :
    public ChannelWatchEventSource
{
public:
    GLibChannelWatchEventSource(GLibEventDispatcher &mainLoop, CallBackFunction callBackFunction, FileDescriptor fileDescriptor,
                Event events);

    ~GLibChannelWatchEventSource();

    void disable() override;

    void enable() override;

private:
    static gboolean onSocketDataAvailableGLibCallback(GIOChannel *gio, GIOCondition condition, gpointer data);

    static Event toEventSource(const GIOCondition condition);

    static GIOCondition toGIOCondition(const Event event);

    bool m_isEnabled = false;
    gint inputSourceID = UNREGISTERED_SOURCE;

    GIOChannel *m_channel = nullptr;
    GLibEventDispatcher &m_mainLoop;
    Event m_events;

};


/**
 * That class implements the EventDispatcher interface using glib's main loop functions
 */
class GLibEventDispatcher :
    public EventDispatcher
{
public:
    typedef GLibIdleEventSource IdleEventSourceType;
    typedef GLibTimeOutEventSource TimoutEventSourceType;
    typedef GLibChannelWatchEventSource FileDescriptorWatchEventSourceType;

    /**
     * Construct an instance using GLib's default main context if we do not have any instance of GLibEventDispatcher using that
     * context yet, otherwise we create a dedicated context
     */
    GLibEventDispatcher();

    IdleEventSource *newIdleEventSource(const IdleEventSource::CallBackFunction &callBackFunction) final override;

    TimeOutEventSource *newTimeOutEventSource(const TimeOutEventSource::CallBackFunction &callBackFunction,
                DurationInMilliseconds duration) final override;

    ChannelWatchEventSource *newChannelWatchEventSource(const ChannelWatchEventSource::CallBackFunction &callBackFunction,
                FileDescriptor filedescriptor,
                ChannelWatchEventSource::Event events) final override;

    void run() final override;

    void quit() final override;

    /**
     * Return the glib main context reference
     */
    GMainContext *getGMainContext()
    {
        return m_context;
    }

private:
    GMainContext *m_context = nullptr;
    GMainLoop *m_mainLoop = nullptr;

    static bool s_bDefaultContextAlreadyUsed;

};

}
