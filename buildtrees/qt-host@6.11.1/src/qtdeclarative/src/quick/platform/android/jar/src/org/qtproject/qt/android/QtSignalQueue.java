// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android;

import java.util.Queue;
import java.util.LinkedList;

class QtSignalQueue
{
    class SignalListenerInfo
    {
        String m_signalName;
        Class<?>[] m_argTypes;
        Object m_listener;
        int m_id;

        public SignalListenerInfo(String signalName, Class<?>[] argTypes, Object listener, int id)
        {
            m_signalName = signalName;
            m_argTypes = argTypes;
            m_listener = listener;
            m_id = id;
        }

        public String signalName() { return m_signalName; }

        public Class<?>[] argTypes() { return m_argTypes; }

        public Object listener() { return m_listener; }

        public int id() { return m_id; }
    }

    private Queue<SignalListenerInfo> m_queuedSignalListeners = new LinkedList<>();

    void connectQueuedSignalListeners(QtQuickView view)
    {
        if (m_queuedSignalListeners.isEmpty())
            return;

        for (SignalListenerInfo info : m_queuedSignalListeners) {
            view.connectSignalListener(info.signalName(), info.argTypes(), info.listener(),
                                       info.id());
        }

        m_queuedSignalListeners.clear();
    }

    void add(SignalListenerInfo info) { m_queuedSignalListeners.add(info); }

    void add(String signalName, Class<?>[] argTypes, Object listener, int id)
    {
        add(new SignalListenerInfo(signalName, argTypes, listener, id));
    }

    boolean remove(int id) { return m_queuedSignalListeners.removeIf(info -> info.id() == id); }
}
