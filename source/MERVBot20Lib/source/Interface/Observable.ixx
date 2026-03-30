export module Observable;

export import BotEvent;

import <format>;
import <functional>;
import <unordered_map>;


/// <summary>
/// Observable class. Provide base functionality for event handling between a host and a plugin spawn.
/// </summary>
export class Observable
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="name">Name of the observable.</param>
    Observable(std::string_view name) : m_name(name) {}

    /// <summary>
    /// Add a bot event with a signature specified by the given variable types. For string parameters only
    /// string_view is allowed.
    /// </summary>
    /// <typeparam name="...Args">Variable event argument types.</typeparam>
    /// <param name="type">Bot event type.</param>
    template<typename... Args>
    void addEvent(BotEvent type)
    {
        m_events[type] = std::make_unique<TBotEvent<Args...>>();
    }

    /// <summary>
    /// Notify all registered observers by calling the related event handlers with the given arguments.
    /// </summary>
    /// <typeparam name="...Args">Variable event argument types.</typeparam>
    /// <param name="type">Bot event type.</param>
    /// <param name="...args">Event handler arguments.</param>
    template<typename... Args>
    void raiseEvent(BotEvent type, Args&&... args) const
    {
        getEvent<TryConvertToStringView<Args>...>(type).raise(std::forward<Args>(args)...);
    }
    
    /// <summary>
    /// Register an event handler.
    /// </summary>
    /// <param name="type">Bot event type.</param>
    /// <param name="handler">Event handler function.</param>
    /// <param name="ob">Observer to be provided for the event handler.</param>
    /// <returns>Handle to unregister the event handler.</returns>
    template<typename ObserverT, typename... Args>
    EventHandle registerEventHandler(BotEvent type, void(ObserverT::*handler)(Args...), ObserverT* ob)
    {
        return getEvent<Args...>(type) += [handler, ob](Args&&... args) {
            (ob->*handler)(std::forward<Args>(args)...); };
    }

    /// <summary>
    /// Unregister an event handler.
    /// </summary>
    /// <param name="type">Bot event type.</param>
    /// <param name="handle">Event handle.</param>
    template<typename... Args>
    void unregisterEventHandler(BotEvent type, EventHandle handle)
    {
        getEvent<Args...>(type) -= handle;
    }

private:
    /// <summary>
    /// If applicable, convert char array reference types and string types to string_view.
    /// </summary>
    /// <typeparam name="T">Argument type.</typeparam>
    template<typename T>
    using TryConvertToStringView = std::conditional_t<std::is_bounded_array_v<std::remove_reference_t<T>>
        || std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::string>
        || std::is_same_v<std::remove_reference_t<T>, const char*>
        || std::is_same_v<std::remove_reference_t<T>, char*>, std::string_view, T>;

    /// <summary>
    /// Get a bot event by type.
    /// </summary>
    /// <param name="type">Bot event type.</param>
    /// <returns>Bot event.</returns>
    template<typename... Args>
    TBotEvent<Args...>& getEvent(BotEvent type) const
    {
        if (!m_events.contains(type)) {
            std::string errMsg{ std::format("Event '{}' is not available for observable '{}'!",
                (uint32_t)type, m_name) };

            if (m_events.contains(BotEvent::Exit)) {
                raiseEvent(BotEvent::Exit, errMsg);
            }
            throw std::invalid_argument(errMsg);
        }
        return *static_cast<TBotEvent<Args...>*>(m_events.at(type).get());
    }
    
private:
    std::unordered_map<BotEvent, std::unique_ptr<BotEventBase>> m_events;
    std::string m_name;
};
