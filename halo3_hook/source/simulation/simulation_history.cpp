#include <simulation/simulation_history.h>

/* ---------- code */

c_simulation_history_manager::c_simulation_history_manager() :
    m_flags(),
    m_undo_stack(),
    m_redo_stack()
{
}

void c_simulation_history_manager::clear()
{
    m_flags.clear();
    m_undo_stack.clear();
    m_redo_stack.clear();
}

bool c_simulation_history_manager::is_recording() const
{
    return m_flags.test(_simulation_history_recording_state_bit);
}

void c_simulation_history_manager::start_recording()
{
    m_flags.set(_simulation_history_recording_state_bit, true);
}

void c_simulation_history_manager::stop_recording()
{
    m_flags.set(_simulation_history_recording_state_bit, false);
}

bool c_simulation_history_manager::can_undo() const
{
    return m_undo_stack.count() != 0;
}

bool c_simulation_history_manager::is_performing_undo() const
{
    return m_flags.test(_simulation_history_performing_undo_bit);
}

void c_simulation_history_manager::request_undo()
{
    m_flags.set(_simulation_history_requesting_undo_bit, true);
}

void c_simulation_history_manager::perform_undo()
{
    if (m_undo_stack.empty())
    {
        return;
    }

    m_flags.set(_simulation_history_performing_undo_bit, true);

    s_simulation_history_action action = m_undo_stack.get_top();
    action.apply(action.data);

    m_flags.set(_simulation_history_performing_undo_bit, false);

    m_undo_stack.pop_top();
    m_redo_stack.push_top(action);
}

bool c_simulation_history_manager::can_redo() const
{
    return m_redo_stack.count() != 0;
}

bool c_simulation_history_manager::is_performing_redo() const
{
    return m_flags.test(_simulation_history_performing_redo_bit);
}

void c_simulation_history_manager::request_redo()
{
    m_flags.set(_simulation_history_requesting_redo_bit, true);
}

void c_simulation_history_manager::perform_redo()
{
    if (m_redo_stack.empty())
    {
        return;
    }

    m_flags.set(_simulation_history_performing_redo_bit, true);
    
    s_simulation_history_action action = m_redo_stack.get_top();
    action.apply(action.data);

    m_flags.set(_simulation_history_performing_redo_bit, false);

    m_redo_stack.pop_top();
    m_undo_stack.push_top(action);
}

void c_simulation_history_manager::push(const wchar_t *description, void *address, void(*apply)(void *))
{
    assert(description);
    assert(address);
    assert(apply);

    if (is_recording())
    {
        s_simulation_history_action action = { description, apply, address };

        if (!is_performing_undo() || is_performing_redo())
        {
            if (m_undo_stack.count() == m_undo_stack.maximum_count() - 1)
            {
                m_undo_stack.pop_bottom();
            }

            m_undo_stack.push_top(action);
            m_redo_stack.clear();
        }
        else
        {
            if (m_redo_stack.count() == m_redo_stack.maximum_count() - 1)
            {
                m_redo_stack.pop_bottom();
            }

            m_redo_stack.push_top(action);
        }
    }
}

s_simulation_history_action &c_simulation_history_manager::get_undo_top()
{
    assert(!m_undo_stack.empty());
    return m_undo_stack.get_top();
}

s_simulation_history_action &c_simulation_history_manager::get_redo_top()
{
    assert(!m_redo_stack.empty());
    return m_redo_stack.get_top();
}

void c_simulation_history_manager::update()
{
    if (m_flags.test(_simulation_history_requesting_undo_bit))
    {
        m_flags.set(_simulation_history_requesting_undo_bit, false);
        perform_undo();
    }

    if (m_flags.test(_simulation_history_requesting_redo_bit))
    {
        m_flags.set(_simulation_history_requesting_redo_bit, false);
        perform_redo();
    }
}
